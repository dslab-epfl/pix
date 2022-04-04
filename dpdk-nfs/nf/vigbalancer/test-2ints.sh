#!/bin/bash

set -euo pipefail

SCRIPT_DIR=$(cd $(dirname ${BASH_SOURCE[0]}) && pwd)

function test_lb {
  INPUT_PCAP=${1}
  NUM_BACKENDS=${2:-2}

  rm -f heartbeat.pcap output.pcap

  for i in $(seq 1 $NUM_BACKENDS); do
    echo "192.168.1.$i 10000 192.168.0.1 80"
  done | $SCRIPT_DIR/create_flows_pcap.py --output heartbeat.pcap

  sudo taskset -c 8 \
      ./build/app/lb \
       --vdev "net_pcap0,rx_pcap=$INPUT_PCAP,tx_pcap=/dev/null" \
       --vdev "net_pcap1,rx_pcap=heartbeat.pcap,tx_pcap=output.pcap" \
        --no-shconf -- \
        --flow-expiration 10 \
        --flow-capacity 65536 \
        --backend-capacity 20 \
        --cht-height 29 \
        --backend-expiration 60 \
      >/dev/null 2>/dev/null

  tcpdump -ner output.pcap 2>/dev/null | sed -e 's/^.*> \([0-9]*\.[0-9]*\.[0-9]*\.[0-9]*\).*$/\1/;t;d' | sort | uniq -c | sort -n
  BACKEND_MIN=$(tcpdump -ner output.pcap 2>/dev/null | sed -e 's/^.*> \([0-9]*\.[0-9]*\.[0-9]*\.[0-9]*\).*$/\1/;t;d' | sort | uniq -c | sort -n | head -n 1 | awk '{print $1}')
  BACKEND_MAX=$(tcpdump -ner output.pcap 2>/dev/null | sed -e 's/^.*> \([0-9]*\.[0-9]*\.[0-9]*\.[0-9]*\).*$/\1/;t;d' | sort | uniq -c | sort -n | tail -n 1 | awk '{print $1}')
  BACKEND_SUM=$(tcpdump -ner output.pcap 2>/dev/null | sed -e 's/^.*> \([0-9]*\.[0-9]*\.[0-9]*\.[0-9]*\).*$/\1/;t;d' | sort | uniq -c  | awk '{ sum += $1; } END { print sum}')
  FRONTEND_TOTAL=$(tcpdump -ner $INPUT_PCAP 2>/dev/null | tail -n +2 | wc -l)

  rm -f heartbeat.pcap output.pcap

  echo "Skew for $NUM_BACKENDS backends: $(( 1000000 * (BACKEND_MAX - BACKEND_MIN) / FRONTEND_TOTAL ))ppm"
  echo "Packets dropped for $NUM_BACKENDS backends: $(( 1000000 * (FRONTEND_TOTAL - BACKEND_SUM) / FRONTEND_TOTAL ))ppm ($((FRONTEND_TOTAL - BACKEND_SUM)) packets)"
}

make clean
make ADDITIONAL_FLAGS=-DSTOP_ON_RX_0

for i in {2..15}; do
  test_lb $SCRIPT_DIR/../pcap/unirand10000.pcap $i
done
