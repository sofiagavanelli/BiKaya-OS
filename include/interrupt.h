#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "handler.h"

#ifdef TARGET_UMPS
#define offset 8

#define INT_DEV_DISK 0x1000003C
#define INT_DEV_TAPE 0x10000040
#define INT_DEV_NETWORK 0x10000044
#define INT_DEV_PRINTER 0x10000048
#define INT_DEV_TERM 0x1000004C
#endif

#ifdef TARGET_UARM
#define offset 24

#define INT_DEV_DISK 0x00006FE0
#define INT_DEV_TAPE 0x00006FE4
#define INT_DEV_NETWORK 0x00006FE8
#define INT_DEV_PRINTER 0x00006FEC
#define INT_DEV_TERM 0x00006FF0
#endif

#define DEV_NOT_INSTALLED    0
#define DEV_READY            1
#define DEV_BUSY             3
#define TRANSMISSION_ERROR   4
#define RECEIVE_ERROR        4
#define CHAR_TRANSMITTED     5
#define CHAR_RECEIVED        5

#define CMD_ACK              1


/* handler per la gestione degli interrupt */
void interrupt_H(void);

/* funzione per la gestione degli interrupt per i sottodevice del terminale */
void int_term(int dev_number);

void outProc(int* semaddr, unsigned int status);

int checkLine(unsigned int line_num);

#endif
