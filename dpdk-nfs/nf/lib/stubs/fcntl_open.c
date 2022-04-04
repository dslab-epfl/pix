#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>


int __libc_fcntl(int fd, int cmd, ...)
{
	long arg;
	va_list list;

	va_start(list, cmd);
	arg = va_arg(list, long);
	va_end(list);

//	return (__syscall_fcntl(fd, cmd, arg));
        return fcntl(fd, cmd, arg);
}

int __libc_open(const char *file, int oflag, ...)
{
	mode_t mode = 0;

	if (oflag & O_CREAT) {
		va_list arg;
		va_start (arg, oflag);
		mode = va_arg (arg, mode_t);
		va_end (arg);
	}

//	return __syscall_open(file, oflag, mode);
        return open(file, oflag, mode);
}
