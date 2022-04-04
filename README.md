# Performance Interface eXtractor (PIX)

This repository hosts PIX, a tool to automatically extract performance interfaces from NF code. 
PIX was published at NSDI'22 ([paper](https://dslab.epfl.ch/pubs/pix.pdf)). 


# Organization

Subdirectories have their own README files.

* `dpdk-nfs` - contains the set of NFs written using Intel's DPDK that we extracted performance interfaces for. 
* `ebpf-nfs` - contains the set of NFs written using the eBPF XDP framework that we extracted performance interfaces for. 
* `install.sh` - script to install all of PIX's dependencies 
* `install` - patches and config files for PIX's dependencies

# Getting Started

The `install.sh` script installs all of PIX's dependencies.
Run `./pix/install.sh` from the directory containing PIX. 

To list all the dependencies, run `./pix/install.sh --help`

To extract performance interfaces for individual NFs, please refer to the READMEs within the respective subdirectories. 

# Other information

- All code in this repository was tested on Linux kernel versions <= 4.15.
- To run the DPDK NFs (but not necessarily to extract performance innterfaces), you need to set up hugepages for DPDK; see the [DPDK documentation](https://doc.dpdk.org/guides/linux_gsg/sys_reqs.html#linux-gsg-hugepages).
- At several points in the code, `pix` will be referred to as `bolt`. This is because internally at DSLAB, `pix` was a successor to the `bolt` tool that extracted performance contracts from NF code ([bolt source](https://github.com/bolt-perf-contracts/bolthttps://github.com/bolt-perf-contracts/bolt))

