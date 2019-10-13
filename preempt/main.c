#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include "sched.h"

static void *alloc(void *pool, int poolsz, void **next, int sz) {
	void *a = *next;
	*next = (char*)*next + sz;
	return a;
}
#define ALLOC(pool, next) \
	alloc(pool, sizeof(pool), &next, sizeof(*pool))

struct app1_aspace {
	int id;
	int sleep;
	int busy;
	int start;
};

static long long reftime(void) {
	struct timespec ts;
	clock_gettime(CLOCK_BOOTTIME, &ts);
	return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

static void print(struct app1_aspace *as, const char *msg) {
	printf("app1 id %d %s time %d reference %lld\n", as->id, msg, sched_gettime(), reftime() - as->start);
}

void app1(void *aspace) {
	struct app1_aspace *as = aspace;
	as->start = reftime();

	while (true)  {
		print(as, "sleep");
		sched_sleep(as->sleep);
		print(as, "busy1");
		for (volatile int i = 100000 * as->busy; 0 < i; --i) {
		}
		print(as, "busy2");
	}
}

int main(int argc, char *argv[]) {
	char name[64];
	int prio;

	printf("reftime %lld\n", reftime());

	struct app1_aspace a1pool[16];
	void *a1next = a1pool;

	while (EOF != scanf("%s %d", name, &prio)) {
		void *asp;
		if (!strcmp("app1", name)) {
			int sleep;
			int busy;
			scanf("%d %d", &sleep, &busy);

			struct app1_aspace *as = ALLOC(a1pool, a1next);
			as->id = as - a1pool;

			as->sleep = sleep;
			as->busy = busy;

			sched_new(app1, as, prio);
		} else {
			fprintf(stderr, "Unknown app: %s\n", name);
			return 1;
		}
	}

	sched_run(100);
	return 0;
}
