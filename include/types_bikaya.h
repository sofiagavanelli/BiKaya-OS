#ifndef TYPES_BIKAYA_H_INCLUDED
#define TYPES_BIKAYA_H_INCLUDED

#ifdef TARGET_UMPS
#include "umps/libumps.h"
#include "umps/arch.h"
#include "umps/types.h"
#endif
#ifdef TARGET_UARM
#include "uarm/libuarm.h"
#include "uarm/arch.h"
#include "uarm/uARMtypes.h"
#include "uarm/uARMconst.h"
#endif

#include "const.h"
#include "listx.h"

typedef unsigned int memaddr;

/* Process Control Block (PCB) data structure */
typedef struct pcb_t {
    /*process queue fields */
    struct list_head link;

    /*process tree fields */
    struct pcb_t *p_parent;
    struct list_head p_child, p_sib;

    /* processor state, etc */
    state_t p_s;

    /* process priority */
    int priority;
    int original_priority;

    /* key of the semaphore on which the process is eventually blocked */
    int *p_semkey;
    
    /* campi per il tracciamento del tempo tracorso */
    
    unsigned int initial_time;
    unsigned int usr_time; 
    unsigned int sys_time; 
    unsigned int total_time;
    
    /* campi per il salvataggio di new/old areasin seguito a specpassup */
    
    state_t *sys_old, *sys_new;
    state_t *tlb_old, *tlb_new;
    state_t *trap_old, *trap_new;
    
} pcb_t;



/* Semaphore Descriptor (SEMD) data structure */
typedef struct semd_t {
    struct list_head s_next;

    // Semaphore key
    int *s_key;

    // Queue of PCBs blocked on the semaphore
    struct list_head s_procQ;
} semd_t;


typedef struct semdev {
    int disk[DEV_PER_INT];
    int tape[DEV_PER_INT];
    int network[DEV_PER_INT];
    int printer[DEV_PER_INT];
    int terminalT[DEV_PER_INT];
    int terminalR[DEV_PER_INT];
} semdev;

#endif
