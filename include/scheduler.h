#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "pcb.h"
#include "asl.h"
#include "listx.h"

#define time_scale *((unsigned int *)BUS_REG_TIME_SCALE)

#ifdef TARGET_UMPS
#include "umps/libumps.h"
#include "umps/types.h"
#include "umps/arch.h"

#define BUS_TODLOW  0x1000001C
#define getTODLO() (*((unsigned int *)BUS_TODLOW))
#endif

#ifdef TARGET_UARM
#include "uarm/libuarm.h"
#endif


void scheduling();

void timing(unsigned int value);

void aging();

#endif
