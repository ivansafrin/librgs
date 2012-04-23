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
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sched.h>
#include <sys/soundcard.h>

#include "../driver.h"

static int format;
static int dspfd;

// fsize is in samples, not bytes(gets translated before ioctl())
int InitSound(int *rate)
{
 int x;

 printf("  Opening /dev/dsp...");
 dspfd=open("/dev/dsp",O_WRONLY);
 if(dspfd==-1) goto __disperror;

 format=0;
 if(ioctl(dspfd,SNDCTL_DSP_GETFMTS,&format)==-1)
 {
   printf("\n  Error getting sound device format capabilities!");
   close(dspfd);
   return(0);
 }
 if(format&AFMT_S16_NE) format&=AFMT_S16_NE;
 else if(format&AFMT_S16_LE) format&=AFMT_S16_LE;
 else if(format&AFMT_S16_BE) format&=AFMT_S16_BE;
 else if(format&AFMT_U8) format&=AFMT_U8;
 else if(format&AFMT_S8) format&=AFMT_S8;
 else
 {
   printf("\n  Required format not supported by sound device!");
   close(dspfd);
   return(0);
 }
 printf("\n   Setting format to ");
 if(format&(AFMT_S16_LE|AFMT_S16_BE)) printf("signed 16-bit");
 else if(format&AFMT_U8) printf("unsigned 8-bit");
 else printf("signed 8-bit");
 printf("...");
 if(ioctl(dspfd,SNDCTL_DSP_SETFMT,&format)==-1)
  goto __disperror;

 x=0;
 printf("\n   Setting mono sound...");  
 if(ioctl(dspfd,SNDCTL_DSP_STEREO,&x)==-1) goto __disperror;
 printf("\n   Setting playback rate of %d hz...",*rate);
 if(ioctl(dspfd,SNDCTL_DSP_SPEED,rate)==-1) goto __disperror;
 printf("Set to %d hz\n",*rate);
 if((*rate<44100 || *rate>48000) && *rate!=96000)
 {
  printf("    Playback rate is out of range(44100-48000,96000, borders preferred).\n");
  close(dspfd);
  return(0);
 }
 return 1;
 __disperror:
 printf("ERROR\n");
 return 0;
}

void KillSound(void)
{
  close(dspfd);
}
static int16 MBuffer[2048];
void WriteSound(int32 *Buffer, int Count)
{
 int P,c;
 int32 *src=Buffer;

 if(format==AFMT_U8)
 {
  uint8 *dest=(uint8 *)MBuffer;
  for(P=Count;P;P--,dest++,src++)
   *dest=((*src+32768)>>8);
  c=Count;
 }
 else if(format==AFMT_S8)
 {
  uint8 *dest=(uint8 *)MBuffer;
  for(P=Count;P;P--,dest++,src++)
   *dest=(*src>>8);
  c=Count;
 }
 else if(format==AFMT_S16_NE)
 {
  int16 *dest=MBuffer;
  for(P=Count;P;P--,dest++,src++)
   *dest=*src;
  c=Count<<1;
 }
 else /* Opposite to native endian endian */
 {
  int16 *dest=MBuffer;
  for(P=Count;P;P--,dest++,src++)
   *dest=(*src<<8)|(*src>>8);
  c=Count<<1;
 }

 write(dspfd,(uint8 *)MBuffer,c);
}

void poosh(void)
{
 ioctl(dspfd,SNDCTL_DSP_RESET,0);
}
