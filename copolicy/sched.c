
#include "sched.h"

void sched_new(void (*entrypoint)(void *aspace),
		void *aspace,
	       	int priority,
		int deadline) {
}

void sched_cont(void (*entrypoint)(void *aspace),
		void *aspace,
		int timeout) {
}

void sched_time_elapsed(unsigned amount) {
}

void sched_set_policy(const char *name) {
}

void sched_run(void) {
}

