# Binary name
APP := nat

# Runtime and verification files
NF_FILES := nat_forward_vignat.c ../lib/nat_config.c ../lib/ethtool.c \
            nat-flow.c nat-state.c nat-flowmanager.c

# Verification files
NF_VERIF_FILES := nat-loop.c

# Verification arguments
NF_VERIF_ARGS := --lan-dev 0 \
                 --wan 1 \
                 --expire 10 \
                 --starting-port 0 \
                 --max-flows 262144 \
                 --extip 0.0.0.0 \
                 --eth-dest 0,01:23:45:67:89:00 \
                 --eth-dest 1,01:23:45:67:89:01

# Object files to link in. TODO: Autogenerate from NF_FILES
NF_EXECUTABLE_OBJ_FILES := nat_forward_vignat.o nat_config.o ethtool.o \
            nat-flow.o nat-state.o nat-flowmanager.o nat-loop.o

# Include parent (in a convoluted way cause of DPDK)
include $(abspath $(dir $(lastword $(MAKEFILE_LIST))))/../Makefile
