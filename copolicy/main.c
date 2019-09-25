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

	if (as->cnt % 1) {
		sched_time_elapsed(1);
	}

	if (0 < as->cnt) {
		sched_cont(app1, aspace, 1);
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
		if (!strcmp(name, "app1")) {
			struct app1_aspace *as = &a1as[a1as_n++];
			scanf("%d", &as->cnt);
			as->id = as - a1as;
			sched_new(app1, as, prio, deadline);

		}
	}

	sched_run();
	return 0;
}
