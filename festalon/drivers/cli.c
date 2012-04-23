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

#include <termios.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <sys/time.h>

#include "../driver.h"
#ifdef DOS
#include <conio.h>
#include <pc.h>
#include "sb.h"
#else
#include "unixdsp.h"
#endif

static uint64 Stime,Ltime;
static uint64 BIGtime(void)
{
 uint64 r=0;
 struct timeval tv;
 if(! gettimeofday(&tv,0))
 {
  r=tv.tv_sec;
  r*=1000000;
  r+=tv.tv_usec;
 }
 return(r);
}

static int disabled=0;
static int current;
static int volume=100;
static NSF_HEADER *info;

static void ShowStatus(void)
{
  char chans[128];
  int maxos=5,x;

  chans[0]=0;
  if(info->SoundChip&1) maxos+=3;
  else if(info->SoundChip&2) maxos+=6;
  else if(info->SoundChip&4) maxos++;
  else if(info->SoundChip&8) maxos+=3;
  else if(info->SoundChip&0x10) maxos+=8;
  else if(info->SoundChip&0x20) maxos+=3;
  for(x=0;x<maxos;x++)
   sprintf(chans+strlen(chans),"%1d ",(disabled>>x)&1);
  #ifdef DOS
  gotoxy(1,wherey()-1);
  #else
  printf("\x1b[A");
  #endif
  Ltime=BIGtime();
  printf("Song:  %3d / %3d | %02d:%02d | Volume: %4d%% | %s\n",current,info->TotalSongs,(int)((Ltime-Stime)/60000000),(int)(((Ltime-Stime)/1000000)%60),volume,chans);
}
static void ShowInfo(NSF_HEADER *NSFHeader)
{
 int x;
 puts("NSF Loaded.  File information:\n");
 printf(" Name:       %s\n Artist:     %s\n Copyright:  %s\n",NSFHeader->SongName,NSFHeader->Artist,NSFHeader->Copyright);
 if(NSFHeader->SoundChip)
 {
  static char *tab[6]={"Konami VRCVI","Konami VRCVII","Nintendo FDS","Nintendo MMC5","Namco 106","Sunsoft FME-07"};
  for(x=0;x<6;x++)
   if(NSFHeader->SoundChip&(1<<x))
   {
    printf(" Expansion hardware:  %s\n",tab[x]);
    break;
   }
 }
 printf(" Speed:      %s\n\n",(NSFHeader->VideoSystem&1)?"PAL":"NTSC");
printf(" Load address:  $%04x\n Init address:  $%04x\n Play address:  $%04x\n\n",
NSFHeader->LoadAddressLow|(NSFHeader->LoadAddressHigh<<8),
NSFHeader->InitAddressLow|(NSFHeader->InitAddressHigh<<8),
NSFHeader->PlayAddressLow|(NSFHeader->PlayAddressHigh<<8));
 puts("");
// if(BSon)
//  puts(" Bank-switched.");
}

static volatile int eexit=0;
static int sa;
static struct termios oldtio,newtio;

static void siggo()
{
 eexit=1;
}

static void rmode()
{
 #ifndef DOS
 puts("\x1b[?25h");
 fcntl(fileno(stdin), F_SETFL, sa);
 #else
 _setcursortype(_NORMALCURSOR);
 #endif
 tcsetattr(0,TCSANOW,&oldtio);
}

static void badexit(int num)
{
 rmode();
 printf("Exiting on signal %d\n",num);
 exit(-1);
}

int main(int argc, char *argv[])
{
 int rate=48000;
 int size;
 char *buf;

 puts("\n*** Festalon v"VSTRING);
 if(argc<=1) 
 {
  printf("Usage:  %s file1.nsf file2.nsf ... fileN.nsf\n",argv[0]);
  return(-1);
 }
 tcgetattr(0,&oldtio);

 newtio=oldtio;
 newtio.c_lflag&=~(ICANON|ECHO);

 if(InitSound(&rate))
  FESTAI_Sound(rate);

 FESTAI_SetSoundQuality(1);

 FESTAI_SetVolume(volume);
 FESTAI_Disable(disabled);

 #ifndef DOS
 {
  int x;
  for(x=0;x<_NSIG;x++) signal(x,badexit);
 }
 #endif
 signal(SIGTERM,siggo);
 signal(SIGINT,siggo);

 while(argc>1 && !eexit)
 {
  argv++;
  {
   FILE *fp;
   printf("\nLoading %s...",*argv);
   if(!(fp=fopen(*argv,"rb"))) {puts("Error opening file!");rmode();return(-1);}
   fseek(fp,0,SEEK_END);
   size=ftell(fp);
   fseek(fp,0,SEEK_SET);
   buf=malloc(size);
   fread(buf,1,size,fp); 
   fclose(fp);
  }
 
  if(!(info=FESTAI_Load(buf,size)))
  {
   puts("Not an NSF!");
   argc--;
   continue;
  }
  puts("");
  poosh();
  #ifndef DOS
  puts("\x1b[?25l");
  #endif
  ShowInfo(info);

 tcsetattr(0,TCSANOW,&newtio);
 #ifndef DOS
 sa = fcntl(fileno(stdin), F_GETFL);
 fcntl(fileno(stdin), F_SETFL, O_NONBLOCK);
 #else
 _setcursortype(_NOCURSOR);
 #endif
 current=FESTAI_NSFControl(0,0);
 Ltime=Stime=BIGtime();
 ShowStatus();
 while(!eexit)
 {
  static char t[3]={0,0,0};
  int len;
  int32 *buf;

  buf=FESTAI_Emulate(&len);
  WriteSound(buf,len);

  if((BIGtime()-Ltime)/1000000) ShowStatus();
  #ifdef DOS
  while(kbhit())  
  {
   read(fileno(stdin),&t[0],1);
  #else
  while(read(fileno(stdin),&t[0],1)>=0)
  {
  #endif
    switch(tolower(t[0]))
    {
     case 10: poosh(); Stime=BIGtime();current=FESTAI_NSFControl(0,0);ShowStatus();break;
     case '`': poosh();break;
     case 'i':t[0]='1'+10;Stime=BIGtime();goto noyo;
     case 'o':t[0]='1'+11;Stime=BIGtime();goto noyo;
     case 'p':t[0]='1'+12;Stime=BIGtime();goto noyo;
     case '0':t[0]='1'+9;
     case '1' ... '9':noyo:disabled^=1<<(t[0]-'1');FESTAI_Disable(disabled);
		      poosh();ShowStatus();break;
     case '-': volume--; FESTAI_SetVolume(volume);poosh();ShowStatus();break;
     case '=': volume++; FESTAI_SetVolume(volume);poosh();ShowStatus();break;
     case 'q': goto exito;

     /* Alternate song selection keys.  Especially needed for DOS port. */
     case '\'':current=FESTAI_NSFControl(10,0);poosh();Stime=BIGtime();ShowStatus();break;
     case ';':current=FESTAI_NSFControl(-10,0);poosh();Stime=BIGtime();ShowStatus();break;
     case '.':current=FESTAI_NSFControl(1,0);poosh();Stime=BIGtime();ShowStatus();break;
     case ',':current=FESTAI_NSFControl(-1,0);poosh();Stime=BIGtime();ShowStatus();break;
    }
    #ifndef DOS
    if(t[2]==27 && t[1]==91)
     switch(t[0])
     {
      case 65:current=FESTAI_NSFControl(10,0);poosh();Stime=BIGtime();ShowStatus();break;
      case 66:current=FESTAI_NSFControl(-10,0);poosh();Stime=BIGtime();ShowStatus();break;
      case 67:current=FESTAI_NSFControl(1,0);poosh();Stime=BIGtime();ShowStatus();break;
      case 68:current=FESTAI_NSFControl(-1,0);poosh();Stime=BIGtime();ShowStatus();break;
     }
    #endif
    t[2]=t[1];
    t[1]=t[0];
  }

 }
 exito:
  argc--;
 }

 poosh();
 rmode();
 FESTAI_Close();
 KillSound();
 return(0);
}
