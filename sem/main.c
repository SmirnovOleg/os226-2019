#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>

#include "sched.h"

int incnt;

static void *alloc(void *pool, int poolsz, void **next, int sz) {
	void *a = *next;
	*next = (char*)*next + sz;
	return a;
}
#define ALLOC(pool, next) \
	alloc(pool, sizeof(pool), &next, sizeof(*pool))

struct app1_aspace {
	int id;
	int start;
	int mid;
};

static long long reftime(void) {
	struct timespec ts;
	clock_gettime(CLOCK_BOOTTIME, &ts);
	return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

static void print(struct app1_aspace *as) {
	printf("app1 id %d incnt %d reference %lld\n", as->id, incnt, reftime() - as->start);
	fflush(stdout);
}

void app1(void *aspace) {
	struct app1_aspace *as = aspace;

	as->start = reftime();

	while (true)  {
		sched_acq(as->mid);
		++incnt;
		sched_sleep(random() % 1000);
		print(as);
		--incnt;
		sched_rel(as->mid);

		sched_acq(as->mid);
		++incnt;
		for (volatile int i = 100000 * (random() % 1000); 0 < i; --i) {
		}
		print(as);
		--incnt;
		sched_rel(as->mid);
	}
}

int main(int argc, char *argv[]) {
	char name[64];
	int prio;

	unsigned seed;
	scanf("%u ", &seed);
	srandom(seed);

	struct app1_aspace a1pool[16];
	void *a1next = a1pool;

	int mid = sched_get_mutex();

	while (EOF != scanf("%s %d", name, &prio)) {
		void *asp;
		if (!strcmp("app1", name)) {
			struct app1_aspace *as = ALLOC(a1pool, a1next);
			as->id = as - a1pool;

			as->mid = mid;

			sched_new(app1, as, prio);
		} else {
			fprintf(stderr, "Unknown app: %s\n", name);
			return 1;
		}
	}

	sched_run(100);
	return 0;
}
