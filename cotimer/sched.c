#include <stdbool.h>
#include <stdio.h>
#include <signal.h>

#include "timer.h"
#include "sched.h"

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

static volatile int time;
static int tick_period;

static struct task *current;
static struct task *runq;
static struct task *waitq;

static struct task taskpool[16];
static int taskpool_n;

static sigset_t irqs;

static void irq_disable(void) {
	sigprocmask(SIG_BLOCK, &irqs, NULL);
}

static void irq_enable(void) {
	sigprocmask(SIG_UNBLOCK, &irqs, NULL);
}

static int prio_cmp(struct task *t1, struct task *t2) {
	return t1->priority - t2->priority;
}

static void policy_run(struct task *t) {
	struct task **c = &runq;

	while (*c && prio_cmp(*c, t) <= 0) {
		c = &(*c)->next;
	}
	t->next = *c;
	*c = t;
}

void sched_new(void (*entrypoint)(void *aspace),
		void *aspace,
	       	int priority) {

	if (ARRAY_SIZE(taskpool) <= taskpool_n) {
		fprintf(stderr, "No mem for new task\n");
		return;
	}
	struct task *t = &taskpool[taskpool_n++];

	t->entry = entrypoint;
	t->as = aspace;
	t->priority = priority;

	irq_disable();
	policy_run(t);
	irq_enable();
}

void sched_cont(void (*entrypoint)(void *aspace),
		void *aspace,
		int timeout) {

	if (current->next != current) {
		fprintf(stderr, "Mulitiple sched_cont\n");
		return;
	}

	if (!timeout) {
		irq_disable();
		policy_run(current);
		irq_enable();
		return;
	}

	current->waketime = sched_gettime() + timeout;

	irq_disable();
	struct task **c = &waitq;
	while (*c && (*c)->waketime < current->waketime) {
		c = &(*c)->next;
	}
	current->next = *c;
	*c = current;
	irq_enable();
}

void sched_sleep(unsigned ms) {
	int endtime = sched_gettime() + ms;
	int curtime;
	while ((curtime = sched_gettime()) <= endtime) {
		// FIXME an extra tick_period of busy wait could be reduced
		if (2 * tick_period <= endtime - curtime) {
			pause();
		}
	}
}

int sched_gettime(void) {
	int cnt1 = timer_cnt();
	int time1 = time;
	int cnt2 = timer_cnt();
	int time2 = time;

	return (cnt1 <= cnt2) ?
		time1 + cnt2 :
		time2 + cnt2;
}

static void sigalrm(int sig) {
	time += tick_period;

	while (waitq && waitq->waketime <= time) {
		struct task *t = waitq;
		waitq = waitq->next;
		policy_run(t);
	}
}

void sched_run(int period_ms) {
	sigemptyset(&irqs);
	sigaddset(&irqs, SIGALRM);

	tick_period = period_ms;

	timer_init_period(period_ms, sigalrm);

	sigset_t none;
	sigemptyset(&none);

	irq_disable();

	while (runq || waitq) {
		if (runq) {
			current = runq;
			runq = current->next;
			current->next = current;

			irq_enable();
			current->entry(current->as);
			irq_disable();
		} else {
			sigsuspend(&none);
		}
	}

	irq_enable();
}
