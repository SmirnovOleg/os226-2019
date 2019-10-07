#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

#include "timer.h"

static struct timeval initv;

int timer_cnt(void) {
	struct itimerval it;
	getitimer(ITIMER_REAL, &it);
	return 1000 * (initv.tv_sec - it.it_value.tv_sec)
		+ (initv.tv_usec - it.it_value.tv_usec) / 1000;
}

void timer_init_period(int ms, void (*hnd)(int sig)) {
	initv.tv_sec  = ms / 1000;
	initv.tv_usec = ms * 1000;

	const struct itimerval setup_it = {
		.it_value    = initv,
		.it_interval = initv,
	};

	if (-1 == setitimer(ITIMER_REAL, &setup_it, NULL)) {
		perror("setitimer");
	}

	if (SIG_ERR == signal(SIGALRM, hnd)) {
		perror("signal");
	}
}


