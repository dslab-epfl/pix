# Skeleton Makefile for Vigor NFs
# Variables that should be defined by inheriting Makefiles:
# - APP := <binary name>
# - NF_FILES := <NF files for both runtime and verif-time>
# - NF_VERIF_FILES := <verif-time only NF files>
# - NF_VERIF_DEFS := <verif-time only defs (-Dxxx)>
# - NF_VERIF_ARGS := <arguments to pass to the NF at verif-time>
# - NF_EXECUTABLE_OBJ_FILES := <Files for the executable> 

# -----------------------------------------------------------------------

# get current dir
# see https://stackoverflow.com/a/8080530
SELF_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))

REPLAY := -DREPLAY

MEASURE_OUTPUT_FILE := FAKEFILE
PCAP_FILE := FAKEFILE
PCAP_FILE2 := pcap/empty.pcap
MEASURE_ADD_FLAGS := -DSTOP_ON_RX_0
KLEE_ROOT=$(KLEE_INCLUDE)/..

# DPDK stuff
include $(RTE_SDK)/mk/rte.vars.mk

# allow the use of extglob in paths
SHELL = /bin/bash -O extglob -c

# NF base source
SRCS-y := $(SELF_DIR)/nf_main.c \
          $(SELF_DIR)/lib/nf_time.c \
          $(SELF_DIR)/lib/nf_util.c \
          $(SELF_DIR)/lib/expirator.c \
          $(SELF_DIR)/lib/containers/vector.c \
          $(SELF_DIR)/router/router_options.c \
					$(SELF_DIR)/lib/containers/libVig_common.c \

# NF specific source
SRCS-y += $(NF_FILES)

# For DS designed for performance predictability
# CFLAGS += -DPRED_DS
# SRCS-y += $(SELF_DIR)/lib/containers/pred-map.c $(SELF_DIR)/lib/containers/pred-map-impl.c $(SELF_DIR)/lib/containers/pred-double-map.c 

# VERIFIED MAP - This is "Unpredictable"
SRCS-y += $(SELF_DIR)/lib/containers/map.c $(SELF_DIR)/lib/containers/map-impl.c $(SELF_DIR)/lib/containers/double-map.c

# SOLAL MAP
#SRCS-y += $(SELF_DIR)/lib/containers/map-buckets.c $(SELF_DIR)/lib/containers/double-map-using-map.c
# DPDK MAP
#SRCS-y += $(SELF_DIR)/lib/containers/map-dpdk.c $(SELF_DIR)/lib/containers/double-map-using-map.c
# RUBY MAP
#SRCS-y += $(SELF_DIR)/lib/containers/map-ruby.c $(SELF_DIR)/unverified-nat/ruby/*.c $(SELF_DIR)/lib/containers/double-map-using-map.c
#CFLAGS += -I $(SELF_DIR)/unverified-nat -I $(SELF_DIR)/unverified-nat/ruby -DNOT_RUBY
# NULL MAP
#SRCS-y += $(SELF_DIR)/lib/containers/map-null.c $(SELF_DIR)/lib/containers/double-map-using-map.c

# VERIFIED DCHAIN
SRCS-y +=  $(SELF_DIR)/lib/containers/double-chain.c $(SELF_DIR)/lib/containers/double-chain-impl.c
# RISHABH DCHAIN
#SRCS-y += $(SELF_DIR)/lib/containers/alternate-double-chain.c

# LPM
SRCS-y +=  $(SELF_DIR)/lpm/lpm_dpdk.c

# compiler flags
CFLAGS += -I $(SELF_DIR) -DNO_STATIC_MAPPING $(ADDITIONAL_FLAGS)
CFLAGS += -std=gnu99
CFLAGS += -O2
# CFLAGS += -O0 -g -rdynamic -Wfatal-errors

# For perf clarity tests
# CFLAGS += -DCLARITY_TEST

ifeq ($(DUMP_LATENCY),YES)
# Dumps NF processing latency into a binary mmap'ed file
CFLAGS += -DDUMP_LATENCY
endif

ifeq ($(DUMP_PERF_VARS),YES)
# Dumps PCV values
CFLAGS += -DDUMP_PERF_VARS
endif

ifeq ($(DUMP_PERF_CTRS),YES)
# Dumps values of hardware perf counters using libpapi
CFLAGS += -I $(PAPI_SRC) -DTN_DEBUG_PERF=1000000
LDFLAGS += $(PAPI_SRC)/libpapi.a
endif

ifeq ($(REPLAY_PCAP),YES)
# To track branches during replay
CFLAGS += -DREPLAY_BR
endif

ifeq ($(FREUD),YES)
CFLAGS += -g -gstrict-dwarf -DFREUD
endif

# DPDK stuff
include $(RTE_SDK)/mk/rte.extapp.mk

### VeriFast verification ###
# we use DPDK stubs in there (e.g. rte_config.h)
verifast:
	verifast -I $(SELF_DIR) -I $(SELF_DIR)/lib/stubs/dpdk -allow_assume -shared \
		$(SELF_DIR)/lib/assumptions_stubs.c \
		$(SELF_DIR)/lib/flow.c \
		$(SELF_DIR)/lib/containers/double-chain-impl.c \
		$(SELF_DIR)/lib/containers/double-chain.c \
		$(SELF_DIR)/lib/containers/map-impl.c \
		$(SELF_DIR)/lib/containers/double-map.c \
		$(SELF_DIR)/lib/containers/vector.c \
		$(SELF_DIR)/lib/coherence.c \
		$(SELF_DIR)/lib/containers/map.c \
		$(SELF_DIR)/lib/expirator.c \
		$(SELF_DIR)/lib/abstract-state-verify-lemmas.c

# ^^^ TODO ^^^ put this back in: bridge_data.c


### KLEE verification ###

# Basic flags: only compile, emit debug code, in LLVM format, with checks for overflows
#              (but not unsigned overflows - they're not UB and DPDK depends on them)
VERIF_FLAGS := -c -g -emit-llvm -fsanitize=signed-integer-overflow $(REPLAY) -DNO_STATIC_MAPPING
VERIF_FLAGS += $(NF_VERIF_FLAGS)

# Klee-runtime executable compilation flags. Replay is unconditional here.
EXEC_FLAGS := -c -g -O2 -std=c99 -DREPLAY -DNO_STATIC_MAPPING -DVIGOR_EXECUTABLE
# Basic includes: NF root and KLEE
VERIF_INCLUDES := -I $(SELF_DIR) -I $(KLEE_INCLUDE)
# ...and the DPDK cmdline library
VERIF_INCLUDES += -I $(RTE_SDK)/lib/librte_cmdline

# Defines. TODO remove _GNU_SOURCE from here, #define it in files
VERIF_DEFS := -D_GNU_SOURCE -DKLEE_VERIFICATION
# Specific NF
VERIF_DEFS += $(NF_VERIF_DEFS)

# Basic files
# NF base
VERIF_FILES := $(SELF_DIR)/nf_main.c $(SELF_DIR)/lib/nf_util.c
# Specific NF
VERIF_FILES += $(NF_FILES)
# NF base stubs
VERIF_FILES += ../lib/stubs/containers/map-stub.c \
               ../lib/stubs/containers/double-map-stub.c \
               ../lib/stubs/containers/double-chain-stub.c \
               ../lib/stubs/containers/vector-stub.c \
               ../lib/stubs/containers/expirator-stub.c \
               ../lib/stubs/containers/traced-variables-stub.c \
							 ../router/router_options_stub.c \
               ../lpm/lpm_stub.c

# Specific NF stubs
VERIF_FILES += $(NF_VERIF_FILES)
# Environment stubs
VERIF_FILES += ../lib/stubs/externals/*.c ../lib/stubs/core_stub.c ../lib/stubs/time_stub.c
# DPDK cmdline parsing library, always included, we don't want/need to stub it
VERIF_FILES += $(RTE_SDK)/lib/librte_cmdline/*.c
# ...and the string function it uses
VERIF_FILES += $(RTE_SDK)/lib/librte_eal/common/eal_common_string_fns.c

# Defines for DPDK
# CPUFLAGS is set to a sentinel value; usually it's passed from the DPDK build system
VERIF_DPDK_DEFS := -DRTE_COMPILE_TIME_CPUFLAGS=424242

# Includes for DPDK
# We need librte_eal/common because eal_private.h is in there, required by eal_thread.c...
# We need bus/pci because the linuxapp PCI stuff requires a private.h file in there...
# bus/vdev and net/ixgbe are for stub drivers (which are vdevs) and hardware (the ixgbe driver) respectively
VERIF_DPDK_INCLUDES := -I $(RTE_SDK)/$(RTE_TARGET)/include \
			-I $(RTE_SDK)/lib/librte_eal/common \
			-I $(RTE_SDK)/drivers/bus/vdev \
			-I $(RTE_SDK)/drivers/bus/pci \
			-I $(RTE_SDK)/drivers/net/ixgbe
# And then some special DPDK includes: builtin_stubs for built-ins DPDK uses, and rte_config which many DPDK files use without including
VERIF_DPDK_INCLUDES += --include=lib/stubs/builtin_stub.h --include=rte_config.h
#As of now, executable does not compile with builtin_stub and gcc, so use the one below
#VERIF_DPDK_INCLUDES +=  --include=rte_config.h

# Files for DPDK
# Low-level stubs for specific functions
VERIF_DPDK_FILES := ../lib/stubs/dpdk_low_level_stub.c
# Platform-independent and Linux-specific EAL
VERIF_DPDK_FILES += $(RTE_SDK)/lib/librte_eal/common/*.c $(RTE_SDK)/lib/librte_eal/linuxapp/eal/*.c
# Default ring mempool driver
VERIF_DPDK_FILES += $(RTE_SDK)/drivers/mempool/ring/rte_mempool_ring.c
# Other libraries, except acl and distributor which use CPU intrinsics (there is a generic version of distributor, but we don't need it),
# and power has been broken for a while: http://dpdk.org/ml/archives/dev/2016-February/033152.html
VERIF_DPDK_FILES += $(RTE_SDK)/lib/!(librte_acl|librte_distributor|librte_power)/*.c
# Virtual devices support (for stub drivers)
VERIF_DPDK_FILES += $(RTE_SDK)/drivers/bus/vdev/*.c
# PCI driver support (for ixgbe driver)
VERIF_DPDK_FILES += $(RTE_SDK)/drivers/bus/pci/*.c $(RTE_SDK)/drivers/bus/pci/linux/*.c
# ixgbe driver (except bypass, SSE and ARM NEON stuff)
VERIF_DPDK_FILES += $(RTE_SDK)/drivers/net/ixgbe/!(ixgbe_bypass|ixgbe_82599_bypass|ixgbe_rxtx_vec_sse|ixgbe_rxtx_vec_neon).c $(RTE_SDK)/drivers/net/ixgbe/base/*.c

# Common object files for executables
# Basic NF stuff
EXECUTABLE_OBJ_FILES := nf_main.o nf_util.o
# DS Stubs
EXECUTABLE_OBJ_FILES += double-chain-stub.o expirator-stub.o map-stub.o double-map-stub.o vector-stub.o traced-variables-stub.o 
# DPDK cmdline parsing library
EXECUTABLE_OBJ_FILES +=	cmdline_socket.o cmdline.o cmdline_cirbuf.o cmdline_parse.o cmdline_parse_*.o cmdline_rdline.o cmdline_vt100.o
# Low-level stubs
EXECUTABLE_OBJ_FILES +=	core_stub.o time_stub.o time.o timerfd.o intrinsics.o
# Klee stuff
EXECUTABLE_OBJ_FILES += KTest.o

# Commands
# Cleanup
CLEAN_COMMAND := rm -f *.bc executable *.o
# Compilation
COMPILE_COMMAND := clang
# Linking
LINK_COMMAND := llvm-link -o nf.bc *.bc
# KLEE verification
# if something takes longer than expected, try --max-solver-time=3 --debug-report-symbdex (to avoid symbolic indices)
VERIF_COMMAND := /usr/bin/time -v \
                 klee -allocate-determ -allocate-determ-start-address=0x00040000000 -allocate-determ-size=1000 -libc=uclibc --external-calls=none --disable-verify -write-sym-paths -dump-call-traces -dump-call-trace-instructions -call-trace-instr-startfn=nf_core_process -call-trace-instr-endfn=nf_core_process -dump-call-trace-tree -dump-constraint-tree -solver-backend=z3 -exit-on-error -max-memory=750000 -search=dfs -condone-undeclared-havocs 
# NF args for the verification
NF_VERIF_BASE_ARGS := nf.bc --no-shconf --

verify-dpdk:
	$(CLEAN_COMMAND)
	# Use DPDK stub headers
	$(COMPILE_COMMAND) $(VERIF_DEFS) -DVIGOR_STUB_DPDK $(VERIF_INCLUDES) -I $(SELF_DIR)/lib/stubs/dpdk $(VERIF_FILES) $(VERIF_FLAGS)
	$(LINK_COMMAND)
	$(VERIF_COMMAND) $(NF_VERIF_BASE_ARGS) $(NF_VERIF_ARGS)
	$(CLEAN_COMMAND)

verify-driver:
	$(CLEAN_COMMAND)
	# Use DPDK stuff + stub driver
	$(COMPILE_COMMAND) $(VERIF_DEFS) $(VERIF_DPDK_DEFS) -DVIGOR_STUB_DRIVER $(VERIF_INCLUDES) $(VERIF_DPDK_INCLUDES) $(VERIF_FILES) $(VERIF_DPDK_FILES) $(SELF_DIR)/lib/stubs/driver_stub.c $(VERIF_FLAGS)
	$(LINK_COMMAND)
	$(VERIF_COMMAND) $(NF_VERIF_BASE_ARGS) $(NF_VERIF_ARGS)
	$(CLEAN_COMMAND)

verify-hardware:
	$(CLEAN_COMMAND)
	# Use DPDK stuff + stub hardware
	$(COMPILE_COMMAND) $(VERIF_DEFS) $(VERIF_DPDK_DEFS) -DVIGOR_STUB_HARDWARE $(VERIF_INCLUDES) $(VERIF_DPDK_INCLUDES) $(VERIF_FILES) $(VERIF_DPDK_FILES) $(SELF_DIR)/lib/stubs/hardware_stub.c $(VERIF_FLAGS)
	$(LINK_COMMAND)
	$(VERIF_COMMAND) $(NF_VERIF_BASE_ARGS) $(NF_VERIF_ARGS)
	$(CLEAN_COMMAND)

executable-vignat executable-bridge executable-vigbalancer executable-lpm executable-vigpol executable-vigfw executable-nop executable-router executable-firewall executable-stateless-toy executable-dpdk_nat executable-natasha:
	$(CLEAN_COMMAND)
	gcc $(VERIF_DEFS) -DVIGOR_STUB_DPDK $(VERIF_INCLUDES) \
	  -I $(SELF_DIR)/lib/stubs/dpdk $(VERIF_FILES) $(EXEC_FLAGS) $(NF_VERIF_FLAGS) \
     $(KLEE_ROOT)/runtime/Runtest/intrinsics.c \
	   $(KLEE_ROOT)/lib/Basic/KTest.cpp
	 gcc -o executable $(NF_EXECUTABLE_OBJ_FILES) $(EXECUTABLE_OBJ_FILES)

executable-click:
	$(CLEAN_COMMAND)
	gcc $(VERIF_DEFS) -DVIGOR_STUB_DPDK $(VERIF_INCLUDES) \
	  -I $(SELF_DIR)/lib/stubs/dpdk $(VERIF_FILES) $(EXEC_FLAGS) \
     $(KLEE_ROOT)/runtime/Runtest/intrinsics.c \
	   $(KLEE_ROOT)/lib/Basic/KTest.cpp
	 gcc -o executable `echo "$(CLICK_FILE)" | sed 's/\.c/\.o/'` $(EXECUTABLE_OBJ_FILES)

executable-vignat-hardware:
	$(CLEAN_COMMAND)
	gcc $(VERIF_DEFS) $(VERIF_DPDK_DEFS) -DVIGOR_STUB_HARDWARE $(VERIF_INCLUDES) $(VERIF_DPDK_INCLUDES) $(VERIF_FILES) $(VERIF_DPDK_FILES) $(SELF_DIR)/lib/stubs/hardware_stub.c $(EXEC_FLAGS) \
	    $(SELF_DIR)/lib/stubs/fcntl_open.c $(SELF_DIR)/lib/stubs/sigaction.c \
	    $(SELF_DIR)/lib/stubs/do_not_optimize_this_value_out.c \
	   $(KLEE_ROOT)/runtime/Runtest/intrinsics.c \
	   $(KLEE_ROOT)/lib/Basic/KTest.cpp
	gcc $(VERIF_DEFS) $(VERIF_DPDK_DEFS)  \
	    $(SELF_DIR)/lib/stubs/do_not_optimize_this_value_out.c \
	    -DVIGOR_STUB_HARDWARE $(VERIF_INCLUDES) $(VERIF_DPDK_INCLUDES) \
	    -c -g -O0 -DREPLAY -DNO_STATIC_MAPPING -DVIGOR_EXECUTABLE
	rm -f rte_table_acl.o
	gcc -g -Wl,--no-as-needed -ldl -lm -o executable \
	        nf_main.o nat_forward_vignat.o nf_util.o nat_config.o \
	        nat-flowmanager.o nat-flow.o nat-loop.o nat-state.o \
	        hardware_stub.o \
		dpdk_mempool_singleton.o \
	        double-chain-stub.o expirator-stub.o map-stub.o double-map-stub.o vector-stub.o traced-variables-stub.o \
	        cmdline_socket.o cmdline.o cmdline_cirbuf.o cmdline_parse.o cmdline_parse_*.o cmdline_rdline.o cmdline_vt100.o \
	        core_stub.o \
	        time_stub.o time.o timerfd.o \
	        eal*.o ixgbe*.o rte*.o \
	        ip*.o gro*.o gso*.o \
	        pci*.o \
	        dpdk_low_level_stub.o \
	        numa.o pipe.o \
	        ethdev_profile.o \
	        malloc_elem.o malloc_heap.o \
	        do_not_optimize_this_value_out.o \
		pthread.o \
	        intrinsics.o KTest.o \
                $(KLEE_UCLIBC_ROOT)/lib/libm.a \
             $(KLEE_UCLIBC_ROOT)/lib/libc.a \
    	stdio_files.o fcntl_open.o sigaction.o \
	    $(SELF_DIR)/lib/stubs/hardware_stub_wrappers.s

	
executable-bridge-hardware:
	$(CLEAN_COMMAND)
	gcc $(VERIF_DEFS) $(VERIF_DPDK_DEFS) -DVIGOR_STUB_HARDWARE $(VERIF_INCLUDES) $(VERIF_DPDK_INCLUDES) $(VERIF_FILES) $(VERIF_DPDK_FILES) $(EXEC_FLAGS) \
	    $(SELF_DIR)/lib/stubs/fcntl_open.c \
	    $(SELF_DIR)/lib/stubs/sigaction.c \
	    $(SELF_DIR)/lib/stubs/hardware_stub.c \
	   $(KLEE_ROOT)/runtime/Runtest/intrinsics.c \
	   $(KLEE_ROOT)/lib/Basic/KTest.cpp
	gcc $(VERIF_DEFS) $(VERIF_DPDK_DEFS)  \
	    $(SELF_DIR)/lib/stubs/do_not_optimize_this_value_out.c \
	    -DVIGOR_STUB_HARDWARE $(VERIF_INCLUDES) $(VERIF_DPDK_INCLUDES) \
	    -c -g -O0  -DREPLAY -DNO_STATIC_MAPPING -DVIGOR_EXECUTABLE
	rm -f rte_table_acl.o
	gcc -g -Wl,--no-as-needed -ldl -lm -o executable \
	        nf_main.o bridge_forward.o nf_util.o bridge_config.o bridge_data.o \
	        hardware_stub.o \
		dpdk_mempool_singleton.o \
	        bridge_loop.o double-chain-stub.o expirator-stub.o double-map-stub.o \
		map-stub.o vector-stub.o traced-variables-stub.o \
	        cmdline_socket.o cmdline.o cmdline_cirbuf.o cmdline_parse.o cmdline_parse_*.o cmdline_rdline.o cmdline_vt100.o \
	        core_stub.o \
	        time_stub.o time.o timerfd.o \
	        eal*.o ixgbe*.o rte*.o \
	        ip*.o gro*.o gso*.o \
	        pci*.o \
	        dpdk_low_level_stub.o \
	        numa.o pipe.o \
	        ethdev_profile.o \
	        malloc_elem.o malloc_heap.o \
	        do_not_optimize_this_value_out.o \
		pthread.o \
	        intrinsics.o KTest.o \
		$(KLEE_UCLIBC_ROOT)/lib/libm.a \
	     $(KLEE_UCLIBC_ROOT)/lib/libc.a \
	    stdio_files.o fcntl_open.o sigaction.o \
	    $(SELF_DIR)/lib/stubs/hardware_stub_wrappers.s

executable-vigbalancer-hardware:
	$(CLEAN_COMMAND)
	gcc $(VERIF_DEFS) $(VERIF_DPDK_DEFS) -DVIGOR_STUB_HARDWARE $(VERIF_INCLUDES) $(VERIF_DPDK_INCLUDES) $(VERIF_FILES) $(VERIF_DPDK_FILES) $(SELF_DIR)/lib/stubs/hardware_stub.c $(EXEC_FLAGS) \
	    $(SELF_DIR)/lib/stubs/fcntl_open.c $(SELF_DIR)/lib/stubs/sigaction.c \
	    $(SELF_DIR)/lib/stubs/do_not_optimize_this_value_out.c \
	   $(KLEE_ROOT)/runtime/Runtest/intrinsics.c \
	   $(KLEE_ROOT)/lib/Basic/KTest.cpp
	gcc $(VERIF_DEFS) $(VERIF_DPDK_DEFS)  \
	    $(SELF_DIR)/lib/stubs/do_not_optimize_this_value_out.c \
	    -DVIGOR_STUB_HARDWARE $(VERIF_INCLUDES) $(VERIF_DPDK_INCLUDES) \
	    -c -g -O0 -DREPLAY -DNO_STATIC_MAPPING -DVIGOR_EXECUTABLE
	rm -f rte_table_acl.o
	gcc -g -Wl,--no-as-needed -ldl -lm -o executable \
	        nf_util.o nf_main.o lb_main.o lb_balancer.o lb_config.o lb_data.o lb_loop.o \
	        hardware_stub.o \
		dpdk_mempool_singleton.o \
	        double-chain-stub.o expirator-stub.o double-map-stub.o traced-variables-stub.o \
		map-stub.o vector-stub.o \
	        cmdline_socket.o cmdline.o cmdline_cirbuf.o cmdline_parse.o cmdline_parse_*.o cmdline_rdline.o cmdline_vt100.o \
	        core_stub.o \
	        time_stub.o time.o timerfd.o \
	        eal*.o ixgbe*.o rte*.o \
	        ip*.o gro*.o gso*.o \
	        pci*.o \
	        dpdk_low_level_stub.o \
	        numa.o pipe.o \
	        ethdev_profile.o \
	        malloc_elem.o malloc_heap.o \
	        do_not_optimize_this_value_out.o \
		pthread.o \
	        intrinsics.o KTest.o \
                $(KLEE_UCLIBC_ROOT)/lib/libm.a \
             $(KLEE_UCLIBC_ROOT)/lib/libc.a \
    	stdio_files.o fcntl_open.o sigaction.o \
	    $(SELF_DIR)/lib/stubs/hardware_stub_wrappers.s



executable-lpm-hardware:
	gcc $(VERIF_DEFS) $(VERIF_DPDK_DEFS) -DVIGOR_STUB_HARDWARE $(VERIF_INCLUDES) $(VERIF_DPDK_INCLUDES) $(VERIF_FILES) $(VERIF_DPDK_FILES) $(SELF_DIR)/lib/stubs/hardware_stub.c $(EXEC_FLAGS) \
	    $(SELF_DIR)/lib/stubs/fcntl_open.c $(SELF_DIR)/lib/stubs/sigaction.c \
	   $(KLEE_ROOT)/runtime/Runtest/intrinsics.c \
	   $(KLEE_ROOT)/lib/Basic/KTest.cpp
	gcc $(VERIF_DEFS) $(VERIF_DPDK_DEFS)  \
	    $(SELF_DIR)/lib/stubs/do_not_optimize_this_value_out.c \
	    -DVIGOR_STUB_HARDWARE $(VERIF_INCLUDES) $(VERIF_DPDK_INCLUDES) \
	    -c -g -O0  -DREPLAY -DNO_STATIC_MAPPING -DVIGOR_EXECUTABLE
	rm -f rte_table_acl.o
	gcc -g -Wl,--no-as-needed -ldl -lm -o executable \
        nf_main.o lpm.o lpm_stub.o nf_util.o nat_config.o\
	        hardware_stub.o \
		dpdk_mempool_singleton.o \
	        double-chain-stub.o expirator-stub.o double-map-stub.o traced-variables-stub.o \
	        cmdline_socket.o cmdline.o cmdline_cirbuf.o cmdline_parse.o cmdline_parse_*.o cmdline_rdline.o cmdline_vt100.o \
	        core_stub.o \
	        time_stub.o time.o timerfd.o \
	        eal*.o ixgbe*.o rte*.o \
	        ip*.o gro*.o gso*.o \
	        pci*.o \
	        dpdk_low_level_stub.o \
	        numa.o pipe.o \
	        ethdev_profile.o \
	        malloc_elem.o malloc_heap.o \
	        do_not_optimize_this_value_out.o \
		pthread.o \
	        intrinsics.o KTest.o \
                $(KLEE_UCLIBC_ROOT)/lib/libm.a \
             $(KLEE_UCLIBC_ROOT)/lib/libc.a \
	    stdio_files.o fcntl_open.o sigaction.o \
	    $(SELF_DIR)/lib/stubs/hardware_stub_wrappers.s

executable-router-hardware:
	$(CLEAN_COMMAND)
	gcc $(VERIF_DEFS) $(VERIF_DPDK_DEFS) -DVIGOR_STUB_HARDWARE $(VERIF_INCLUDES) $(VERIF_DPDK_INCLUDES) $(VERIF_FILES) $(VERIF_DPDK_FILES) $(SELF_DIR)/lib/stubs/hardware_stub.c $(EXEC_FLAGS) \
	    $(SELF_DIR)/lib/stubs/fcntl_open.c $(SELF_DIR)/lib/stubs/sigaction.c \
	    $(SELF_DIR)/lib/stubs/do_not_optimize_this_value_out.c \
	    router_options.c \
	   $(KLEE_ROOT)/runtime/Runtest/intrinsics.c \
	   $(KLEE_ROOT)/lib/Basic/KTest.cpp
	gcc $(VERIF_DEFS) $(VERIF_DPDK_DEFS)  \
	    $(SELF_DIR)/lib/stubs/do_not_optimize_this_value_out.c \
	    -DVIGOR_STUB_HARDWARE $(VERIF_INCLUDES) $(VERIF_DPDK_INCLUDES) \
	    -c -g -O0  -DREPLAY -DNO_STATIC_MAPPING -DVIGOR_EXECUTABLE
	rm -f rte_table_acl.o
	gcc -g -Wl,--no-as-needed -ldl -lm -o executable \
	        nf_main.o router.o router_options.o nf_util.o nat_config.o\
	        hardware_stub.o \
		dpdk_mempool_singleton.o \
	        double-chain-stub.o expirator-stub.o double-map-stub.o traced-variables-stub.o \
	        cmdline_socket.o cmdline.o cmdline_cirbuf.o cmdline_parse.o cmdline_parse_*.o cmdline_rdline.o cmdline_vt100.o \
	        core_stub.o \
	        time_stub.o time.o timerfd.o \
	        eal*.o ixgbe*.o rte*.o \
	        ip*.o gro*.o gso*.o \
	        pci*.o \
	        dpdk_low_level_stub.o \
	        numa.o pipe.o \
	        ethdev_profile.o \
	        malloc_elem.o malloc_heap.o \
	        do_not_optimize_this_value_out.o \
		pthread.o \
	        intrinsics.o KTest.o \
                $(KLEE_UCLIBC_ROOT)/lib/libm.a \
             $(KLEE_UCLIBC_ROOT)/lib/libc.a \
   	stdio_files.o fcntl_open.o sigaction.o \
	    $(SELF_DIR)/lib/stubs/hardware_stub_wrappers.s

executable-firewall-hardware:
	$(CLEAN_COMMAND)
	gcc $(VERIF_DEFS) $(VERIF_DPDK_DEFS) -DVIGOR_STUB_HARDWARE $(VERIF_INCLUDES) $(VERIF_DPDK_INCLUDES) $(VERIF_FILES) $(VERIF_DPDK_FILES) $(SELF_DIR)/lib/stubs/hardware_stub.c $(EXEC_FLAGS) \
	    $(SELF_DIR)/lib/stubs/fcntl_open.c $(SELF_DIR)/lib/stubs/sigaction.c \
	    $(SELF_DIR)/lib/stubs/do_not_optimize_this_value_out.c \
	   $(KLEE_ROOT)/runtime/Runtest/intrinsics.c \
	   $(KLEE_ROOT)/lib/Basic/KTest.cpp
	gcc $(VERIF_DEFS) $(VERIF_DPDK_DEFS)  \
	    $(SELF_DIR)/lib/stubs/do_not_optimize_this_value_out.c \
	    -DVIGOR_STUB_HARDWARE $(VERIF_INCLUDES) $(VERIF_DPDK_INCLUDES) \
	    -c -g -O0  -DREPLAY -DNO_STATIC_MAPPING -DVIGOR_EXECUTABLE
	rm -f rte_table_acl.o
	gcc -g -Wl,--no-as-needed -ldl -lm -o executable \
		nf_main.o firewall.o nf_util.o nat_config.o\
	        hardware_stub.o \
		dpdk_mempool_singleton.o \
	        double-chain-stub.o expirator-stub.o double-map-stub.o traced-variables-stub.o \
	        cmdline_socket.o cmdline.o cmdline_cirbuf.o cmdline_parse.o cmdline_parse_*.o cmdline_rdline.o cmdline_vt100.o \
	        core_stub.o \
	        time_stub.o time.o timerfd.o \
	        eal*.o ixgbe*.o rte*.o \
	        ip*.o gro*.o gso*.o \
	        pci*.o \
	        dpdk_low_level_stub.o \
	        numa.o pipe.o \
	        ethdev_profile.o \
	        malloc_elem.o malloc_heap.o \
	        do_not_optimize_this_value_out.o \
		pthread.o \
	        intrinsics.o KTest.o \
                $(KLEE_UCLIBC_ROOT)/lib/libm.a \
             $(KLEE_UCLIBC_ROOT)/lib/libc.a \
	    stdio_files.o fcntl_open.o sigaction.o \
	    $(SELF_DIR)/lib/stubs/hardware_stub_wrappers.s

measure:
	@if [ '$(MEASURE_OUTPUT_FILE)' = 'FAKEFILE' ]; then \
	  echo ''; echo 'Please override MEASURE_OUTPUT_FILE on the command-line with a file name.'; echo ''; \
	  exit 1; \
	fi

	@if [ '$(PCAP_FILE)' = 'FAKEFILE' ]; then \
	  echo ''; echo 'Please override PCAP_FILE on the command-line with a path relative to vnds/nf/.'; echo ''; \
	  exit 1; \
	fi

	@# First, times
	make clean
	make ADDITIONAL_FLAGS=$(MEASURE_ADD_FLAGS)
	sudo taskset -c 8 ./build/app/$(APP) --vdev 'net_pcap0,rx_pcap=$(SELF_DIR)/$(PCAP_FILE),tx_pcap=/dev/null' --vdev 'net_pcap1,rx_pcap=$(SELF_DIR)/$(PCAP_FILE2),tx_pcap=/dev/null' $(NF_VERIF_BASE_ARGS) $(NF_VERIF_ARGS) | tee measure.log

	@# Then, instructions
	make clean -C $(SELF_DIR)/pin
	make -C $(SELF_DIR)/pin
	@# Measuring only the NF with pcap drivers
	sudo `which pin` -t $(SELF_DIR)/pin/counts.so -- ./build/app/$(APP) --vdev 'net_pcap0,rx_pcap=$(SELF_DIR)/$(PCAP_FILE),tx_pcap=/dev/null' --vdev 'net_pcap1,rx_pcap=$(SELF_DIR)/$(PCAP_FILE2),tx_pcap=/dev/null' $(NF_VERIF_BASE_ARGS) $(NF_VERIF_ARGS)
	@# Measuring NF+DPDK+driver 
	sudo taskset -c 8 ./build/app/$(APP) $(NF_VERIF_BASE_ARGS) $(NF_VERIF_ARGS) | tee measure.log

	@# Now, write a report
	@grep 'Time is' measure.log | cut -d ' ' -f3 > measure_times.log
	@paste -d ' ' measure_times.log pincounts.log > $(MEASURE_OUTPUT_FILE)
	@rm -f measure.log measure_times.log pincounts.log
	@echo ''; echo 'Times/instrs/mem_reads/mem_writes per packet, sorted by times, are in $(MEASURE_OUTPUT_FILE)';

	@# idk why those exist
	@rm -rf _install _postbuild _postinstall _preinstall $(APP) $(APP).map

# run "make klee-last/test000100.tracelog" to exercise this rule
klee-last/%.tracelog: klee-last/%.ktest executable
	@mkdir tmpdir_$*
	@# HOME variable messes with the rte config file location, so better without it.
	@cd tmpdir_$* && unset HOME && KTEST_FILE=../$< pin -t $(KLEE_ROOT)/trace-instructions/pin-trace.so -start_fn $(START_FN) -end_fn $(END_FN) -- ../executable --no-shconf --  $(NF_VERIF_ARGS) || true
	@mv tmpdir_$*/trace.out $@
	@rmdir tmpdir_$*

KTESTS=$(shell find klee-last/ -name *.ktest)
INSTR_TRACES=$(KTESTS:%.ktest=%.tracelog)

.PHONY: instr-traces

# run "make -j 40 instr-traces" to exercise this rule in parallel
instr-traces: $(INSTR_TRACES)

perf-descriptions:
	@bash $(KLEE_ROOT)/scripts/tree-gen/build_trees.sh -m $(MAX_PERF) -n $(MIN_PERF)
