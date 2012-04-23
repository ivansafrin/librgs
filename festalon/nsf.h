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

typedef struct {
                char ID[5]; /*NESM^Z*/
                uint8 Version;
                uint8 TotalSongs;
                uint8 StartingSong;
                uint8 LoadAddressLow;
                uint8 LoadAddressHigh;
                uint8 InitAddressLow;
                uint8 InitAddressHigh;
                uint8 PlayAddressLow;
                uint8 PlayAddressHigh;
                uint8 SongName[32];
                uint8 Artist[32];
                uint8 Copyright[32];
                uint8 NTSCspeed[2];              // Unused
                uint8 BankSwitch[8];
                uint8 PALspeed[2];               // Unused
                uint8 VideoSystem;
                uint8 SoundChip;
                uint8 Expansion[4];
                uint8 reserve[8];
        } NSF_HEADER;

void FESTA_NSFinit(void);

extern uint8 PAL;

typedef struct {
           int disabled;
           uint32 SndRate;
	   int soundq;
} FCEUS;
                
extern FCEUS FSettings;


void NSF_init(void);
void DrawNSF(uint8 *XBuf);
void NSFDealloc(void);
void NSFDodo(void);
void DoNSFFrame(void);
