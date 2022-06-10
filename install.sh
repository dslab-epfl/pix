#!/bin/bash
# See usage().
set -eux

# Note that opam needs bash, not just sh
# Also it uses undefined variables so let's set them now otherwise it'll fail due to strict mode
if [ -z ${PERL5LIB+x} ]; then
  export PERL5LIB=''
fi
if [ -z ${MANPATH+x} ]; then
  export MANPATH=''
fi
if [ -z ${PROMPT_COMMAND+x} ]; then
  export PROMPT_COMMAND=''
fi

DPDK_RELEASE='18.11'
KLEE_RELEASE='master'
KLEE_UCLIBC_RELEASE='klee_uclibc_v1.2'
LLVM_RELEASE=8.0.0
PIN_RELEASE='3.16-98275-ge0db48c31'
Z3_RELEASE='z3-4.5.0'
OCAML_RELEASE='4.06.0'

## Utility functions

# Stop script if we do not have root access
check_sudo()
{
	echo 'Checking for sudo rights:'
	if ! sudo -v;
	then
		echo 'sudo rights not obtained, or sudo not installed.' >&2;
		exit 1;
	fi
}

# Detect the running operating system
# stdout: 'windows', 'docker' or 'linux'
detect_os()
{
	# Detect WSL
	case $(uname -r) in
		*Microsoft*)
			echo 'windows'
			return 0
			;;
	esac

	# Or docker ?
	if grep docker /proc/1/cgroup -qa;
	then
		echo 'docker'
		return 0
	fi

	# Use generic "linux" tag for the rest.
	# XXX: Support some more distributions ?
	echo 'linux'
	return 0
}


# Checks if a variable is set in a file. If it is not in the file, add it with
# given value, otherwise change the value to match the current one.
# $1 : the name of the file
# $2 : the name of the variable
# $3 : the value to set
line()
{
	if grep "^export $2" "$1" >/dev/null;
	then
		# Using sed directly to change the value would be dangerous as
		# we would need to correctly escape the value, which is hard.
		sed -i "/^export $2/d" "$1"
	fi
	echo "export ${2}=${3}" >> "$1"
}

# Same as line, but without the unicity checks.
# $1 : the name of the file
# $2 : the name of the variable
# $3 : the value to set
line_multi()
{
	if ! grep "^export ${2}=${3}" "$1" >/dev/null;
	then
		echo "export ${2}=${3}" >> "$1"
	fi
}

# Install arguments using system's package manager.
# XXX: Make the package manager depend on "$OS".
# shellcheck disable=SC2086
package_install()
{
	# Concatenate arguments into a list
	old_ifs="$IFS"
	IFS=' '
	packages="$*"
	IFS="$old_ifs"

	sudo apt-get install -yqq $packages
}

# Update list of available packages.
# XXX: Make the package manager depend on "$OS".
package_sync()
{
	sudo apt-get update -qq
}

# Print script usage.
usage()
{
	cat <<- EOF
	Usage: $0 [-p <dir>] [-i <component>] [-c <component>]
	       $0 -h
	
	       When invoked with no options, $0 downloads and configures all
	       the required dependencies; else using -i and -c, one can select
	       the components to be installed or cleaned.

	       The -p option sets the directory to use as a root for the project: by default,
	       the parent of the bolt directory is used.

	       This script generates a paths.sh file that must be sourced to
	       obtain the correct working environment.

	       Components: dpdk, pin, z3, klee-uclibc, klee
	EOF
}

## Constants
VNDSDIR="$(dirname "$(realpath "$0")")"
BUILDDIR="$(realpath -e "$VNDSDIR"/..)"
PATHSFILE="$BUILDDIR/paths.sh"
KERNEL_VER=$(uname -r | sed 's/-Microsoft//')
OS="$(detect_os)"

INSTALL_ALL=true

CLEAN_DPDK=
INSTALL_DPDK=

CLEAN_PIN=
INSTALL_PIN=

CLEAN_Z3=
INSTALL_Z3=

CLEAN_LLVM=
INSTALL_LLVM=

CLEAN_KLEE_UCLIBC=
INSTALL_KLEE_UCLIBC=

CLEAN_KLEE=
INSTALL_KLEE=

CLEAN_OCAML=
INSTALL_OCAML=

## Clean and installation routines

source_install_dpdk()
{
	cd "$BUILDDIR"

	# Install kernel headers
	case "$OS" in
		'microsoft')
			package_install "linux-headers-$KERNEL_VER-generic"

			# Fix the kernel dir, since the WSL doesn't have an actual Linux kernel.
			export RTE_KERNELDIR="/usr/src/linux-headers-$KERNEL_VER-generic/"
			;;
		'linux')
			package_install linux-headers-generic
			;;
	esac

	# Install other dependencies
	package_install \
		gperf \
		libgoogle-perftools-dev \
		libpcap-dev

	# Ensure environment is correct.
	line "$PATHSFILE" 'RTE_TARGET' 'x86_64-native-linuxapp-gcc'
	line "$PATHSFILE" 'RTE_SDK' "$BUILDDIR/dpdk"

	# shellcheck source=../paths.sh
	. "$PATHSFILE"

	# Get, Patch and Compile
	if [ ! -f dpdk/.version ] || [ "$(cat dpdk/.version)" != "$DPDK_RELEASE" ]
	then
		# get sources
		rm -rf dpdk
		curl -s "https://fast.dpdk.org/rel/dpdk-$DPDK_RELEASE.tar.xz" | tar xJf -
		mv "dpdk-$DPDK_RELEASE" dpdk


		# patch
		cd dpdk
		for p in "$VNDSDIR"/install/dpdk.*.patch;
		do
			patch -p 1 < "$p"
		done
		# Must be applied last.
		patch -p1 < "$VNDSDIR/install/replay.dpdk.patch"

		# Compile
		make config T=x86_64-native-linuxapp-gcc
		make install -j T=x86_64-native-linuxapp-gcc DESTDIR=.

		#Small hack for compilation of parse_fns required for NF only verif. 
		cp x86_64-native-linuxapp-gcc/include/rte_string_fns.h lib/librte_cmdline/

		echo "$DPDK_RELEASE" > .version
	fi
}

clean_dpdk()
{
	cd "$BUILDDIR"
	rm -rf dpdk
}

source_install_pin()
{
	line "$PATHSFILE" 'PINDIR' "$BUILDDIR/pin"
	line_multi "$PATHSFILE" 'PATH' "$BUILDDIR/pin:\$PATH"

	# shellcheck source=../paths.sh
	. "$PATHSFILE"

	# Line width for readability.
	url="https://software.intel.com/sites/landingpage/pintool/downloads/"
	file="pin-$PIN_RELEASE-gcc-linux.tar.gz"

	cd "$BUILDDIR"
	if [ ! -f pin/.version ] || [ "$(cat pin/.version)" != "$PIN_RELEASE" ]
	then
		rm -rf pin
		curl -s "${url}${file}" | tar xzf -
		mv "pin-$PIN_RELEASE-gcc-linux" pin
		echo "$PIN_RELEASE" > pin/.version
	fi
}

clean_pin()
{
	cd "$BUILDDIR"
	rm -rf pin
}

source_install_z3()
{
	cd "$BUILDDIR"
	if [ -d 'z3/.git' ];
	then
		cd z3;
		git fetch && git checkout "$Z3_RELEASE"
	else
		git clone --depth 1 --branch "$Z3_RELEASE" https://github.com/Z3Prover/z3 "$BUILDDIR/z3"
		cd z3;
	fi

	if  [ ! -f "build/z3" ] || [ ! "z3-$(build/z3 --version | cut -f3 -d' ')" = "$Z3_RELEASE" ];	then
		python scripts/mk_make.py -p "$BUILDDIR/z3/build"
		cd build
		make -kj || make
		make install
	fi
}

clean_z3()
{
	cd "$BUILDDIR"
	rm -rf z3
}

source_install_llvm()
{
	package_install bison flex zlib1g-dev libncurses5-dev libpcap-dev
	# Python2 needs to be available as python for some configure scripts, which is not the case in Ubuntu 20.04
	if [ ! -e /usr/bin/python ] ; then
  		sudo ln -s /usr/bin/python2.7 /usr/bin/python
	fi

	line_multi "$PATHSFILE" 'PATH' "$BUILDDIR/llvm/build/bin:\$PATH"
	# shellcheck source=../paths.sh
	. "$PATHSFILE"

	cd "$BUILDDIR"

	# TODO: Optimize. Currently we clone and build from scratch even if source is present but hasn't been built
	if [ ! -f llvm/build/bin/clang-8 ] || [ ! -f llvm/build/bin/llvm-config ];
	then
		git clone --branch llvmorg-$LLVM_RELEASE --depth 1  \
		https://github.com/llvm/llvm-project "$BUILDDIR/llvm-project"
		mv "$BUILDDIR/llvm-project/llvm" "$BUILDDIR/llvm"
		mv "$BUILDDIR/llvm-project/clang" "$BUILDDIR/llvm/tools/clang"
		rm -rf "$BUILDDIR/llvm-project"
		cd llvm
	       	mkdir build
		cd build
		[ -f "Makefile" ] || \
			CXXFLAGS="-D_GLIBCXX_USE_CXX11_ABI=0" \
			cmake ../
		make -kj
	fi
}

clean_llvm()
{
	cd "$BUILDDIR"
	rm -rf llvm
}

source_install_klee_uclibc()
{
	cd "$BUILDDIR"
	if [ -d 'klee-uclibc/.git' ];
	then
		cd klee-uclibc
		git fetch && git checkout "$KLEE_UCLIBC_RELEASE"
	else
		git clone --depth 1 --branch "$KLEE_UCLIBC_RELEASE" https://github.com/klee/klee-uclibc.git "$BUILDDIR/klee-uclibc";
		cd klee-uclibc
	fi

	./configure \
		--make-llvm-lib \
		--with-llvm-config="../llvm/build/bin/llvm-config" \
		--with-cc="../llvm/build/bin/clang"

	cp "$VNDSDIR/install/klee-uclibc.config" '.config'
	make -kj
}

clean_klee_uclibc()
{
	cd "$BUILDDIR"
	rm -rf klee-uclibc
}

source_install_klee()
{
	line "$PATHSFILE" 'KLEE_INCLUDE' "$BUILDDIR/klee/include"
	line_multi "$PATHSFILE" 'PATH' "$BUILDDIR/klee/build/bin:\$PATH"
	# shellcheck source=../paths.sh
	. "$PATHSFILE"

	cd "$BUILDDIR"
	if [ -d 'klee/.git' ];
	then
		cd klee
		git fetch && git checkout "$KLEE_RELEASE"
	else
		git clone --recurse-submodules https://github.com/bolt-perf-contracts/klee.git
		cd klee
		git checkout "$KLEE_RELEASE"
	fi

	./build.sh
}

clean_klee()
{
	cd "$BUILDDIR"
	rm -rf klee
}

bin_install_ocaml()
{
	# we depend on an OCaml package that needs libgmp-dev
	package_install opam m4 libgmp-dev

	opam init -y
	eval "$(opam config env)"
	# Opam 1.x doesn't have "create", later versions require it but only the first time
	if opam --version | grep '^1.' >/dev/null ; then
		opam switch $OCAML_RELEASE
	else
		opam switch list
		if ! opam switch list 2>&1 | grep -Fq 4.06 ; then
			opam switch create $OCAML_RELEASE
		fi
	fi

	line_multi "$PATHSFILE" 'PATH' "$HOME/.opam/system/bin:\$PATH"
    # `|| :` at the end of the following command ensures that in the event the
    # init.sh script fails, the shell will not exit. opam suggests we do this.
	echo ". $HOME/.opam/opam-init/init.sh || :" >> "$PATHSFILE"
	. "$PATHSFILE"

	# Codegenerator dependencies.
	opam install goblint-cil core -y
	opam install ocamlfind num -y
	opam install ocamlfind sexplib menhir -y

}

clean_ocaml()
{
	rm -rf $HOME/.opam
}

# Options
while getopts "hp:i:c:" o;
do
	case "${o}" in
	'h')
		usage;
		exit 0;
		;;
	'p')
		BUILDDIR="${OPTARG}"
		;;
	'i')
		INSTALL_ALL=
		case "${OPTARG}" in
		'dpdk')
			INSTALL_DPDK=true
			;;
		'pin')
			INSTALL_PIN=true
			;;
		'z3')
			INSTALL_Z3=true
			;;
		'llvm')
			INSTALL_LLVM=true
			;;
		'klee-uclibc')
			INSTALL_KLEE_UCLIBC=true
			;;
		'klee')
			INSTALL_KLEE=true
			;;
		'ocaml')
			INSTALL_OCAML=true
			;;
		'*')
			echo "Unkown component to install: $OPTARG" >&2
			usage;
			exit 1;
			;;
		esac
		;;
	'c')
		INSTALL_ALL=
		case "${OPTARG}" in
		'dpdk')
			CLEAN_DPDK=true
			;;
		'pin')
			CLEAN_PIN=true
			;;
		'z3')
			CLEAN_Z3=true
			;;
		'llvm')
			CLEAN_LLVM=true
			;;
		'klee-uclibc')
			CLEAN_KLEE_UCLIBC=true
			;;
		'klee')
			CLEAN_KLEE=true
			;;
		'ocaml')
			CLEAN_OCAML=true
			;;
		'*')
			echo "Unknown component to remove: $OPTARG" >&2
			usage;
			exit 1;
			;;
		esac
		;;
	'*')
		usage;
		exit 1;
		;;
	esac
done

# Environment
check_sudo
package_sync

# Common dependencies
package_install \
	build-essential \
	curl \
	git \
	libgoogle-perftools-dev \
	python2.7 \
	python3-minimal \
	python3-pip \
	parallel \
	gcc-multilib \
	graphviz \
	libnuma-dev \
	cmake

# Clean things
[ -n "$CLEAN_DPDK" ]        && clean_dpdk
[ -n "$CLEAN_PIN" ]         && clean_pin
[ -n "$CLEAN_Z3" ]          && clean_z3
[ -n "$CLEAN_LLVM" ]        && clean_llvm
[ -n "$CLEAN_KLEE_UCLIBC" ] && clean_klee_uclibc
[ -n "$CLEAN_KLEE" ]        && clean_klee
[ -n "$CLEAN_OCAML" ]       && clean_ocaml

# Install things
{ [ -n "$INSTALL_ALL" ] || [ -n "$INSTALL_DPDK" ] ; } && source_install_dpdk
{ [ -n "$INSTALL_ALL" ] || [ -n "$INSTALL_PIN" ]  ; } && source_install_pin
{ [ -n "$INSTALL_ALL" ] || [ -n "$INSTALL_Z3" ]   ; } && source_install_z3
{ [ -n "$INSTALL_ALL" ] || [ -n "$INSTALL_LLVM" ] ; } && source_install_llvm
{ [ -n "$INSTALL_ALL" ] || [ -n "$INSTALL_KLEE_UCLIBC" ] ; } && source_install_klee_uclibc
{ [ -n "$INSTALL_ALL" ] || [ -n "$INSTALL_OCAML" ] ; } && bin_install_ocaml
{ [ -n "$INSTALL_ALL" ] || [ -n "$INSTALL_KLEE" ] ; } && source_install_klee
