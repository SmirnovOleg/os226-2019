#pragma once

// Add new task
extern void sched_new(void (*entrypoint)(void *aspace), // entrypoint function
		void *aspace, // addresses the process can access
	       	int priority); // priority, [0 - 10], smaller means it have more priority

// Continue process from function after some amount of time
extern void sched_cont(void (*entrypoint)(void *aspace), // entrypoint function
		void *aspace,// addresses the process can access
		int timeout); // when the continuation became runnable (in ms)

// Wait for specific amount of millliseconds (do some work);
extern void sched_sleep(unsigned ms);

extern int sched_gettime(void);

extern void sched_passed(int ms);

// Scheduler loop, start executing tasks until all of them finish
extern void sched_run(int period_ms);

extern int sched_get_mutex(void);

extern void sched_acq(int mid);

extern void sched_rel(int mid);
