#include "interrupt.h"

extern pcb_t* selected;
extern struct list_head* readyQueue;
extern struct semdev semaphore;
extern unsigned int time;


void interrupt_H(void) {
    
    state_t* old = (state_t*)INT_OLD_AREA;
    
    selected->usr_time += (getTODLO() - time);
    time = getTODLO();

    /* controllo (ulteriore) che verifica che sia stato invocato un interrupt */
    if(getExcCode(old) == INTEXCEPTION) {  
    
        #ifdef TARGET_UARM
            pc_decrease(old);
        #endif
      
        /* carichiamo in selected lo stato presente nell'old area relativa agli interrupt */     
        memCopy(&(selected->p_s), old);
        
        unsigned int instance;

        if(checkLine(INT_TIMER)) {
           timing(3000);  
        }
       
        if(checkLine(INT_DISK)) {
            instance = *((unsigned int*)INT_DEV_DISK);
            /* scorriamo le device bit map relativa ad una classe di dispositivi e controlliamo quali 
                istanze di quella classe hanno sollevato un interrupt */
            for(int i = 0; i < 8; i++) {
               if(instance & 0x1) {
                   dtpreg_t *dev_reg = (dtpreg_t *) DEV_REG_ADDR(INT_DISK, i);
                   
                   /* se è presente un'istruzione nel campo relativo, viene mandato un comando di 
                      acknoledgement  al dispositivo che ha sollevato l'interrupt, il quale 
                      viene sbloccato dal semaforo sul quale era in attesa  */
                   if(dev_reg->command != 0) {
                         dev_reg->command = CMD_ACK;
                         int* semaddr = &(semaphore.disk[i]);
                         outProc(semaddr, dev_reg->status);
                   }
               }
               instance = instance >> 1;
            }
        }
   
        if(checkLine(INT_TAPE)) {
            instance = *((unsigned int*)INT_DEV_TAPE);
            for(int i = 0; i < 8; i++) {
                if(instance & 0x1) {
                    dtpreg_t *dev_reg = (dtpreg_t *) DEV_REG_ADDR(INT_TAPE, i);
                    if(dev_reg->command != 0) {
                         dev_reg->command = CMD_ACK;
                         int* semaddr = &(semaphore.tape[i]);
                         outProc(semaddr, dev_reg->status);
                    }
                }
                
                instance = instance >> 1;
             }
        }

        if(checkLine(INT_NETWORK)) {
            instance = *((unsigned int*)INT_DEV_NETWORK);
            for(int i = 0; i < 8; i++) {
               if(instance & 0x1) {
                   dtpreg_t *dev_reg = (dtpreg_t *) DEV_REG_ADDR(INT_NETWORK, i); 
                   
                   if(dev_reg->command != 0) {
                         dev_reg->command = CMD_ACK;
                         int* semaddr = &(semaphore.network[i]);
                         outProc(semaddr, dev_reg->status);
                   }
                }
                instance = instance >> 1;
            }   
        }
        if(checkLine(INT_PRINTER)) {
             instance = *((unsigned int*)INT_DEV_PRINTER);
             for(int i = 0; i < 8; i++) {
                  if(instance & 0x1) {
                      dtpreg_t *dev_reg = (dtpreg_t *) DEV_REG_ADDR(INT_PRINTER, i); 
                      if(dev_reg->command != 0) {
                         dev_reg->command = CMD_ACK;
                         int* semaddr = &(semaphore.printer[i]);                     
                         outProc(semaddr, dev_reg->status);
                      }
                  }
               instance = instance >> 1;        
             }       
        }

        if(checkLine(INT_TERMINAL)) {
           instance = *((unsigned int*)INT_DEV_TERM);
           for(int i = 0; i < 8; i++) {
                if(instance & 0x1) {
                    int_term(i); 
                }
                instance = instance >> 1;
           }  
        }    
             
        scheduling();
             
    }
    else 
        PANIC();
}


void int_term(int instance) {
       termreg_t *dev_reg = (termreg_t *) DEV_REG_ADDR(INT_TERMINAL, instance);
       unsigned int status_t = dev_reg->transm_status;
       unsigned int status_r = dev_reg->recv_status;
       
       int* semaddr ;
       
       /* sottodevice di trassmissione */
       if((dev_reg->transm_command & 0xFF) != 0){
             semaddr = &(semaphore.terminalT[instance]);
             dev_reg->transm_command = CMD_ACK;
             outProc(semaddr, status_t);
	   }
       
       /* sottodevice di ricezione */
       if((dev_reg->recv_command & 0xFF)!= 0){
            semaddr = &(semaphore.terminalR[instance]);
            dev_reg->recv_command = CMD_ACK;
            outProc(semaddr, status_r);
	   }	   
	    
}


void outProc(int* semaddr, unsigned int status) {   
    /* se c'e' un processo in coda sul semaforo, esso viene rimosso 
       e inserito nella readyQueue */ 
    (*semaddr)++;
    
    pcb_t* tmp = removeBlocked(semaddr);
             
    if (tmp != NULL) {
        /* il valore di ritorno è il valore del registro di 
            status del dispositivo sul quale è avvenuta l'operazione */
        tmp->p_s.reg_v0 = status;
        insertProcQ(readyQueue, tmp);
    }
}

/* funzione che controlla se è presente un interrupt pendente
   su una determinata linea, il cui numero è preso in input */
int checkLine(unsigned int line_num) {

    unsigned int x = getCAUSE();
    x = x >> offset;

    for(int i = 0; i < line_num; i++) {
            x = x >> 1; 
    }
    if(x & 0x1)
        return(1);
    else 
        return(0);
}
