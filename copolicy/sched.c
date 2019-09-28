#include <string.h>
#include "sched.h"
#include <stdio.h>
#include <stdlib.h>

enum policy_type
{
    FIFO,
    PRIORITY,
    DEADLINE
} policy;

typedef struct TASK_INFO {
	int id;
	int cnt;
} task_info;

typedef struct SCHEDULE_TASK {
	void (*entrypoint)(void *aspace);
	task_info *aspace;
	int priority;
	int deadline;

	int endlock_time;
	struct SCHEDULE_TASK *next;
} sched_task;

sched_task *head_of_queue = NULL;
sched_task sched_dict[100];
int time = 0;

// Return 1, if task1 is more preferable then task2, and 0 otherwise
// deadline > timeout > priority > id
_Bool cmp(sched_task *t1, sched_task* t2) {
	switch (policy) {
	case FIFO:
		return t1->aspace->id < t2->aspace->id;
	case PRIORITY:
		if (t1->priority < t2->priority)
			return 1;
		if (t1->priority == t2->priority)
			return t1->aspace->id < t2->aspace->id;
		return 0;
	case DEADLINE:
		if (t1->deadline == -1 && t2->deadline != -1) 
			return 0;
		if (t1->deadline < t2->deadline || t1->deadline != -1 && t2->deadline == -1)
			return 1;
		if (t1->deadline == t1->deadline) {
			if (t1->endlock_time < t2->endlock_time)
				return 1;
			if (t1->endlock_time == t2->endlock_time) {
				if (t1->priority < t2->priority)
					return 1;
				if (t1->priority == t2->priority)
					return t1->aspace->id < t2->aspace->id;
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
		//printf("cur %d", current->aspace->id);
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

void sched_new(void (*entrypoint)(void *aspace),
		void *aspace,
	       	int priority,
		int deadline) {
	int task_id = ((task_info *) aspace)->id;
	sched_dict[task_id].aspace = (task_info *) aspace;
	sched_dict[task_id].entrypoint = entrypoint;
	sched_dict[task_id].priority = priority;
	sched_dict[task_id].deadline = deadline;
	sched_dict[task_id].endlock_time = 0;
	// Push to the "priority queue" with tasks which are ready to work
	sched_task *node = &sched_dict[task_id];
	insert(node);
}

void sched_cont(void (*entrypoint)(void *aspace),
		void *aspace,
		int timeout) {
	int task_id = ((task_info *) aspace)->id;
	sched_dict[task_id].endlock_time = time + timeout;
	// Push to the timeout "priority queue"
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
			head_of_queue = head_of_queue->next;
		}
		else {
			time++;
			continue;
		}		
		// Run the task
		void (*run_app)(void *) = optimal->entrypoint;
        run_app(optimal->aspace);
	}
}

