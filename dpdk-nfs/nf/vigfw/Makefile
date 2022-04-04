# Binary name
APP := fw

# Runtime and verification files
NF_FILES := fw.c fw_config.c \
            fw-flow.c fw-state.c fw-flowmanager.c

# Verification files
# NF_VERIF_FILES := 

# Verification arguments
NF_VERIF_ARGS := --lan-dev 0 \
                 --wan 1 \
                 --expire 60000000000 \
                 --capacity 65536 \
                 --eth-dest 0,01:23:45:67:89:00 \
                 --eth-dest 1,01:23:45:67:89:01

# Object files to link in. TODO: Autogenerate from NF_FILES
NF_EXECUTABLE_OBJ_FILES := fw.o fw_config.o \
            fw-flow.o fw-state.o fw-flowmanager.o

# Include parent (in a convoluted way cause of DPDK)
include $(abspath $(dir $(lastword $(MAKEFILE_LIST))))/../Makefile
