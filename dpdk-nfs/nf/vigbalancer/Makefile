# Binary name
APP := lb

# Runtime and verification files
NF_FILES := lb_main.c lb_balancer.c lb_config.c lb_data.c

# Verification files
NF_VERIF_FILES := lb_loop.c

## Verification definitions
## Need 3 devices otherwise the concept of a load-balancer doesn't make sense
#NF_VERIF_DEFS := -DSTUB_DEVICES_COUNT=3
# ^^^ No need for 3 interfaces. We differentiate backends using their IP address.
NF_VERIF_DEFS := -DSTUB_DEVICES_COUNT=2

# Verification arguments
NF_VERIF_ARGS := --flow-expiration 10 \
                 --flow-capacity 65536 \
                 --backend-capacity 20 \
                 --cht-height 29 \
                 --backend-expiration 60000000000

# Object files to link in. TODO: Autogenerate from NF_FILES
NF_EXECUTABLE_OBJ_FILES := lb_main.o lb_balancer.o \
            lb_config.o lb_data.o lb_loop.o

# Include parent (in a convoluted way cause of DPDK)
include $(abspath $(dir $(lastword $(MAKEFILE_LIST))))/../Makefile
