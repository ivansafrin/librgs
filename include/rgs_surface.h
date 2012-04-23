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
 
#ifndef RGS_SURFACE_H_
#define RGS_SURFACE_H_

#include "SDL.h"
#include "rgs_data.h"
#include "rgs_core.h"

#ifdef __cplusplus
extern "C" {
#endif

/* @package Graphics */
/* @beginmodule Surface */

typedef struct {
	Uint16 numcolors;
	RGS_Color *colors;
} RGS_Palette;

typedef struct {
	SDL_Surface *surface;
	SDL_Palette* pOrigPal;
	int numPals;
	int curPaletteIndex;
	int loaded;
	RGS_Palette palette;
} RGS_Surface;

typedef struct {
	RGS_Object object;
	RGS_Surface *surface;
} RGS_SurfaceImage;

extern DECLSPEC RGS_Surface *RGS_SurfaceFromMemory(char *imageData);
extern DECLSPEC RGS_Surface *RGS_CreateSurfaceFromSurface(RGS_Surface *srcSurface, Uint16 width, Uint16 height);
extern DECLSPEC RGS_Surface *RGS_LoadBMPSurface(char *filename);
extern DECLSPEC RGS_Surface *RGS_LoadSurface(char *filename);
extern DECLSPEC RGS_Surface *RGS_SurfaceFromBMPMemory(char *imageData);
extern DECLSPEC void RGS_CopySurface(RGS_Surface *src, RGS_Rect *srcRect, RGS_Surface *dest, RGS_Rect *destRect);
extern DECLSPEC void RGS_MirrorSurfaceX(RGS_Surface *surface);
extern DECLSPEC void RGS_MirrorSurfaceY(RGS_Surface *surface);
extern DECLSPEC RGS_SurfaceImage *RGS_CreateSurfaceImage(RGS_Screen *screen, RGS_Surface *surface, short x, short y);
extern DECLSPEC RGS_SurfaceImage *RGS_CreateSurfaceImageFromFile(RGS_Screen *screen, char *filename, short x, short y);
extern DECLSPEC void RGS_RenderSurfaceImage(SDL_Surface *target, void *object);
extern DECLSPEC void RGS_WriteSurface(RGS_Surface *surface, char *filename, int append);
extern DECLSPEC void RGS_CopySurfacePalette(RGS_Surface *destSurface, RGS_Surface *srcSurface);
extern DECLSPEC RGS_Color RGS_GetSurfaceColorByIndex(RGS_Surface *surface, Uint8 index);
extern DECLSPEC void RGS_FreeSurface(RGS_Surface *surface);
extern DECLSPEC void RGS_LoadSurfacePalette(RGS_Surface *surface, char *palette);
	
/* @endmodule Surface */

#ifdef __cplusplus
}
#endif

#endif