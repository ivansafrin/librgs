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
 
#ifndef RGS_UI_H_
#define RGS_UI_H_

#include "rgs_data.h"
#include "rgs_core.h"
#include "rgs_label.h"
#include "rgs_shape.h"
#include "isutil.h"

#ifdef __cplusplus
extern "C" {
#endif

/* @package UI */
/* @beginmodule UIObject */

// -------------------------------------------------------------------------
// BASICS
// -------------------------------------------------------------------------

typedef struct {
	RGS_Object object;
	int rolledOver;
	void *parent;
	int disabled;
	void (*onRelease)(void *target);
} RGS_UIObject;

extern DECLSPEC void RGS_UIObjectRollOver(void *object);
extern DECLSPEC void RGS_UIObjectRollOut(void *object);
extern DECLSPEC void RGS_UIObjectOnRelease(void *object);

// -------------------------------------------------------------------------
// BUTTON
// -------------------------------------------------------------------------

/* @beginmodule UIButton */

typedef struct {
	RGS_UIObject uiObject;
	RGS_Label *label;
	RGS_Shape *bg;
	int selected;
	int color;
} RGS_UIButton;

extern DECLSPEC RGS_UIButton *RGS_CreateUIButton(RGS_Screen *screen, char *text, short x, short y, short w, short h);
extern DECLSPEC void RGS_RenderUIButton(SDL_Surface *target, void *object);

/* @beginmodule UIHorizontalScroller */

typedef struct {
	RGS_UIObject uiObject;
	RGS_Shape *bg;
	RGS_Shape *fg;
	short offset;
	short scrolling;
	short percentage;
	float scrollValue;
	void *userData;
	void (*onChange)(void *target);
} RGS_UIHorizontalScroller;

extern DECLSPEC RGS_UIHorizontalScroller *RGS_CreateUIHorizontalScroller(RGS_Screen *screen, short x, short y, short w, short h);
extern DECLSPEC void RGS_UIHscrollerOnRelease(void *target);
extern DECLSPEC void RGS_UIHscrollerOnPress(void *target);
extern DECLSPEC void RGS_UIHscrollerRender(SDL_Surface *target, void *object);

/* @beginmodule UIVerticalScroller */

typedef struct {
	RGS_UIObject uiObject;
	RGS_Shape *bg;
	RGS_Shape *fg;
	short offset;
	short scrolling;
	short percentage;
	float scrollValue;
	void *userData;
	void (*onChange)(void *target);
} RGS_UIVerticalScroller;

extern DECLSPEC RGS_UIVerticalScroller *RGS_CreateUIVerticalScroller(RGS_Screen *screen, short x, short y, short w, short h);
extern DECLSPEC void RGS_UIVscrollerOnRelease(void *target);
extern DECLSPEC void RGS_UIVscrollerOnPress(void *target);
extern DECLSPEC void RGS_UIVscrollerRender(SDL_Surface *target, void *object);

/* @beginmodule UICheckBox */

typedef struct {
	RGS_UIObject uiObject;
	RGS_Label *label;
	RGS_Shape *bg;
	RGS_Shape *fg;
	int checked;
	void (*onChange)(void *target);
} RGS_UICheckBox;

extern DECLSPEC RGS_UICheckBox *RGS_CreateUICheckBox(RGS_Screen *screen, char *text, short x, short y);
extern DECLSPEC void RGS_RenderUICheckBox(SDL_Surface *target, void *object);
extern DECLSPEC void RGS_UICheckBoxOnRelease(void *target);

/* @beginmodule UITextInput */

typedef struct {
	RGS_UIObject uiObject;
	RGS_Label *label;
	RGS_Shape *bg;
	RGS_Shape *caret;
} RGS_UITextInput;

extern DECLSPEC RGS_UITextInput *RGS_CreateUITextInput(RGS_Screen *screen, short x, short y, short w, short h);
extern DECLSPEC void RGS_RenderUITextInput(SDL_Surface *target, void *object);
extern DECLSPEC void RGS_OnKeyDownUITextInput(void *object, int eventCode);
extern DECLSPEC void RGS_OnCharUITextInput(void *object, int charCode);

/* @beginmodule UIOptionList */

typedef struct {
	RGS_UIObject uiObject;
	ISArray *listButtons;
	ISArray *listOptions;
	RGS_UIVerticalScroller *scrollBar;
	Uint16 scrollOffset;
	RGS_Shape *bg;
	int selectedID;
	void (*onSelect)(int id, void *data);
} RGS_UIOptionList;

typedef struct {
	ISString *name;
	int id;
	void *data;
	int color;
} RGS_UIOption;

/* @endmodule */

extern DECLSPEC RGS_UIOptionList *RGS_CreateUIOptionList(RGS_Screen *screen, short buttonHeight, short x, short y, short w, short h);
extern DECLSPEC void RGS_RenderUIOptionList(SDL_Surface *target, void *object);
extern DECLSPEC void RGS_AddUIOption(RGS_UIOptionList *list, char *name, int id, void *data, int color);
extern DECLSPEC void RGS_RefreshUIOptionList(RGS_UIOptionList *list);
extern DECLSPEC void RGS_ClearUIOptionList(RGS_UIOptionList *list);
extern DECLSPEC void RGS_UIOptionOnRelease(void *target);
extern DECLSPEC RGS_UIOption *RGS_GetOptionIdFromButton(RGS_UIOptionList *list, RGS_UIButton *button);
extern DECLSPEC void RGS_SelectButtonByID(RGS_UIOptionList *list, int id);
extern DECLSPEC void RGS_ScrollUIOptionList(void *target);

#ifdef __cplusplus
}
#endif

#endif