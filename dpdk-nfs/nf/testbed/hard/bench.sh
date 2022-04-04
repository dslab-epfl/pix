#!/bin/bash
. ./config.sh

# Master script to benchmark VigNAT-related programs.
# Can benchmark different implementations, including non-NATs,
# using different scenarios.

# Parameters:
# $1: The app, either a known name or a DPDK NAT-like app.
#     Known names: "netfilter".
#     Otherwise, a folder name containing a DPDK NAT-like app, e.g. "~/vnds/nat"
# $2: The scenario, one of the following:
#     "mg-1p": Measure throughput: find the rate at which the middlebox
#              starts losing 1% of packets.
#     "mg-existing-flows-latency": Measure the forwarding latency for existing
#                                  flows.
#     "mg-new-flows-latency": Measure the forwarding latency for new flows.
#     "replay-pcap": Replays a PCAP file
#     "replay-pcap-instr": Replays a PCAP file, while running it through the Pin instrumentation tool
#     "loopback": Measure throughput.
#                 Tester and middlebox are connected together in a loop,
#                 using 2 interfaces on each, in different subnets; server is ignored.
#     "1p": Measure throughput.
#           Find the point at which the middlebox starts dropping 1% of packets.
#     "passthrough": Measure latency.
#                    Tester sends packets to server, which pass through the middlebox;
#                    all machines are in the same subnet.
#     "rr": Measure latency.
#           Tester sends packets to server, which are modified by the middlebox;
#           there are two subnets, tester-middlebox and middlebox-server.
#           a.k.a. request/response
# $3: The type of NF, either NAT/Br/LB/Pol/FW/NOP
#     For running programs such as netfilter please provide the NF it is being used as a baseline for:.
# $4: The build type for the NF - whether to run the NFs in native mode, or with instrumention for latency, PCVs, perf counters. 
# $5: PCAP file replayed for the scenarios "replay-pcap" and "replay-pcap-instr"

MIDDLEBOX=$1
SCENARIO=$2
NF_TYPE=$3
BUILD_TYPE=${4:-DEFAULT}
PCAP_FILE=${5:-replay.pcap} #The PCAP file to be replayed

if [ -z $MIDDLEBOX ]; then
    echo "[bench] No app specified" 1>&2
    exit 1
fi

if [ -z $SCENARIO ]; then
    echo "[bench] No scenario specified" 1>&2
    exit 2
fi

if [ -z $NF_TYPE ]; then 
    echo "[bench] NF Type not specified " 1>&2
    exit 3
fi

if ! [[ "$BUILD_TYPE" =~ ^(DEFAULT|DUMP_LATENCY|DUMP_PERF_CTRS|DUMP_PERF_VARS)$ ]]; then
  echo "Unsupported build type: $BUILD_TYPE"
  echo "Supported build types are DEFAULT, DUMP_LATENCY, DUMP_PERF_CTRS, DUMP_PERF_VARS"
  exit 4
fi

CLEAN_APP_NAME=`echo "$MIDDLEBOX" | tr '/' '_'`
RESULTS_FILE="bench-$CLEAN_APP_NAME-$SCENARIO.results"

if [ -f "$RESULTS_FILE" ]; then
    rm "$RESULTS_FILE"
fi


# Initialize the machines, i.e. software+scripts
. ./init-machines.sh

# Clean first, just in case
. ./clean.sh

. init.sh $MIDDLEBOX $SCENARIO

. start-middlebox.sh $MIDDLEBOX $SCENARIO $BUILD_TYPE

. run.sh $MIDDLEBOX $SCENARIO $NF_TYPE $RESULTS_FILE $PCAP_FILE

. stop-middlebox.sh $MIDDLEBOX

. clean.sh
