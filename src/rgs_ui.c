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

#include "rgs_ui.h"
#include "rgs_core.h"

RGS_UIButton *RGS_CreateUIButton(RGS_Screen *screen, char *text, short x, short y, short w, short h) {
	RGS_UIButton *newButton;
	newButton = (RGS_UIButton*)malloc(sizeof(RGS_UIButton));
	memset(newButton, 0, sizeof(RGS_UIButton));


	RGS_InitializeObject(&newButton->uiObject.object, x, y);
	newButton->uiObject.object.w = w;
	newButton->uiObject.object.h = h;

	strcpy(newButton->uiObject.object.typeString, "button");
	
	newButton->uiObject.object.focusable = 1;
	
	newButton->uiObject.object.onRollOver = RGS_UIObjectRollOver;
	newButton->uiObject.object.onRollOut = RGS_UIObjectRollOut;
	newButton->uiObject.object.onRelease = RGS_UIObjectOnRelease;
	newButton->uiObject.object.Render = RGS_RenderUIButton;

	newButton->color = 0xffffff;

	RGS_AddScreenObject(screen, (RGS_Object*)newButton);
	newButton->bg = RGS_CreateShape(screen, RGS_SHAPE_RECT, 0x2c2c2c, x,y,w,h);
	newButton->label = RGS_CreateLabel(screen, text,  RGS_FONT_SMALL, x+((w-BFont_TextWidth(text))/2), y+((h-8)/2)-1, 0xffffff);	
	return newButton;
}

void RGS_RenderUIButton(SDL_Surface *target, void *object) {
	RGS_UIButton *button = (RGS_UIButton*)object;
	
	if(button->selected == 1) {
		button->bg->color = RGS_SplitRGB(0xffffff);
		button->label->color = RGS_SplitRGB(0x000000);
		return;
	}
	
	button->label->color = RGS_SplitRGB(button->color);
	
	if(button->uiObject.rolledOver == 1 || button->uiObject.object.focused == 1)
		button->bg->color = RGS_SplitRGB(0x3c3c3c);
	else
		button->bg->color = RGS_SplitRGB(0x2c2c2c);
}

RGS_UICheckBox *RGS_CreateUICheckBox(RGS_Screen *screen, char *text, short x, short y) {
	RGS_UICheckBox *newCheckBox;
	newCheckBox = (RGS_UICheckBox*)malloc(sizeof(RGS_UICheckBox));
	memset(newCheckBox, 0, sizeof(RGS_UICheckBox));

	RGS_InitializeObject(&newCheckBox->uiObject.object, x, y);
	
	strcpy(newCheckBox->uiObject.object.typeString, "checkbox");
	newCheckBox->uiObject.object.focusable = 1;
	
	newCheckBox->uiObject.object.Render = RGS_RenderUICheckBox;
	newCheckBox->uiObject.object.onRelease = RGS_UIObjectOnRelease;
	newCheckBox->uiObject.onRelease = RGS_UICheckBoxOnRelease;

	RGS_AddScreenObject(screen, (RGS_Object*)newCheckBox);
	newCheckBox->bg = RGS_CreateShape(screen, RGS_SHAPE_RECT, 0x2c2c2c, x,y,10,10);
	newCheckBox->fg = RGS_CreateShape(screen, RGS_SHAPE_RECT, 0xffffff, x+2,y+2,6,6);
	newCheckBox->label = RGS_CreateLabel(screen, text,  RGS_FONT_SMALL, x+15, y+1, 0xffffff);	
	
	newCheckBox->uiObject.object.w = 15+newCheckBox->label->object.w;
	newCheckBox->uiObject.object.h = 10;
	
	return newCheckBox;
}

void RGS_UIHscrollerOnPress(void *target) {
	RGS_UIHorizontalScroller *scroller = (RGS_UIHorizontalScroller*)target;
	if(RGS_ObjectHitTest((RGS_Object*)scroller->fg, RGS_GetMouseX(), RGS_GetMouseY()) == 1) {
		scroller->scrolling = 1;
		scroller->offset = RGS_GetMouseX() - scroller->fg->object.x;
	}
}

void RGS_UIHscrollerOnRelease(void *target) {
	RGS_UIHorizontalScroller *scroller = (RGS_UIHorizontalScroller*)target;
	scroller->scrolling = 0;
}

void RGS_UIVscrollerOnPress(void *target) {
	RGS_UIHorizontalScroller *scroller = (RGS_UIHorizontalScroller*)target;
	if(RGS_ObjectHitTest((RGS_Object*)scroller->fg, RGS_GetMouseX(), RGS_GetMouseY()) == 1) {
		scroller->scrolling = 1;
		scroller->offset = RGS_GetMouseY() - scroller->fg->object.y;
	}
}

void RGS_UIVscrollerOnRelease(void *target) {
	RGS_UIHorizontalScroller *scroller = (RGS_UIHorizontalScroller*)target;
	scroller->scrolling = 0;
}

void RGS_UIHscrollerRender(SDL_Surface *target, void *object) {
	RGS_UIHorizontalScroller *scroller = (RGS_UIHorizontalScroller*)object;
	
	if(scroller->percentage > 100)
		scroller->percentage = 100;
	if(scroller->percentage < 0)
		scroller->percentage = 0;
	
	scroller->fg->object.w = (( ((float)scroller->uiObject.object.w) / 100.0f)) * scroller->percentage;
	
	short oldx = scroller->fg->object.x;
	
	if(scroller->scrolling == 1) {
		scroller->fg->object.x = RGS_GetMouseX() - scroller->offset;
	}
	
	if(scroller->fg->object.x < scroller->uiObject.object.x)
		scroller->fg->object.x = scroller->uiObject.object.x; 

	if(scroller->fg->object.x > scroller->uiObject.object.x + scroller->uiObject.object.w - scroller->fg->object.w)
		scroller->fg->object.x = scroller->uiObject.object.x + scroller->uiObject.object.w - scroller->fg->object.w;
	
	if(scroller->fg->object.x != oldx) {
		scroller->scrollValue = ((float)(scroller->fg->object.x - scroller->uiObject.object.x)) / ((float)( scroller->uiObject.object.w - scroller->fg->object.w));
		if(scroller->onChange != NULL)
			scroller->onChange((void*)scroller);
	}
	
	if(scroller->uiObject.rolledOver == 1) {
		if(RGS_ObjectHitTest((RGS_Object*)scroller->fg, RGS_GetMouseX(), RGS_GetMouseY()) == 1)
			scroller->fg->color = RGS_SplitRGB(0x3c3c3c);
		else
			scroller->fg->color = RGS_SplitRGB(0x2c2c2c);
	} else {
		scroller->fg->color = RGS_SplitRGB(0x2c2c2c);
	}
	
	if(scroller->scrolling == 1)
		scroller->fg->color = RGS_SplitRGB(0x3c3c3c);
}

void RGS_UIVscrollerRender(SDL_Surface *target, void *object) {
	RGS_UIVerticalScroller *scroller = (RGS_UIVerticalScroller*)object;
	
	if(scroller->percentage > 100)
		scroller->percentage = 100;
	if(scroller->percentage < 0)
		scroller->percentage = 0;
	
	scroller->fg->object.h = (( ((float)scroller->uiObject.object.h) / 100.0f)) * scroller->percentage;
	
	short oldy = scroller->fg->object.y;
	
	if(scroller->scrolling == 1) {
		scroller->fg->object.y = RGS_GetMouseY() - scroller->offset;
	}
	
	if(scroller->fg->object.y < scroller->uiObject.object.y)
		scroller->fg->object.y = scroller->uiObject.object.y; 

	if(scroller->fg->object.y > scroller->uiObject.object.y + scroller->uiObject.object.h - scroller->fg->object.h)
		scroller->fg->object.y = scroller->uiObject.object.y + scroller->uiObject.object.h - scroller->fg->object.h;
	
	if(scroller->fg->object.y != oldy) {
		scroller->scrollValue = ((float)(scroller->fg->object.y - scroller->uiObject.object.y)) / ((float)( scroller->uiObject.object.h - scroller->fg->object.h));
		if(scroller->onChange != NULL)
			scroller->onChange((void*)scroller);
	}
	
	if(scroller->uiObject.rolledOver == 1) {
		if(RGS_ObjectHitTest((RGS_Object*)scroller->fg, RGS_GetMouseX(), RGS_GetMouseY()) == 1)
			scroller->fg->color = RGS_SplitRGB(0x3c3c3c);
		else
			scroller->fg->color = RGS_SplitRGB(0x2c2c2c);
	} else {
		scroller->fg->color = RGS_SplitRGB(0x2c2c2c);
	}
	
	if(scroller->scrolling == 1)
		scroller->fg->color = RGS_SplitRGB(0x3c3c3c);
}

RGS_UIHorizontalScroller *RGS_CreateUIHorizontalScroller(RGS_Screen *screen, short x, short y, short w, short h) {
	RGS_UIHorizontalScroller *newScroller;
	newScroller = (RGS_UIHorizontalScroller*)malloc(sizeof(RGS_UIHorizontalScroller));
	memset(newScroller, 0, sizeof(RGS_UIHorizontalScroller));

	RGS_InitializeObject(&newScroller->uiObject.object, x, y);
	
	newScroller->uiObject.object.Render = RGS_UIHscrollerRender;
	newScroller->uiObject.object.onRollOver = RGS_UIObjectRollOver;
	newScroller->uiObject.object.onRollOut = RGS_UIObjectRollOut;
	
	newScroller->userData = NULL;
	
	newScroller->percentage = 100;

	RGS_AddScreenObject(screen, (RGS_Object*)newScroller);
	newScroller->bg = RGS_CreateShape(screen, RGS_SHAPE_RECT, 0x000000, x,y,w,h);
	newScroller->fg = RGS_CreateShape(screen, RGS_SHAPE_RECT, 0x2c2c2c, x,y,10,h);

	newScroller->uiObject.object.onPress = RGS_UIHscrollerOnPress;	
	newScroller->uiObject.object.onRelease = RGS_UIHscrollerOnRelease;
	newScroller->uiObject.object.onReleaseOutside = RGS_UIHscrollerOnRelease;
		
	newScroller->uiObject.object.w = w;
	newScroller->uiObject.object.h = h;
	
	return newScroller;
}

RGS_UIVerticalScroller *RGS_CreateUIVerticalScroller(RGS_Screen *screen, short x, short y, short w, short h) {
	RGS_UIVerticalScroller *newScroller;
	newScroller = (RGS_UIVerticalScroller*)malloc(sizeof(RGS_UIVerticalScroller));
	memset(newScroller, 0, sizeof(RGS_UIVerticalScroller));

	RGS_InitializeObject(&newScroller->uiObject.object, x, y);
	
	newScroller->uiObject.object.Render = RGS_UIVscrollerRender;
	newScroller->uiObject.object.onRollOver = RGS_UIObjectRollOver;
	newScroller->uiObject.object.onRollOut = RGS_UIObjectRollOut;
	
	newScroller->percentage = 100;
	
	newScroller->userData = NULL;

	RGS_AddScreenObject(screen, (RGS_Object*)newScroller);
	newScroller->bg = RGS_CreateShape(screen, RGS_SHAPE_RECT, 0x000000, x,y,w,h);
	newScroller->fg = RGS_CreateShape(screen, RGS_SHAPE_RECT, 0x2c2c2c, x,y,w,10);

	newScroller->uiObject.object.onPress = RGS_UIVscrollerOnPress;	
	newScroller->uiObject.object.onRelease = RGS_UIVscrollerOnRelease;
	newScroller->uiObject.object.onReleaseOutside = RGS_UIVscrollerOnRelease;
		
	newScroller->uiObject.object.w = w;
	newScroller->uiObject.object.h = h;
	
	return newScroller;
}


void RGS_UICheckBoxOnRelease(void *target) {
	RGS_UICheckBox *checkbox = (RGS_UICheckBox*)target;
	if(checkbox->checked == 1)
		checkbox->checked = 0;
	else
		checkbox->checked = 1;
		
	if(checkbox->onChange != NULL)
		checkbox->onChange((void*)checkbox);
}

void RGS_RenderUICheckBox(SDL_Surface *target, void *object) {
	RGS_UICheckBox *checkbox = (RGS_UICheckBox*)object;
	if(checkbox->checked == 1)
		checkbox->fg->object.visible = 1;
	else
		checkbox->fg->object.visible = 0;
}

void RGS_RenderUITextInput(SDL_Surface *target, void *object) {
	RGS_UITextInput *ti = (RGS_UITextInput*)object;
	if(ti->uiObject.object.focused == 1) {
		ti->caret->object.visible = 1;
		ti->caret->object.x = ti->uiObject.object.x+3+ti->label->object.w;
	} else {
		ti->caret->object.visible = 0;
	}
}

RGS_UITextInput *RGS_CreateUITextInput(RGS_Screen *screen, short x, short y, short w, short h) {
	RGS_UITextInput *newTextInput;
	newTextInput = (RGS_UITextInput*)malloc(sizeof(RGS_UITextInput));
	memset(newTextInput, 0, sizeof(RGS_UITextInput));

	RGS_InitializeObject(&newTextInput->uiObject.object, x, y);
	newTextInput->uiObject.object.w = w;
	newTextInput->uiObject.object.h = h;
	
	newTextInput->uiObject.object.focusable = 1;
	
	newTextInput->uiObject.object.Render = RGS_RenderUITextInput;
	newTextInput->uiObject.object.onKeyDown = RGS_OnKeyDownUITextInput;
	newTextInput->uiObject.object.onCharInput = RGS_OnCharUITextInput;

	RGS_AddScreenObject(screen, (RGS_Object*)newTextInput);
	newTextInput->bg = RGS_CreateShape(screen, RGS_SHAPE_RECT, 0x2c2c2c, x,y,w,h);
	newTextInput->label = RGS_CreateLabel(screen, "",  RGS_FONT_SMALL, x+2, y+((h-8)/2)-1, 0xffffff);	
	newTextInput->caret = RGS_CreateShape(screen, RGS_SHAPE_RECT, 0xffffff, x+3+newTextInput->label->object.w,y+2,5,h-4);
	return newTextInput;
}

void RGS_OnCharUITextInput(void *object, int charCode) {
	RGS_UITextInput *ti = (RGS_UITextInput*) object;
	char *newText = (char*)malloc(strlen(ti->label->text)+2);
	if(charCode < 127 && charCode != SDLK_BACKSPACE) {
		strcpy(newText, ti->label->text);
		newText[strlen(ti->label->text)] = charCode;
		newText[strlen(ti->label->text)+1] = '\0';
		RGS_SetLabelText(ti->label, newText);
	}
	free(newText);
}

void RGS_OnKeyDownUITextInput(void *object, int eventCode) {
	RGS_UITextInput *ti = (RGS_UITextInput*) object;
	char *newText = (char*)malloc(strlen(ti->label->text)+2);
	switch(eventCode) {
		case SDLK_BACKSPACE:
			if(strlen(ti->label->text) > 0) {
				strcpy(newText, ti->label->text);
				newText[strlen(ti->label->text)-1] = '\0';
				newText[strlen(ti->label->text)] = '\0';
				RGS_SetLabelText(ti->label, newText);
			}
		break;
	}
	if(newText != NULL)
		free(newText);
}

void RGS_UIObjectRollOver(void *object) {
	RGS_UIObject *uiObject = (RGS_UIObject*) object;

	if(uiObject->disabled == 1)
		return;
	
	uiObject->rolledOver = 1;
}

void RGS_UIObjectRollOut(void *object) {
	RGS_UIObject *uiObject = (RGS_UIObject*) object;

	if(uiObject->disabled == 1)
		return;

	uiObject->rolledOver = 0;
}

void RGS_UIObjectOnRelease(void *object) {
	RGS_UIObject *uiObject = (RGS_UIObject*) object;

	if(uiObject->disabled == 1)
		return;
		
	if(uiObject->onRelease != NULL) {
		uiObject->onRelease((void*)uiObject);
	}
}

RGS_UIOptionList *RGS_CreateUIOptionList(RGS_Screen *screen, short buttonHeight, short x, short y, short w, short h) {
	RGS_UIOptionList *newList;
	newList = (RGS_UIOptionList*)malloc(sizeof(RGS_UIOptionList));
	memset(newList, 0, sizeof(RGS_UIOptionList));

	RGS_InitializeObject(&newList->uiObject.object, x, y);
	newList->uiObject.object.w = w;
	newList->uiObject.object.h = h;
	
	newList->uiObject.object.focusable = 1;
	
	newList->selectedID = -1;
	
	newList->uiObject.object.Render = RGS_RenderUIOptionList;
	
	RGS_AddScreenObject(screen, (RGS_Object*)newList);
	newList->bg = RGS_CreateShape(screen, RGS_SHAPE_RECT, 0x2c2c2c, x,y,w,h);
	
	newList->scrollBar = RGS_CreateUIVerticalScroller(screen, x+w-10, y, 10, h);
	newList->scrollBar->userData = (void*)newList;
	newList->scrollBar->onChange = RGS_ScrollUIOptionList;

	newList->listButtons = ISArray_Create(sizeof(RGS_UIButton));
	newList->listOptions = ISArray_Create(sizeof(RGS_UIOption));
	int i;
	RGS_UIButton *button;
	for(i=0; i<h/buttonHeight; i++) {
		button = RGS_CreateUIButton(screen, "", x+2, y+2+(i*buttonHeight), w-14, buttonHeight);
		button->label->object.x = x+4;
		button->uiObject.object.focusable = 0;
		button->uiObject.onRelease = RGS_UIOptionOnRelease;
		button->uiObject.parent = (void*)newList;
		ISArray_PushElement(newList->listButtons, (void*)button);
	}
	RGS_RefreshUIOptionList(newList);

	return newList;
}

void RGS_ScrollUIOptionList(void *target) {
	RGS_UIVerticalScroller *scroller = (RGS_UIVerticalScroller*)target;
	RGS_UIOptionList *list = (RGS_UIOptionList*)scroller->userData;
	list->scrollOffset = ((float)(list->listOptions->length - list->listButtons->length))*scroller->scrollValue;
	RGS_RefreshUIOptionList(list);
}

void RGS_UIOptionOnRelease(void *target) {
	RGS_UIButton *button = (RGS_UIButton*)target;
	RGS_UIOptionList *list = (RGS_UIOptionList*)button->uiObject.parent;

	int i; 
	for(i=0; i<list->listButtons->length; i++) {
		((RGS_UIButton*)list->listButtons->data[i])->selected = 0;
	}
	button->selected = 1;
	
	RGS_UIOption *option = RGS_GetOptionIdFromButton(list, button);
	if(option == NULL)
		return;
	
	list->selectedID = option->id;
				
	if(list->onSelect != NULL) {
		list->onSelect(option->id, option->data);
	}
}

void RGS_SelectButtonByID(RGS_UIOptionList *list, int id) {
	int i;
	RGS_UIOption *option;
	RGS_UIButton *button;
	
	
	for(i=0; i<list->listButtons->length; i++) {
		button = (RGS_UIButton*)list->listButtons->data[i];
		option = RGS_GetOptionIdFromButton(list, button);
		if(option != NULL) {
			if(option->id == id) {
				button->selected = 1;
			} else {
				button->selected = 0;
			}
		}
	}
}

RGS_UIOption *RGS_GetOptionIdFromButton(RGS_UIOptionList *list, RGS_UIButton *button) {
	int i;
	for(i=0; i<list->listButtons->length; i++) {
		if(list->listButtons->data[i] == button) {
			if(i+list->scrollOffset < list->listOptions->length)
				return (RGS_UIOption*)list->listOptions->data[i+list->scrollOffset];
			else
				return NULL;
		}
	}
	return NULL;
}

void RGS_RenderUIOptionList(SDL_Surface *target, void *object) {
	RGS_UIOptionList *list = (RGS_UIOptionList*)object;
	float tmp = ((float)list->listButtons->length)/((float)list->listOptions->length);
	list->scrollBar->percentage = tmp * 100.0f;
	if(list->scrollBar->percentage > 100)
		list->scrollBar->percentage = 100;
}

void RGS_AddUIOption(RGS_UIOptionList *list, char *name, int id, void *data, int color) {
	RGS_UIOption *newOption = (RGS_UIOption*)ISArray_CreateElement(list->listOptions);
	newOption->data = data;
	newOption->id = id;
	newOption->name = ISString_Create(name);
	newOption->color = color;
	RGS_RefreshUIOptionList(list);
}

void RGS_ClearUIOptionList(RGS_UIOptionList *list) {
	ISArray_Clear(list->listOptions);
	list->scrollOffset = 0;
	list->selectedID = -1;
	int i;
	RGS_UIButton *button;
	for(i=0; i<list->listButtons->length; i++) {
		button = (RGS_UIButton*)list->listButtons->data[i];
		button->selected = 0;
	}
	RGS_RefreshUIOptionList(list);
}

void RGS_RefreshUIOptionList(RGS_UIOptionList *list) {
	int i;
	int len = list->listButtons->length;

	for(i=0; i<len; i++) {
		RGS_SetLabelText(((RGS_UIButton*)list->listButtons->data[i])->label, "");
		((RGS_UIButton*)list->listButtons->data[i])->label->color = RGS_SplitRGB(0x000000);
		((RGS_UIButton*)list->listButtons->data[i])->uiObject.disabled = 1;
	}

	if(list->listOptions->length < len)
		len = list->listOptions->length;
	
	for(i=0; i<len; i++) {
		RGS_SetLabelText(((RGS_UIButton*)list->listButtons->data[i])->label, ((RGS_UIOption*)list->listOptions->data[i+list->scrollOffset])->name->text);
		((RGS_UIButton*)list->listButtons->data[i])->color = ((RGS_UIOption*)list->listOptions->data[i+list->scrollOffset])->color;
		((RGS_UIButton*)list->listButtons->data[i])->uiObject.disabled = 0;
	}
}
