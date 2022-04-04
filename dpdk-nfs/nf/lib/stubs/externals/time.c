#include <time.h>
#include <string.h>

int
clock_gettime(clockid_t clk_id, struct timespec* tp)
{
	// Not supported!
	return -1;
}

// Quick hack for router
int
gettimeofday(struct timeval* tv, void* tz)
{
	memset(tv, 0, sizeof(struct timeval));
	return 0;
}

