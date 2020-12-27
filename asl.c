#include <asl.h>

HIDDEN semd_t semd_table[MAXPROC];

HIDDEN struct list_head semd_h;
HIDDEN struct list_head* ASL = &(semd_h); 
HIDDEN struct list_head semdFree_h;
HIDDEN struct list_head* semdFree = &semdFree_h;

/* 
inizializzazione della semdFree in modo che contenga tutti gli 
elementi della semd_table 
*/
void initASL() {
    INIT_LIST_HEAD(&(semdFree_h));
    INIT_LIST_HEAD(&(semd_h));

	for (int i=0; i<MAXPROC; i++){
		list_add(&semd_table[i].s_next, semdFree);
	}
}

/*
14. restituisce il puntatore al SEMD nella ASL la cui chiave è pari a key. 
Se non esiste un elemento nella ASL con chiave eguale a key, viene restituito NULL.
*/
semd_t* getSemd(int *key) {
    semd_t* eq_key = NULL;
    struct list_head* pos;

    //iteriamo sulla lista partendo da un puntatore alla testa e 
    //procediamo fino a tornare alla sentinella
    for(pos = ASL->next; pos != (ASL); pos = pos->next) {  
        eq_key = container_of(pos, semd_t, s_next);
        if(eq_key->s_key == key) {
            return(eq_key);
          }
        }
    return(NULL);
}


/*
15.  Viene inserito il PCB puntato da p nella coda dei processi bloccati
associata al SEMD con chiave key. Se il semaforo corrispondente non è presente
nella ASL, alloca un nuovo SEMD dalla lista di quelli liberi (semdFree) e lo 
inserisce nella ASL, settando I campi in maniera opportuna. 
Se la lista dei semafori liberi è vuota, restituisce TRUE. 
In tutti gli altri casi, restituisce FALSE.
*/
int insertBlocked(int *key,pcb_t *p) {
     //ad eq_key viene assegnato il puntatore al semaforo con chiave key
     semd_t* eq_key = getSemd(key);  

    //il semaforo esiste, inseriamo 'p' nella lista dei processi bloccati sul semaforo
     if(eq_key != NULL) {
       list_add_tail(&(p->link),&(eq_key->s_procQ));
       p->p_semkey = key;
       return(FALSE);
      }
    //nessun semaforo con chiave key: inseriamo nella ASL un semaforo da semdFree
     else if(!list_empty(semdFree)) {
            semd_t* new = container_of(semdFree->next, semd_t, s_next);
            //il semaforo inserito in ASL viene rimosso da semdFree
            list_del(semdFree->next); 
            new->s_key = key;
            INIT_LIST_HEAD(&(new->s_procQ));
            list_add(&(new->s_next),ASL);
	        list_add(&(p->link),&(new->s_procQ));
            p->p_semkey = key;
            return(FALSE);
          }
    //nessun semaforo allocabile
      else 
        return(TRUE);	
}

/*
16.Ritorna il primo PCB dalla coda dei processi bloccati (s_ProcQ) associata al SEMD
della ASL con chiave key.Se tale descrittore non esiste nella ASL, restituisce NULL. 
Altrimenti, restituisce l’elemento rimosso. 
*/

pcb_t* removeBlocked(int *key) {
    semd_t* eq_key = getSemd(key);
    pcb_t* to_remove = headBlocked(key);

    if(to_remove != NULL) {
		list_del(&to_remove->link);
		to_remove->p_semkey = NULL;
       //il semaforo e' rimosso dalla ASL se la sua s_procQ risulta vuota
       //e reinserito in semdFree
       if(list_empty(&(eq_key->s_procQ))) {
            list_del(&(eq_key->s_next));
            list_add(&(eq_key->s_next), semdFree);
       }
    }
    
    return(to_remove);
}

/*
17.Rimuove il PCB puntato da p dalla coda del semaforo su cui è bloccato.
Se il PCB non compare in tale coda, allora restituisce NULL. Altrimenti, restituisce p.
*/
pcb_t* outBlocked(pcb_t *p) {
    semd_t* eq_key = getSemd(p->p_semkey);

    if(eq_key == NULL) {
          return NULL;
     }
    else {
       list_del(&p->link);
       p->p_semkey = NULL;
       if(list_empty(&(eq_key->s_procQ))) {  
           list_del(&(eq_key->s_next));
           list_add(&(eq_key->s_next), semdFree);
        }
        return(p);
    }
}



/*
18.  Restituisce (senza rimuovere) il puntatore al PCB che si trova in testa 
alla coda dei processi associata al SEMD con chiave key. Ritorna NULL se il SEMD 
non compare nella ASL oppure se compare ma la sua coda dei processi è vuota.
*/
pcb_t* headBlocked(int *key) {
     semd_t* eq_key = getSemd(key);

     if((eq_key == NULL) || list_empty(&(eq_key->s_procQ)))
            return(NULL);
     else {
         pcb_t* first = container_of((&(eq_key->s_procQ))->next, pcb_t, link);
         return(first);
     }  
}

/*
19. Rimuove il PCB puntato da p dalla coda del semaforo su cui è bloccato.
Inoltre, elimina tutti i processi dell’albero radicato in p (ossia tutti i processi
che hanno come avo p) dalle eventuali code dei semafori su cui sono bloccati.
*/
void outChildBlocked(pcb_t *p) {
    semd_t* eq_key = getSemd(p->p_semkey);
   
    if(!(eq_key == NULL)) {
        //p viene rimosso dalla s_procQ del semaforo corrispondente
        outBlocked(p);
        
        struct list_head* pos;
        
        //la funzione viene chiamata ricorsivamente su tutti i figli
        if(!list_empty(&p->p_child)) {
            for(pos = (&(p->p_child))->next; pos != &(p->p_child); pos = pos->next)
                outChildBlocked(container_of(pos, pcb_t, p_sib));
        }
    }          
}
