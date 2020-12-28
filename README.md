# BiKaya

## About  

BiKaya is an educational operating system based upon a 6 levels of astraction architecure; it was developed to run on 2 different architectures, MIPS and ARM. In particular, the goal of this project was to develop only two of those levels: queue's managment and OS kernel.  

## Requirements

In order to compile the project the first thing to do is install the emulators: µMPS2 and µARM.     

This is possible using the following links: https://github.com/mellotanica/uARM for uarm and https://github.com/tjonjic/umps for umps.  

Packages required are:  
- arm-none-eabi-gcc
- mipsel-linux-gnu-gcc
- uarm 
- umps 
- make  

## Installation  

Once the requirements are met and the repository has been downloaded the steps to follow are:  

Invoking make on one specific make file, using:  

```  
$ make -f uarmmake  
$ make -f umpsmake  
```

or, it is also possible, invoking `make` specifying a target:  

```
$ make umps  
$ make uarm  
```

In both cases only the requested emulator's compilation will be done.  

Depending on which emulator was chosen there will be a new folder (umps_obj or uarm_obj) containing the files for the execution.  

## Running  

Finally, through the Terminal, an emulator can be open using `umps2` or `uarm`, then a new configuration (with the requested files) is needed to start.  

To remove the files just created:

```
$ make clean  
$ make umpsclean  
$ make uarmcle  
```

## Phases  

This project was developed following four different phases: 0, 1, 1.5 and 2.  Inside this repository there are files from all of them but the documentation 
is strictly about the last one.

This was the structure of the phases:
/////inserire imm slide

Files congruent with phase 2, are:  

Sources: 
   - main.c
   - scheduler.c
   - handler.c
   - syscall.c
   - interrupt.c
   - asl.c
   - pcb.c
   - p2test_bikaya_v0.2.c (this was a test file used to make sure the implementation was right)

Headers:  
   - scheduler.h  
   - handler.h  
   - syscall.h  
   - interrupt.h 
   - asl.h
   - pcb.h
   - listx.h
   - const.h
   - types_bikaya.h  

### Design  

The structure of this phase was based upon these files:  

#### main.c: 
file sorgente in cui avvengono tutte le inizializzazioni: vengono inizializzate le new areas,
          i pcbs (processo test e processo idle) e le variabili del kernel. 
          I processi vengono inseriti nella readyQueue e viene chiamato lo scheduler per decidere che processo 
          mettere in esecuzione.   
          Inizializzazione new areas: la funzione 'initArea(state_t* newArea, void (*handler)())' imposta
          lo stack pointer a RAMTOP ed il program counter all'indirizzo della funzione (handler) dedicata 
          a gestire l'eccezione associata alla new area. Infine viene settato lo status.   
          Inizializzazione dei pcbs: viene prelevato un processo dalla coda dei processi liberi, 
          utilizzando la funzione 'allocPcb()' sviluppata in fase 1. Lo stack pointer relativo viene 
          impostato a (ramtop - framesize*n), vengono impostati i campi priority ed original priority 
          e viene settato lo stato.     
          umps-uarm: lo stato viene settato dalla funzione 'init' (il bit IEp è attivo in modo che, una volta invocata la 
                     funzione LDST, IEc risulti attivo). Tutti gli interrupt vengono abilitati impostando tutti i bit 
                     dell'IM a 1.
           

#### scheduler.c:  
lo scheduler seleziona un processo dalla readyQueue, se questa non è vuota, utilizzando la funzione
               removeProcQ; essa rimuove il primo pcb della lista che è anche quello a priorità più alta. Dopo aver fatto
               l'aging degli altri processi, il processo selezionato viene reinserito in coda con la sua priorità originaria.
               A questo punto, la funzione di scheduling setta il timer a 3 millisecondi (il valore viene settato a 3000 poiche'
               tale valore viene decrementato ogni 10^(-3) millisecondi) e carica lo stato del processo selezionato nel processore,
               utilizzando la funzione LDST.
               Quando non sono più presenti processi nella readyQueue(), viene invocata la funzione HALT() per terminare.

#### handler.c:  

questo sorgente contiene le funzioni per la gestione delle eccezioni di tipo trap e tlb, più alcune funzioni di 'supporto'.
          Sia nel caso degli interrupt sia nel caso di una syscall, le funzioni deputate alla gestione di tali eccezioni verificano
             che effettivamente siano stati generati un interrupt o una syscall, rispettivamente, altrimenti il sistema viene messo in 
             stato PANIC. Dopo questo primo controllo, lo stato presente nella old area (relativa all'eccezione) viene copiato nello stato 
             del processo corrente (selected); prima di fare ciò il program counter dello stato salvato viene decrementato di una word, 
             nel caso di interrupt (su uarm) e incrementato di una word in caso di syscall (su umps), come da specifiche.
             
#### interrupt.c:  

Function `interrupt_H` : its main job is to understand which lines have raised the interrupt starting from the lowest with the higher priority. If hanging interrupts are find on a line then the function controls all the waiting devices on that line. If the interrupt has been raised because the time was up then the timer is reset. In all the other cases, the acknowledgment command is set on the device register and the process requesting the operation is released.

/////la funzione 'interrupt_H' controlla su quali linee è stato sollevato, guardandole tutta partendo da quella più bassa e a priorità 
               più alta. Se su una linea si trovano degli interrupt pendenti, controlla tutti i dispositivi in attesa sulla linea. 
               Se l'interrupt è stato scatenato dallo scadere del timer, il timer viene ricaricato.
               Negli altri casi viene settato il comando di acknoledgement sul registro del disposito e viene sbloccato il processo che aveva 
               richiesto l'operazione.
             
#### syscall.c:  

Function `syscall_H` : its task is to check if any syscall or breakpoints have been thrown. Then, it controls the obtained value in the register a0/a1 to deal with the requested exception. 

///la funzione 'syscall_H' controlla se è stata sollevata una syscall o un breakpoint, controlla il valore contenuto nel registro a0/a1 e si occupa di gestire la syscall richiesta.  
             
             
## Authors  

This project has been developed has part of 'Operating Systems' course at University of Bologna. 
This code has been written by *Gavanelli Sofia*, *Lena Erika* and *Leoncini Matteo*, contact informations:  

https://github.com/sofiagavanelli  
https://github.com/erikalena  
matteo.leoncini@studio.unibo.it

Feel free to contact anyone of us for informations or doubts.
