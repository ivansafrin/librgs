/*  Festalon - NSF Player
 *  Copyright (C) 2004 Xodnizel
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef _X6502H

#include "x6502struct.h"

extern uint32 timestamp;
extern uint64 timestampbase;

extern X6502 X;
void X6502_Run(int32 cycles);

#define N_FLAG  0x80
#define V_FLAG  0x40
#define U_FLAG  0x20
#define B_FLAG  0x10
#define D_FLAG  0x08
#define I_FLAG  0x04
#define Z_FLAG  0x02
#define C_FLAG  0x01

extern void FP_FASTAPASS(1) (*MapIRQHook)(int a);

#define NTSC_CPU 1789772.7272727272727272
#define PAL_CPU  1662607.125

#define FCEU_IQEXT      0x001
#define FCEU_IQEXT2     0x002
/* ... */
#define FCEU_IQRESET    0x020
#define FCEU_IQNMI2	0x040	// Delayed NMI, gets converted to *_IQNMI
#define FCEU_IQNMI	0x080
#define FCEU_IQDPCM     0x100
#define FCEU_IQFCOUNT   0x200
#define FCEU_IQTEMP     0x800

void X6502_Init(void);
void X6502_Reset(void);
void X6502_Power(void);

void TriggerNMI(void);
void TriggerNMI2(void);

uint8 FASTAPASS(1) X6502_DMR(uint32 A);
void FASTAPASS(2) X6502_DMW(uint32 A, uint8 V);

void FASTAPASS(1) X6502_IRQBegin(int w);
void FASTAPASS(1) X6502_IRQEnd(int w);

#define DECLFR(x) uint8 FP_FASTAPASS(1) x (uint32 A)
#define DECLFW(x) void FP_FASTAPASS(2) x (uint32 A, uint8 V)
  
void SetReadHandler(int32 start, int32 end, readfunc func);
void SetWriteHandler(int32 start, int32 end, writefunc func);
writefunc GetWriteHandler(int32 a);
readfunc GetReadHandler(int32 a);

/* Used in sound.c and nsf.c */
extern readfunc ARead[0x10000];
extern writefunc BWrite[0x10000];

#define _X6502H
#endif
