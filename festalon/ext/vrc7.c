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

static uint8 indox;

#include "emu2413.h"

static OPLL *VRC7Sound=NULL;

void UpdateOPLNEO(int32 *Wave, int Count)
{ 
 /* Fixme.  It's silly to check this each frame...or is it? */
 if(FSettings.disabled>>5)
 {
  OPLL_setMask (VRC7Sound, FSettings.disabled>>5);
 }

 moocow(VRC7Sound, Wave, Count, 4);
}  

static DECLFW(Mapper85_write)
{
	A|=(A&8)<<1;

	if(A==0x9030)
	{
	 if(FSettings.SndRate)
	 {
	  OPLL_writeReg(VRC7Sound, indox, V);
	  GameExpSound.NeoFill=UpdateOPLNEO;
	 }
	}
	else if(A==0x9010)
	 indox=V;
}

static void M85SC(void)
{
 if(VRC7Sound)
  OPLL_set_rate(VRC7Sound, FSettings.SndRate);
}

static void VRC7SI(void)
{
  GameExpSound.RChange=M85SC;
 
  VRC7Sound=OPLL_new(3579545, FSettings.SndRate?FSettings.SndRate:44100);
  OPLL_reset(VRC7Sound);
  OPLL_reset(VRC7Sound);
}

void NSFVRC7_Init(void)
{
    SetWriteHandler(0x9010,0x901F,Mapper85_write);
    SetWriteHandler(0x9030,0x903F,Mapper85_write);
    VRC7SI();
}

void NSFVRC7_Close(void)
{
 if(VRC7Sound)
  OPLL_delete(VRC7Sound);
 VRC7Sound=NULL;
}
