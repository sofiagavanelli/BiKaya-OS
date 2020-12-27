#ifndef SYSCALL_H
#define SYSCALL_H

#include "handler.h"

/* handler per la gestione delle system call */
void syscall_H(void);

void resume(void);

void get_cputime(unsigned int *user, unsigned int *kernel, unsigned int *wallclock);

void create_process(state_t* statep, int p_value, void **cpid);

void term_process(void *pid);

void delete_ric(pcb_t* p);

void verhogen(int *semaddr);

void passeren(int *semaddr);

void waitio(unsigned int instruction, unsigned int* dev_reg, int subdevice);

void specpassup(int type, state_t* old, state_t* new);

void getpid(void **pid, void **ppid);

#endif
