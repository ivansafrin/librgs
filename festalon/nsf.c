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
#include <math.h>

#include "types.h"
#include "x6502.h"
#include "sound.h"
#include "nsf.h"
#include "memory.h"
#include "cart.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

FCEUS FSettings;

uint8 RAM[0x800];
uint8 PAL=0;

static DECLFW(BRAML)
{
        RAM[A&0x7FF]=V;
}
             
static DECLFR(ARAML)
{
        return RAM[A&0x7FF];
}

void ResetMapping(void)
{
        SetReadHandler(0x0000,0xFFFF,0);
        SetWriteHandler(0x0000,0xFFFF,0);

        SetReadHandler(0,0x1FFF,ARAML);
        SetWriteHandler(0,0x1FFF,BRAML);
        SetNESSoundMap();
}


static uint8 SongReload;
static int CurrentSong;

static DECLFW(NSF_write);
static DECLFR(NSF_read);

static uint8 NSFROM[0x30+6]=
{
/* 0x00 - NMI */
0x8D,0xF4,0x3F,                         /* Stop play routine NMIs. */
0xA2,0xFF,0x9A,                         /* Initialize the stack pointer. */
0xAD,0xF0,0x3F,                         /* See if we need to init. */
0xF0,0x09,                              /* If 0, go to play routine playing. */

0xAD,0xF1,0x3F,                         /* Confirm and load A      */
0xAE,0xF3,0x3F,                         /* Load X with PAL/NTSC byte */

0x20,0x00,0x00,                         /* JSR to init routine     */

0xA9,0x00,
0xAA,
0xA8,
0x20,0x00,0x00,                         /* JSR to play routine  */
0x8D,0xF5,0x3F,				/* Start play routine NMIs. */
0x90,0xFE,                               /* Loopie time. */

/* 0x20 */
0x8D,0xF3,0x3F,				/* Init init NMIs */
0x18,
0x90,0xFE				/* Loopie time. */
};

static DECLFR(NSFROMRead)
{
 return (NSFROM-0x3800)[A];
}

static int doreset=0;
static int NSFNMIFlags;
static uint8 *NSFDATA=0;
static int NSFMaxBank;

static int NSFSize;
static uint8 BSon;
static uint16 PlayAddr;
static uint16 InitAddr;
static uint16 LoadAddr;

static NSF_HEADER NSFHeader;

void NSFMMC5_Close(void);
void NSFVRC7_Close(void);

static uint8 *ExWRAM=0;

void FESTAI_Close(void)
{
  if(NSFDATA) {free(NSFDATA);NSFDATA=0;}
  if(ExWRAM) {free(ExWRAM);ExWRAM=0;}
  if(NSFHeader.SoundChip&1) {
//   NSFVRC6_Init();
  } else if (NSFHeader.SoundChip&2) {
   NSFVRC7_Close();
  } else if (NSFHeader.SoundChip&4) {
//   FDSSoundReset();
  } else if (NSFHeader.SoundChip&8) { 
   NSFMMC5_Close();
  } else if (NSFHeader.SoundChip&0x10) {
//   NSFN106_Init();
  } else if (NSFHeader.SoundChip&0x20) {
//   NSFAY_Init();
  }
}

// First 32KB is reserved for sound chip emulation in the iNES mapper code.

static INLINE void BANKSET(uint32 A, uint32 bank)
{
 bank&=NSFMaxBank;
 if(NSFHeader.SoundChip&4)
  memcpy(ExWRAM+(A-0x6000),NSFDATA+(bank<<12),4096);
 else 
  setprg4(A,bank);
}

static uint32 uppow2(uint32 n)
{
 int x;

 for(x=31;x>=0;x--)
  if(n&(1<<x))
  {
   if((1<<x)!=n)   
    return(1<<(x+1));
   break;
  }
 return n;
}


NSF_HEADER *FESTAI_Load(uint8 *buf, uint32 size)
{
  int x;

  memcpy(&NSFHeader,buf,0x80);
  buf+=0x80;
  if (memcmp(NSFHeader.ID,"NESM\x1a",5))
                  return 0;
  NSFHeader.SongName[31]=NSFHeader.Artist[31]=NSFHeader.Copyright[31]=0;

  LoadAddr=NSFHeader.LoadAddressLow;
  LoadAddr|=NSFHeader.LoadAddressHigh<<8;

  if(LoadAddr<0x6000)
   return(0);

  InitAddr=NSFHeader.InitAddressLow;
  InitAddr|=NSFHeader.InitAddressHigh<<8;

  PlayAddr=NSFHeader.PlayAddressLow;
  PlayAddr|=NSFHeader.PlayAddressHigh<<8;

  NSFSize=size-0x80;

  NSFMaxBank=((NSFSize+(LoadAddr&0xfff)+4095)/4096);
  NSFMaxBank=uppow2(NSFMaxBank);

  if(!(NSFDATA=(uint8 *)malloc(NSFMaxBank*4096)))
   return 0;

  memset(NSFDATA,0x00,NSFMaxBank*4096);
  memcpy(NSFDATA+(LoadAddr&0xfff),buf,NSFSize);

  NSFMaxBank--;

  BSon=0;
  for(x=0;x<8;x++)
   BSon|=NSFHeader.BankSwitch[x];

 for(x=0;;x++)
 {
  if(NSFROM[x]==0x20)
  {
   NSFROM[x+1]=InitAddr&0xFF;
   NSFROM[x+2]=InitAddr>>8;
   NSFROM[x+8]=PlayAddr&0xFF;
   NSFROM[x+9]=PlayAddr>>8;
   break;
  }
 }

 memset(&GameExpSound,0,sizeof(GameExpSound));
 if(NSFHeader.VideoSystem==0)
  PAL=0;
 else if(NSFHeader.VideoSystem==1)
  PAL=1;
 SetSoundVariables();

 if(NSFHeader.SoundChip)
 {
  for(x=0;x<6;x++)
   if(NSFHeader.SoundChip&(1<<x))
   {
    NSFHeader.SoundChip=1<<x;	/* Prevent confusing weirdness if more than one bit is set. */
    break;
   }
 }

 if(NSFHeader.SoundChip&4)
  ExWRAM=(uint8*)malloc(32768+8192);
 else
  ExWRAM=(uint8*)malloc(8192);


 memset(RAM,0,sizeof(RAM));
 ResetMapping();
 NSF_init();
 X6502_Init();
 X6502_Power();
 FCEUSND_Power();
 timestampbase=0;
 return(&NSFHeader);
}

static DECLFR(NSFVectorRead)
{
 if(((NSFNMIFlags&1) && SongReload) || (NSFNMIFlags&2) || doreset)
 {
  if(A==0xFFFA) return(0x00);
  else if(A==0xFFFB) return(0x38);
  else if(A==0xFFFC) return(0x20);
  else if(A==0xFFFD) {doreset=0;return(0x38);}
  return(X.DB);
 }
 else
  return(CartBR(A));
}

void NSFVRC6_Init(void);
void NSFVRC7_Init(void);
void NSFMMC5_Init(void);
void NSFN106_Init(void);
void NSFAY_Init(void);
void NSFFDS_Init(void);

void NSF_init(void)
{
  doreset=1;

  ResetCartMapping();
  if(NSFHeader.SoundChip&4)
  {
   SetupCartPRGMapping(0,ExWRAM,32768+8192,1);
   setprg32(0x6000,0);
   setprg8(0xE000,4);
   memset(ExWRAM,0x00,32768+8192);
   SetWriteHandler(0x6000,0xDFFF,CartBW);
   SetReadHandler(0x6000,0xFFFF,CartBR);
  }
  else
  {
   memset(ExWRAM,0x00,8192);
   SetReadHandler(0x6000,0x7FFF,CartBR);
   SetWriteHandler(0x6000,0x7FFF,CartBW);
   SetupCartPRGMapping(0,NSFDATA,((NSFMaxBank+1)*4096),0);
   SetupCartPRGMapping(1,ExWRAM,8192,1);
   setprg8r(1,0x6000,0);
   SetReadHandler(0x8000,0xFFFF,CartBR);
  }

  if(BSon)
  {
   int32 x;
   for(x=0;x<8;x++)
   {
    if(NSFHeader.SoundChip&4 && x>=6)
     BANKSET(0x6000+(x-6)*4096,NSFHeader.BankSwitch[x]);
    BANKSET(0x8000+x*4096,NSFHeader.BankSwitch[x]);
   }
  }
  else
  {
   int32 x;
   for(x=(LoadAddr&0xF000);x<0x10000;x+=0x1000)
    BANKSET(x,((x-(LoadAddr&0x7000))>>12));
  }

  SetReadHandler(0xFFFA,0xFFFD,NSFVectorRead);

  SetWriteHandler(0x2000,0x3fff,0);
  SetReadHandler(0x2000,0x37ff,0);
  SetReadHandler(0x3836,0x3FFF,0);
  SetReadHandler(0x3800,0x3835,NSFROMRead);

  SetWriteHandler(0x5ff6,0x5fff,NSF_write);

  SetWriteHandler(0x3ff0,0x3fff,NSF_write);
  SetReadHandler(0x3ff0,0x3fff,NSF_read);


  if(NSFHeader.SoundChip&1) { 
   NSFVRC6_Init();
  } else if (NSFHeader.SoundChip&2) {
   NSFVRC7_Init();
  } else if (NSFHeader.SoundChip&4) {
   NSFFDS_Init();
  } else if (NSFHeader.SoundChip&8) {
   NSFMMC5_Init();
  } else if (NSFHeader.SoundChip&0x10) {
   NSFN106_Init();
  } else if (NSFHeader.SoundChip&0x20) {
   NSFAY_Init();
  }
  CurrentSong=NSFHeader.StartingSong;
  SongReload=0xFF;
  NSFNMIFlags=0;
}

static DECLFW(NSF_write)
{
 switch(A)
 {
  case 0x3FF3:NSFNMIFlags|=1;break;
  case 0x3FF4:NSFNMIFlags&=~2;break;
  case 0x3FF5:NSFNMIFlags|=2;break;

  case 0x5FF6:
  case 0x5FF7:if(!(NSFHeader.SoundChip&4)) return;
  case 0x5FF8:
  case 0x5FF9:
  case 0x5FFA:
  case 0x5FFB:
  case 0x5FFC:
  case 0x5FFD:
  case 0x5FFE:
  case 0x5FFF:if(!BSon) return;
              A&=0xF;
              BANKSET((A*4096),V);
  	      break;
 } 
}

static DECLFR(NSF_read)
{
 int x;

 switch(A)
 {
 case 0x3ff0:x=SongReload;
	     SongReload=0;
	     return x;
 case 0x3ff1:
             memset(RAM,0x00,0x800);

             BWrite[0x4015](0x4015,0x0);
             for(x=0;x<0x14;x++)
              BWrite[0x4000+x](0x4000+x,0);
             BWrite[0x4015](0x4015,0xF);

	     if(NSFHeader.SoundChip&4) 
	     {
	      BWrite[0x4017](0x4017,0xC0);	/* FDS BIOS writes $C0 */
	      BWrite[0x4089](0x4089,0x80);
	      BWrite[0x408A](0x408A,0xE8);
	     }
	     else 
	     {
	      memset(ExWRAM,0x00,8192);
	      BWrite[0x4017](0x4017,0xC0);
              BWrite[0x4017](0x4017,0xC0);
              BWrite[0x4017](0x4017,0x40);
	     }

             if(BSon)
             {
              for(x=0;x<8;x++)
	       BANKSET(0x8000+x*4096,NSFHeader.BankSwitch[x]);
             }
             return (CurrentSong-1);
 case 0x3FF3:return PAL;
 }
 return 0;
}

void DoNSFFrame(void)
{
 if(((NSFNMIFlags&1) && SongReload) || (NSFNMIFlags&2))
 {
  TriggerNMI();
 }
}

int FCEUI_NSFChange(int amount)
{
   CurrentSong+=amount;
   if(CurrentSong<1) CurrentSong=1;
   else if(CurrentSong>NSFHeader.TotalSongs) CurrentSong=NSFHeader.TotalSongs;
   SongReload=0xFF;

   return(CurrentSong);
}

int FESTAI_NSFControl(int z, int o)
{
 if(o)
  CurrentSong=z;
 else
  CurrentSong+=z;  

 if(CurrentSong<1) CurrentSong=1;
 else if(CurrentSong>NSFHeader.TotalSongs) CurrentSong=NSFHeader.TotalSongs;

 SongReload=0xFF;

 return(CurrentSong);
}

int32 *FESTAI_Emulate(int *Count)
{
 static int doodoo=0;

 if(PAL)
  X6502_Run(312*(256+85)-doodoo);
 else
  X6502_Run(262*(256+85)-doodoo);
 doodoo^=1;

 DoNSFFrame();
 *Count=FlushEmulateSound();
 return(WaveFinal);
}
