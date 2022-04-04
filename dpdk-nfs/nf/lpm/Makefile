# Binary name
APP := lpm

# Runtime and verification files
NF_FILES := lpm.c ../lib/nat_config.c ../lib/ethtool.c

# Verification files
#NF_VERIF_FILES := 

# Verification arguments
NF_VERIF_ARGS := --lan-dev 0 \
                 --wan 1 \
                 --expire 10 \
                 --starting-port 0 \
                 --max-flows 65536 \
                 --extip 0.0.0.0 \
                 --eth-dest 0,01:23:45:67:89:00 \
                 --eth-dest 1,01:23:45:67:89:01

# Object files to link in. TODO: Autogenerate from NF_FILES
NF_EXECUTABLE_OBJ_FILES := lpm.o nat_config.o ethtool.o  \
            lpm_stub.o

# Include parent (in a convoluted way cause of DPDK)
include $(abspath $(dir $(lastword $(MAKEFILE_LIST))))/../Makefile
