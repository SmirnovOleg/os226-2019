#pragma once

extern int sem_rw_alloc(void);

extern void sem_read_acq(int id);

extern void sem_read_rel(int id);

extern void sem_write_acq(int id);

extern void sem_write_rel(int id);
