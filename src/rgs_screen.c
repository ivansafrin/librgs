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

#include "rgs_screen.h"

RGS_Screen *RGS_CreateScreen() {
	RGS_Screen *newScreen;
	newScreen = (RGS_Screen*)malloc(sizeof(RGS_Screen));
	memset(newScreen, 0, sizeof(RGS_Screen));
	newScreen->visible = 1;
	newScreen->objects = ISArray_Create(sizeof(RGS_Object));
	RGS_AddScreen(newScreen);
	return newScreen;
}

void RGS_AddScreenObject(RGS_Screen *screen, RGS_Object *object) {
	if(screen == NULL)
		return;
		
	if(object->focusable == 1) {
		screen->focusedObject = object;
		screen->focusedIndex = screen->objects->length;
	}

	object->index = screen->objects->length;		
	ISArray_PushElement(screen->objects, (void*)object);
}

void RGS_ScreenCharInputEvent(RGS_Screen *screen, int charCode) {
	if(screen->focusedObject == NULL)
		return;
		
	if(screen->focusedObject->focusable && screen->focusedObject->onCharInput != NULL)
			screen->focusedObject->onCharInput((void*)screen->focusedObject, charCode);
}

void RGS_AddScreen(RGS_Screen *screen) {
	ISArray_PushElement(g_rgs->screens, (void*)screen);
}

void RGS_ScreenDestroyObjects(RGS_Screen *screen) {
	int i;
	RGS_Object *object;
	for(i = 0; i < screen->objects->length; i++) {
		object = (RGS_Object*)screen->objects->data[i];
		if(object->Destroy != NULL)
			object->Destroy(object);
	}
	
	ISArray_Clear(screen->objects);
}

int RGS_ScreenRemoveObject(RGS_Screen *screen, RGS_Object *object) {
	int i;
	RGS_Object *pobject;
	for(i=0; i< screen->objects->length; i++) {
		pobject = (RGS_Object*)screen->objects->data[i];
		if(object == pobject) {
			ISArray_RemoveElement(screen->objects, (void*)object);
			return 1;
		}
	}
	return 0; 
}

void RGS_FocusScreen(RGS_Screen *screen) {
	int i;
	RGS_Object *object;
	if(g_rgs->focusedScreen != NULL && g_rgs->focusedScreen != screen) {
		for(i=0; i< g_rgs->focusedScreen->objects->length; i++) {
			object = (RGS_Object*)g_rgs->focusedScreen->objects->data[i];
			object->focused = 0;
		}
	}
	g_rgs->focusedScreen = screen;
}

void RGS_ScreenFocusObject(RGS_Screen *screen, RGS_Object *object) {
	if(screen->focusedObject != NULL)
		screen->focusedObject->focused = 0;

	screen->focusedIndex = object->index;
	object->focused = 1;
	screen->focusedObject = object;
}

void RGS_ScreenFocusNext(RGS_Screen *screen) {
	int i = screen->focusedObject->index;
	RGS_Object *object;
	do {
		object = (RGS_Object*)screen->objects->data[i];
		if(object == screen->objects->data[screen->objects->length-1])
			i = 0;
		else
			i++;
	} while(object->focusable != 1);

	RGS_ScreenFocusObject(screen, object);
}

void RGS_RenderScreen(RGS_Screen *screen) {
	int i;
	RGS_Object *object;
	
	for(i=0; i<screen->objects->length; i++) {
		object = (RGS_Object*)screen->objects->data[i];
		if(object->Update != NULL)
			object->Update((void*)object);
		if(object->Render != NULL && object->visible == 1)
			object->Render(g_rgs->mainSurface, (void*)object);
	}	
}
