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

static void AYSoundHQ(void);
static void DoAYSQHQ(int x);

static uint8 sunindex;
static uint8 PSG[64];

static DECLFW(Mapper69_SWL)
{
  sunindex=V%14;
}

static DECLFW(Mapper69_SWH)
{
	     int x;
	     GameExpSound.HiFill=AYSoundHQ;
	     if(FSettings.SndRate);
             switch(sunindex)
             {
              case 0:
              case 1:
              case 8:DoAYSQHQ(0);break;
              case 2:
              case 3:
              case 9:DoAYSQHQ(1);break;
              case 4:
              case 5:
              case 10:DoAYSQHQ(2);break;
              case 7:
		     for(x=0;x<2;x++)
  		      DoAYSQHQ(x);
		     break;
             }
             PSG[sunindex]=V; 
}

static int32 vcount[3];
static int32 dcount[3];
static int CAYBC[3];

static void DoAYSQHQ(int x) 
{
 int32 V;
 int32 freq=((PSG[x<<1]|((PSG[(x<<1)+1]&15)<<8))+1)<<4;
 int32 amp=(PSG[0x8+x]&15)<<6;

 amp+=amp>>1;

 if(!(PSG[0x7]&(1<<x)) && !(FSettings.disabled&(0x20<<x)))
 {
  for(V=CAYBC[x];V<timestamp;V++)
  {
   if(dcount[x])
    WaveHi[V]+=amp;
   vcount[x]--;
   if(vcount[x]<=0)
   {
    dcount[x]^=1;
    vcount[x]=freq;
   }
  } 
 }
 CAYBC[x]=timestamp;
}

static void AYSoundHQ(void)
{
    DoAYSQHQ(0);
    DoAYSQHQ(1);
    DoAYSQHQ(2);
}

static void AYHiSync(int32 ts)
{
 int x;

 for(x=0;x<3;x++)
  CAYBC[x]=ts;
}

void Mapper69_ESI(void)
{
 GameExpSound.RChange=Mapper69_ESI;
 GameExpSound.HiSync=AYHiSync;
 memset(dcount,0,sizeof(dcount));
 memset(vcount,0,sizeof(vcount));
 memset(CAYBC,0,sizeof(CAYBC));
}

void NSFAY_Init(void)
{
 sunindex=0;
 SetWriteHandler(0xc000,0xdfff,Mapper69_SWL);
 SetWriteHandler(0xe000,0xffff,Mapper69_SWH);
 Mapper69_ESI();
}

