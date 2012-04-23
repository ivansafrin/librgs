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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "types.h"
#include "x6502.h"

#include "cart.h"
#include "memory.h"

/* 
   This file contains all code for coordinating the mapping in of the
   address space external to the NES.
   It's also (ab)used by the NSF code.
*/

uint8 *Page[32];

static uint8 PRGIsRAM[32];	/* This page is/is not PRG RAM. */

/* 16 are (sort of) reserved for UNIF/iNES and 16 to map other stuff. */
static int PRGram[32];

uint8 *PRGptr[32];

uint32 PRGsize[32];

uint32 PRGmask2[32];
uint32 PRGmask4[32];
uint32 PRGmask8[32];
uint32 PRGmask16[32];
uint32 PRGmask32[32];

static INLINE void setpageptr(int s, uint32 A, uint8 *p, int ram)
{
 uint32 AB=A>>11;
 int x;

 if(p)
  for(x=(s>>1)-1;x>=0;x--)
  {
   PRGIsRAM[AB+x]=ram;
   Page[AB+x]=p-A;
  }
 else
  for(x=(s>>1)-1;x>=0;x--)
  {
   PRGIsRAM[AB+x]=0;
   Page[AB+x]=0;
  }
}

static uint8 nothing[8192];
void ResetCartMapping(void)
{
 int x;

 for(x=0;x<32;x++)
 {
  Page[x]=nothing-x*2048;
  PRGptr[x]=0;
  PRGsize[x]=0;
 }
}

void SetupCartPRGMapping(int chip, uint8 *p, uint32 size, int ram)
{
 PRGptr[chip]=p;
 PRGsize[chip]=size;

 PRGmask2[chip]=(size>>11)-1;
 PRGmask4[chip]=(size>>12)-1;
 PRGmask8[chip]=(size>>13)-1;
 PRGmask16[chip]=(size>>14)-1;
 PRGmask32[chip]=(size>>15)-1; 

 PRGram[chip]=ram?1:0;
}

DECLFR(CartBR)
{
 return Page[A>>11][A];
}

DECLFW(CartBW)
{
 //printf("Ok: %04x:%02x, %d\n",A,V,PRGIsRAM[A>>11]);
 if(PRGIsRAM[A>>11] && Page[A>>11])
  Page[A>>11][A]=V;
}

DECLFR(CartBROB)
{
 if(!Page[A>>11]) return(X.DB);
 return Page[A>>11][A];
}

void FASTAPASS(3) setprg2r(int r, unsigned int A, unsigned int V)
{
  V&=PRGmask2[r];

  setpageptr(2,A,PRGptr[r]?(&PRGptr[r][V<<11]):0,PRGram[r]);
}

void FASTAPASS(2) setprg2(uint32 A, uint32 V)
{
 setprg2r(0,A,V);
}

void FASTAPASS(3) setprg4r(int r, unsigned int A, unsigned int V)
{
  V&=PRGmask4[r];
  setpageptr(4,A,PRGptr[r]?(&PRGptr[r][V<<12]):0,PRGram[r]);
}

void FASTAPASS(2) setprg4(uint32 A, uint32 V)
{
 setprg4r(0,A,V);
}

void FASTAPASS(3) setprg8r(int r, unsigned int A, unsigned int V)
{
  if(PRGsize[r]>=8192)
  {
   V&=PRGmask8[r];
   setpageptr(8,A,PRGptr[r]?(&PRGptr[r][V<<13]):0,PRGram[r]);
  }
  else
  {
   uint32 VA=V<<2;
   int x;
   for(x=0;x<4;x++)
    setpageptr(2,A+(x<<11),PRGptr[r]?(&PRGptr[r][((VA+x)&PRGmask2[r])<<11]):0,PRGram[r]);
  }
}

void FASTAPASS(2) setprg8(uint32 A, uint32 V)
{
 setprg8r(0,A,V);
}

void FASTAPASS(3) setprg16r(int r, unsigned int A, unsigned int V)
{
  if(PRGsize[r]>=16384)
  {
   V&=PRGmask16[r];
   setpageptr(16,A,PRGptr[r]?(&PRGptr[r][V<<14]):0,PRGram[r]);
  }
  else
  {
   uint32 VA=V<<3;
   int x;

   for(x=0;x<8;x++)
    setpageptr(2,A+(x<<11),PRGptr[r]?(&PRGptr[r][((VA+x)&PRGmask2[r])<<11]):0,PRGram[r]);
  }
}

void FASTAPASS(2) setprg16(uint32 A, uint32 V)
{
 setprg16r(0,A,V);
}

void FASTAPASS(3) setprg32r(int r,unsigned int A, unsigned int V)
{
  if(PRGsize[r]>=32768)
  {
   V&=PRGmask32[r];
   setpageptr(32,A,PRGptr[r]?(&PRGptr[r][V<<15]):0,PRGram[r]);
  }
  else
  {
   uint32 VA=V<<4;
   int x;

   for(x=0;x<16;x++)
    setpageptr(2,A+(x<<11),PRGptr[r]?(&PRGptr[r][((VA+x)&PRGmask2[r])<<11]):0,PRGram[r]);
  }
}

void FASTAPASS(2) setprg32(uint32 A, uint32 V)
{
 setprg32r(0,A,V);
}
