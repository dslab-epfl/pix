#!/bin/bash

set -euo pipefail

SCRIPT_DIR=$(cd $(dirname ${BASH_SOURCE[0]}) && pwd)

function cleanup {
  sudo killall bridge 2>/dev/null || true
  sudo ip netns delete lan0 2>/dev/null || true
  sudo ip netns delete lan1 2>/dev/null || true
  sudo ip netns delete lan2 2>/dev/null || true
}
trap cleanup EXIT


function test_bridge {
  sudo taskset -c 8 \
      ./build/app/bridge \
        --vdev "net_tap0,iface=test_lan0" \
        --vdev "net_tap1,iface=test_lan1" \
        --vdev "net_tap2,iface=test_lan2" \
        --no-shconf -- \
        --expire 3600 \
        --capacity 8192 \
        --config /dev/null \
        >/dev/null 2>/dev/null &
  NF_PID=$!

  while [ ! -f /sys/class/net/test_lan0/tun_flags -o \
          ! -f /sys/class/net/test_lan1/tun_flags -o \
          ! -f /sys/class/net/test_lan2/tun_flags ]; do
    echo "Waiting for NF to launch...";
    sleep 1;
  done
  sleep 2

  sudo ip netns add lan0
  sudo ip link set test_lan0 netns lan0
  sudo ip netns exec lan0 ifconfig test_lan0 up 10.0.0.1

  sudo ip netns add lan1
  sudo ip link set test_lan1 netns lan1
  sudo ip netns exec lan1 ifconfig test_lan1 up 10.0.0.2

  sudo ip netns add lan2
  sudo ip link set test_lan2 netns lan2
  sudo ip netns exec lan2 ifconfig test_lan2 up 10.0.0.3

  sudo ip netns exec lan0 ping -c 1 10.0.0.2
  sudo ip netns exec lan0 ping -c 1 10.0.0.3
  sudo ip netns exec lan1 ping -c 1 10.0.0.1
  sudo ip netns exec lan1 ping -c 1 10.0.0.3
  sudo ip netns exec lan2 ping -c 1 10.0.0.1
  sudo ip netns exec lan2 ping -c 1 10.0.0.2

  sudo killall bridge
  wait $NF_PID 2>/dev/null || true

  sudo ip netns delete lan0
  sudo ip netns delete lan1
  sudo ip netns delete lan2
}


make clean
make -j$(nproc)

test_bridge

echo "Done."
