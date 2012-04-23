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

#include <xmms/plugin.h>
#include <xmms/util.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../driver.h"

#define CMD_SEEK	0x8000
#define CMD_STOP	0x4000

static volatile uint32 command=0;
static uint32 current;
static volatile int playing=0;

static NSF_HEADER *NSFHeader;

extern InputPlugin festa_ip;

InputPlugin *get_iplugin_info(void)
      {
         festa_ip.description = "Festalon NSF Plugin v"VSTRING;
         return &festa_ip;
      }

void init(void)
{
}
void about(void)
{

}
void config(void)
{

}

int testfile(char *fn)
{
 char buf[5];
 FILE *fp;

 if(!(fp=fopen(fn,"rb"))) return(0);
 if(fread(buf,1,5,fp)!=5) {fclose(fp);return(0);}
 fclose(fp);
 if(memcmp(buf,"NESM\x1a",5))
   return 0;

 return(1);
}

static void SI(void)
{
 char tmp[64];
 sprintf(tmp,"[%d/%d] %s",(int)current,NSFHeader->TotalSongs,NSFHeader->SongName);
 festa_ip.set_info(tmp,(NSFHeader->TotalSongs-1)*1000,48000*2*8,48000,1);
}
static pthread_t dethread;
int FESTAD_Update(int32 *Buffer, int Count)
{
 static int16 buf[1024];
 char *tmp=(char *)buf;
 int x;

 for(x=0;x<Count;x++) buf[x]=Buffer[x];
 Count*=2;

 while(Count>0)
 {
  int t=festa_ip.output->buffer_free();
  if(t>Count)
   festa_ip.output->write_audio(tmp,Count);
  else
  {
   festa_ip.output->write_audio(tmp,t);
   usleep((Count-t)*125/12); // 1000*1000/48000/2
  }
  Count-=t;
  tmp+=t;
 }
 if(command&CMD_STOP)
 {
  playing=0;   
  festa_ip.output->close_audio();
  command=0;
 }
 if(command&CMD_SEEK)
 {
  current=command&255;
  current=FESTAI_NSFControl(((command&255)+1),1);
  SI();
  festa_ip.output->flush(0);
 }
 command=0;
 return(playing);
}

static void *playloop(void *arg)
{
 int count;
 int32 *buf;
  
 do 
 {
  buf=FESTAI_Emulate(&count);
 } while(FESTAD_Update(buf,count));
 pthread_exit(0);
}

void play(char *fn)
{
 int size;
 char *buf;

 //puts("play");
 if(playing)
  return;

 {
  FILE *fp=fopen(fn,"rb");
  fseek(fp,0,SEEK_END);
  size=ftell(fp);
  fseek(fp,0,SEEK_SET);
  buf=malloc(size);
  fread(buf,1,size,fp);
  fclose(fp);
 }
 if(!festa_ip.output->open_audio(FMT_S16_LE, 48000, 1))
 {
  puts("Error opening audio.");
  return;
 }
 FESTAI_Sound(48000);
 FESTAI_SetSoundQuality(1);
 FESTAI_SetVolume(100);

 NSFHeader=FESTAI_Load(buf,size);
 current=NSFHeader->StartingSong;
 SI();
 playing=1;
 pthread_create(&dethread,0,playloop,0);
}

void stop(void)
{
 //puts("stop");
 festa_ip.output->pause(0);
 command=CMD_STOP;
 pthread_join(dethread,0);
 FESTAI_Close();
}

void festa_pause(short paused)
{
 festa_ip.output->pause(paused);
}

void seek(int time)
{
 //puts("seek");
 command=CMD_SEEK|time;
// festa_ip.output->flush(0); 
}

int gettime(void)
{
// return festa_ip.output->output_time();
 //puts("gettime");
 return((current-1)*1000);
}

void getsonginfo(char *fn, char **title, int *length)
{
  FILE *fp=fopen(fn,"rb");
  NSF_HEADER head;
  *title=malloc(64);

  if(fp) {fread(&head,1,0x80,fp);
  fclose(fp);}
  head.SongName[31]=0;
  *length=(head.TotalSongs-1)*1000;
  strcpy(*title,head.SongName);
}

InputPlugin festa_ip =
{
 0,0,"Some description",0,0,0,testfile,0,play,stop,festa_pause,
 seek,0,gettime,0,0,0,0,0,0,0,getsonginfo,0,0
};

