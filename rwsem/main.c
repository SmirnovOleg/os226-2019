#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>

#include "sched.h"
#include "sem.h"

struct app_aspace {
	int *reads;
	int *writes;
};

static struct app_aspace apool[16];
static void *anext = apool;

static int secidpool[128];
static void *secidnext = secidpool;

static void *alloc(void *pool, int poolsz, void **next, int sz) {
	void *a = *next;
	*next = (char*)*next + sz;
	return a;
}
#define ALLOC(pool, next) \
	alloc(pool, sizeof(pool), &next, sizeof(*pool))

void app(void *aspace) {
	struct app_aspace *as = aspace;

	while (true)  {
		for (int *r = as->reads; *r != 0; ++r) {
			sem_read_acq(*r);
		}

		for (int *w = as->writes; *w != 0; ++w) {
			sem_write_acq(*w);
		}

		printf("%d in (read", as - apool);
		for (int *r = as->reads; *r != 0; ++r) {
			printf(" %d", *r);
		}
		printf(" write");
		for (int *w = as->writes; *w != 0; ++w) {
			printf(" %d", *w);
		}
		printf(")\n");

		const int process_n = (int*)secidnext - secidpool;
		for (int i = 0; i <= random() % (process_n + 1) ; ++i) {
			sched_sleep(0);
		}

		printf("%d out\n", as - apool);

		for (int *w = as->writes; *w != 0; ++w) {
			sem_write_rel(*w);
		}

		for (int *r = as->reads; *r != 0; ++r) {
			sem_read_rel(*r);
		}
	}
}

int main(int argc, char *argv[]) {
	unsigned seed;
	scanf("%u", &seed);
	srandom(seed);

	struct app_aspace *as = ALLOC(apool, anext);
	as->reads = secidpool;
	as->writes = NULL;

	int sec_n = 0;
	int s;
	while (EOF != scanf("%d", &s)) {
		if (sec_n < s) {
			sec_n = s;
		}

		int *sec = ALLOC(secidpool, secidnext);
		*sec = s;

		if (s == 0) {
		       	if (as->writes == NULL) {
				as->writes = (int *)secidnext;
			} else {
				sched_new(app, as, 0);
				as = ALLOC(apool, anext);
				as->reads = (int *)secidnext;
				as->writes = NULL;
			}
		}
	}

	for (int i = 0; i <= sec_n; ++i) {
		sem_rw_alloc();
	}

	sched_run(100);
	return 0;
}
