# get current dir
# see https://stackoverflow.com/a/8080530
SELF_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))

SRCS_DEP :=	$(SELF_DIR)/dpdk-contracts.cpp \
						$(SELF_DIR)/helper-contracts.cpp \
						$(SELF_DIR)/vector-contracts.cpp \
						$(SELF_DIR)/ip-opt-contracts.cpp \
						$(SELF_DIR)/dpdk-lpm-contracts.cpp \
						$(SELF_DIR)/expirator-contracts.cpp \
						$(SELF_DIR)/cht-contracts.cpp \
						$(SELF_DIR)/natasha-contracts.cpp \
						$(SELF_DIR)/bpf-map-contracts.cpp \
						

# MAP CONTRACT- Pick one of the following

# Map 1: map, dmap from VigNAT
SRCS_MAP := $(SELF_DIR)/map-impl-contracts.cpp \
						$(SELF_DIR)/map-contracts.cpp \

# Map 2: Rehashing map, dmap
#SRCS_MAP := $(SELF_DIR)/rehashing-map-impl-contracts.cpp \
						$(SELF_DIR)/rehashing-map-contracts.cpp \
						$(SELF_DIR)/dmap-contracts.cpp \

# Map 3: Predictable map, dmap

# DCHAIN CONTRACT - Pick one of the following

# Dchain 1: Double chain from VigNAT
SRCS_DCHAIN := $(SELF_DIR)/dchain-contracts.cpp
# Dchain 2: Altchain with faster de-allocation, slower allocation
#SRCS_DCHAIN := $(SELF_DIR)/alt-chain-contracts.cpp


SRCS_DEP += $(SRCS_MAP)
SRCS_DEP += $(SRCS_DCHAIN)

				
SRCS_MAIN := $(SELF_DIR)/perf-contracts.cpp 

#Compilation
COMPILE_COMMAND := g++
#Compiler flags
CXXFLAGS = -std=c++11 -Wall -g -I$(KLEE_INCLUDE) -I$(SELF_DIR)
#Compiler flags for different metrics
ifeq ($(LLVM),TRUE)
CXXFLAGS+= -DMETRICS_LLVM
else
CXXFLAGS+= -DMETRICS_X86
endif

#Linked libraries
LDLIBS = -ldl


default: perf-contracts.so test-dl

clean:
	rm -f *.so test-dl *.o

perf-contracts.so: $(SRCS_DEP) $(SRCS_MAIN)
	$(COMPILE_COMMAND) $(CXXFLAGS) -c -fPIC $(SRCS_DEP) 
	$(COMPILE_COMMAND) $(CXXFLAGS) -shared -fPIC $(SRCS_MAIN) *.o -o $@

test-dl: test-dl.cpp
	$(COMPILE_COMMAND) $(CXXFLAGS) $< -o $@ $(LDLIBS)

# Flag for contracts with alternate data structures
#CXXFLAGS += -DALT_CHAIN
#CXXFLAGS += -DREHASHING_MAP
#CXXFLAGS += -DPRED_DS
