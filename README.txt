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
$ make uarmclean

