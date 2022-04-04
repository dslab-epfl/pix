#!/bin/bash
set -e

NF=$1
METRICS=${2-x86}
TEST=${3:-all}

if ! [[ "$NF" =~ ^(vignat|bridge|vigbalancer|lpm|vigpol|vigfw)$ ]]; then
  echo "Unsupported NF: $NF"
  echo "Supported NFs are vignat, bridge, vigbalancer, lpm, vigpol, vigfw"
  exit
fi


if [ "$TEST" != "all" ] && [ "$TEST" != "contracts-only" ]; then
  echo "Unsupported parameter: $TEST" 
  echo "Support parameters : all, contracts-only"
  exit
fi

export START_FN="nf_core_process"
export END_FN="nf_core_process"

if [ "$TEST" == "all" ]; then 
	make verify-dpdk
	make executable-$NF
	make -j $(nproc) instr-traces
fi

pushd ../../perf-contracts
    if [ $METRICS == "llvm" ]; then
    make clean && make LLVM=TRUE
    else
    make clean && make
    fi

popd 

pushd klee-last
	$KLEE_INCLUDE/../scripts/process-traces.sh . verify-dpdk $METRICS Num_bucket_traversals 1 Num_hash_collisions 0  expired_flows 0 
popd

export MAX_PERF=1000 && export MIN_PERF=100
make perf-descriptions
