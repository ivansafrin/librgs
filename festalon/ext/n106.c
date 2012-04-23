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

static uint8 IRAM[128];

static uint8 dopol;

static void NamcoSoundHack(void);
static void DoNamcoSoundHQ(void);
static void SyncHQ(int32 ts);

static DECLFR(Namco_Read4800)
{
	uint8 ret=IRAM[dopol&0x7f];

	/* Maybe I should call NamcoSoundHack() here? */
	if(dopol&0x80)
	 dopol=(dopol&0x80)|((dopol+1)&0x7f);
	return ret;
}

static uint32 FreqCache[8];
static uint32 EnvCache[8];
static uint32 LengthCache[8];

static void FixCache(int a,int V)
{
                     int w=(a>>3)&0x7;
                     switch(a&0x07)
                     {
                      case 0x00:FreqCache[w]&=~0x000000FF;FreqCache[w]|=V;break;
                      case 0x02:FreqCache[w]&=~0x0000FF00;FreqCache[w]|=V<<8;break;
                      case 0x04:FreqCache[w]&=~0x00030000;FreqCache[w]|=(V&3)<<16;
                                LengthCache[w]=(8-((V>>2)&7))<<2;
                                break;
                      case 0x07:EnvCache[w]=(double)(V&0xF)*576716;break;
                     }

}

static DECLFW(Mapper19_write)
{
	A&=0xF800;

        switch(A)
	{
	 case 0x4800:
		   if(dopol&0x40)
                   {
                    if(FSettings.SndRate)
	  	    {
		     NamcoSoundHack();
		     GameExpSound.HiFill=DoNamcoSoundHQ;
		     GameExpSound.HiSync=SyncHQ;
		    }
		    FixCache(dopol,V);
                   }
		   IRAM[dopol&0x7f]=V;
		   
                   if(dopol&0x80)
                    dopol=(dopol&0x80)|((dopol+1)&0x7f);
                   break;

        case 0xf800: dopol=V;break;
        }
}

static void NamcoSoundHack(void)
{
 DoNamcoSoundHQ();
}

static uint32 PlayIndex[8];
static int32 vcount[8];
static int32 CVBC;

#define TOINDEX	(16+1)

// 16:15
static void SyncHQ(int32 ts)
{
 CVBC=ts;
}


/* Things to do:
	1	Read freq low
	2	Read freq mid
	3	Read freq high
	4	Read envelope
	...?
*/

static INLINE uint32 FetchDuff(uint32 P, uint32 envelope)
{
    uint32 duff;
    duff=IRAM[((IRAM[0x46+(P<<3)]+(PlayIndex[P]>>TOINDEX))&0xFF)>>1];
    if((IRAM[0x46+(P<<3)]+(PlayIndex[P]>>TOINDEX))&1)
     duff>>=4;
    duff&=0xF;
    duff=(duff*envelope)>>16;
    return(duff);
}

static void DoNamcoSoundHQ(void)
{
 int32 P,V;
 int32 cyclesuck=(((IRAM[0x7F]>>4)&7)+1)*15;

 for(P=7;P>=(7-((IRAM[0x7F]>>4)&7));P--)
 {
  if((IRAM[0x44+(P<<3)]&0xE0) && (IRAM[0x47+(P<<3)]&0xF) && !(FSettings.disabled&(0x20<<P)))
  {
   uint32 freq;
   int32 vco;
   uint32 duff2,lengo,envelope;

   vco=vcount[P];
   freq=FreqCache[P];
   envelope=EnvCache[P];
   lengo=LengthCache[P];

   duff2=FetchDuff(P,envelope);
   for(V=CVBC<<1;V<timestamp<<1;V++)
   {
    WaveHi[V>>1]+=duff2;
    if(!vco)
    {
     PlayIndex[P]+=freq;
     while((PlayIndex[P]>>TOINDEX)>=lengo)
      PlayIndex[P]-=lengo<<TOINDEX;
     duff2=FetchDuff(P,envelope);
     vco=cyclesuck;
    }
    vco--;
   }
   vcount[P]=vco;
  }
 }
 CVBC=timestamp;
}


void Mapper19_ESI(void);

static void M19SC(void)
{
 if(FSettings.SndRate)
  Mapper19_ESI();
}

void Mapper19_ESI(void)
{
  GameExpSound.RChange=M19SC;
  memset(vcount,0,sizeof(vcount));
  memset(PlayIndex,0,sizeof(PlayIndex));
  CVBC=0;
}

void NSFN106_Init(void)
{
  SetWriteHandler(0xf800,0xffff,Mapper19_write);
  SetWriteHandler(0x4800,0x4fff,Mapper19_write);
  SetReadHandler(0x4800,0x4fff,Namco_Read4800);
  Mapper19_ESI();
}

