#!/bin/sh

# Unconstrained
make MEASURE_OUTPUT_FILE=perf_unconstrained.log PCAP_FILE=pcap/unirand10000.pcap measure

# Known MAC
make MEASURE_OUTPUT_FILE=perf_knownmac.log PCAP_FILE=pcap/1packetinverted.pcap PCAP_FILE2=pcap/unirand10000.pcap MEASURE_ADD_FLAGS=-DSTOP_ON_RX_1 measure

# Unknown MAC
make MEASURE_OUTPUT_FILE=perf_unknownmac.log PCAP_FILE=pcap/1packet10000.pcap measure

# No collisions
make MEASURE_OUTPUT_FILE=perf_nohashcollisions.log PCAP_FILE=pcap/unirandnobridgehashcollisions10000.pcap measure
