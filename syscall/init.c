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

int global_calls_counter = 0;

/**
 * Handler for segfault signal
 * 
 * That solution works only if array index is less than 256 
 * (can be stored in one byte). Full solution is much more
 * complicated, because it should consider how many bytes 
 * that index occupies.
 */
void handler(int sig, siginfo_t *info, void *ctx)
{
	// Get current context
	ucontext_t *uc = (ucontext_t *)ctx;

	// Extract next command from RIP
	uint64_t command = *(uint64_t *)uc->uc_mcontext.gregs[REG_RIP];
	// Extract offset from b4 to b0
	uint64_t base_offset = ((uint64_t)uc->uc_mcontext.gregs[REG_RBP] -
							(uint64_t)uc->uc_mcontext.gregs[REG_RBX]) >> 2;

	// Parse next command
	uint8_t flag_byte = command & 0xff;
	if (flag_byte != 0x8b)
		return;
	uint8_t info_byte = (command >> 8) & 0xff;

	// Extract required bits
	char has_offset_bit = (info_byte >> 6) & 1;
	char is_second_arg_bit = (info_byte >> 3) & 1;
	char is_b0_bit = (info_byte >> 1) & 1;
 
	// If our value is the second argument in printf(),
	// register to place output data should be changed
	#define OUTPUT_REG (is_second_arg_bit ? REG_RCX : REG_RDX)
	// Extract array index
	uint64_t index = has_offset_bit ? ((command >> 16) & 0xff) >> 2 : 0;
	// Increase index by base_offset if b4 called instead b0
	index += is_b0_bit ? 0 : base_offset;

	uc->uc_mcontext.gregs[OUTPUT_REG] = 100000 + 1000 * index + (++global_calls_counter);
	// Change RIP for correct continuation
	uc->uc_mcontext.gregs[REG_RIP] += 2 + has_offset_bit;
}

void init(void *base)
{
	struct sigaction act;

	act.sa_sigaction = handler,
	act.sa_flags = SA_RESTART,

	sigemptyset(&act.sa_mask);

	if (-1 == sigaction(SIGSEGV, &act, NULL))
	{
		perror("signal set failed");
		exit(1);
	}
}
