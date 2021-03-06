#!/bin/bash
. ./config.sh

# Master script to initialize VigNAT-related programs benchmarks.
# Can work with different implementations, including non-NATs,
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
# $3: The build type for the NF - whether to run the NFs in native mode, or with instrumention for latency, PCVs, perf counters.

MIDDLEBOX=$1
SCENARIO=$2
BUILD_TYPE=$3

if [ -z $MIDDLEBOX ]; then
    echo "[bench] No app specified" 1>&2
    exit 1
fi

if [ -z $SCENARIO ]; then
    echo "[bench] No scenario specified" 1>&2
    exit 2
fi

CLEAN_APP_NAME=`echo "$MIDDLEBOX" | tr '/' '_'`
LOG_FILE="bench-$CLEAN_APP_NAME-$SCENARIO-init.log"

if [ -f "$LOG_FILE" ]; then
    rm "$LOG_FILE"
fi


if [ "$MIDDLEBOX" = "netfilter" -o "$MIDDLEBOX" = "ipvs" ]; then
    case $SCENARIO in
	"mg-new-flows-latency")
	    EXPIRATION_TIME=1
	    ;;
        "1p"|"loopback"|"mg-1p"|"mg-existing-flows-latency"|"rr"|"passthrough")
            EXPIRATION_TIME=60
            ;;
    esac

    bash ./util/netfilter-short-timeout.sh $EXPIRATION_TIME
else
    echo "[bench] Launching $MIDDLEBOX..."

    EXPIRATION_TIME=60

    case $SCENARIO in
        "mg-new-flows-latency")
            SIMPLE_SCENARIO="loopback"
            ;;
        "1p"|"loopback"|"mg-1p"|"mg-existing-flows-latency"|"replay-pcap")
            SIMPLE_SCENARIO="loopback"
            EXPIRATION_TIME=60000000000 #One minute
	    ;;
        "replay-pcap-instr")
            SIMPLE_SCENARIO="loopback-instr"
            EXPIRATION_TIME=60000 #One minute
            ;;
        "rr"|"passthrough")
            SIMPLE_SCENARIO="rr"
            EXPIRATION_TIME=60
            ;;
        *)
            echo "Unknown scenario $SCENARIO" 1>&2
            exit 10
            ;;
    esac

    # Run the app in the background
    # The arguments are not always necessary, but they'll be ignored if unneeded
    (bash ./bench/run-middlebox.sh $SIMPLE_SCENARIO "$MIDDLEBOX" $BUILD_TYPE\
        "$EXPIRATION_TIME" \
        0<&- &>"$LOG_FILE") &

    # Wait for it to have started
    sleep 20
fi
