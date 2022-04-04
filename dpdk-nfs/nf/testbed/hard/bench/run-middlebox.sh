#!/bin/bash
. ./config.sh

# Runs a DPDK app for the specified scenario

# Parameters:
# $1: Scenario, can be "loopback" or "rr"
# $2: Folder of the app; the app must be named "nat"
#     and take the usual arguments e.g. "--extip"
# $3: The build type for the NF - whether to run the NFs in native mode (DEFAULT), or with instrumention for latency (DUMP_LATENCY), PCVs (DUMP_PERF_VARS), perf counters (DUMP_PERF_CTRS).
# $4: Additional parameters for the NF

SCENARIO=$1
MIDDLEBOX=$2
BUILD_TYPE=$3
PARAMS=$4

BUILD_PARAM="$BUILD_TYPE=YES"
CONFIG=$(realpath config.sh)

pushd $MIDDLEBOX >> /dev/null

   echo "[bench] Building $MIDDLEBOX..."
   sudo rm build -rf
   make clean
   make $BUILD_PARAM
  
   echo "[bench] Running $MIDDLEBOX..."
   if [ ! -f run.sh ]; then 
	   echo "Cannot find run file for Middlebox:$MIDDLEBOX"
   else
           bash run.sh $SCENARIO $CONFIG "$PARAMS" #Params is hacky here, but ehh
   fi
 
popd >> /dev/null
