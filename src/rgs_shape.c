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
 
#include "rgs_data.h"
#include "rgs_core.h"
#include "rgs_shape.h"

RGS_Shape *RGS_CreateShape(RGS_Screen *screen, short type, int color, short x, short y, short w, short h) {
	RGS_Shape *newShape;
	newShape = (RGS_Shape*)malloc(sizeof(RGS_Shape));
	memset(newShape, 0, sizeof(RGS_Shape));
	RGS_InitializeObject(&newShape->object, 0, 0);
	RGS_AddScreenObject(screen, (RGS_Object*)newShape);
	newShape->object.Render = RGS_RenderShape;
	newShape->object.x = x;
	newShape->object.y = y;
	newShape->object.w = w;
	newShape->object.h = h;
	newShape->type = type;
	
	RGS_Color tmp = RGS_SplitRGB(color);
	newShape->color = tmp;
	return newShape;	
}

void RGS_RenderShape(SDL_Surface *target, void *object) {
	RGS_Shape *shape = (RGS_Shape*)object;
	RGS_Rect rect;
	rect.h = shape->object.h;
	rect.w = shape->object.w;
	rect.x = shape->object.x;
	rect.y = shape->object.y;
				
	switch(shape->type) {
		case RGS_SHAPE_RECT:
			RGS_FillRect(&rect, shape->color.rgb);
		break;
		case RGS_SHAPE_RECT_OUTLINE:
			RGS_DrawLine(rect.x, rect.y, rect.x+rect.w, rect.y, shape->color.rgb);
			RGS_DrawLine(rect.x+rect.w, rect.y, rect.x+rect.w, rect.y+rect.h, shape->color.rgb);
			RGS_DrawLine(rect.x+rect.w, rect.y+rect.h, rect.x, rect.y+rect.h, shape->color.rgb);
			RGS_DrawLine(rect.x, rect.y+rect.h, rect.x, rect.y, shape->color.rgb);
		break;
		default:
		break;
	}
}