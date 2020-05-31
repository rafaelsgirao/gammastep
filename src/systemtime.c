// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2010-2014  Jon Lund Steffensen <jonlst@gmail.com>
// systemtime.c -- Portable system time source

#include <stdio.h>
#include <unistd.h>

#if _POSIX_TIMERS > 0
# include <time.h>
#else
# include <time.h>
# include <sys/time.h>
#endif

#include "systemtime.h"


/* Return current time in T as the number of seconds since the epoch. */
int
systemtime_get_time(double *t)
{
#if _POSIX_TIMERS > 0 /* POSIX timers */
	struct timespec now;
	int r = clock_gettime(CLOCK_REALTIME, &now);
	if (r < 0) {
		perror("clock_gettime");
		return -1;
	}

	*t = now.tv_sec + (now.tv_nsec / 1000000000.0);
#else /* other platforms */
	struct timeval now;
	int r = gettimeofday(&now, NULL);
	if (r < 0) {
		perror("gettimeofday");
		return -1;
	}

	*t = now.tv_sec + (now.tv_usec / 1000000.0);
#endif

	return 0;
}

/* Sleep for a number of milliseconds. */
void
systemtime_msleep(unsigned int msecs)
{
	struct timespec sleep;
	sleep.tv_sec = msecs / 1000;
	sleep.tv_nsec = (msecs % 1000)*1000000;
	nanosleep(&sleep, NULL);
}
