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
 
#include "rgs_sound.h"
#include "rgs_core.h"
#include "driver.h"


float frnd(float range) {
	return (float)rnd(10000)/10000*range;
}


void RGS_InitSound() {

	g_rgs->soundSystem = (RGS_SoundSystem*)malloc(sizeof(RGS_SoundSystem));
	memset((void*)g_rgs->soundSystem, 0, sizeof(RGS_SoundSystem));
	g_rgs->soundSystem->playingMusic = 0;
	g_rgs->soundSystem->sampleRate = 44100;
	g_rgs->soundSystem->bufferSize = 4096;
	g_rgs->soundSystem->playingAudio = 1;
	
	g_rgs->soundSystem->sfsPlaybacks = ISArray_Create(sizeof(RGS_SFSPlayback));
	
	static SDL_AudioSpec as; // making static clears all fields to 0	
	as.freq     = g_rgs->soundSystem->sampleRate;
	as.format   = AUDIO_S16SYS;
	as.channels = 2;
//	as.silence = 0;
	as.userdata = NULL;
	as.callback = RGS_SoundCallback;
	as.samples  = g_rgs->soundSystem->bufferSize/4;
	SDL_OpenAudio( &as, 0 );
	SDL_PauseAudio(0);
	
	g_rgs->soundSystem->tmpBuffer = (Uint32*)malloc( g_rgs->soundSystem->bufferSize);
	
	g_rgs->soundSystem->finalBuffer = (Uint8*)malloc( g_rgs->soundSystem->bufferSize);
	g_rgs->soundSystem->soundBuffer = (Uint8*)malloc( g_rgs->soundSystem->bufferSize);
	memset(g_rgs->soundSystem->soundBuffer, 0, g_rgs->soundSystem->bufferSize);
	g_rgs->soundSystem->musicBuffer = (Uint8*)malloc( g_rgs->soundSystem->bufferSize);
	memset(g_rgs->soundSystem->musicBuffer, 0, g_rgs->soundSystem->bufferSize);
	g_rgs->soundSystem->musicBufferStart = g_rgs->soundSystem->musicBuffer;
	
	g_rgs->soundSystem->bufferAvailable =  g_rgs->soundSystem->bufferSize;
	
//	g_rgs->soundSystem->soundBuffer = NULL;

	FESTAI_Sound(g_rgs->soundSystem->sampleRate);
	FESTAI_SetSoundQuality(0);
	FESTAI_SetVolume(100);

}

void RGS_SoundCallback( void* data, Uint8* out, int count ) {
	
	if(g_rgs->soundSystem->playingMusic == 1) {
		while(g_rgs->soundSystem->bufferAvailable > 0) {
			RGS_UpdateMusic();
		}
	}
	
	RGS_UpdateSoundSystem(count);

	g_rgs->soundSystem->bufferAvailable =  g_rgs->soundSystem->bufferSize;
	g_rgs->soundSystem->musicBuffer = g_rgs->soundSystem->musicBufferStart;

	memcpy(out, g_rgs->soundSystem->finalBuffer, count);
}

void RGS_UpdateMusic() {

	int count = 0;
	int copySize;
	int32 *tmp = NULL;
	int16 *tmp2 = NULL;
	
	if(g_rgs->soundSystem->bufferAvailable <= 0)
		return;

		
	if(g_rgs->soundSystem->holdoverSize > 0) {
		memcpy(g_rgs->soundSystem->musicBuffer,g_rgs->soundSystem->tmpBuffer,g_rgs->soundSystem->holdoverSize);
		g_rgs->soundSystem->musicBuffer += g_rgs->soundSystem->holdoverSize;
		g_rgs->soundSystem->bufferAvailable -= g_rgs->soundSystem->holdoverSize;
		g_rgs->soundSystem->holdoverSize = 0;
	}
	
	if(g_rgs->soundSystem->playingAudio == 1 && g_rgs->soundSystem->musicBuffer != NULL) {
		tmp = FESTAI_Emulate(&count);
		
//		if(tmp == NULL)
//			return;
		
		int i;
		tmp2 = (int16*)tmp;
		for(i =1; i < count*2; i += 2) {
			tmp2[i] = tmp2[i-1];
		}
						
		copySize = count*4;
		if(copySize > g_rgs->soundSystem->bufferAvailable) {
			copySize = 	g_rgs->soundSystem->bufferAvailable;
			g_rgs->soundSystem->holdoverSize = (count*4) - copySize;
			memcpy(g_rgs->soundSystem->tmpBuffer,tmp+(copySize/4), g_rgs->soundSystem->holdoverSize);
		}
		
		memcpy(g_rgs->soundSystem->musicBuffer,tmp,copySize);
		g_rgs->soundSystem->musicBuffer += copySize;
		g_rgs->soundSystem->bufferAvailable -= copySize;
	}
	
}

void RGS_UpdateSoundSystem(int count) {
	if(g_rgs->soundSystem->playingAudio == 0)
		return;
		
	RGS_UpdateSounds(count);
	
	memset(g_rgs->soundSystem->finalBuffer, 0, g_rgs->soundSystem->bufferSize);
	
	if(g_rgs->soundSystem->playingMusic == 1) 
		SDL_MixAudio(g_rgs->soundSystem->finalBuffer, g_rgs->soundSystem->musicBufferStart, count, SDL_MIX_MAXVOLUME);
	SDL_MixAudio(g_rgs->soundSystem->finalBuffer, g_rgs->soundSystem->soundBuffer, count, SDL_MIX_MAXVOLUME);
}

void RGS_UpdateSounds(int count) {

	int i;

	float *fbuf = (float*)malloc((count/4)*sizeof(float));
	Uint8 *tmpBuf = (Uint8*)malloc(g_rgs->soundSystem->bufferSize*sizeof(Uint8));
	memset(fbuf, 0, sizeof(fbuf));
	unsigned int l,n;
	float f;
	
	memset(g_rgs->soundSystem->soundBuffer, 0, g_rgs->soundSystem->bufferSize);
	
	RGS_SFSPlayback *playback;
	for(i=0;i<g_rgs->soundSystem->sfsPlaybacks->length;i++) {		
		playback = (RGS_SFSPlayback*)g_rgs->soundSystem->sfsPlaybacks->data[i];
		if(playback->playing_sample == 0) {
			if(i == g_rgs->soundSystem->sfsPlaybacks->length-1) {
				ISArray_RemoveElement(g_rgs->soundSystem->sfsPlaybacks, (void*)playback);
				free(playback);
				free(fbuf);
				free(tmpBuf);
				return;
			} else {
				ISArray_RemoveElement(g_rgs->soundSystem->sfsPlaybacks, (void*)playback);
				free(playback);
				playback = (RGS_SFSPlayback*)g_rgs->soundSystem->sfsPlaybacks->data[i];				
			}
		}
		
		
		RGS_SynthSFS(playback, count/4, fbuf);
		l = count/4;
		n = count/2;
		while (l--)
		{
			n-=2;
			f = fbuf[l];
			if (f < -1.0) f = -1.0;
			if (f > 1.0) f = 1.0;
			((Sint16*)tmpBuf)[n] = (Sint16)(f * 32767);
			((Sint16*)tmpBuf)[n+1] = (Sint16)(f * 32767);
		}
		SDL_MixAudio(g_rgs->soundSystem->soundBuffer, tmpBuf, count, SDL_MIX_MAXVOLUME);
	}
	
	free(fbuf);
	free(tmpBuf);

}

void RGS_ShutdownSound() {

	if(g_rgs->soundSystem->playingMusic == 1) {
		FESTAI_Close();
	}

	SDL_PauseAudio(1);
	SDL_LockAudio();
	SDL_UnlockAudio();
	SDL_CloseAudio();
	free(g_rgs->soundSystem);
}

void RGS_StopMusic() {
	FESTAI_SetVolume(0);
}

void RGS_PlayNSF(char *filename) {
	char *data = (char*)RGS_OpenDataPointer(filename);
	if(data != NULL) {
		FESTAI_SetVolume(100);
		FESTAI_Load((uint8*)data,g_rgs->currentDataSize);
//		FESTAI_Disable(0);
		FESTAI_NSFControl(0,0);
		free(data);
		g_rgs->soundSystem->playingMusic = 1;
	}
}

RGS_SFSSound *RGS_LoadSFS(char *filename) {
	RGS_SFSSound *newSound = (RGS_SFSSound*)malloc(sizeof(RGS_SFSSound));
	FILE *file = fopen(filename, "rb");

	newSound->masterVolume = 0.05;

	int version=0;
	fread(&version, 1, sizeof(int), file);
	if(version!=100 && version!=101 && version!=102)
		return 0;

	fread(&newSound->waveType, 1, sizeof(int), file);

	newSound->soundVol=0.5f;
	if(version==102)
		fread(&newSound->soundVol, 1, sizeof(float), file);

	fread(&newSound->baseFreq, 1, sizeof(float), file);
	fread(&newSound->freqLimit, 1, sizeof(float), file);
	fread(&newSound->freqRamp, 1, sizeof(float), file);
	if(version>=101)
		fread(&newSound->freqDRamp, 1, sizeof(float), file);
		
	fread(&newSound->duty, 1, sizeof(float), file);
	fread(&newSound->dutyRamp, 1, sizeof(float), file);

	fread(&newSound->vibStrength, 1, sizeof(float), file);
	fread(&newSound->vibSpeed, 1, sizeof(float), file);
	fread(&newSound->vibDelay, 1, sizeof(float), file);

	fread(&newSound->envAttack, 1, sizeof(float), file);
	fread(&newSound->envSustain, 1, sizeof(float), file);
	fread(&newSound->envDecay, 1, sizeof(float), file);
	fread(&newSound->envPunch, 1, sizeof(float), file);

	fread(&newSound->filterOn, 1, sizeof(int), file);
	fread(&newSound->lpfResonance, 1, sizeof(float), file);
	fread(&newSound->lpfFreq, 1, sizeof(float), file);
	fread(&newSound->lpfRamp, 1, sizeof(float), file);
	fread(&newSound->hpfFreq, 1, sizeof(float), file);
	fread(&newSound->hpfRamp, 1, sizeof(float), file);
	
	fread(&newSound->phaOffset, 1, sizeof(float), file);
	fread(&newSound->phaRamp, 1, sizeof(float), file);

	fread(&newSound->repeatSpeed, 1, sizeof(float), file);

	if(version>=101)
	{
		fread(&newSound->arpSpeed, 1, sizeof(float), file);
		fread(&newSound->arpMod, 1, sizeof(float), file);
	}

	fclose(file);
	return newSound;
}

void RGS_PlaySFS(RGS_SFSSound *sound) {
	RGS_SFSPlayback *newPlayback = (RGS_SFSPlayback*)ISArray_CreateElement(	g_rgs->soundSystem->sfsPlaybacks);
	newPlayback->sound = sound;
	newPlayback->playing_sample = 1;
	RGS_ResetSFS(newPlayback, 0);
}

void RGS_ResetSFS(RGS_SFSPlayback *playback, int restart) {

	int i;
	if(!restart)
		playback->phase=0;
	
	playback->fperiod=100.0/(playback->sound->baseFreq*playback->sound->baseFreq+0.001);
	playback->period=(int)playback->fperiod;
	playback->fmaxperiod=100.0/(playback->sound->freqLimit*playback->sound->freqLimit+0.001);
	playback->fslide=1.0-pow((double)playback->sound->freqRamp, 3.0)*0.01;
	playback->fdslide=-pow((double)playback->sound->freqDRamp, 3.0)*0.000001;
	playback->square_duty=0.5f-playback->sound->duty*0.5f;
	playback->square_slide=-playback->sound->dutyRamp*0.00005f;
	if(playback->sound->arpMod>=0.0f)
		playback->arp_mod=1.0-pow((double)playback->sound->arpMod, 2.0)*0.9;
	else
		playback->arp_mod=1.0+pow((double)playback->sound->arpMod, 2.0)*10.0;
	playback->arp_time=0;
	playback->arp_limit=(int)(pow(1.0f-playback->sound->arpSpeed, 2.0f)*20000+32);
	if(playback->sound->arpSpeed==1.0f)
		playback->arp_limit=0;
	if(!restart)
	{
		// reset filter
		playback->fltp=0.0f;
		playback->fltdp=0.0f;
		playback->fltw=pow(playback->sound->lpfFreq, 3.0f)*0.1f;
		playback->fltw_d=1.0f+playback->sound->lpfRamp*0.0001f;
		playback->fltdmp=5.0f/(1.0f+pow(playback->sound->lpfResonance, 2.0f)*20.0f)*(0.01f+playback->fltw);
		if(playback->fltdmp>0.8f) playback->fltdmp=0.8f;
		playback->fltphp=0.0f;
		playback->flthp=pow(playback->sound->hpfFreq, 2.0f)*0.1f;
		playback->flthp_d=1.0+playback->sound->hpfRamp*0.0003f;
		// reset vibrato
		playback->vib_phase=0.0f;
		playback->vib_speed=pow(playback->sound->vibSpeed, 2.0f)*0.01f;
		playback->vib_amp=playback->sound->vibStrength*0.5f;
		// reset envelope
		playback->env_vol=0.0f;
		playback->env_stage=0;
		playback->env_time=0;
		playback->env_length[0]=(int)(playback->sound->envAttack*playback->sound->envAttack*100000.0f);
		playback->env_length[1]=(int)(playback->sound->envSustain*playback->sound->envSustain*100000.0f);
		playback->env_length[2]=(int)(playback->sound->envDecay*playback->sound->envDecay*100000.0f);

		playback->fphase=pow(playback->sound->phaOffset, 2.0f)*1020.0f;
		if(playback->sound->phaOffset<0.0f) playback->fphase=-playback->fphase;
		playback->fdphase=pow(playback->sound->phaRamp, 2.0f)*1.0f;
		if(playback->sound->phaRamp<0.0f) playback->fdphase=-playback->fdphase;
		playback->iphase=abs((int)playback->fphase);
		playback->ipp=0;
		for(i=0;i<1024;i++)
			playback->phaser_buffer[i]=0.0f;

		for(i=0;i<32;i++)
			playback->noise_buffer[i]=frnd(2.0f)-1.0f;

		playback->rep_time=0;
		playback->rep_limit=(int)(pow(1.0f-playback->sound->repeatSpeed, 2.0f)*20000+32);
		if(playback->sound->repeatSpeed==0.0f)
			playback->rep_limit=0;
	}
}

void RGS_SynthSFS(RGS_SFSPlayback *playback, int length, float *buffer)
{
	int i,j;
	for(i=0;i<length;i++)
	{
		playback->rep_time++;
		if(playback->rep_limit !=0 && playback->rep_time>=playback->rep_limit)
		{
			playback->rep_time=0;
			RGS_ResetSFS(playback, 1);
		}

		// frequency envelopes/arpeggios
		playback->arp_time++;
		if(playback->arp_limit!=0 && playback->arp_time>=playback->arp_limit)
		{
			playback->arp_limit=0;
			playback->fperiod*=playback->arp_mod;
		}
		playback->fslide+=playback->fdslide;
		playback->fperiod*=playback->fslide;
		if(playback->fperiod>playback->fmaxperiod)
		{
			playback->fperiod=playback->fmaxperiod;
			if(playback->sound->freqLimit>0.0f)
				playback->playing_sample=0;
		}
		
		float rfperiod=playback->fperiod;
		if(playback->vib_amp>0.0f)
		{
			playback->vib_phase+=playback->vib_speed;
			rfperiod=playback->fperiod*(1.0+sin(playback->vib_phase)*playback->vib_amp);
		}
		
		playback->period=(int)rfperiod;
		if(playback->period<8) playback->period=8;
		playback->square_duty+=playback->square_slide;
		if(playback->square_duty<0.0f) playback->square_duty=0.0f;
		if(playback->square_duty>0.5f) playback->square_duty=0.5f;		
		// volume envelope
		playback->env_time++;
		if(playback->env_time>playback->env_length[playback->env_stage])
		{
			playback->env_time=0;
			playback->env_stage++;
			if(playback->env_stage==3)
				playback->playing_sample=0;
		}
		if(playback->env_stage==0)
			playback->env_vol=(float)playback->env_time/playback->env_length[0];
		if(playback->env_stage==1)
			playback->env_vol=1.0f+pow(1.0f-(float)playback->env_time/playback->env_length[1], 1.0f)*2.0f*playback->sound->envPunch;
		if(playback->env_stage==2)
			playback->env_vol=1.0f-(float)playback->env_time/playback->env_length[2];

		// phaser step
		playback->fphase+=playback->fdphase;
		playback->iphase=abs((int)playback->fphase);
		if(playback->iphase>1023) playback->iphase=1023;

		if(playback->flthp_d!=0.0f)
		{
			playback->flthp*=playback->flthp_d;
			if(playback->flthp<0.00001f) playback->flthp=0.00001f;
			if(playback->flthp>0.1f) playback->flthp=0.1f;
		}

		float ssample=0.0f;
		int si;
		for(si=0;si<8;si++) // 8x supersampling
		{
			float sample=0.0f;
			playback->phase++;
			if(playback->phase>=playback->period)
			{
//				phase=0;
				playback->phase%=playback->period;
				if(playback->sound->waveType==3) 
					for(j=0;j<32;j++)
						playback->noise_buffer[j]=frnd(2.0f)-1.0f;
			}
			// base waveform
			float fp=(float)playback->phase/playback->period;
			switch(playback->sound->waveType)
			{
			case 0: // square
				if(fp<playback->square_duty)
					sample=0.5f;
				else
					sample=-0.5f;
				break;
			case 1: // sawtooth
				sample=1.0f-fp*2;
				break;
			case 2: // sine
				sample=(float)sin(fp*2*PI);
				break;
			case 3: // noise
				sample=playback->noise_buffer[playback->phase*32/playback->period];
				break;
			}
			// lp filter
			float pp=playback->fltp;
			playback->fltw*=playback->fltw_d;
			if(playback->fltw<0.0f) playback->fltw=0.0f;
			if(playback->fltw>0.1f) playback->fltw=0.1f;
			if(playback->sound->lpfFreq != 1.0f)
			{
				playback->fltdp+=(sample-playback->fltp)*playback->fltw;
				playback->fltdp-=playback->fltdp*playback->fltdmp;
			}
			else
			{
				playback->fltp=sample;
				playback->fltdp=0.0f;
			}
			playback->fltp+=playback->fltdp;
			// hp filter
			playback->fltphp+=playback->fltp-pp;
			playback->fltphp-=playback->fltphp*playback->flthp;
			sample=playback->fltphp;
			// phaser
			playback->phaser_buffer[playback->ipp&1023]=sample;
			sample+=playback->phaser_buffer[(playback->ipp-playback->iphase+1024)&1023];
			playback->ipp=(playback->ipp+1)&1023;
			// final accumulation and envelope application
			ssample+=sample*playback->env_vol;
		}
		ssample=ssample/8*playback->sound->masterVolume;

		ssample*=2.0f*playback->sound->soundVol;

		if(buffer!=NULL)
		{
			if(ssample>1.0f) ssample=1.0f;
			if(ssample<-1.0f) ssample=-1.0f;
			*buffer++=ssample;
		}
	}
}

