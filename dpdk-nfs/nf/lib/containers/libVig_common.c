#include "libVig_common.h"

/* Including some path determination functions in here */
#define DEF_DS_PATH(num)                                                       \
  __attribute__((noinline)) void ds_path_##num() {}
DEF_DS_PATH(1);
DEF_DS_PATH(2);
DEF_DS_PATH(3);
#undef DEF_DS_PATH