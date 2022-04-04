#!/bin/sh

# No options
make MEASURE_OUTPUT_FILE=perf_random.log PCAP_FILE=pcap/unirand10000.pcap measure

# With IP options
make MEASURE_OUTPUT_FILE=perf_ipoptions.log PCAP_FILE=pcap/ipoptions10000.pcap measure
