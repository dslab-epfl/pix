#!/bin/bash

set -euo pipefail

SCRIPT_DIR=$(cd $(dirname ${BASH_SOURCE[0]}) && pwd)

function test_lb {
  INPUT_PCAP=${1}
  NUM_BACKENDS=${2:-2}

  BACKEND_VDEV_ARGS=""
  BACKEND_MAC_ARGS=""
  OUTPUT_PCAPS=""
  for i in $(seq 1 $NUM_BACKENDS); do
    BACKEND_VDEV_ARGS="$BACKEND_VDEV_ARGS --vdev 'net_pcap$i,rx_pcap=$SCRIPT_DIR/../pcap/100/1packet-$i.pcap,tx_pcap=$i.pcap'"
    OUTPUT_PCAPS="$OUTPUT_PCAPS $i.pcap"
  done

  rm -f $OUTPUT_PCAPS

  eval sudo taskset -c 8 \
      ./build/app/lb \
       --vdev "net_pcap0,rx_pcap=$INPUT_PCAP,tx_pcap=/dev/null" \
        $BACKEND_VDEV_ARGS \
        --no-shconf -- \
        --flow-expiration 10 \
        --flow-capacity 65536 \
        --backend-capacity 20 \
        --cht-height 29 \
        --backend-expiration 60 \
      >/dev/null 2>/dev/null

  parallel 'tcpdump -ner {} 2>/dev/null | tail -n +2 | wc -l' ::: $OUTPUT_PCAPS | sort -n
  BACKEND_MIN=$(parallel 'tcpdump -ner {} 2>/dev/null | tail -n +2 | wc -l' ::: $OUTPUT_PCAPS | sort -n | head -n 1)
  BACKEND_MAX=$(parallel 'tcpdump -ner {} 2>/dev/null | tail -n +2 | wc -l' ::: $OUTPUT_PCAPS | sort -n | tail -n 1)
  BACKEND_SUM=$(parallel 'tcpdump -ner {} 2>/dev/null | tail -n +2 | wc -l' ::: $OUTPUT_PCAPS | awk '{ sum += $1; } END { print sum}')
  FRONTEND_TOTAL=$(tcpdump -ner $INPUT_PCAP 2>/dev/null | tail -n +2 | wc -l)

  rm -f $OUTPUT_PCAPS

  echo "Skew for $NUM_BACKENDS backends: $(( 1000000 * (BACKEND_MAX - BACKEND_MIN) / FRONTEND_TOTAL ))ppm"
  echo "Packets dropped for $NUM_BACKENDS backends: $(( 1000000 * (FRONTEND_TOTAL - BACKEND_SUM) / FRONTEND_TOTAL ))ppm ($((FRONTEND_TOTAL - BACKEND_SUM)) packets)"
}

make clean
make ADDITIONAL_FLAGS=-DSTOP_ON_RX_0

for i in {2..15}; do
  test_lb $SCRIPT_DIR/../pcap/unirand10000.pcap $i
done
