#ifndef _LIBVIG_COMMON_H_INCLUDED_
#define _LIBVIG_COMMON_H_INCLUDED_

#define DS_PATH(num) __attribute__((noinline)) void ds_path_##num()
DS_PATH(1);
DS_PATH(2);
DS_PATH(3);
#undef DS_PATH

#endif //_LIBVIG_COMMON_H_INCLUDED_