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

#ifndef RGS_DATA_H_
#define RGS_DATA_H_

#include <SDL.h>
#include "isutil.h"
#include "snes_ntsc.h"
#include "BFont.h"

#ifdef __cplusplus
extern "C" {
#endif

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
  #define RMASK 0xff000000
  #define GMASK 0x00ff0000
  #define BMASK 0x0000ff00
  #define AMASK 0x000000ff
#else
  #define RMASK 0x000000ff
  #define GMASK 0x0000ff00
  #define BMASK 0x00ff0000
  #define AMASK 0xff000000
#endif

#define RGS_ESCAPE			0x00
#define RGS_UP				0x01
#define RGS_DOWN			0x02
#define RGS_LEFT			0x03
#define RGS_RIGHT			0x04
#define RGS_BUTTON1			0x05
#define RGS_BUTTON2			0x06
#define RGS_BUTTON3			0x07
#define RGS_BUTTON4			0x08
#define RGS_START			0x09
#define RGS_ANYKEY			0x0A
#define RGS_DELETE			0x0B

/* @package Graphics */
/* @beginmodule Basic_Types */

typedef struct {
	int r;
	int g;
	int b;
	int rgb;
} RGS_Color;

typedef struct {
	Uint16 x;
	Uint16 y;
	Uint16 w;
	Uint16 h;
} RGS_Rect;

/* @endmodule */

/* @package Core */
/** @beginmodule Timer */

/* @desc A basic timer
 * @param ticks The current ticks
 * @param last The last tick value
 */

typedef struct {
	long ticks;
	long last;
} RGS_Timer;

/* @endmodule */

/* @package Graphics */
/* @beginmodule Objects */

/** 
 * @desc The basic building block. Added to screens for automatic rendering
 * @param objectID Unique object id assigned by the screen upon addition
 * @param x X position
 * @param y Y position
 * @param w Object width.
 * @param h Object height.
 * @param focused Indicates if the object is currently focused or not
 * @param focusable Indicates if the object is focusable
 * @param index Focus index
 * @param visible Object visibility. 1 for visible, 0 for hidden  
 */
 
 
typedef struct {
	int objectID;
	int x;
	int y;
	int w;
	int h;
	char focused;
	char focusable;
	char visible;
	int index;
	char typeString[9];
	void (*Destroy)(void *object);
	void (*Render)(SDL_Surface *targetSurface, void *object);
	void (*Update)(void *object);
	void (*onRollOver)(void *object);
	void (*onRollOut)(void *object);
	void (*onRelease)(void *object);
	void (*onReleaseOutside)(void *object);
	void (*onPress)(void *object);
	void (*onKeyDown)(void *object, int eventCode);
	void (*onCharInput)(void *object, int charCode);
} RGS_Object;

/* @beginmodule Screens */
/** 
 * @desc Screens hold child objects and are rendered automatically in the render queue.
 * @param objects Child objects to render on every frame
 * @param focusedObject Currently focused object. The focused object receives input callbacks.
 * @param focusedIndex The index of the currently focused object
 * @param visible Screen visibility. 1 for visible, 0 for hidden  
 */

typedef struct {
	ISArray *objects;
	RGS_Object *focusedObject;
	int focusedIndex;
	int visible;
} RGS_Screen;
/* @endmodule */

typedef struct {
	Uint16 sampleRate;
	Uint16 bufferSize;
	SDL_AudioSpec *as;
	Uint8 playingAudio;
	Uint8 playingMusic;

	Uint8 *finalBuffer;
	Uint8 *soundBuffer;
	Uint8 *musicBuffer;	
	Uint8 *musicBufferStart;	
	
	ISArray *sfsPlaybacks;
		
	Uint32 *tmpBuffer;
	Uint16 holdoverSize;
	
	
	Uint16 bufferAvailable;
} RGS_SoundSystem;

typedef struct {
	SDL_Surface *mainSurface;
	SDL_Surface *screenSurface;
	
	SDL_Surface *ntscSurface;
	SDL_Surface *tmpSurface;
	int burst_phase;
	snes_ntsc_t* ntsc;
	
	ISArray *entityRegistrations;
	void *defaultEntity;
	
	int screenType;
	int renderScale;
	int running;
	ISArray *screens;
	RGS_Screen *focusedScreen;
	BFont_Info *smallFont;
	BFont_Info *bigFont;
	Uint8 keyCodes[16];
	int screenWidth;
	int screenHeight;
	int actScreenWidth;
	int actScreenHeight;
	RGS_Color clearColor;
	void (*CustomUpdate)();
	int mouseX;
	int mouseY;
	int paused;
	RGS_Timer frameTimer;
	void (*keyPressCallback)(int keyCode);
	
	//hack
	Uint32 currentDataSize;
	RGS_SoundSystem *soundSystem;
} RGS_Core;

#ifdef __cplusplus
}
#endif


#endif
