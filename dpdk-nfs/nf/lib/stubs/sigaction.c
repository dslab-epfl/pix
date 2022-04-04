#include <stdio.h>
#include <sys/syscall.h>

#include <signal.h>

int __syscall_rt_sigaction (int __signum, const struct sigaction *__act, struct sigaction *__oldact, size_t __size) {
    printf("silently ignoring __syscall_rt_sigaction\n");
    return 0;
    //abort();
}
