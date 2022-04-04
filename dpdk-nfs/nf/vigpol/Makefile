# Binary name
APP := pol

# Runtime and verification files
NF_FILES := policer_main.c policer_config.c policer_flow.c \
            policer_state.c 

# Verification files
# NF_VERIF_FILES := 

# Verification arguments
NF_VERIF_ARGS := --lan-dev 1 \
                 --wan 0 \
								 --rate 100000000 \
								 --burst 100000 \
                 --capacity 65536 \
                 --eth-dest 0,01:23:45:67:89:00 \
                 --eth-dest 1,01:23:45:67:89:01

# Object files to link in. TODO: Autogenerate from NF_FILES
NF_EXECUTABLE_OBJ_FILES := policer_main.o policer_config.o policer_flow.o \
            policer_state.o 

# Include parent (in a convoluted way cause of DPDK)
include $(abspath $(dir $(lastword $(MAKEFILE_LIST))))/../Makefile
