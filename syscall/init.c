#define _GNU_SOURCE

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <ucontext.h>
#include <sys/ucontext.h>

#include "init.h"

static int g_ncalls;
static void *g_base;

unsigned f(unsigned val, int h, int l) {
	return (val & ((1ul << (h + 1)) - 1)) >> l;
}

int enc2reg(unsigned enc) {
	switch(enc) {
	case 0: return REG_RAX;
	case 1: return REG_RCX;
	case 2: return REG_RDX;
	case 3: return REG_RBX;
	case 4: return REG_RSP;
	case 5: return REG_RBP;
	case 6: return REG_RSI;
	case 7: return REG_RDI;
	default: break;
	}
	abort();
}

void sighnd(int sig, siginfo_t *info, void *ctx) {
	ucontext_t *uc = (ucontext_t *) ctx;
	greg_t *regs = uc->uc_mcontext.gregs;

	uint8_t *ins = (uint8_t *)regs[REG_RIP];
	if (ins[0] != 0x8b) {
		abort();
	}

	uint8_t *next = &ins[2];

	int dst = enc2reg(f(ins[1], 5, 3));

	int rm = f(ins[1], 3, 0);
	if (rm == 4) {
		abort();
	}
	int base = enc2reg(rm);

	int off = 0;
	switch(f(ins[1], 7, 6)) {
	case 0:
		break;
	case 1:
		off = *(int8_t*)next;
		next += 1;
		break;
	case 2:
		off = *(uint32_t *)&next;
		next += 4;
		break;
	default:
		break;
	}

	regs[dst] = 100000 +
		1000 * (regs[base] - (unsigned long)g_base + off) / 4 +
		(++g_ncalls);
	regs[REG_RIP] = (unsigned long)next;
}

void init(void *base) {
	struct sigaction act = {
		.sa_sigaction = sighnd,
		.sa_flags = SA_RESTART,
	};
	sigemptyset(&act.sa_mask);

	if (-1 == sigaction(SIGSEGV, &act, NULL)) {
		perror("signal set failed");
		exit(1);
	}

	g_base = base;
}
