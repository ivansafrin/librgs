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

static void (*sfun[3])(void);

static uint8 VPSG2[8];
static uint8 VPSG[8];

static DECLFW(VRC6SW)
{
        A&=0xF003;
        if(A>=0x9000 && A<=0x9002)
        {
         VPSG[A&3]=V;
         if(sfun[0]) sfun[0]();
        }
        else if(A>=0xa000 && A<=0xa002)
        {
         VPSG[4|(A&3)]=V;
         if(sfun[1]) sfun[1]();
        }
        else if(A>=0xb000 && A<=0xb002)
        {
         VPSG2[A&3]=V;
         if(sfun[2]) sfun[2]();
	}

}

static int32 CVBC[3];
static int32 vcount[3];
static int32 dcount[2];

static INLINE void DoSQVHQ(int x)
{
 int32 V;
 int32 amp=((VPSG[x<<2]&15)<<8)*6/8;
     
 if(VPSG[(x<<2)|0x2]&0x80 && !(FSettings.disabled&(0x20<<x)))  
 {
  if(VPSG[x<<2]&0x80)
  {
   for(V=CVBC[x];V<timestamp;V++)
    WaveHi[V]+=amp;
  }
  else   
  {
   int32 thresh=(VPSG[x<<2]>>4)&7;
   for(V=CVBC[x];V<timestamp;V++)
   {
    if(dcount[x]>thresh)        /* Greater than, not >=.  Important. */
     WaveHi[V]+=amp;
    vcount[x]--;
    if(vcount[x]<=0)            /* Should only be <0 in a few circumstances. */
    {
     vcount[x]=(VPSG[(x<<2)|0x1]|((VPSG[(x<<2)|0x2]&15)<<8))+1;
     dcount[x]=(dcount[x]+1)&15;
    }
   }
  }
 }
 CVBC[x]=timestamp;
}
   
static void DoSQV1HQ(void)
{
 DoSQVHQ(0);
}

static void DoSQV2HQ(void)
{ 
 DoSQVHQ(1);
}
   
static void DoSawVHQ(void)
{
 static uint8 b3=0;
 static int32 phaseacc=0;
 int32 V;
 
 if(VPSG2[2]&0x80 && !(FSettings.disabled&0x80))
 {
  for(V=CVBC[2];V<timestamp;V++)
  {
   WaveHi[V]+=(((phaseacc>>3)&0x1f)<<8)*6/8;
   vcount[2]--;
   if(vcount[2]<=0)
   {
    vcount[2]=(VPSG2[1]+((VPSG2[2]&15)<<8)+1)<<1;
    phaseacc+=VPSG2[0]&0x3f;
    b3++;
    if(b3==7)
    {
     b3=0;
     phaseacc=0;
   }
 
   }
  }
 }
 CVBC[2]=timestamp;
}


void VRC6SoundHQ(void)
{
    DoSQV1HQ();
    DoSQV2HQ();
    DoSawVHQ();
}

void VRC6SyncHQ(int32 ts)
{
 int x;
 for(x=0;x<3;x++) CVBC[x]=ts;
}

static void VRC6_ESI(void)
{
        GameExpSound.RChange=VRC6_ESI;
        GameExpSound.HiFill=VRC6SoundHQ;
        GameExpSound.HiSync=VRC6SyncHQ;

	memset(CVBC,0,sizeof(CVBC));
	memset(vcount,0,sizeof(vcount));
	memset(dcount,0,sizeof(dcount));
	if(FSettings.SndRate)
	{
	 sfun[0]=DoSQV1HQ;
	 sfun[1]=DoSQV2HQ;
	 sfun[2]=DoSawVHQ;
	}
	else
	 memset(sfun,0,sizeof(sfun));
}

void NSFVRC6_Init(void)
{
	VRC6_ESI();
	SetWriteHandler(0x8000,0xbfff,VRC6SW);
}
