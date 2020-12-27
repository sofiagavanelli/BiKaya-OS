#include "scheduler.h"

extern int n_proc;
pcb_t* selected = NULL;         // processo corrente, in esecuzione
extern struct list_head* readyQueue;   // coda dei processi ready
extern pcb_t* idle;   

unsigned int time = 0;    

void scheduling(){

    if(!list_empty(readyQueue) && n_proc > 0) {
    
        /* se il tempo è diverso da zero, allora c'è già un processo in 
            esecuzione e incrementiamo il kernel time di tale processo */
        if(selected != NULL) {
            if (time != 0) 
                    selected->sys_time += (getTODLO() - time); 
        }
        
        /* riaggiorniamo il "tempo" all'istante attuale */
        time = getTODLO();
        
        /* selezioniamo il processo a priorita' piu' alta dalla coda dei processi 
            pronti e reimpostiamo la sua priorita' a quella originaria */
        selected = removeProcQ(readyQueue);  
        selected->priority = selected->original_priority;

        /* aumentiamo la priorita' di tutti i processi rimasti in coda e settiamo il timer */
        aging();

        insertProcQ(readyQueue,selected);
        timing(3000);
        
        LDST(&(selected->p_s));
        
    } 
    else
        HALT();
}

void timing(unsigned int value) {
    unsigned int *regTimer = (memaddr*)BUS_REG_TIMER; 
    *regTimer = value/time_scale;
}


/* funzione che incrementa la priorita' di tutti i processi in attesa nella readyQueue, eccetto idle */
void aging() {
    struct list_head* pos;
    if (!emptyProcQ(readyQueue)) {
        for(pos = readyQueue->next; pos != (readyQueue); pos = pos->next) {   
            pcb_t* current =  container_of(pos, pcb_t, link);
            if(current != idle) 
                current->priority = current->priority + 1;
        }
    } 
}
