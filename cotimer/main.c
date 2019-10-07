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
	int duration;
	int period;
	int cnt;
};

static long long reftime(void) {
	struct timespec ts;
	clock_gettime(CLOCK_BOOTTIME, &ts);
	return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

void app1(void *aspace) {
	struct app1_aspace *as = aspace;
	--as->cnt;

	printf("%s id %d cnt %d time1 %d reference %lld\n", __func__, as->id, as->cnt, sched_gettime(), reftime());

	sched_sleep(as->duration);

	printf("%s id %d cnt %d time2 %d reference %lld\n", __func__, as->id, as->cnt, sched_gettime(), reftime());

	if (0 < as->cnt) {
		sched_cont(app1, aspace, as->period);
	}
}

struct work {
	int duration;
	int delay_cont;
};
struct aireg_space {
	int id;
	struct work *work;
};
void aireg(void *aspace) {
	struct aireg_space *as = aspace;
	printf("%s id %d cnt %d time1 %d reference %lld\n", __func__, as->id, sched_gettime(), reftime());

	sched_sleep(as->work->duration);

	printf("%s id %d cnt %d time2 %d reference %lld\n", __func__, as->id, sched_gettime(), reftime());

	if (0 <= as->work->delay_cont) {
		sched_cont(app1, aspace, as->work->delay_cont);
		++as->work;
	}
}

int main(int argc, char *argv[]) {
	char name[64];
	int prio;

	printf("reftime %lld\n", reftime());

	struct app1_aspace a1pool[16];
	void *a1next = a1pool;

	struct work workpool[128];
	void *worknext = workpool;
	struct aireg_space airpool[16];
	void *airnext = airpool;

	while (EOF != scanf("%s %d", name, &prio)) {
		void *asp;
		if (!strcmp("app1", name)) {
			int duration;
			int period;
			int cnt;
			scanf("%d %d %d", &duration, &period, &cnt);

			struct app1_aspace *as = ALLOC(a1pool, a1next);
			as->id = as - a1pool;

			as->duration = duration;
			as->period = period;
			as->cnt = cnt;

			sched_new(app1, as, prio);
		} else if (!strcmp("aireg", name)) {
			struct aireg_space *as = ALLOC(airpool, airnext);
			as->id = as - airpool;
			as->work = worknext;

			struct work *w;
			do {
				w = ALLOC(workpool, worknext);
				scanf("%d %d", &w->duration, &w->delay_cont);
			} while (w->delay_cont != -1);

			sched_new(app1, as, prio);
		} else {
			fprintf(stderr, "Unknown app: %s\n", name);
			return 1;
		}
	}

	sched_run(100);
	return 0;
}
