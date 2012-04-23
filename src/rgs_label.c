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

#include "rgs_label.h"
#include "BFont.h"
#include "rgs_core.h"
#include "SDL.h"

RGS_Label *RGS_CreateLabel(RGS_Screen *screen, char *caption, int fontType, int x, int y, int color) {
	RGS_Label *newLabel;
	newLabel = (RGS_Label*)malloc(sizeof(RGS_Label));
	memset(newLabel, 0, sizeof(RGS_Label));
	RGS_InitializeObject(&newLabel->object, x, y);
	RGS_AddScreenObject(screen, (RGS_Object*)newLabel);
	newLabel->object.Render = RGS_RenderLabel;
	
	switch(fontType) {
		case RGS_FONT_SMALL:
			newLabel->font = g_rgs->smallFont;
		break;
		case RGS_FONT_BIG:
			newLabel->font = g_rgs->bigFont;
		break;
		default:
			newLabel->font = g_rgs->smallFont;
		break;
	}
	
	RGS_SetLabelText(newLabel, caption);

	newLabel->color = RGS_SplitRGB(color);
	
	return newLabel;
}

void RGS_RenderLabel(SDL_Surface *target, void *object) {
	RGS_Label *label = (RGS_Label*)object;
	
	label->font->Surface->format->palette->colors[2].r = label->color.r;
	label->font->Surface->format->palette->colors[2].g = label->color.g;
	label->font->Surface->format->palette->colors[2].b = label->color.b;
	

	BFont_SetCurrentFont(label->font);
	BFont_PutString(target, ((RGS_Object*)object)->x, ((RGS_Object*)object)->y, label->text);
}

void RGS_SetLabelText(RGS_Label *label, char *text) {
	BFont_SetCurrentFont(label->font);
	label->object.w = BFont_TextWidth(text);
	
	if(label->text != NULL)
		free(label->text);
		
	label->text = (char*)malloc(strlen(text)+1);
	strcpy(label->text, text);
}