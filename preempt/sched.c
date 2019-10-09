#define _GNU_SOURCE

#include <stdbool.h>
#include <stdio.h>

#include "timer.h"
#include "sched.h"
#include "ctx.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*a))

/* AMD64 Sys V ABI, 3.2.2 The Stack Frame:
The 128-byte area beyond the location pointed to by %rsp is considered to
be reserved and shall not be modified by signal or interrupt handlers */
#define SYSV_REDST_SZ 128

extern void tramptramp(void);

static void bottom(void) {
	fprintf(stderr, "%s\n", __func__);
}

static void hctx_push(greg_t *regs, unsigned long val) {
	regs[REG_RSP] -= sizeof(unsigned long);
	*(unsigned long *) regs[REG_RSP] = val;
}

static void top(int sig, siginfo_t *info, void *ctx) {
	fprintf(stderr, "%s\n", __func__);

	ucontext_t *uc = (ucontext_t *) ctx;
	greg_t *regs = uc->uc_mcontext.gregs;

	unsigned long oldsp = regs[REG_RSP];
	regs[REG_RSP] -= SYSV_REDST_SZ;
	hctx_push(regs, regs[REG_RIP]);
	hctx_push(regs, sig);
	hctx_push(regs, regs[REG_RBP]);
	hctx_push(regs, oldsp);
	hctx_push(regs, (unsigned long) bottom);
	regs[REG_RIP] = (greg_t) tramptramp;
}

void sched_new(void (*entrypoint)(void *aspace),
		void *aspace,
	       	int priority) {
}

void sched_cont(void (*entrypoint)(void *aspace),
		void *aspace,
		int timeout) {
}

void sched_sleep(unsigned ms) {
}

int sched_gettime(void) {
	return 0;
}

static int delay = 100000000;

static struct ctx mainctx;
static char stack[8192];
static struct ctx octx;

static void other(void) {
	printf("hello from other\n");
	ctx_switch(&octx, &mainctx);

	while (true) {
		for (volatile int i = delay; 0 < i; --i) {
		}
		printf("hello from other\n");
	}
}

void sched_run(int period_ms) {
	timer_init_period(period_ms, top);

	ctx_make(&octx, other, stack, sizeof(stack));
	ctx_switch(&mainctx, &octx);

	while (true) {
		for (volatile int i = delay; 0 < i; --i) {
		}
		printf("hello from main\n");
	}
}
