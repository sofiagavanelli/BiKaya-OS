#include "syscall.h"

extern int n_proc;
extern pcb_t* selected;
extern struct list_head* readyQueue;
extern struct semdev semaphore;

extern unsigned int time;


void syscall_H(void) {

    state_t* old = (state_t*) SYS_OLD_AREA; 
    
    /* aggiorniamo lo user-time del processo corrente */
    selected->usr_time += (getTODLO() - time);
    time = getTODLO();
    
    /* controllo che verifica se e' stata invocata una syscall o un breakpoint */    
    if(getExcCode(old) == SYSEXCEPTION || getExcCode(old) == BPEXCEPTION) {
        
        #ifdef TARGET_UMPS
         pc_increase(old);
        #endif
        
        memCopy(&(selected->p_s), old);
        
        /* verifichiamo qual [ la syscall richiesta guardando il valore del registro a0(umps)/a1(uarm)*/ 
        int value = old->reg_a0;
        
        switch (value) {
            case GETCPUTIME:  
                    
                    selected->total_time = getTODLO() - selected->initial_time;
                    get_cputime((unsigned int*) old->reg_a1, (unsigned int*) old->reg_a2, (unsigned int*) old->reg_a3);
                    resume();    
                    break;
                    
            case CREATEPROCESS:
                    create_process((state_t*) old->reg_a1, old->reg_a2, (void **)old->reg_a3); 
                    resume();
                    break;
                    
            case TERMINATEPROCESS:
                 /* Se il secondo parametro è null, invochiamo la funzione term_process dandogli come input
                    il processo corrente. */
                    
                    if((void *) old->reg_a1 == NULL) {
                        term_process(selected);
                        scheduling();
                    }
                    else {
                       term_process((void *) old->reg_a1);
                       resume();
                    }
                    break;
                    
            case VERHOGEN:
                    verhogen((int *) old->reg_a1);
                    resume();
                    break;
                    
            case PASSEREN:
                    passeren((int *) old->reg_a1);
                    break;   
                                               
            case WAITIO:
                    waitio(old->reg_a1, (unsigned int*) old->reg_a2, old->reg_a3);
                    break;
                    
            case SPECPASSUP:
                    specpassup(old->reg_a1, (state_t*) old->reg_a2, (state_t*) old->reg_a3);
                    break;  
                      
            case GETPID:
                    getpid((void **)old->reg_a1, (void **)old->reg_a2);
                    resume();
                    break;
                    
            default:
                    /* se è stato specificato un gestore di livello superiore, salvimo lo stato presente
                        nella oldArea e carichiamo lo stato specificato nella sys_new (newArea di livello superiore) */
                    if(selected->sys_new != NULL) {
                        memCopy(selected->sys_old, old);
                        selected->sys_time += (getTODLO() - time);
                        time = getTODLO();
                        LDST(selected->sys_new);
                    }
                    /* se tale gestore non è stato specificato, il processo viene terminato */
                    else {
                        term_process(selected); 
                        scheduling();
                    }

        }
        
   }
   else 
        PANIC();
}

/* funzione che ricarica lo stato del processo corrente, dopo aver calcolato il tempo 
    passato in "kernel mode" */
void resume(void) {
    selected->sys_time += (getTODLO() - time);    
    time = getTODLO();
    LDST(&selected->p_s);
}

/* SYS1 : restituisce il tempo di esecuzione del processo che l’ha chiamata fino a
quel momento, separato in user time, kenel time e tempo totale di vita del processo */
void get_cputime(unsigned int *user, unsigned int *kernel, unsigned int *wallclock) {
    *user = selected->usr_time;   
    *kernel = selected->sys_time;
    *wallclock = selected->total_time;      
}

/* SYS2 : creazione di un nuovo processo come figlio del chiamante */
void create_process(state_t* statep, int p_value, void **cpid){

    int rvalue = 0;
    pcb_t* new_proc = allocPcb();
    n_proc++; 
       
    if(new_proc != NULL) {
        /* inizializzazione del processo new_proc */
        memCopy(&(new_proc->p_s),statep);
        
        new_proc->priority = p_value;
        new_proc->original_priority = new_proc->priority;
        
        new_proc->initial_time = getTODLO();
        new_proc->sys_new = NULL;
        
        if (cpid != NULL) 
           *cpid = new_proc;
        
        /* il processo creato viene inserito nella coda dei processi ready
           e aggiunto alla lista dei figli del processo corrente */   
        insertProcQ(readyQueue, new_proc);
        insertChild(selected, new_proc);
     }
    else
        rvalue = -1;
    
    selected->p_s.reg_v0 = rvalue;
}

/* SYS3 : eliminazione del processo corrente e di tutta 
           la progenie dalla lista dei processi ready */
void term_process(void *pid) {
    pcb_t* p = (pcb_t*) pid;
    int *key = p->p_semkey;
    
    /* se il processo non è presente nella readyQueue e non è neanche
        bloccato su un semaforo, la funzione restituisce -1 */       
    if(!getProc(readyQueue, pid) && key == NULL) {
         selected->p_s.reg_v0 = -1;
    }
    else {
        /* eliminiamo il processo associato a pid, lo togliamo dalla lista dei figli del padre */
        delete_ric(p);

        if(p->p_parent != NULL) {
            outChild(pid);
        }
              
        selected->p_s.reg_v0 = 0;
    }

}

/* funzione che termina il processo corrente e ricorsivamente
    tutta la sua progenie */
void delete_ric(pcb_t* p) {

    n_proc--;
    int *key = p->p_semkey;
    
    /* se il processo non è bloccato su un semaforo, lo togliamo dalla readyQueue
        altrimenti lo eliminiamo dalla coda del semaforo corrispondente */
    if(key == NULL) {
        outProcQ(readyQueue, p);     
    } 
    else {
        (*key)++;
        outBlocked(p);
    }     
    
    /* reinseriamo il processo nella coda dei processi liberi */
    freePcb(p);
    
    struct list_head* pos;
    struct list_head* child = &(p->p_child);
    if (!emptyProcQ(child)) {
        for(pos = child->next; pos != child; pos = pos->next) {   
            pcb_t* to_delete =  container_of(pos, pcb_t, p_sib);
            delete_ric(to_delete);
        }
    }     
} 

/* SYS4 : operazione di rilascio (V) su un semaforo */
void verhogen(int *semaddr) {
    (*semaddr)++;
    
    pcb_t* proc = removeBlocked(semaddr);
    if(proc != NULL) 
       insertProcQ(readyQueue, proc);
}

/* SYS5 : operazione di richiesta (P) di un semaforo */
void passeren(int *semaddr) {
    (*semaddr)--; 
    
    /* se il valore del semaforo diventa negativo, togliamo il processo dalla 
        readyQueue e lo inseriamo nella coda dei processi associata al semaforo */
    if(*semaddr < 0) {
        outProcQ(readyQueue, selected);
        insertBlocked(semaddr, selected);
        scheduling();
    }
    else resume();
}

/* SYS6 : richiesta di operazione di I/O */
void waitio(unsigned int instruction, unsigned int *dev_reg, int subdevice) {
    
    int line, device;
    int* sem = &(semaphore.disk[0]);

    for(int i = 3; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            if (DEV_REG_ADDR(i, j) == (unsigned int)dev_reg) { 
                line = i; device = j;
            }
        }
    }
    
    /* scorriamo la struttura dei semafori in modo da avere in sem
       un puntatore al semaforo corrispondente al device richiesto */
    int sem_num = (line-3)*8 + device + subdevice*8;
    for(int i = 0; i < sem_num; i++) 
       sem++; 
    
    if(line < 7) {     
        ((dtpreg_t*) dev_reg)->command =  instruction;

    } 
    else {
        /* gestione dei terminali */
        if(subdevice == 0) {
             ((termreg_t*) dev_reg)->transm_command =  instruction;
        } else { 
            ((termreg_t*) dev_reg)->recv_command =  instruction;
        }
    }
    
    /* l'operazione è asincrona, per cui blocchiamo il processo
       sul relativo semaforo */
    passeren(sem);
}

/* SYS7 : specifica di un handler di livello superiore */
void specpassup(int type, state_t* old, state_t* new) {
    
    if(type == 0 && selected->sys_old == NULL) {
            selected->sys_old = old;
            selected->sys_new = new;
            selected->p_s.reg_v0 = 0;
            resume();
    }
    else if(type == 1 && selected->tlb_old == NULL) {
            selected->tlb_old = old;
            selected->tlb_new = new;
            selected->p_s.reg_v0 = 0;   
            resume();
    }
    else if (type == 2 && selected->trap_old == NULL) {
            selected->trap_old = old;
            selected->trap_new = new;
            selected->p_s.reg_v0 = 0;    
            resume();
    } 
    else {
            /* se il processo aveva già invocato una spec_passup sul tipo
               richiesto (o il tipo specificato non è corretto), 
               il processo viene terminato */
            term_process(selected);
            selected->p_s.reg_v0 = -1;    
            scheduling();
    }

}

/* SYS8 : assegnazione dell’identificativo del processo corrente e 
          di quello del processo genitore */
void getpid(void **pid, void **ppid) {
    if(pid != NULL) 
           *pid = selected;
    if(ppid != NULL)
           *ppid = selected->p_parent;
            
}
