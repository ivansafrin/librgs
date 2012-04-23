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

#ifndef RGS_CORE_H_
#define RGS_CORE_H_

/* @package Core */

#include "SDL.h"

#include "rgs_screen.h"
#include "rgs_data.h"
#include "rgs_surface.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RGS_NORMAL	1
#define RGS_NTSC	2

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

/** @beginmodule Program */
/**
	@desc Initializes RGS
	@param width Render surface width
	@param height Render surface height
	@param screenWidth Actual width of the screen the render surface is scaled to. Ignored when screenType is RGS_NTSC
	@param screenHeight Actual height of the screen the render surface is scaled to. Ignored when screenType is RGS_NTSC
	@param screenType Type of rendering engine. Possible values are RGS_NORMAL or RGS_NTSC for NTSC effect
*/
extern DECLSPEC int RGS_Init(int width, int height, int screenWidth, int screenHeight, int screenType);
/**
	@desc Main RGS update loop. Must be called every frame.
*/
extern DECLSPEC int RGS_Update();
/**
	@desc Sends the exit flag to RGS
*/
extern DECLSPEC void RGS_Exit();

/**
	@desc Must be called to shutdown and cleanup at the end
*/
extern DECLSPEC int RGS_Shutdown();
/* @endmodule */

extern DECLSPEC int RGS_RenderScreens();
void DoRenderNormal();
void DoRenderNTSC();

// data stuff
extern DECLSPEC char *RGS_OpenDataPointer(char *filename);

/** @beginmodule Timer */
/**
 * @desc Updates the timer with the system ticks.
 * @param timer The timer to update */
extern DECLSPEC void RGS_UpdateTimer(RGS_Timer *timer);

/**
 * @desc Resets the timer.
 * @param timer The timer to reset */
 
extern DECLSPEC void RGS_ResetTimer(RGS_Timer *timer);

/**
* @desc Checks if a certain amount of time has elapsed
* @param msecs Check if this much time has elapsed in milliseconds
* @return 1 if the amount of time has elapsed, 0 if it hasn't
*/
extern DECLSPEC int RGS_TimerHasElapsed(RGS_Timer *timer, int msecs);
extern DECLSPEC int RGS_TimerElapsed(RGS_Timer *timer);
/** @endmodule */

extern DECLSPEC int RGS_SetClearColor(int r, int g, int b);

// input stuff
extern DECLSPEC Uint8 RGS_KeyDown(Uint8 keyCode);
extern DECLSPEC int RGS_GetKeyMapping(int keycode);

/* @package Input */
/* @beginmodule Keyboard */
extern DECLSPEC void RGS_SetKeyCallback(void (*keyPressCallback)(int keyCode));
/* @beginmodule Mouse */
extern DECLSPEC void RGS_InputMouseMove(int x, int y);
extern DECLSPEC void RGS_InputMouseUp(int x, int y);
extern DECLSPEC void RGS_InputMouseDown(int x, int y);
extern DECLSPEC int RGS_GetMouseX();
extern DECLSPEC int RGS_GetMouseY();
/* @endmodule */

extern DECLSPEC void RGS_Pause(int pauseVal);
extern DECLSPEC int RGS_isPaused();

// misc stuff
extern DECLSPEC RGS_Color RGS_SplitRGB(int color);
extern DECLSPEC void RGS_FillRect(RGS_Rect *rect, int color);
extern DECLSPEC void RGS_DrawLine(int x, int y, int x2, int y2, int color);
extern DECLSPEC void RGS_SetPixel(int x, int y, Uint16 color);

extern DECLSPEC void RGS_SetCustomUpdateFunction(void (*customUpdate)());
extern DECLSPEC int RGS_ObjectHitTest(RGS_Object *object, short x, short y);

/* @package Graphics */
/* @beginmodule Objects */
extern DECLSPEC void RGS_InitializeObject(RGS_Object *object, int x, int y);
extern DECLSPEC void RGS_DestroyObject(RGS_Object *object);
/* @endmodule */

#ifdef __cplusplus
}
#endif

#endif
