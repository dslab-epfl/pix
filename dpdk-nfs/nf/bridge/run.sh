#!/bin/bash


# Runs VigNAT for a specified scenario

# Parameters:
# $1: The scenario: can be "loopback" or "rr"
# $2: Config file 
# $3: Expiration time for flows (Hack)

SCENARIO=$1
CONFIG_FILE=$2
EXP_TIME=$3

MAX_FLOWS=65536

. $CONFIG_FILE
set -x 
# List of config variables for NAT to run - see lib/nat_config.c 

if [ $SCENARIO = "loopback" ]; then
    sudo taskset -c 8 ./build/bridge -n 2 -- --capacity $MAX_FLOWS \
	  --expire $EXP_TIME
elif [ $SCENARIO = "loopback-instr" ]; then
    pushd ../pin
        make clean && make
    popd
    make clean && make REPLAY_PCAP=YES
    sudo taskset -c 8 $PINDIR/pin -t ../pin/build/counts.so -start_fn nf_core_process -end_fn nf_core_process -- ./build/bridge -n 2 -- --capacity $MAX_FLOWS \
	  --expire $EXP_TIME   
else
    echo "[bench] ERROR: non-loopback is not supported" 1>&2
    exit 1
fi
