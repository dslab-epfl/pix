#!/bin/sh

# Unconstrained
make MEASURE_OUTPUT_FILE=perf_unconstrained.log PCAP_FILE=pcap/unirand10000.pcap measure

# External
make MEASURE_OUTPUT_FILE=perf_externalonly.log PCAP_FILE=pcap/empty.pcap PCAP_FILE2=pcap/unirand10000.pcap MEASURE_ADD_FLAGS=-DSTOP_ON_RX_1 measure

# Single flow
make MEASURE_OUTPUT_FILE=perf_singleflow.log PCAP_FILE=pcap/1packet10000.pcap measure

# No hash collision
make MEASURE_OUTPUT_FILE=perf_nohashcollisions.log PCAP_FILE=pcap/unirandnohashcollisions10000.pcap measure
