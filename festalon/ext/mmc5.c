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

#include "common.h"

static void (*sfun)(int P);
static void (*psfun)(void);

void MMC5RunSoundHQ(void);

static uint8 *ExRAM=NULL;
static uint8 mul[2];

static DECLFW(Mapper5_write)
{
 switch(A)
 {
  case 0x5205:mul[0]=V;break;
  case 0x5206:mul[1]=V;break;
 }
}

static DECLFW(MMC5_ExRAMWr)
{
 ExRAM[A&0x3ff]=V;
}

static DECLFR(MMC5_ExRAMRd)
{
 return ExRAM[A&0x3ff];
}

static DECLFR(MMC5_read)
{
 switch(A)
 {
  case 0x5205:return (mul[0]*mul[1]);
  case 0x5206:return ((mul[0]*mul[1])>>8);
 }
 return(X.DB);
}

typedef struct {
        uint16 wl[2];
        uint8 env[2];
        uint8 enable;
	uint8 running;
        uint8 raw;
        uint8 rawcontrol;

        int32 dcount[2];

        int32 BC[3];
        int32 vcount[2];
} MMC5APU;

static MMC5APU MMC5Sound;


static void Do5PCMHQ()
{
   int32 V;
   if(!(MMC5Sound.rawcontrol&0x40) && MMC5Sound.raw && !(FSettings.disabled&0x80))
    for(V=MMC5Sound.BC[2];V<timestamp;V++)
     WaveHi[V]+=MMC5Sound.raw<<5;
   MMC5Sound.BC[2]=timestamp;
} 


static DECLFW(Mapper5_SW)
{
 A&=0x1F;

 GameExpSound.HiFill=MMC5RunSoundHQ;

 switch(A)
 {
  case 0x10:if(psfun) psfun();MMC5Sound.rawcontrol=V;break;
  case 0x11:if(psfun) psfun();MMC5Sound.raw=V;break;

  case 0x0:
  case 0x4://printf("%04x:$%02x\n",A,V&0x30);
	   if(sfun) sfun(A>>2);
           MMC5Sound.env[A>>2]=V;
           break;
  case 0x2:
  case 0x6:if(sfun) sfun(A>>2);
           MMC5Sound.wl[A>>2]&=~0x00FF;
           MMC5Sound.wl[A>>2]|=V&0xFF;
           break;
  case 0x3:
  case 0x7://printf("%04x:$%02x\n",A,V>>3);
	   MMC5Sound.wl[A>>2]&=~0x0700;
           MMC5Sound.wl[A>>2]|=(V&0x07)<<8;           
	   MMC5Sound.running|=1<<(A>>2);
	   break;
  case 0x15:if(sfun)
            {
             sfun(0);
             sfun(1);
            }
	    MMC5Sound.running&=V;
            MMC5Sound.enable=V;
		//printf("%02x\n",V);
            break;
 }
}

static void Do5SQHQ(int P)
{
 static int tal[4]={1,2,4,6};
 int32 V,amp,rthresh,wl;
  
 wl=MMC5Sound.wl[P]+1;
 amp=((MMC5Sound.env[P]&0xF)<<8);
 rthresh=tal[(MMC5Sound.env[P]&0xC0)>>6];
     
 if(wl>=8 && (MMC5Sound.running&(P+1)) && !(FSettings.disabled&(0x20<<P)))
 {
  int dc,vc;
  
  wl<<=1;

  dc=MMC5Sound.dcount[P];   
  vc=MMC5Sound.vcount[P];
  for(V=MMC5Sound.BC[P];V<timestamp;V++)
  {
    if(dc<rthresh)
     WaveHi[V]+=amp;
    vc--;
    if(vc<=0)   /* Less than zero when first started. */
    {
     vc=wl;
     dc=(dc+1)&7;
    }
  }  
  MMC5Sound.dcount[P]=dc;
  MMC5Sound.vcount[P]=vc;
 }
 MMC5Sound.BC[P]=timestamp;
}

void MMC5RunSoundHQ(void)
{  
  Do5SQHQ(0);
  Do5SQHQ(1);
  Do5PCMHQ();
}

void MMC5HiSync(int32 ts)
{
 int x;
 for(x=0;x<3;x++) MMC5Sound.BC[x]=ts;
}

void Mapper5_ESI(void)
{
 GameExpSound.RChange=Mapper5_ESI;
 if(FSettings.SndRate)
 {
  sfun=Do5SQHQ;
  psfun=Do5PCMHQ;   
 }
 else
 {
  sfun=0;
  psfun=0;
 }
 memset(MMC5Sound.BC,0,sizeof(MMC5Sound.BC));
 memset(MMC5Sound.vcount,0,sizeof(MMC5Sound.vcount));
 GameExpSound.HiSync=MMC5HiSync;
}

void NSFMMC5_Init(void)
{
  memset(&MMC5Sound,0,sizeof(MMC5Sound));
  mul[0]=mul[1]=0;
  ExRAM=(uint8*)malloc(1024);
  Mapper5_ESI();
  SetWriteHandler(0x5c00,0x5fef,MMC5_ExRAMWr);
  SetReadHandler(0x5c00,0x5fef,MMC5_ExRAMRd); 
  SetWriteHandler(0x5000,0x5015,Mapper5_SW);
  SetWriteHandler(0x5205,0x5206,Mapper5_write);
  SetReadHandler(0x5205,0x5206,MMC5_read);
}

void NSFMMC5_Close(void)
{
 free(ExRAM);
 ExRAM=0;
}

