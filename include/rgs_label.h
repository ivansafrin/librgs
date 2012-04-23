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
 
#ifndef RGS_LABEL_H_
#define RGS_LABEL_H_

#include "SDL.h"
#include "rgs_data.h"
#include "BFont.h"

#ifdef __cplusplus
extern "C" {
#endif

/* @package Graphics */

#define RGS_FONT_SMALL	0
#define RGS_FONT_BIG	1

extern RGS_Core *g_rgs;

/** @beginmodule Label */

typedef struct {
	RGS_Object object;
	RGS_Color color;
	BFont_Info *font;
	char *text;
} RGS_Label;


extern DECLSPEC RGS_Label *RGS_CreateLabel(RGS_Screen *screen, char *caption, int fontType, int x, int y, int color);
extern DECLSPEC void RGS_RenderLabel(SDL_Surface *target, void *object);
extern DECLSPEC void RGS_SetLabelText(RGS_Label *label, char *text);

/* @endmodule */

#ifdef __cplusplus
}
#endif


#endif

