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

#include <stdlib.h>
#include <stdio.h>

#include <string.h>

#include "types.h"
#include "x6502.h"

#include "nsf.h"
#include "sound.h"
#include "filter.h"

static uint32 wlookup1[32];
static uint32 wlookup2[203];

int32 Wave[2048+512];
int32 WaveHi[40000];
int32 WaveFinal[2048+512];

EXPSOUND GameExpSound={0,0,0};

static uint8 TriCount;
static uint8 TriMode;

static int32 tristep;

static int32 wlcount[4];	/* Wave length counters.	*/

static uint8 IRQFrameMode;	/* $4017 / xx000000 */
static uint8 PSG[0x10];
static uint8 RawDALatch;	/* $4011 0xxxxxxx */

uint8 EnabledChannels;		/* Byte written to $4015 */

typedef struct {
	uint8 Speed;
	uint8 Mode;	/* Fixed volume(1), and loop(2) */
	uint8 DecCountTo1;
	uint8 decvolume;
	int reloaddec;
} ENVUNIT;

static ENVUNIT EnvUnits[3];

static const int RectDuties[4]={1,2,4,6};

static int32 RectDutyCount[2];
static uint8 sweepon[2];
static int32 curfreq[2];
static uint8 SweepCount[2];

static uint16 nreg;  

static uint8 fcnt;
static int32 fhcnt;
static int32 fhinc;

static int32 lengthcount[4]; 
static const uint8 lengthtable[0x20]=
{
 0x5*2,0x7f*2,0xA*2,0x1*2,0x14*2,0x2*2,0x28*2,0x3*2,0x50*2,0x4*2,0x1E*2,0x5*2,0x7*2,0x6*2,0x0E*2,0x7*2,
 0x6*2,0x08*2,0xC*2,0x9*2,0x18*2,0xa*2,0x30*2,0xb*2,0x60*2,0xc*2,0x24*2,0xd*2,0x8*2,0xe*2,0x10*2,0xf*2
};

static const uint32 NoiseFreqTable[0x10]=
{
 2,4,8,0x10,0x20,0x30,0x40,0x50,0x65,0x7f,0xbe,0xfe,0x17d,0x1fc,0x3f9,0x7f2
};

static const uint32 NTSCDMCTable[0x10]=
{
 428,380,340,320,286,254,226,214,
 190,160,142,128,106, 84 ,72,54
};

static const uint32 PALDMCTable[0x10]=
{
 397, 353, 315, 297, 265, 235, 209, 198, 
 176, 148, 131, 118, 98, 78, 66, 50, 
};

// $4010        -        Frequency
// $4011        -        Actual data outputted
// $4012        -        Address register: $c000 + V*64
// $4013        -        Size register:  Size in bytes = (V+1)*64

static int32 DMCacc=1;
static int32 DMCPeriod;
static uint8 DMCBitCount=0;

static uint8 DMCAddressLatch,DMCSizeLatch; /* writes to 4012 and 4013 */
static uint8 DMCFormat;	/* Write to $4010 */

static uint32 DMCAddress=0;
static int32 DMCSize=0;
static uint8 DMCShift=0; 
static uint8 SIRQStat=0;

static int DMCHaveDMA=0;
static uint8 DMCDMABuf=0;  
static int DMCHaveSample=0;

static void Dummyfunc(void) {};
static void (*DoNoise)(void)=Dummyfunc;
static void (*DoTriangle)(void)=Dummyfunc;
static void (*DoPCM)(void)=Dummyfunc;
static void (*DoSQ1)(void)=Dummyfunc;
static void (*DoSQ2)(void)=Dummyfunc;

static uint32 ChannelBC[5];

static void LoadDMCPeriod(uint8 V)
{
 if(PAL)
  DMCPeriod=PALDMCTable[V];
 else
  DMCPeriod=NTSCDMCTable[V];
}

static void PrepDPCM()
{
 DMCAddress=0x4000+(DMCAddressLatch<<6);
 DMCSize=(DMCSizeLatch<<4)+1;
}

/* Instantaneous?  Maybe the new freq value is being calculated all of the time... */

static int FASTAPASS(2) CheckFreq(uint32 cf, uint8 sr)
{
 uint32 mod;
 if(!(sr&0x8))
 {
  mod=cf>>(sr&7);
  if((mod+cf)&0x800)
   return(0);
 }
 return(1);
}

static void SQReload(int x, uint8 V)
{
           if(EnabledChannels&(1<<x))
           {           
            if(x)
             DoSQ2();
            else
             DoSQ1();
            lengthcount[x]=lengthtable[(V>>3)&0x1f];
	   }

           sweepon[x]=PSG[(x<<2)|1]&0x80;
           curfreq[x]=PSG[(x<<2)|0x2]|((V&7)<<8);
           SweepCount[x]=((PSG[(x<<2)|0x1]>>4)&7)+1;           

           RectDutyCount[x]=7;
	   EnvUnits[x].reloaddec=1;
	   //reloadfreq[x]=1;
}

static DECLFW(Write_PSG)
{
 A&=0x1F;
 switch(A)
 {
  case 0x0:DoSQ1();
	   EnvUnits[0].Mode=(V&0x30)>>4;
	   EnvUnits[0].Speed=(V&0xF);
           break;
  case 0x1:
           sweepon[0]=V&0x80;
           break;
  case 0x2:
           DoSQ1();
           curfreq[0]&=0xFF00;
           curfreq[0]|=V;
           break;
  case 0x3:
           SQReload(0,V);
           break;
  case 0x4:           
	   DoSQ2();
           EnvUnits[1].Mode=(V&0x30)>>4;
           EnvUnits[1].Speed=(V&0xF);
	   break;
  case 0x5:        
          sweepon[1]=V&0x80;
          break;
  case 0x6:DoSQ2();
          curfreq[1]&=0xFF00;
          curfreq[1]|=V;
          break;
  case 0x7:          
          SQReload(1,V);
          break;
  case 0xa:DoTriangle();
	   break;
  case 0xb:
          DoTriangle();
	  if(EnabledChannels&0x4)
           lengthcount[2]=lengthtable[(V>>3)&0x1f];
	  TriMode=1;	// Load mode
          break;
  case 0xC:DoNoise();
           EnvUnits[2].Mode=(V&0x30)>>4;
           EnvUnits[2].Speed=(V&0xF);
           break;
  case 0xE:DoNoise();
           break;
  case 0xF:
	   DoNoise();
           if(EnabledChannels&0x8)
	    lengthcount[3]=lengthtable[(V>>3)&0x1f];
	   EnvUnits[2].reloaddec=1;
           break;
 case 0x10:DoPCM();
	   LoadDMCPeriod(V&0xF);

	   if(SIRQStat&0x80)
	   {
	    if(!(V&0x80))
	    {
	     X6502_IRQEnd(FCEU_IQDPCM);
 	     SIRQStat&=~0x80;
	    }
            else X6502_IRQBegin(FCEU_IQDPCM);
	   }
	   break;
 }
 PSG[A]=V;
}

static DECLFW(Write_DMCRegs)
{
 A&=0xF;

 switch(A)
 {
  case 0x00:DoPCM();
            LoadDMCPeriod(V&0xF);

            if(SIRQStat&0x80)
            {
             if(!(V&0x80))
             { 
              X6502_IRQEnd(FCEU_IQDPCM);
              SIRQStat&=~0x80;
             }
             else X6502_IRQBegin(FCEU_IQDPCM);
            }
	    DMCFormat=V;
	    break;
  case 0x01:DoPCM();
	    RawDALatch=V&0x7F;
	    break;
  case 0x02:DMCAddressLatch=V;break;
  case 0x03:DMCSizeLatch=V;break;
 }


}

static DECLFW(StatusWrite)
{
	int x;

        DoSQ1();
        DoSQ2();
        DoTriangle();
        DoNoise();
        DoPCM();
        for(x=0;x<4;x++)
         if(!(V&(1<<x))) lengthcount[x]=0;   /* Force length counters to 0. */

        if(V&0x10)
        {
         if(!DMCSize)
          PrepDPCM();
        }
	else
	{
	 DMCSize=0;
	}
	SIRQStat&=~0x80;
        X6502_IRQEnd(FCEU_IQDPCM);
	EnabledChannels=V&0x1F;
}

static DECLFR(StatusRead)
{
   int x;
   uint8 ret;

   ret=SIRQStat;

   for(x=0;x<4;x++) ret|=lengthcount[x]?(1<<x):0;
   if(DMCSize) ret|=0x10;

   SIRQStat&=~0x40;
   X6502_IRQEnd(FCEU_IQFCOUNT);

   return ret;
}

static void FASTAPASS(1) FrameSoundStuff(int V)
{
 int P;

 DoSQ1();
 DoSQ2();
 DoNoise();
 DoTriangle();

 if(!(V&1)) /* Envelope decay, linear counter, length counter, freq sweep */
 {
  if(!(PSG[8]&0x80))
   if(lengthcount[2]>0)
    lengthcount[2]--;

  if(!(PSG[0xC]&0x20))	/* Make sure loop flag is not set. */
   if(lengthcount[3]>0)
    lengthcount[3]--;

  for(P=0;P<2;P++)
  {
   if(!(PSG[P<<2]&0x20))	/* Make sure loop flag is not set. */
    if(lengthcount[P]>0)
     lengthcount[P]--;            

   /* Frequency Sweep Code Here */
   /* xxxx 0000 */
   /* xxxx = hz.  120/(x+1)*/
   if(sweepon[P])
   {
    int32 mod=0;

    if(SweepCount[P]>0) SweepCount[P]--; 
    if(SweepCount[P]<=0)
    {
     SweepCount[P]=((PSG[(P<<2)+0x1]>>4)&7)+1; //+1;
     if(PSG[(P<<2)+0x1]&0x8)
     {
      mod-=(P^1)+((curfreq[P])>>(PSG[(P<<2)+0x1]&7));          
      if(curfreq[P] && (PSG[(P<<2)+0x1]&7)/* && sweepon[P]&0x80*/)
      {
       curfreq[P]+=mod;
      }
     }
     else
     {
      mod=curfreq[P]>>(PSG[(P<<2)+0x1]&7);
      if((mod+curfreq[P])&0x800)
      {
       sweepon[P]=0;
       curfreq[P]=0;
      }
      else
      {
       if(curfreq[P] && (PSG[(P<<2)+0x1]&7)/* && sweepon[P]&0x80*/)
       {
        curfreq[P]+=mod;
       }
      }     
     }
    }
   }
   else	/* Sweeping is disabled: */
   {
    //curfreq[P]&=0xFF00;
    //curfreq[P]|=PSG[(P<<2)|0x2]; //|((PSG[(P<<2)|3]&7)<<8); 
   }
  }
 }

 /* Now do envelope decay + linear counter. */

  if(TriMode) // In load mode?
   TriCount=PSG[0x8]&0x7F;
  else if(TriCount)
   TriCount--;

  if(!(PSG[0x8]&0x80))
   TriMode=0;

  for(P=0;P<3;P++)
  {
   if(EnvUnits[P].reloaddec)
   {
    EnvUnits[P].decvolume=0xF;
    EnvUnits[P].DecCountTo1=EnvUnits[P].Speed+1;
    EnvUnits[P].reloaddec=0;
    continue;
   }

   if(EnvUnits[P].DecCountTo1>0) EnvUnits[P].DecCountTo1--;
   if(EnvUnits[P].DecCountTo1==0)
   {
    EnvUnits[P].DecCountTo1=EnvUnits[P].Speed+1;
    if(EnvUnits[P].decvolume || (EnvUnits[P].Mode&0x2))
    {
     EnvUnits[P].decvolume--;
     EnvUnits[P].decvolume&=0xF;
    }
   }
  }
}

void FrameSoundUpdate(void)
{
 // Linear counter:  Bit 0-6 of $4008
 // Length counter:  Bit 4-7 of $4003, $4007, $400b, $400f

 if(!fcnt && !(IRQFrameMode&0x3))
 {
         SIRQStat|=0x40;
         X6502_IRQBegin(FCEU_IQFCOUNT);
 }

 if(fcnt==3)
 {
	if(IRQFrameMode&0x2)
	 fhcnt+=fhinc;
 }
 FrameSoundStuff(fcnt);
 fcnt=(fcnt+1)&3;
}


static INLINE void tester(void)
{
 if(DMCBitCount==0)
 {
  if(!DMCHaveDMA)
   DMCHaveSample=0;
  else
  {
   DMCHaveSample=1;
   DMCShift=DMCDMABuf;
   DMCHaveDMA=0;
  }
 }    
}

static INLINE void DMCDMA(void)
{
  if(DMCSize && !DMCHaveDMA)
  {
   X6502_DMR(0x8000+DMCAddress);
   X6502_DMR(0x8000+DMCAddress);
   X6502_DMR(0x8000+DMCAddress);
   DMCDMABuf=X6502_DMR(0x8000+DMCAddress);
   DMCHaveDMA=1;
   DMCAddress=(DMCAddress+1)&0x7fff;
   DMCSize--;
   if(!DMCSize)
   {
    if(DMCFormat&0x40)
     PrepDPCM();
    else
    {
     SIRQStat|=0x80;
     if(DMCFormat&0x80)
      X6502_IRQBegin(FCEU_IQDPCM);
    }
   }
 }
}

void FASTAPASS(1) FCEU_SoundCPUHook(int cycles)
{
 fhcnt-=cycles*48;
 if(fhcnt<=0)
 {
  FrameSoundUpdate();
  fhcnt+=fhinc;
 }

 DMCDMA();
 DMCacc-=cycles;

 while(DMCacc<=0)
 {
  if(DMCHaveSample)
  {
   uint8 bah=RawDALatch;
   int t=((DMCShift&1)<<2)-2;

   /* Unbelievably ugly hack */ 
   if(FSettings.SndRate)
   {
    timestamp+=DMCacc;
    DoPCM();
    timestamp-=DMCacc;
   }
   RawDALatch+=t;
   if(RawDALatch&0x80)
    RawDALatch=bah;
  }

  DMCacc+=DMCPeriod;
  DMCBitCount=(DMCBitCount+1)&7;
  DMCShift>>=1;  
  tester();
 }
}

void RDoPCM(void)
{
 int32 V;

 if(!(FSettings.disabled&0x10))
  for(V=ChannelBC[4];V<timestamp;V++)
   WaveHi[V]+=RawDALatch<<16;

 ChannelBC[4]=timestamp;
}

/* This has the correct phase.  Don't mess with it. */
static INLINE void RDoSQ(int x)
{
   int32 V;
   int32 amp;
   int32 rthresh;
   int32 *D;
   int32 currdc;
   int32 cf;
   int32 rc;

   if(FSettings.disabled&(1<<x)) 
    goto endit;

   if(curfreq[x]<8 || curfreq[x]>0x7ff)
    goto endit;
   if(!CheckFreq(curfreq[x],PSG[(x<<2)|0x1]))
    goto endit;
   if(!lengthcount[x])
    goto endit;

   if(EnvUnits[x].Mode&0x1)
    amp=EnvUnits[x].Speed;
   else
    amp=EnvUnits[x].decvolume;
//   printf("%d\n",amp);
   amp<<=24;

   rthresh=RectDuties[(PSG[(x<<2)]&0xC0)>>6];

   D=&WaveHi[ChannelBC[x]];
   V=timestamp-ChannelBC[x];
   
   currdc=RectDutyCount[x];
   cf=(curfreq[x]+1)*2;
   rc=wlcount[x];

   while(V>0)
   {
    if(currdc<rthresh)
     *D+=amp;
    rc--;
    if(!rc)
    {
     rc=cf;
     currdc=(currdc+1)&7;
    }
    V--;
    D++;
   }   
  
   RectDutyCount[x]=currdc;
   wlcount[x]=rc;

   endit:
   ChannelBC[x]=timestamp;
}

static void RDoSQ1(void)
{
 RDoSQ(0);
}

static void RDoSQ2(void)
{
 RDoSQ(1);
}

static void RDoTriangle(void)
{
 int32 V;
 int32 tcout;

 tcout=(tristep&0xF);
 if(!(tristep&0x10)) tcout^=0xF;
 tcout=tcout*3;	//(tcout<<1);

 if(FSettings.disabled&4)       
 {
  ChannelBC[2]=timestamp;
  return;
 }

 if(!lengthcount[2] || !TriCount)
 {   				/* Counter is halted, but we still need to output. */
  for(V=ChannelBC[2];V<timestamp;V++)
   WaveHi[V]+=tcout<<16;
 }
 else
  for(V=ChannelBC[2];V<timestamp;V++)
  {
    WaveHi[V]+=tcout<<16;
    wlcount[2]--;
    if(!wlcount[2])
    {
     wlcount[2]=(PSG[0xa]|((PSG[0xb]&7)<<8))+1;
     tristep++;
     tcout=(tristep&0xF);
     if(!(tristep&0x10)) tcout^=0xF;
     tcout=tcout*3;	//(tcout<<1);
    }
  }

 ChannelBC[2]=timestamp;
}

static void RDoNoise(void)
{
 int32 V;
 int32 outo;
 uint32 amptab[2];

 if(EnvUnits[2].Mode&0x1)
  amptab[0]=EnvUnits[2].Speed;
 else
  amptab[0]=EnvUnits[2].decvolume;

 amptab[0]<<=16;
 amptab[1]=0;

 amptab[0]<<=1;

 if(FSettings.disabled&8) 
 {
  ChannelBC[3]=timestamp;
  return;
 }

 outo=amptab[nreg&1]; //(nreg>>0xe)&1];

 if(!lengthcount[3])
 {
  //for(V=ChannelBC[3];V<timestamp;V++) 
  // WaveHi[V]+=outo;  
  //ChannelBC[3]=timestamp;
  //return;
  outo=amptab[0]=0;
 }

 if(PSG[0xE]&0x80)        // "short" noise
  for(V=ChannelBC[3];V<timestamp;V++)
  {
   WaveHi[V]+=outo;
   wlcount[3]--;
   if(!wlcount[3])
   {
    uint8 feedback;
    wlcount[3]=NoiseFreqTable[PSG[0xE]&0xF]<<1;
    feedback=((nreg>>8)&1)^((nreg>>14)&1);
    nreg=(nreg<<1)+feedback;
    nreg&=0x7fff;
    outo=amptab[(nreg>>0xe)&1];
   }
  }
 else
  for(V=ChannelBC[3];V<timestamp;V++)
  {
   WaveHi[V]+=outo;
   wlcount[3]--;
   if(!wlcount[3])
   {
    uint8 feedback;
    wlcount[3]=NoiseFreqTable[PSG[0xE]&0xF]<<1;
    feedback=((nreg>>13)&1)^((nreg>>14)&1);
    nreg=(nreg<<1)+feedback;
    nreg&=0x7fff;
    outo=amptab[(nreg>>0xe)&1];
   }
  }
 ChannelBC[3]=timestamp;
}

static DECLFW(Write_IRQFM)
{
 V=(V&0xC0)>>6;
 fcnt=0;
 if(V&0x2)  
  FrameSoundUpdate();
 fcnt=1;
 fhcnt=fhinc;
 X6502_IRQEnd(FCEU_IQFCOUNT);
 SIRQStat&=~0x40;
 IRQFrameMode=V;
}

void SetNESSoundMap(void)
{ 
  SetWriteHandler(0x4000,0x400F,Write_PSG);
  SetWriteHandler(0x4010,0x4013,Write_DMCRegs);
  SetWriteHandler(0x4017,0x4017,Write_IRQFM);

  SetWriteHandler(0x4015,0x4015,StatusWrite);
  SetReadHandler(0x4015,0x4015,StatusRead);
}

static int32 inbuf=0;
static uint32 lastpoo=0;
int FlushEmulateSound(void)
{
  int x;
  int32 end,left;

  if(!timestamp) return(0);

  if(!FSettings.SndRate)
  {
   left=0;
   end=0;
   goto nosoundo;
  }

  DoSQ1();
  DoSQ2();
  DoTriangle();
  DoNoise();
  DoPCM();

  {
   int32 *tmpo=&WaveHi[lastpoo];

   if(GameExpSound.HiFill) GameExpSound.HiFill();

   for(x=timestamp-lastpoo;x;x--)
   {
    uint32 b=*tmpo;    
    *tmpo=(b&65535)+wlookup2[(b>>16)&255]+wlookup1[b>>24];
    tmpo++;
   }
   end=NeoFilterSound(WaveHi,WaveFinal,timestamp,&left);

   memmove(WaveHi,WaveHi+timestamp-left,left*sizeof(uint32));
   memset(WaveHi+left,0,sizeof(WaveHi)-left*sizeof(uint32));

   if(GameExpSound.HiSync) GameExpSound.HiSync(left);
   for(x=0;x<5;x++)
    ChannelBC[x]=left;
  }
  nosoundo:

  {
   timestampbase+=timestamp;
   timestamp=left;
   timestampbase-=timestamp;
   lastpoo=timestamp;
  }
  inbuf=end;

  return(end);
}

/* FIXME:  Find out what sound registers get reset on reset.  I know $4001/$4005 don't,
due to that whole MegaMan 2 Game Genie thing.
*/

void FCEUSND_Reset(void)
{
        int x;

	IRQFrameMode=0x0;
        fhcnt=fhinc;
        fcnt=0;

        nreg=1;
        for(x=0;x<2;x++)
	{
         wlcount[x]=2048;
	 sweepon[x]=0;
	 curfreq[x]=0;
	}
        wlcount[2]=1;	//2048;
        wlcount[3]=2048;
	DMCHaveDMA=DMCHaveSample=0;
	SIRQStat=0x00;

	RawDALatch=0x00;
	TriCount=0;
	TriMode=0;
	tristep=0;
	EnabledChannels=0;
	for(x=0;x<4;x++)
	 lengthcount[x]=0;

	DMCAddressLatch=0;
	DMCSizeLatch=0;
	DMCFormat=0;
	DMCAddress=0;
	DMCSize=0;
	DMCShift=0;
}

void FCEUSND_Power(void)
{
        int x;
 
        SetNESSoundMap();
        memset(PSG,0x00,sizeof(PSG));
	FCEUSND_Reset();

	memset(Wave,0,sizeof(Wave));
        memset(WaveHi,0,sizeof(WaveHi));
	memset(&EnvUnits,0,sizeof(EnvUnits));

        for(x=0;x<5;x++)
         ChannelBC[x]=0;
        lastpoo=0;
        LoadDMCPeriod(DMCFormat&0xF);
}


void SetSoundVariables(void)
{
  int x;  

  fhinc=PAL?16626:14915;	// *2 CPU clock rate
  fhinc*=24;

  if(FSettings.SndRate)
  {
   wlookup1[0]=0;
   for(x=1;x<32;x++)
   {
    wlookup1[x]=(double)16*16*16*4*95.52/((double)8128/(double)x+100);
   }
   wlookup2[0]=0;
   for(x=1;x<203;x++)
   {
    wlookup2[x]=(double)16*16*16*4*163.67/((double)24329/(double)x+100);
   }
   DoNoise=RDoNoise;
   DoTriangle=RDoTriangle;
   DoPCM=RDoPCM;
   DoSQ1=RDoSQ1;
   DoSQ2=RDoSQ2;
  }  
  else
  {
   DoNoise=DoTriangle=DoPCM=DoSQ1=DoSQ2=Dummyfunc;
   return;
  }

  MakeFilters(FSettings.SndRate);

  if(GameExpSound.RChange)
   GameExpSound.RChange();

  memset(ChannelBC,0,sizeof(ChannelBC));

  LoadDMCPeriod(DMCFormat&0xF);	// For changing from PAL to NTSC
}

void FESTAI_Sound(int Rate)
{
 FSettings.SndRate=Rate;
 SetSoundVariables();
}

void FESTAI_SetSoundQuality(int q)
{
 FSettings.soundq=q;
}

void FESTAI_Disable(int t)
{
 FSettings.disabled=t;
}

