#pragma once

extern int timer_cnt(void);

extern void timer_init_period(int ms, void (*hnd)(int sig));


