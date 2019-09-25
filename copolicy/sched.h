#pragma once

// Set a policy by a name. Expected values
// * fifo: first-in, first-out; run tasks in order of their arrival
// * priority: highest priority task (lowest priority value) should be executed
//             first. Use round-robin for processes with same priority
//             (task from 1st process, from 2nd, ... Nth, 1st, 2nd, ...)
// * deadline: consider deadline, execute process with earliest deadline first.
//             Fallback to priority policy if deadlines are equal
//
extern void sched_set_policy(const char *name);

// Add new task
extern void sched_new(void (*entrypoint)(void *aspace), // entrypoint function
		void *aspace, // addresses the process can access
	       	int priority, // priority, [0 - 10], smaller means it have more priority
		int deadline); // absolute time till the task should be completed, negative means no deadline

// Continue process from function after some amount of time
extern void sched_cont(void (*entrypoint)(void *aspace), // entrypoint function
		void *aspace,// addresses the process can access
		int timeout); // when the continuation became runnable

// Notify scheduler that some amount of time passed
extern void sched_time_elapsed(unsigned amount);

// Scheduler loop, start executing tasks until all of them finish
extern void sched_run(void);

