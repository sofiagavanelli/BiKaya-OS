#ifndef HANDLER_H
#define HANDLER_H

#include "scheduler.h"
#include "const_bikaya.h"

#ifdef TARGET_UMPS
#define pc pc_epc

#define INTEXCEPTION 0
#define SYSEXCEPTION 8
#define BPEXCEPTION  9

#define SYS_OLD_AREA 0x20000348
#define INT_OLD_AREA 0x20000000
#define TRAP_OLD_AREA 0x20000230
#define TLB_OLD_AREA 0x20000118
#endif

#ifdef TARGET_UARM

#define reg_a0 a1
#define reg_a1 a2
#define reg_a2 a3
#define reg_a3 a4
#define reg_v0 a1

#define SYS_OLD_AREA SYSBK_OLDAREA
#define INT_OLD_AREA INT_OLDAREA
#define TRAP_OLD_AREA PGMTRAP_OLDAREA
#define TLB_OLD_AREA TLB_OLDAREA
#endif

#define WORD_SIZE 4


/* funzioni per l'aumento/decremento del pc dello stato preso in input di una word */
void pc_increase(state_t* state);
void pc_decrease(state_t* state);

unsigned int getExcCode(state_t* status);
void memCopy(state_t* status, state_t* oldArea);

/* handler per la gestione di trap e tlb */
void trap_H(void);
void tlb_H(void);

#endif
