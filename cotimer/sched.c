#include <stdbool.h>
#include <stdio.h>

#include "timer.h"
#include "sched.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*a))

static void sigalrm(int sig) {
	fprintf(stderr, "%s\n", __func__);
}

void sched_new(void (*entrypoint)(void *aspace),
		void *aspace,
	       	int priority) {
}

void sched_cont(void (*entrypoint)(void *aspace),
		void *aspace,
		int timeout) {
}

void sched_sleep(unsigned ms) {
}

int sched_gettime(void) {
	return 0;
}

void sched_run(int period_ms) {
	timer_init_period(period_ms, sigalrm);
	for (int i = 0; i < 10; ++i) {
		fprintf(stderr, "timer cnt %d\n", timer_cnt());
	}
}
