#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "sched.h"

struct app1_aspace {
	int id;
	int cnt;
};

void app1(void *aspace) {
	struct app1_aspace *as = aspace;
	--as->cnt;
	printf("%s id %d cnt %d\n", __func__, as->id, as->cnt);

	if (as->cnt % 2) {
		sched_time_elapsed(1);
	}

	if (0 < as->cnt) {
		sched_cont(app1, aspace, 1);
	}
}

void rtapp(void *aspace) {
	struct app1_aspace *as = aspace;
	--as->cnt;
	printf("%s id %d cnt %d\n", __func__, as->id, as->cnt);

	sched_time_elapsed(1);

	if (0 < as->cnt) {
		sched_cont(rtapp, aspace, 0);
	}
}


int main(int argc, char *argv[]) {
	char name[64];
	int prio;
	int deadline;

	struct app1_aspace a1as[16];
	int a1as_n = 0;

	scanf("%s", name);
	sched_set_policy(name);

	while (EOF != scanf("%s %d %d", name, &prio, &deadline)) {
		struct app1_aspace *as = &a1as[a1as_n++];
		scanf("%d", &as->cnt);
		as->id = as - a1as;
		void (*entry)(void*);

		if (!strcmp("app1", name)) {
			entry = app1;
		} else if (!strcmp("rtapp", name)) {
			entry = rtapp;
		} else {
			fprintf(stderr, "Unknown app: %s\n", name);
			return 1;
		}
		sched_new(entry, as, prio, deadline);
	}

	sched_run();
	return 0;
}
