# BiKaya

## About

## Installation

Per la compilazione del progetto occorre aver installato gli emulatori umps2 e uarm, 
per i quali si può fare riferimento ai seguenti link:
https://github.com/mellotanica/uARM
https://github.com/tjonjic/umps

Una volta installati tutti i pacchetti richiesti dalle macchine, 
occore decomprire la cartella 'SO_phase2.tar.gz' fornita e accedervi da terminale.

In particolare sono necessari i seguenti pacchetti:
- arm-none-eabi-gcc
- mipsel-linux-gnu-gcc
- uarm 
- umps 
- make

E' possibile invocare make su uno specifo make file, utilizzando i comandi:

$ make -f uarmmake
$ make -f umpsmake

in alternativa, è possibile invocare 'make' specificando un target

$ make umps
$ make uarm

in entrambi i casi verrà eseguita la compilazione per l'emulatore richiesto.

I file necessari per l'esecuzione saranno disponibili in due sottodirectory: umps_obj e uarm_obj.

A questo punto è possibile aprire uno dei due emulatori lanciando il comando 'umps2' o 'uarm',
creare una configurazione per la macchina caricando i file richiesti e far partire l'esecuzione.

Per eliminare i file creati utilizzare i comandi:

$ make clean
$ make umpsclean
$ make uarmcle

## Phases

### Phase 2

File relativi alla fase 2:
(gli altri sono stati lasciati per coerenza con le fasi precedenti)

Sorgenti: 
   - main.c
   - scheduler.c
   - handler.c
   - syscall.c
   - interrupt.c
   - asl.c
   - pcb.c
   - p2test_bikaya_v0.2.c

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



Lo progettazione relativa a questa fase del progetto è stata strutturata nella seguente modalità:

- main.c: file sorgente in cui avvengono tutte le inizializzazioni: vengono inizializzate le new areas,
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
           
                 

## Authors  

This project has been developed has part of 'Operating Systems' course at University of Bologna. 
This code has been developed by *Gavanelli Sofia*, *Lena Erika* and *Leoncini Matteo*, contact informations:  

https://github.com/sofiagavanelli  
https://github.com/erikalena  
matteo.leoncini@studio.unibo.it

Feel free to contact anyone of us for informations or doubts.
