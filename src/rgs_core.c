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
 
#include "SDL.h"
#include "rgs_data.h"
#include "rgs_tilemap.h"
#include "rgs_core.h"
#include "rgs_sound.h"
#include "smallfont.h"
#include "bigfont.h"
#include "snes_ntsc.h"
#include "BFont.h"

RGS_Core *g_rgs;

int RGS_Init(int width, int height, int screenWidth, int screenHeight, int screenType) {
		if( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_NOPARACHUTE) < 0 ) {
		return 0;
	}
	

	
	g_rgs = (RGS_Core*)malloc(sizeof(RGS_Core));
	memset(g_rgs, 0, sizeof(RGS_Core));
	
	RGS_InitSound();

	g_rgs->defaultEntity = NULL;
	
	g_rgs->paused = 0;
	g_rgs->keyPressCallback = NULL;
	
	g_rgs->screens = ISArray_Create(sizeof(RGS_Screen));
	
	g_rgs->screenType = screenType;
	g_rgs->running = 1;
	
	RGS_ResetTimer(&g_rgs->frameTimer);
	
	g_rgs->entityRegistrations = ISArray_Create(sizeof(RGS_EntityRegistration));
	
	switch(screenType) {
		case RGS_NORMAL:
			if(screenWidth < width)
				screenWidth = width;
			
			if(screenHeight < height)
				screenHeight = height;
			
			g_rgs->screenSurface = SDL_SetVideoMode(screenWidth, screenHeight, 32, SDL_HWSURFACE);
			
			g_rgs->screenWidth = width;
			g_rgs->screenHeight = height;
			g_rgs->actScreenWidth = screenWidth;	
			g_rgs->actScreenHeight = screenHeight;
			
			
			if(width == screenWidth && height == screenHeight) {
				g_rgs->mainSurface = g_rgs->screenSurface;
				g_rgs->renderScale = 1;
			} else {
				g_rgs->mainSurface = SDL_CreateRGBSurface(SDL_HWSURFACE, width, height, 32, 0,0,0,0);
				if(screenWidth/width > screenHeight/height)
					g_rgs->renderScale = screenWidth/width;
				else
					g_rgs->renderScale = screenHeight/height;
			}
			
			break;
		case RGS_NTSC:
			g_rgs->screenWidth = width;
			g_rgs->screenHeight = height;
			g_rgs->actScreenWidth = SNES_NTSC_OUT_WIDTH(width);	
			g_rgs->actScreenHeight = height*2;
			
			g_rgs->screenSurface = SDL_SetVideoMode(SNES_NTSC_OUT_WIDTH(width), height*2, 32, SDL_HWSURFACE);
			g_rgs->mainSurface = SDL_CreateRGBSurface(SDL_HWSURFACE, width, height, 32, 0,0,0,0);
			g_rgs->ntscSurface = SDL_CreateRGBSurface(SDL_HWSURFACE, width, height, 16, 0,0,0,0);
			g_rgs->tmpSurface = SDL_CreateRGBSurface(SDL_HWSURFACE, SNES_NTSC_OUT_WIDTH(width), height*2, 32, 0,0,0,0);
			
			g_rgs->ntsc = (snes_ntsc_t*) malloc( sizeof (snes_ntsc_t) );
			snes_ntsc_setup_t setup = snes_ntsc_composite;
			memset((void*)&setup, 0, sizeof(snes_ntsc_composite));
			setup.sharpness = 0.7;
//			setup.saturation = 0.1;
//			setup.contrast = 0.2;
			setup.artifacts = -0.5;
			setup.bleed = -0.9;
			snes_ntsc_init( g_rgs->ntsc, &setup );
			g_rgs->burst_phase = 0;
	
			break;
	}
	
	
	if(g_rgs->mainSurface == NULL) {
		return 0;
	}
	
	SDL_ShowCursor(SDL_ENABLE);
	SDL_EnableUNICODE(1);
//	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	
	SDL_WM_SetCaption("RETRO GAME SYSTEM", "");
	
	g_rgs->smallFont = BFont_LoadFont(small_font);
	g_rgs->bigFont = BFont_LoadFont(big_font);
	return 1;
}


int RGS_GetKeyMapping(int keycode) {
	switch(keycode) {
		case SDLK_ESCAPE:
			return RGS_ESCAPE;
			break;
		case SDLK_LEFT:
			return RGS_LEFT;
			break;
		case SDLK_RIGHT:
			return RGS_RIGHT;
			break;
		case SDLK_UP:
			return RGS_UP;
			break;
		case SDLK_DOWN:
			return RGS_DOWN;
			break;
		case SDLK_z:
			return RGS_BUTTON1;
			break;
		case SDLK_x:
			return RGS_BUTTON2;
			break;
		case SDLK_RETURN:
			return RGS_START;
			break;
		case SDLK_DELETE:
			return RGS_DELETE;
			break;
		default:
			return RGS_ANYKEY;
			break;
	}
	return 0;
}

void RGS_Exit() {
	g_rgs->running = 0;
}

int RGS_SetClearColor(int r, int g, int b) {
	g_rgs->clearColor.r = r;
	g_rgs->clearColor.g = g;
	g_rgs->clearColor.b = b;
	return 1;
}

void RGS_InputMouseMove(int x, int y) {
	int i,j;
	g_rgs->mouseX = x;
	g_rgs->mouseY = y;
	RGS_Screen *screen;
	RGS_Object *object;
	
	for(i=0; i<g_rgs->screens->length; i++) {
		screen = (RGS_Screen*)g_rgs->screens->data[i];
		for(j=0; j<screen->objects->length; j++) {
			object = (RGS_Object*)screen->objects->data[j];
			if(object != NULL) {
				if(x > object->x && x < object->x + object->w && y > object->y && y < object->y + object->h) {
					if(object->onRollOver != NULL)
						object->onRollOver((void*)object);
				} else {
					if(object->onRollOut != NULL)
						object->onRollOut((void*)object);			
				}
			}
		}
	}
}

void RGS_InputMouseDown(int x, int y) {
	int i,j;
	int screenhit = 0;
	RGS_Screen *screen;
	RGS_Object *object;
	
	for(i=0; i<g_rgs->screens->length; i++) {
		screen = (RGS_Screen*)g_rgs->screens->data[i];
		if(screenhit == 1)
			return;
		if(screen->visible == 1) {
			for(j=0; j<screen->objects->length; j++) {
				object = (RGS_Object*)screen->objects->data[j];
				if(object != NULL) {
					if(x > object->x && x < object->x + object->w && y > object->y && y < object->y + object->h) {
						if(object->onPress != NULL) {
							object->onPress((void*)object);
							screenhit = 1;
						}
						// focus the clicked object
						if(object->focusable == 1) {
							RGS_ScreenFocusObject(screen, object);
							RGS_FocusScreen(screen);
						}
					}
				}
			}
		}
	}
}

void RGS_InputMouseUp(int x, int y) {
	int i,j;
	int screenhit = 0;
	RGS_Screen *screen;
	RGS_Object *object;
	
	for(i=0; i<g_rgs->screens->length; i++) {
		screen = (RGS_Screen*)g_rgs->screens->data[i];
		if(screen->visible == 1) {
			for(j=0; j< screen->objects->length; j++) {
				object = (RGS_Object*)screen->objects->data[j];
				if(object != NULL) {
					if(x > object->x && x < object->x + object->w && y > object->y && y < object->y + object->h && screenhit != 1) {
						if(object->onRelease != NULL) {
							object->onRelease((void*)object);
							screenhit = 1;
						}
						// focus the clicked object
						if(object->focusable == 1) {
							RGS_ScreenFocusObject(screen, object);
							RGS_FocusScreen(screen);
						}
					} else {
						if(object->onReleaseOutside != NULL)
							object->onReleaseOutside((void*)object);
					}
				}
			}
		}
	}
}

int RGS_GetMouseX() {
	return g_rgs->mouseX;
}

int RGS_GetMouseY() {
	return g_rgs->mouseY;
}

void RGS_SetKeyCallback(void (*keyPressCallback)(int keyCode)) {
	g_rgs->keyPressCallback = keyPressCallback;
}

int RGS_Update() {
	SDL_Event event;
	while(SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_QUIT:
				g_rgs->running = 0;
				break;
			
			case SDL_KEYDOWN:
				// call user callback if registered
				if(g_rgs->keyPressCallback != NULL)
					g_rgs->keyPressCallback(RGS_GetKeyMapping(event.key.keysym.sym));
					
				g_rgs->keyCodes[RGS_GetKeyMapping(event.key.keysym.sym)] = 1;
				if(event.key.keysym.sym == SDLK_TAB) {
					RGS_ScreenFocusNext(g_rgs->focusedScreen);
				} else if(g_rgs->focusedScreen != NULL) {
					RGS_ScreenKeyDownEvent(g_rgs->focusedScreen, event.key.keysym.sym);
					RGS_ScreenCharInputEvent(g_rgs->focusedScreen, (char)event.key.keysym.unicode);
				}
				break;
				case SDL_KEYUP:
				g_rgs->keyCodes[RGS_GetKeyMapping(event.key.keysym.sym)] = 0;
				break;
				case SDL_MOUSEBUTTONDOWN:
					RGS_InputMouseDown(event.button.x, event.button.y);
				break;
				case SDL_MOUSEBUTTONUP:
					RGS_InputMouseUp(event.button.x, event.button.y);
				break;
				case SDL_MOUSEMOTION:
					RGS_InputMouseMove(event.motion.x, event.motion.y);
				break;
				default:
				break;
		}
	}
	
	// not sure if it helps anythig
	RGS_UpdateTimer(&g_rgs->frameTimer);
	if(RGS_TimerHasElapsed(&g_rgs->frameTimer, 33) == 1)
			g_rgs->burst_phase ^= 1;
		
	
	//SDL_FillRect(g_rgs->mainSurface, NULL, SDL_MapRGB(g_rgs->mainSurface->format, 49, 94, 111) );
	if(g_rgs->paused == 0) {
		SDL_FillRect(g_rgs->mainSurface, NULL, SDL_MapRGB(g_rgs->mainSurface->format, g_rgs->clearColor.r, g_rgs->clearColor.g, g_rgs->clearColor.b) );
		RGS_RenderScreens();
	}
	//RGS_UpdateSoundSystem();
	
	if(g_rgs->CustomUpdate != NULL)
		g_rgs->CustomUpdate();
	
	
	switch(g_rgs->screenType) {
		case RGS_NORMAL:
			DoRenderNormal();
		break;
		case RGS_NTSC:
			DoRenderNTSC();			
		break;
	}
	
	if ( SDL_MUSTLOCK(g_rgs->screenSurface) ) {
		SDL_UnlockSurface(g_rgs->screenSurface);
	}
	SDL_Flip(g_rgs->screenSurface);
	
	return g_rgs->running;
}

int RGS_isPaused() {
	return g_rgs->paused;
}

void RGS_Pause(int pauseVal) {
	g_rgs->paused = pauseVal;
}

void DoRenderNormal() {
	int i,h,j,k;
	int lineoffset = 0;
	if(g_rgs->renderScale > 1) {
		SDL_LockSurface(g_rgs->mainSurface);
		SDL_LockSurface(g_rgs->screenSurface);
		unsigned int *ptr = (unsigned int*)g_rgs->mainSurface->pixels;
		unsigned int *destPtr = (unsigned int*)g_rgs->screenSurface->pixels;
		
		for(j=0; j < g_rgs->screenHeight;j++){
			lineoffset = j * (g_rgs->mainSurface->pitch / 4);
			for(i=0; i < g_rgs->screenWidth;i++){
				for(h=0; h<g_rgs->renderScale;h++){
					for(k=0; k<g_rgs->renderScale;k++){
						if(((j*g_rgs->renderScale)+h) < g_rgs->actScreenHeight && ((i*g_rgs->renderScale)+k) < g_rgs->actScreenWidth)
							destPtr[( ((j*g_rgs->renderScale)+h) * (g_rgs->screenSurface->pitch / 4)) + ((i*g_rgs->renderScale)+k)] = ptr[lineoffset + i];
					}
				}
			}
		}
		SDL_UnlockSurface(g_rgs->mainSurface);
		SDL_UnlockSurface(g_rgs->screenSurface);
	}
}


void DoRenderNTSC() {
//	SDL_LockSurface(g_rgs->mainSurface);
	SDL_LockSurface(g_rgs->screenSurface);
//	SDL_LockSurface(g_rgs->ntscSurface);
//	SDL_LockSurface(g_rgs->tmpSurface);

	
	SDL_Rect bRect;
	bRect.x = 0;
	bRect.y = 0;
	bRect.w = g_rgs->screenWidth;
	bRect.h = g_rgs->screenHeight;
	
				
	SDL_BlitSurface(g_rgs->mainSurface, &bRect, g_rgs->ntscSurface, &bRect);
//	SDL_Surface *tmpSurface = SDL_ConvertSurface(g_rgs->mainSurface, g_rgs->ntscSurface->format, SDL_HWSURFACE);
	snes_ntsc_blit(g_rgs->ntsc, (SNES_NTSC_IN_T const*)g_rgs->ntscSurface->pixels, g_rgs->mainSurface->w, g_rgs->burst_phase,
				   g_rgs->mainSurface->w, g_rgs->ntscSurface->h, g_rgs->tmpSurface->pixels, g_rgs->screenSurface->pitch);
				   
	int y;
	
	unsigned int output_pitch = g_rgs->screenSurface->pitch;
	
	for ( y = g_rgs->screenSurface->h / 2; --y >= 0; )
	{
		unsigned char const* in = (unsigned char*)g_rgs->tmpSurface->pixels + y * output_pitch;
		unsigned char* out = (unsigned char*)g_rgs->screenSurface->pixels + y * 2 * output_pitch;
		int n;
		
		for ( n = g_rgs->screenSurface->w; n; --n )
		{
			unsigned prev = *(Uint32*) in;
			unsigned next = *(Uint32*) (in + output_pitch);
			unsigned mixed = prev + next + ((prev ^ next) & 0x0821);
			*(Uint32*) out = prev;
			*(Uint32*) (out + output_pitch) = (mixed >> 1)- (mixed >> 2 & 0x18);
			
			in += 4;
			out += 4;
			
		}
	}
	
//	SDL_UnlockSurface(g_rgs->mainSurface);
	SDL_UnlockSurface(g_rgs->screenSurface);
//	SDL_UnlockSurface(g_rgs->ntscSurface);
//	SDL_UnlockSurface(g_rgs->tmpSurface);
	
//	SDL_FreeSurface(tmpSurface);
}

void RGS_DestroyObject(RGS_Object *object) {
	int i;
	RGS_Screen *screen;
	for(i=0; i<g_rgs->screens->length; i++) {
		screen = (RGS_Screen*) g_rgs->screens->data[i];
		if(RGS_ScreenRemoveObject(screen, object) == 1) {
			if(object->Destroy != NULL)
				object->Destroy(object);
			free(object);
			return;
		}
	}
	
	if(object->Destroy != NULL)
		object->Destroy(object);
	free(object);
}

int RGS_Shutdown() {
	int i,j;
	SDL_FreeSurface(g_rgs->mainSurface);
	SDL_FreeSurface(g_rgs->screenSurface);
	RGS_Screen *screen;
	RGS_Object *object;
	
	// free everything
	if(g_rgs->screens->length > 0) {
		for(i=0; i<g_rgs->screens->length; i++) {
			screen = (RGS_Screen*) g_rgs->screens->data[i];
			for(j=0;j < screen->objects->length;j++) {
				object = (RGS_Object*) screen->objects->data[j];
				if(object->Destroy != NULL)
					object->Destroy(object);
				free(object);
			}
			free(screen->objects);			
			free(screen);
		}
		free(g_rgs->screens);
	}
	
	RGS_ShutdownSound();
	
	/*
	 free(g_rgs->smallFont->Surface);
	 free(g_rgs->smallFont);
	 free(g_rgs->bigFont->Surface);
	 free(g_rgs->bigFont);
	 */
	
	free(g_rgs);
	return 1;
}

int RGS_RenderScreens() {
	int i;
	RGS_Screen *screen;
	for(i=0; i<g_rgs->screens->length; i++) {
		screen = (RGS_Screen*)g_rgs->screens->data[i];
		if(screen->visible == 1)
			RGS_RenderScreen(screen);
	}
	return 1;
}

void RGS_ScreenKeyDownEvent(RGS_Screen *screen, int eventCode) {
	if(screen->focusedObject == NULL)
		return;
	
	if(screen->focusedObject->focusable && screen->focusedObject->onKeyDown != NULL)
		screen->focusedObject->onKeyDown((void*)screen->focusedObject, eventCode);
}

void RGS_InitializeObject(RGS_Object *object, int x, int y) {
	object->x = x;
	object->y = y;
	object->visible = 1;
	strcpy(object->typeString, "unknown");
	object->Render = NULL;
	object->Destroy = NULL;	
	object->Update = NULL;	
	object->onRollOver = NULL;
	object->onRollOut = NULL;
	object->onRelease = NULL;
	object->onReleaseOutside = NULL;
	object->onPress = NULL;
	object->onKeyDown = NULL;
	object->onCharInput = NULL;
}

char *RGS_OpenDataPointer(char *filename) {
	char *data = NULL;
	FILE *inPtr = fopen(filename, "rb");
	if(!inPtr){
		return data;
	}
	fseek (inPtr, 0, SEEK_END);
	long dataSize = ftell(inPtr);
	fseek(inPtr, 0, SEEK_SET);
	data = (char*)malloc(dataSize);
	fread(data, 1, dataSize, inPtr);
	fclose(inPtr);
	g_rgs->currentDataSize = dataSize;
	return data;
}


void RGS_UpdateTimer(RGS_Timer *timer) {
	timer->ticks = SDL_GetTicks();
}

void RGS_ResetTimer(RGS_Timer *timer) {
	timer->ticks = SDL_GetTicks();
	timer->last = timer->ticks;
}

int RGS_TimerHasElapsed(RGS_Timer *timer, int msecs) {
	if(timer->ticks-timer->last > msecs) {
		timer->last = timer->ticks;
		return 1;
	}
	return 0;
}

int RGS_TimerElapsed(RGS_Timer *timer) {
	int elapsed = timer->ticks-timer->last;
	timer->last = timer->ticks;
	return elapsed;
}

Uint8 RGS_KeyDown(Uint8 keyCode) {
	return g_rgs->keyCodes[keyCode];
}

RGS_Color RGS_SplitRGB(int color) {
	RGS_Color retColor;
	int tmp;
	retColor.r = color>>16;
	tmp = color-(retColor.r<<16);
	retColor.g = tmp>>8;
	retColor.b = tmp-(retColor.g<<8);
	retColor.rgb = color;
	return retColor;
}

void RGS_FillRect(RGS_Rect *rect, int color) {
	RGS_Color rcolor = RGS_SplitRGB(color);
	SDL_Rect srect;
	srect.x = rect->x;
	srect.y = rect->y;
	srect.w = rect->w;
	srect.h = rect->h;
	SDL_FillRect(g_rgs->mainSurface, &srect, SDL_MapRGB(g_rgs->mainSurface->format, rcolor.r, rcolor.g, rcolor.b));
}

void RGS_DrawLine(int x, int y, int x2, int y2, int color) {
	int i;
	int yLonger=0;
	int incrementVal;
	
	int shortLen=y2-y;
	int longLen=x2-x;
	if (abs(shortLen)>abs(longLen)) {
		int swap=shortLen;
		shortLen=longLen;
		longLen=swap;
		yLonger=1;
	}
	
	if (longLen<0) incrementVal=-1;
	else incrementVal=1;
	
	double divDiff;
	if (shortLen==0) divDiff=longLen;
	else divDiff=(double)longLen/(double)shortLen;
	if (yLonger == 1) {
		for (i=0;i!=longLen;i+=incrementVal) {
			RGS_SetPixel(x+(int)((double)i/divDiff),y+i, color);
		}
	} else {
		for (i=0;i!=longLen;i+=incrementVal) {
			RGS_SetPixel(x+i,y+(int)((double)i/divDiff), color);
		}
	}
}

void RGS_SetPixel(int x, int y, Uint16 color) {
	if(x < 0 || y < 0 || x > g_rgs->mainSurface->w || y > g_rgs->mainSurface->h)
		return;
		
	Uint32 *surface = (Uint32*)g_rgs->mainSurface->pixels;
	surface[(y*g_rgs->mainSurface->w)+x] = color;
}

int RGS_ObjectHitTest(RGS_Object *object, short x, short y) {
	if(x > object->x && x < object->x + object->w && y > object->y && y < object->y + object->h)
		return 1;
	else
		return 0;
}

void RGS_SetCustomUpdateFunction(void (*customUpdate)()) {
	g_rgs->CustomUpdate = customUpdate;
}
