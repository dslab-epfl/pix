# Organization

* `vignat, bridge, vigfw, vigbalancer, vigpolicer` - DPDK NFs taken from the [Vigor project](https://vigor-nf.github.io/).
* `lib` - source code for the common data structures used by all the above NFs. `lib/stubs` contains the symbolic models for each data structure. 


# Extracting python performance interfaces

To extract a performance interface for any of the 5 NFs, run the `test-pix.sh` script from within the corresponding directory. 

For example:

```bash
# Extracting performance interface for VigNAT
$ cd vignat
$ bash ../test-pix.sh
```

This should produce a set of files named `res-tree-*.py` where `*` refers to the value of the performance resolution. 