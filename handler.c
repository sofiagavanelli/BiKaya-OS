#include "handler.h"

extern pcb_t* selected;
extern void term_process(void *pid);

void pc_increase(state_t* state) {
    for(int i= 0; i < WORD_SIZE; i++) {
        state->pc++;
    }
}

void pc_decrease(state_t* state) {
    for(int i= 0; i< WORD_SIZE; i++) {
        state->pc--;
    }
}

unsigned int getExcCode(state_t* status) {

    unsigned int x = 0;
  
    #ifdef TARGET_UMPS
    /* prendiamo il valore del registro cause, eliminiamo le prime due cifre meno significative, 
       dopo di che estraiamo le prime 5 cifre, corrispondenti al valore ExcCode */
     x = getCAUSE();
     x = x >> 2;
     x = x & (0x1F);
   
    #elif TARGET_UARM
     x = CAUSE_EXCCODE_GET(status->CP15_Cause); 
    #endif
    
    return(x);
}


void memCopy(state_t* status, state_t* oldArea){
     
    char* unit = (char*)status;
    char* source = (char*)oldArea; 
    
    int x = sizeof(state_t);
    while (x > 0) {
	  *unit = *source;
	  unit++;
	  source++;
	  x--;
    }
    
}

void trap_H(void) {
    /* se è stato specificato un gestore di livello superiore
        salviamo lo stato della oldArea nel registro apposito e
        carichiamo lo stato presente nella newArea, altrimenti
        terminiamo il processo e richiamiamo lo scheduler */
        
    state_t* old = (state_t*) TRAP_OLD_AREA;  
        
    if(selected->trap_new != NULL) {
        memCopy(selected->trap_old, old);
        LDST(selected->trap_new);
    }
    else {
       term_process(selected);
       scheduling();
    }

}

void tlb_H(void) {
    state_t* old = (state_t*) TLB_OLD_AREA; 

    if(selected->tlb_new != NULL) {
        memCopy(selected->tlb_old, old);
        LDST(selected->tlb_new);
    }
    else {
        term_process(selected);
        scheduling();
    }

}
