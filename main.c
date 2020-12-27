#include "scheduler.h"

#ifdef TARGET_UMPS
#define pc pc_epc
#define sp reg_sp

#define FRAME_SIZE 4096
#define INT_NEW_AREA 0x2000008C   
#define SYSCALL_NEW_AREA 0x200003D4
#define TRAP_NEW_AREA 0x200002BC
#define TLB_NEW_AREA 0x200001A4

#define STATUS_SYS_MODE 0x10000004
#define STATUS_ENABLE_INT(status) ((status) | 0xFF00)
#define newarea_state  0x10000000

#define RAMBASE *((unsigned int *)0x10000000)
#define RAMSIZE *((unsigned int *)0x10000004)
#define RAMTOP (RAMBASE + RAMSIZE)
#endif

#ifdef TARGET_UARM
#define status cpsr
#define RAMTOP RAM_TOP

#define INT_NEW_AREA 0x00007058   
#define SYSCALL_NEW_AREA 0x00007268
#define TRAP_NEW_AREA 0x000071B8
#define TLB_NEW_AREA 0x00007108
#define newarea_state  0x000000DF
#endif

#define DEFAULT_PRIORITY 1
#define IDLE_PRIORITY 0

int n_proc = 0;
pcb_t *pcb_test, *idle;

struct list_head readyQueue_h;
struct list_head* readyQueue;
struct semdev semaphore;  

extern void test();

extern void interrupt_H(void);
extern void syscall_H(void);
extern void trap_H(void);
extern void tlb_H(void);



void initArea(state_t* newArea, void (*handler)()){
    state_t *new = (state_t *) newArea;
	new->pc = (memaddr)handler;
	new->sp = RAMTOP;
	new->status = newarea_state;
}

#ifdef TARGET_UMPS
void init(state_t *const s) {
	s->status = STATUS_SYS_MODE;
	s->status = STATUS_ENABLE_INT(s->status);
}
#elif TARGET_UARM
void init(state_t *const s) {
	s->cpsr = STATUS_SYS_MODE;
	s->cpsr = STATUS_ENABLE_TIMER(s->cpsr);
	s->cpsr = STATUS_ENABLE_INT(s->cpsr);
	s->CP15_Control = CP15_DISABLE_VM(s->CP15_Control);
}
#endif

/* funzione di 'attesa' eseguita dal processo idle */
void wait(){
    while(1) 
        ;
}

int main() {
   /* primo step: inizializzazione del sistema */
   
    /* inizializzazione delle new areas */
    initArea((state_t*)INT_NEW_AREA, interrupt_H);
	initArea((state_t*)SYSCALL_NEW_AREA, syscall_H);
    initArea((state_t*)TRAP_NEW_AREA, trap_H);
	initArea((state_t*)TLB_NEW_AREA, tlb_H);
   
    
    /* inizializzazione della struttura relativa ai
       semafori per la gestione dei dispositivi */
    int* s = &(semaphore.disk[0]);
    for(int i = 0; i < 48; i++) {
        *s = 0;
        s++;
    }

    /* inizializzazione dei pcbs e della asl */
    initPcbs();
    initASL(); 
    
    pcb_test = allocPcb(); 
    n_proc++;
    
    pcb_test->initial_time = getTODLO();
    pcb_test->p_s.sp = RAMTOP-FRAME_SIZE;
	
	pcb_test->priority = DEFAULT_PRIORITY;
	pcb_test->original_priority = pcb_test->priority; 
	
	/* processo fittizio idle */
	idle = allocPcb();
	idle->p_s.sp = RAMTOP-FRAME_SIZE*2;
	idle->priority = IDLE_PRIORITY;
	idle->original_priority = IDLE_PRIORITY;
	idle->initial_time = getTODLO();
	
    init(&pcb_test->p_s);
    init(&idle->p_s);
    
    /* pc impostato all'entry point del relativo processo*/
    pcb_test->p_s.pc = (unsigned int) test;
    idle->p_s.pc = (unsigned int) wait; 
    
    #ifdef TARGET_UMPS
    pcb_test->p_s.reg_t9 = pcb_test->p_s.pc;
	idle->p_s.reg_t9 = idle->p_s.pc;
	#endif
	
    /* inizializzazione della ready queue */
    INIT_LIST_HEAD(&(readyQueue_h));
    readyQueue = &readyQueue_h;
	
    insertProcQ(readyQueue, pcb_test);
    insertProcQ(readyQueue, idle);
    
    /* chiamiamo lo scheduler che inizia l'esecuzione
        dei processi */
    scheduling();

	return 0;
}





