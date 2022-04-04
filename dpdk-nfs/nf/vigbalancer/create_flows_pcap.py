#!/usr/bin/python

import argparse
import csv
import sys
from random import randint

from scapy.all import *
from scapy.utils import PcapWriter

if __name__ == "__main__":
    # parse params
    parser = argparse.ArgumentParser(description="create a pcap file with packets for the given flows from stdin")
    parser.add_argument('--output',  help='name of output pcap file', required=True)

    args = parser.parse_args()

    pktdump = PcapWriter(args.output, append=False)

    for line in sys.stdin:
        src_ip, sport, dst_ip, dport = line.split(' ')

        pkt = Ether(src="08:00:27:53:8b:38", dst="08:00:27:c1:13:47")
        pkt = pkt/IP(src=src_ip, dst=dst_ip)
        pkt = pkt/UDP(sport=int(sport), dport=int(dport))

        pktdump.write(pkt)
