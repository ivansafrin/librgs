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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

static DECLFW(FDSWaveWrite);
static DECLFR(FDSWaveRead);

static DECLFR(FDSSRead);
static DECLFW(FDSSWrite);

void FDSSound();
static void RenderSoundHQ(void);

/* Begin FDS sound */

#define FDSClock (1789772.7272727272727272/2)

typedef struct {
        int64 cycles;           // Cycles per PCM sample
        int64 count;		// Cycle counter
	int64 envcount;		// Envelope cycle counter
	uint32 b19shiftreg60;
	uint32 b24adder66;
	uint32 b24latch68;
	uint32 b17latch76;
        int32 clockcount;	// Counter to divide frequency by 8.
	uint8 b8shiftreg88;	// Modulation register.
	uint8 amplitude[2];	// Current amplitudes.
	uint8 speedo[2];
	uint8 mwcount;
	uint8 mwstart;
        uint8 mwave[0x20];      // Modulation waveform
        uint8 cwave[0x40];      // Game-defined waveform(carrier)
        uint8 SPSG[0xB];
} FDSSOUND;

static FDSSOUND fdso;

#define	SPSG	fdso.SPSG
#define b19shiftreg60	fdso.b19shiftreg60
#define b24adder66	fdso.b24adder66
#define b24latch68	fdso.b24latch68
#define b17latch76	fdso.b17latch76
#define b8shiftreg88	fdso.b8shiftreg88
#define clockcount	fdso.clockcount
#define amplitude	fdso.amplitude
#define speedo		fdso.speedo

static DECLFR(FDSSRead)
{
 switch(A&0xF)
 {
  case 0x0:return(amplitude[0]|(X.DB&0xC0));
  case 0x2:return(amplitude[1]|(X.DB&0xC0));
 }
 return(X.DB);
}

static DECLFW(FDSSWrite)
{
 if(FSettings.SndRate)
 {
  RenderSoundHQ();
 }
 A-=0x4080;
 switch(A)
 {
  case 0x0: 
  case 0x4: if(V&0x80)
	     amplitude[(A&0xF)>>2]=V&0x3F; //)>0x20?0x20:(V&0x3F);
	    break;
  case 0x5://printf("$%04x:$%02x\n",A,V);
		break;
  case 0x7: b17latch76=0;SPSG[0x5]=0;//printf("$%04x:$%02x\n",A,V);
		break;
  case 0x8:
	   b17latch76=0;
	//   printf("%d:$%02x, $%02x\n",SPSG[0x5],V,b17latch76);
	   fdso.mwave[SPSG[0x5]&0x1F]=V&0x7;
           SPSG[0x5]=(SPSG[0x5]+1)&0x1F;
	   break;
 }
 //if(A>=0x7 && A!=0x8 && A<=0xF)
 //if(A==0xA || A==0x9) 
 //printf("$%04x:$%02x\n",A,V);
 SPSG[A]=V;
}

// $4080 - Fundamental wave amplitude data register 92
// $4082 - Fundamental wave frequency data register 58
// $4083 - Same as $4082($4083 is the upper 4 bits).

// $4084 - Modulation amplitude data register 78
// $4086 - Modulation frequency data register 72
// $4087 - Same as $4086($4087 is the upper 4 bits)


static void DoEnv()
{
 int x;

 for(x=0;x<2;x++)
  if(!(SPSG[x<<2]&0x80) && !(SPSG[0x3]&0x40))
  {
   static int counto[2]={0,0};

   if(counto[x]<=0)
   {
    if(!(SPSG[x<<2]&0x80))
    {
     if(SPSG[x<<2]&0x40)
     {
      if(amplitude[x]<0x3F)
       amplitude[x]++;
     }
     else
     {
      if(amplitude[x]>0)
       amplitude[x]--;
     }
    }
    counto[x]=(SPSG[x<<2]&0x3F);
   }
   else
    counto[x]--;
  }
}

static DECLFR(FDSWaveRead)
{
 return(fdso.cwave[A&0x3f]|(X.DB&0xC0));
}

static DECLFW(FDSWaveWrite)
{
 //printf("$%04x:$%02x, %d\n",A,V,SPSG[0x9]&0x80);
 if(SPSG[0x9]&0x80)
  fdso.cwave[A&0x3f]=V&0x3F;
}

static int ta;
static INLINE void ClockRise(void)
{
 if(!clockcount)
 {
  ta++;

  b19shiftreg60=(SPSG[0x2]|((SPSG[0x3]&0xF)<<8));
  b17latch76=(SPSG[0x6]|((SPSG[0x07]&0xF)<<8))+b17latch76;

  if(!(SPSG[0x7]&0x80)) 
  {
   int t=fdso.mwave[(b17latch76>>13)&0x1F]&7;
   int t2=amplitude[1];

   if(t2>0x20) t2=0x20;

   /*if(t&4)
   {
    if(t==4) t=0;
    else t=t2*((3-t)&3);
    t=0x80-t;
   }
   else
   {
    t=t2*(t&3);
    t=0x80+t;
   }
   */
   t=0x80+t2*t/2;

   //if(t&4)
   // t=0x80-((t&3))*t2;
   //else
   // t=0x80+((t&3))*t2; //(0x80+(t&3))*(amplitude[1]); //t; //amplitude[1]*3; //t; //(amplitude[1])*(fdso.mwave[(b17latch76>>13)&0x1F]&7);

   b8shiftreg88=t; //(t+0x80)*(amplitude[1]);

   // t=0;
   //t=(t-4)*(amplitude[1]);
   //FCEU_DispMessage("%d",amplitude[1]);
   //b8shiftreg88=((fdso.mwave[(b17latch76>>11)&0x1F]&7))|(amplitude[1]<<3);
  }
  else
  { 
   b8shiftreg88=0x80;
  }
  //b8shiftreg88=0x80;
 }
 else
 {
  b19shiftreg60<<=1;  
  b8shiftreg88>>=1;
 }
// b24adder66=(b24latch68+b19shiftreg60)&0x3FFFFFF;
 b24adder66=(b24latch68+b19shiftreg60)&0x1FFFFFF;
}

static INLINE void ClockFall(void)
{
 //if(!(SPSG[0x7]&0x80))
 {
  if((b8shiftreg88&1)) // || clockcount==7)
   b24latch68=b24adder66;
 }
 clockcount=(clockcount+1)&7;
}

static INLINE int32 FDSDoSound(void)
{
 fdso.count+=fdso.cycles;
 if(fdso.count>=((int64)1<<40))
 {
  dogk:
  fdso.count-=(int64)1<<40;
  ClockRise();
  ClockFall();
  fdso.envcount--;
  if(fdso.envcount<=0)
  {
   fdso.envcount+=SPSG[0xA]*3;
   DoEnv(); 
  }
 }
 if(fdso.count>=32768) goto dogk;

 // Might need to emulate applying the amplitude to the waveform a bit better...
 {
  int k=amplitude[0];
  if(k>0x20) k=0x20;
  return (fdso.cwave[b24latch68>>19]*k)*4/((SPSG[0x9]&0x3)+2);
 }
}

static int32 FBC=0;

static void RenderSoundHQ(void)
{
 int32 x;
  
 if(!(SPSG[0x9]&0x80) && !(FSettings.disabled&0x20))
  for(x=FBC;x<timestamp;x++)
  {
   uint32 t=FDSDoSound();
   t+=t>>1;
   WaveHi[x]+=t; //(t<<2)-(t<<1);
  }
 FBC=timestamp;
}

static void HQSync(int32 ts)
{
 FBC=ts;
}

static void FDS_ESI(void)
{
 if(FSettings.SndRate)
 {
   fdso.cycles=(int64)1<<39;
 }
//  fdso.cycles=(int64)32768*FDSClock/(FSettings.SndRate *16);
 SetReadHandler(0x4040,0x407f,FDSWaveRead);
 SetWriteHandler(0x4040,0x407f,FDSWaveWrite);
 SetWriteHandler(0x4080,0x408A,FDSSWrite);
 SetReadHandler(0x4090,0x4092,FDSSRead);
}

void NSFFDS_Init(void)
{
 memset(&fdso,0,sizeof(fdso));
 FDS_ESI();
 GameExpSound.HiSync=HQSync;
 GameExpSound.HiFill=RenderSoundHQ;
 GameExpSound.RChange=FDS_ESI;
}

