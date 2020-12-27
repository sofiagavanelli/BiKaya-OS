#include <pcb.h>

HIDDEN pcb_t pcbFree_table[MAXPROC];
HIDDEN struct list_head pcbfree_h;
HIDDEN struct list_head* pcbFree = &pcbfree_h;


/*
1. Inizializza la pcbFree in modo da contenere tutti gli elementi della pcbFree_table. 
*/
void initPcbs(void) {
    //inizializza la lista pcbFree
	INIT_LIST_HEAD(&(pcbfree_h));
	
	//ad ogni iterazione viene inizializzato il campo link di tipo list head 
	//corrispondente al pcb considerato, ovvero aggiunto alla lista pcbFree
	for (int i=0; i<MAXPROC; i++){
		list_add(&pcbFree_table[i].link, pcbFree); 
	}
}

/*
2. Inserisce il PCB puntato da p nella lista dei PCB liberi (pcbFree)
*/
void freePcb(pcb_t* p) {
    list_add(&(p->link), pcbFree); 
}


/*
3. Restituisce NULL se la pcbFree è vuota. Altrimenti rimuove un elemento dalla pcbFree, 
inizializza tutti i campi (NULL/0) e restituisce l’elemento rimosso.
*/
pcb_t *allocPcb() {

    if(list_empty(pcbFree)) {
        return NULL;
    }
    else {
        pcb_t* to_remove ;
        /* otteniamo il primo pcb dalla pcbFree e lo eliminiamo dalla lista */
        to_remove = container_of(pcbFree->next, pcb_t, link); 
        list_del(pcbFree->next);
        
        /* inizializziamo tutti i campi della struttura a zero */
        char* unit;
        unit = (char*)to_remove;  
        int x = sizeof(pcb_t);
        while (x > 0) {
		   *unit=0;
			unit++;
			x--;
	    }
	    
	    INIT_LIST_HEAD(&(to_remove->link));     //inizializziamo tutti i list head in esso contenuti
	    INIT_LIST_HEAD(&(to_remove->p_child));  // in modo che risultino vuoti
	    INIT_LIST_HEAD(&(to_remove->p_sib));
        
        return(to_remove);    
    }
}

//LISTA PCBS

/*
4. Inizializza la lista dei PCB, inizializzando l’elemento sentinella.
*/
void mkEmptyProcQ(struct list_head* head) {
    INIT_LIST_HEAD(head);
}

/*
5. Restituisce TRUE se la lista puntata da head è vuota, FALSE altrimenti.
*/
int emptyProcQ(struct list_head* head) {
    return(list_empty(head));
}

/*    
6. inserisce l’elemento puntato da p nella coda dei processi puntata da head
*/
void insertProcQ(struct list_head* head, pcb_t* p) {
    // se la lista e' vuota, inseriamo direttamente il pcb richiesto nella lista, 
    if (emptyProcQ(head)) 
        list_add(&p->link,head);        
    else {
        // altrimenti confrontiamo il pcb con gli altri nella lista e lo inseriamo 
        // in base alla sua priorita'
        
        struct list_head* next = head;
        pcb_t* current = container_of(head->next, pcb_t, link);   
        
        while((p->priority <= current->priority) && (!list_is_last(next,head))) {  
            
            next = next->next;   
            current = container_of(next->next, pcb_t, link);                                                 
        }

        list_add(&p->link,next);
      
    }
}

/*
7. Restituisce l’elemento di testa della coda dei processi da head, SENZA RIMUOVERLO. 
Ritorna NULL se la coda non ha elementi.
*/
pcb_t* headProcQ(struct list_head* head) {
    if (emptyProcQ(head)) 
        return NULL;
    else {
    //ritorna il pcb contenente il list head puntato da head->next
        return(container_of(head->next, pcb_t, link)); 
    }
}

/*
8. Rimuove il primo elemento dalla coda dei processi puntata da head. Ritorna NULL se la coda è vuota. 
Altrimenti ritorna il puntatore all’elemento rimosso dalla lista.
*/
pcb_t* removeProcQ(struct list_head* head) {
    //recupera l'elemento in testa alla lista e lo toglie dalla cosa dei processi
    pcb_t* to_remove = headProcQ(head);
    
    list_del(head->next);      
    return(to_remove);
}

/*
9. Rimuove il PCB puntato da p dalla coda dei processi puntata da head. 
Se p non è presente nella coda, restituisce NULL.
*/
pcb_t* outProcQ(struct list_head *head, pcb_t *p){
    pcb_t* to_remove = NULL;
    struct list_head* pos;
    if (!emptyProcQ(head)) {
    //scorre la lista dal primo elemento fino alla sentinella confrontando il pcb corrente con p
        for(pos = head->next; pos != (head); pos = pos->next) {   
             pcb_t* current =  container_of(pos, pcb_t, link);
                  
             //se i due pcb coincidono, memorizziamo il pcb corrente in to_remove ed eliminiamo il processo dalla lista 
             if (current == p) {                                
                 to_remove = current;                               
                 list_del(pos);
             }
        }
     }
   return(to_remove);
}

/*
funzione che restituisce true se il processo p è presente in head, false altrimenti
*/

int getProc(struct list_head *head, pcb_t *p) {
    
    int rval = 0;
    
    struct list_head* pos;
    if (!emptyProcQ(head)) {

        for(pos = head->next; pos != (head); pos = pos->next) {   
             pcb_t* current =  container_of(pos, pcb_t, link);
                 
             if (current == p) {                                
                 rval = 1;
             }
           }
     }
    return(rval);
}


//ALBERI DI PCB

/*
10. restituisce TRUE se il PCB puntato da p non ha figli, restituisce FALSE altrimenti.
*/
int emptyChild(pcb_t *p) {
    return(emptyProcQ(&(p->p_child)));  
}

/*
11. Inserisce il PCB puntato da p come figlio del PCB puntato da prnt.
*/
void insertChild(pcb_t *prnt,pcb_t *p) {
    p->p_parent = prnt;        
    list_add_tail(&(p->p_sib),&(prnt->p_child));  
}

/*
12. Rimuove il primo figlio del PCB puntato da p. Se p non ha figli, restituisce NULL
*/
pcb_t* removeChild(pcb_t *p) {
    if(emptyChild(p)) 
        return NULL;
    else {
        pcb_t* to_remove ;
        to_remove = container_of((&(p->p_child))->next, pcb_t, p_sib);
        
        list_del((&(p->p_child))->next);
        to_remove->p_parent = NULL;
        return(to_remove);
    }
}

/*
13. Rimuove il PCB puntato da p dalla lista dei figli del padre. 
Se il PCB puntato da p non ha un padre, restituisce NULL. Altrimenti restituisce l’elemento rimosso (cioè p). 
A differenza della removeChild, p può trovarsi in una posizione arbitraria
*/
pcb_t* outChild(pcb_t* p) {
   if (p->p_parent == NULL)
      return NULL;
   else { 
     list_del(&(p->p_sib));
     p->p_parent = NULL;
      return(p);

     //implementazione iniziale a partire dalla specifica data, poi semplificata nella versione non commentata
     // pcb_t* parent = p->p_parent;
     // pcb_t* to_remove = NULL;
     // struct list_head* pos; 
     //
     //   for(pos = (&(parent->p_child))->next; pos != &(parent->p_child); pos = pos->next) { 
     //        pcb_t* current =  container_of(pos, pcb_t, p_sib);
     //        if (current == p) {
     //            to_remove = current;
     //            list_del(pos);
     //        }
     //      }
     // to_remove->p_parent = NULL;
     //return(to_remove);

   }
}


