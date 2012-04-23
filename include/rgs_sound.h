/*
 Copyright (C) 2012 by Ivan Safrin
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
*/

#ifndef RGS_SOUND_H_
#define RGS_SOUND_H_

#include "rgs_data.h"
#include <time.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PI 3.14159265f
#define rnd(n) (rand()%(n+1))

extern RGS_Core *g_rgs;

/* @package Sound */
/* @beginmodule SFX */

/**
 * @desc SFXR Sound
 */ 
typedef struct {
	float masterVolume;
	int waveType;
	float soundVol;
	float baseFreq;
	float freqLimit;
	float freqRamp;
	float freqDRamp;
	float duty;
	float dutyRamp;
	float vibStrength;
	float vibSpeed;
	float vibDelay;
	float envAttack;
	float envSustain;
	float envDecay;
	float envPunch;
	int filterOn;
	float lpfResonance;
	float lpfFreq;
	float lpfRamp;
	float hpfFreq;
	float hpfRamp;
	float phaOffset;
	float phaRamp;
	float repeatSpeed;
	float arpSpeed;
	float arpMod;
	char *cachedSample;
} RGS_SFSSound;

typedef struct {
	RGS_SFSSound *sound;
	int playing_sample;
	int phase;
	double fperiod;
	double fmaxperiod;
	double fslide;
	double fdslide;
	int period;
	float square_duty;
	float square_slide;
	int env_stage;
	int env_time;
	int env_length[3];
	float env_vol;
	float fphase;
	float fdphase;
	int iphase;
	float phaser_buffer[1024];
	int ipp;
	float noise_buffer[32];
	float fltp;
	float fltdp;
	float fltw;
	float fltw_d;
	float fltdmp;
	float fltphp;
	float flthp;
	float flthp_d;
	float vib_phase;
	float vib_speed;
	float vib_amp;
	int rep_time;
	int rep_limit;
	int arp_time;
	int arp_limit;
	double arp_mod;
} RGS_SFSPlayback;


/** @desc Plays an SFS sound
 *  @param sound Pointer to the sound to play
 */
extern DECLSPEC void RGS_PlaySFS(RGS_SFSSound *sound);

/** @desc Loads an SFS sound from .sfs file
 *  @param filename Path to the file
 */
extern DECLSPEC RGS_SFSSound *RGS_LoadSFS(char *filename);

/* @endmodule */


extern DECLSPEC void RGS_InitSound();
extern DECLSPEC void RGS_ShutdownSound();

/* @beginmodule Music */
/** @desc Plays music from the .nsf file 
 * @param filename Path to NSF file
 */
extern DECLSPEC void RGS_PlayNSF(char *filename);
extern DECLSPEC void RGS_StopMusic();
/* @endmodule */

extern DECLSPEC void RGS_SoundCallback( void* data, Uint8* out, int count );
extern DECLSPEC void RGS_UpdateSoundSystem(int count);
extern DECLSPEC void RGS_UpdateMusic();
extern DECLSPEC void RGS_UpdateSounds(int count);

extern DECLSPEC void RGS_SynthSFS(RGS_SFSPlayback *playback, int length, float* buffer);
extern DECLSPEC void RGS_ResetSFS(RGS_SFSPlayback *playback, int restart);

#ifdef __cplusplus
}
#endif

#endif
