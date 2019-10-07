#include <string.h>
#include <limits.h>
#include "sched.h"
#include <stdio.h>
#include <stdlib.h>

enum policy_type
{
    FIFO,
    PRIORITY,
    DEADLINE
} policy;

typedef struct SCHEDULE_TASK {
	void (*entrypoint)(void *aspace);
	int id;
	void *aspace;
	int priority;
	int deadline;
	int endlock_time;
	struct SCHEDULE_TASK *next;
} sched_task;

sched_task *head_of_queue = NULL;
sched_task sched_dict[100];
int time = 0;
int id_counter = 0;
int current_id = -1;

// Return 1, if task1 is more preferable then task2, and 0 otherwise
// deadline > timeout > priority > id
_Bool cmp(sched_task *t1, sched_task* t2) {
	switch (policy) {
	case FIFO:
		return t1->id < t2->id;
	case PRIORITY:
		if (t1->priority < t2->priority)
			return 1;
		if (t1->priority == t2->priority)
			return t1->id < t2->id;
		return 0;
	case DEADLINE:
		if (t1->deadline < t2->deadline)
			return 1;
		if (t1->deadline == t2->deadline) {
			if (t1->endlock_time < t2->endlock_time)
				return 1;
			if (t1->endlock_time == t2->endlock_time) {
				if (t1->priority < t2->priority)
					return 1;
				if (t1->priority == t2->priority)
					return t1->id < t2->id;
			}
		}
		return 0;
	}
}

void insert(sched_task *node) {
	if (head_of_queue == NULL) {
		head_of_queue = node;
		head_of_queue->next = NULL;
		return;
	}
	node->next = NULL;
	sched_task *prev = head_of_queue;
	_Bool was_inserted = 0;
	for (sched_task *current = head_of_queue; current != NULL; current = current->next) {
		if (cmp(node, current)) {
			node->next = current;
			if (prev == head_of_queue)
				head_of_queue = node;
			else
				prev->next = node;
			was_inserted = 1;
			break;
		}
		prev = current;
	}
	if (!was_inserted)
		prev->next = node;
}

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*a))

struct task {
	void (*entry)(void *as);
	void *as;
	int priority;
	int deadline;

	// timeout support
	int waketime;

	// policy support
	struct task *next;
};

static int time;

static struct task *current;
static struct task *runq;
static struct task *waitq;

static int (*policy_cmp)(struct task *t1, struct task *t2);

static struct task taskpool[16];
static int taskpool_n;

static void policy_run(struct task *t) {
	struct task **c = &runq;

	while (*c && policy_cmp(*c, t) <= 0) {
		c = &(*c)->next;
	}
	t->next = *c;
	*c = t;
}

void sched_new(void (*entrypoint)(void *aspace),
		void *aspace,
	       	int priority,
		int deadline) {
	int task_id = id_counter++;
	sched_dict[task_id].id = task_id;
	sched_dict[task_id].aspace = aspace;
	sched_dict[task_id].entrypoint = entrypoint;
	sched_dict[task_id].priority = priority;
	sched_dict[task_id].deadline = (deadline == -1) ? __INT32_MAX__ : deadline;
	sched_dict[task_id].endlock_time = 0;
	sched_task *node = &sched_dict[task_id];
	insert(node);
}

void sched_cont(void (*entrypoint)(void *aspace),
		void *aspace,
		int timeout) {
	if (current_id == -1) exit(1);
	int task_id = current_id;
	current_id = -1;
	sched_dict[task_id].endlock_time = time + timeout;
	sched_task *node = &sched_dict[task_id];
	insert(node);

}

void sched_time_elapsed(unsigned amount) {
	time += amount;
}

void sched_set_policy(const char *name) {
	if (!strcmp(name, "fifo"))
		policy = FIFO;
	else if (!strcmp(name, "priority"))
		policy = PRIORITY;
	else if (!strcmp(name, "deadline"))
		policy = DEADLINE;
	else {
		printf("Wrong policy\n");
		exit(1);
	}
}

void sched_run(void) {
	while (head_of_queue != NULL) {
		sched_task *optimal = NULL;
		if (head_of_queue->endlock_time <= time) {
			optimal = head_of_queue;
			current_id = optimal->id;
			head_of_queue = head_of_queue->next;
		}
		else {
			sched_time_elapsed(1);
			continue;
		}		
		// Run the task
		void (*run_app)(void *) = optimal->entrypoint;
        run_app(optimal->aspace);
	}
}
