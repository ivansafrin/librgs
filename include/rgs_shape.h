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
 
#ifndef RGS_SHAPE_H_
#define RGS_SHAPE_H_

#define RGS_SHAPE_RECT			0
#define RGS_SHAPE_TRIANGLE		1
#define RGS_SHAPE_RECT_OUTLINE	2

#include "rgs_data.h"

#ifdef __cplusplus
extern "C" {
#endif

/* @package Graphics */
/* @beginmodule Shapes */

typedef struct {
	RGS_Object object;
	RGS_Color color;
	short type;
} RGS_Shape;

extern DECLSPEC RGS_Shape *RGS_CreateShape(RGS_Screen *screen, short type, int color, short x, short y, short w, short h);
extern DECLSPEC void RGS_RenderShape(SDL_Surface *target, void *object);

/* @endmodule */

#ifdef __cplusplus
}
#endif

#endif