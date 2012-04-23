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

#include "sprite_editor.h"
#include "rgs.h"
#include "globals.h"

RGS_Screen *spriteEditorScreen;

RGS_Sprite *workingSprite;
RGS_Animation *workingAnimation;
RGS_SurfaceImage *spriteImage;


RGS_UIOptionList *spriteAnimations;
RGS_UIButton *newAnimButton;
RGS_UIButton *removeAnimButton;


RGS_UIButton *spriteUpdateButton;
RGS_UIButton *setFramesButton;
int showFrameSelector = 0;
RGS_Shape *frameSelector;
int selectedFrameIndex = 0;

RGS_UITextInput *animName;
RGS_UITextInput *spriteSpeed;
RGS_Label *spriteSheetLabel;

RGS_UITextInput *animBboxX;
RGS_UITextInput *animBboxY;
RGS_UITextInput *animBboxW;
RGS_UITextInput *animBboxH;

RGS_UICheckBox *showBBox;

extern RGS_Screen *bmpScreen;
extern RGS_Surface *bmpSurface;

void saveSpriteAnimation(void *target) {
	workingAnimation->speed = atoi(spriteSpeed->label->text);

	workingAnimation->bBox.x = atoi(animBboxX->label->text);
	workingAnimation->bBox.y = atoi(animBboxY->label->text);
	workingAnimation->bBox.w = atoi(animBboxW->label->text);
	workingAnimation->bBox.h = atoi(animBboxH->label->text);
			
	
	if(strcmp(workingAnimation->name, "default") != 0)
		strcpy(workingAnimation->name, animName->label->text);
	reloadAnimList();
	RGS_PlaySpriteAnimation(workingSprite, workingAnimation->name,0);
}

void openSprite(char *newSprite) {
	if(workingSprite != NULL) {
		RGS_DestroyObject((RGS_Object*)workingSprite);
	}
	if(spriteImage != NULL) {
		RGS_DestroyObject((RGS_Object*)spriteImage);
	}	
	workingSprite = RGS_CreateSprite(spriteEditorScreen, newSprite);
	spriteImage = RGS_CreateSurfaceImage(spriteEditorScreen, workingSprite->allFrames, EDITOR_LEFT_MARGIN, EDITOR_TOP_MARGIN+5+50);
	spriteEditorScreen->visible = 1;
	bmpScreen->visible = 0;
	refreshSpriteEditor();
}

void Editor_UpdateSpriteEditor() {
	if(spriteImage == NULL || workingSprite == NULL || 
	RGS_GetMouseX() < spriteImage->object.x || RGS_GetMouseY() < spriteImage->object.y ||
	RGS_GetMouseX() > spriteImage->object.x + spriteImage->surface->surface->w || RGS_GetMouseY() > spriteImage->object.y  + spriteImage->surface->surface->h)
		return;

	frameSelector->object.x = spriteImage->object.x + (RGS_GetMouseX() - spriteImage->object.x) - ((RGS_GetMouseX() - spriteImage->object.x) % workingSprite->frameWidth);
	frameSelector->object.y = spriteImage->object.y + (RGS_GetMouseY() - spriteImage->object.y) - ((RGS_GetMouseY() - spriteImage->object.y) % workingSprite->frameHeight);

	selectedFrameIndex = ((((frameSelector->object.y - spriteImage->object.y)/workingSprite->frameHeight))*(workingSprite->allFrames->surface->w/workingSprite->frameWidth))+((frameSelector->object.x-spriteImage->object.x)/workingSprite->frameWidth);
}

void Sprite_CreateFromBMP(int width, int height) {
	if(workingSprite != NULL) {
		RGS_DestroyObject((RGS_Object*)workingSprite);
	}
	if(spriteImage != NULL) {
		RGS_DestroyObject((RGS_Object*)spriteImage);
	}
	
	workingSprite = RGS_CreateEmptySprite(spriteEditorScreen, bmpSurface, width, height);
	spriteImage = RGS_CreateSurfaceImage(spriteEditorScreen, workingSprite->allFrames, EDITOR_LEFT_MARGIN, EDITOR_TOP_MARGIN+5+50);
	refreshSpriteEditor();
	spriteEditorScreen->visible = 1;
	bmpScreen->visible = 0;
}

void initSpriteEditor() {
	spriteEditorScreen = RGS_CreateScreen();
	RGS_FocusScreen(spriteEditorScreen);
	spriteEditorScreen->visible = 0;
		
	newAnimButton = RGS_CreateUIButton(spriteEditorScreen, "New", EDITOR_LEFT_MARGIN+5, EDITOR_TOP_MARGIN-23, 40, 18);
	newAnimButton->uiObject.onRelease = Editor_NewAnimation;
	removeAnimButton = RGS_CreateUIButton(spriteEditorScreen, "Remove", EDITOR_LEFT_MARGIN+50, EDITOR_TOP_MARGIN-23, 55, 18);
	removeAnimButton->uiObject.onRelease = Editor_RemoveAnimation;
			
	animName = RGS_CreateUITextInput(spriteEditorScreen, EDITOR_LEFT_MARGIN+160, 40, 100, 18);
	spriteSpeed = RGS_CreateUITextInput(spriteEditorScreen, EDITOR_LEFT_MARGIN+160, 65, 100, 18);
	spriteUpdateButton = RGS_CreateUIButton(spriteEditorScreen, "Update Animation", EDITOR_LEFT_MARGIN+120, EDITOR_TOP_MARGIN-23, 150, 18);
	
	RGS_CreateLabel(spriteEditorScreen, "Name:", RGS_FONT_SMALL, EDITOR_LEFT_MARGIN+120, 44,0x4c4c4c);
	RGS_CreateLabel(spriteEditorScreen, "	Speed:", RGS_FONT_SMALL, EDITOR_LEFT_MARGIN+120, 69,0x4c4c4c);
	spriteAnimations = RGS_CreateUIOptionList(spriteEditorScreen, 15, EDITOR_LEFT_MARGIN+5, 40, 100, 100);
	spriteAnimations->onSelect = playSpriteAnimation;
	
	RGS_CreateLabel(spriteEditorScreen, "ANIMATIONS", RGS_FONT_BIG, EDITOR_LEFT_MARGIN+5,24,0x79CA00);
	RGS_CreateLabel(spriteEditorScreen, "ANIMATION PROPERTIES", RGS_FONT_BIG, EDITOR_LEFT_MARGIN+120,24,0x6F7FF8);
	RGS_CreateLabel(spriteEditorScreen, "ACTIONS", RGS_FONT_BIG, EDITOR_LEFT_MARGIN+485,24,0xFF8C49);

	RGS_CreateLabel(spriteEditorScreen, "PREVIEW", RGS_FONT_BIG, EDITOR_LEFT_MARGIN+5,EDITOR_TOP_MARGIN+5,0x4c4c4c);
	spriteSheetLabel = RGS_CreateLabel(spriteEditorScreen, "SPRITE SHEET (click on frames to add them to the current animation)", RGS_FONT_BIG, EDITOR_LEFT_MARGIN+5,EDITOR_TOP_MARGIN+5,0x4c4c4c);
	
	
	RGS_CreateLabel(spriteEditorScreen, "BBox X:", RGS_FONT_SMALL, EDITOR_LEFT_MARGIN+285, 44,0x4c4c4c);
	RGS_CreateLabel(spriteEditorScreen, "BBox Y:", RGS_FONT_SMALL, EDITOR_LEFT_MARGIN+285, 69,0x4c4c4c);
	RGS_CreateLabel(spriteEditorScreen, "BBox W:", RGS_FONT_SMALL, EDITOR_LEFT_MARGIN+285, 94,0x4c4c4c);
	RGS_CreateLabel(spriteEditorScreen, "BBox H:", RGS_FONT_SMALL, EDITOR_LEFT_MARGIN+285, 119,0x4c4c4c);
	
	showBBox = RGS_CreateUICheckBox(spriteEditorScreen, "Show bbox", EDITOR_LEFT_MARGIN+285, 150);
	showBBox->onChange = changeShowBBox;
	
	animBboxX = RGS_CreateUITextInput(spriteEditorScreen, EDITOR_LEFT_MARGIN+325, 40, 40, 18);	
	animBboxY = RGS_CreateUITextInput(spriteEditorScreen, EDITOR_LEFT_MARGIN+325, 65, 40, 18);
	animBboxW = RGS_CreateUITextInput(spriteEditorScreen, EDITOR_LEFT_MARGIN+325, 90, 40, 18);	
	animBboxH = RGS_CreateUITextInput(spriteEditorScreen, EDITOR_LEFT_MARGIN+325, 115, 40, 18);
				
	setFramesButton = RGS_CreateUIButton(spriteEditorScreen, "Clear Frames", EDITOR_LEFT_MARGIN+485, 40, 100, 18);
	frameSelector = RGS_CreateShape(spriteEditorScreen, RGS_SHAPE_RECT, 0x121212, 0,0,10,10);
	
	spriteUpdateButton->uiObject.onRelease = saveSpriteAnimation;
}

void changeShowBBox(void *target) {
	if(workingSprite != NULL)
		workingSprite->showBBox = showBBox->checked;
}

void Editor_NewAnimation(void *target) {
	RGS_AddSpriteAnimation(workingSprite, "new anim", 10);
	refreshSpriteEditor();
}

void Editor_RemoveAnimation(void *target) {
	if(strcmp(workingAnimation->name, "default") == 0)
		return;
		
	RGS_RemoveAnimation(workingSprite, workingAnimation);
	refreshSpriteEditor();
}

void reloadAnimList() {
	int i;
	RGS_ClearUIOptionList(spriteAnimations);
	for (i=0; i < workingSprite->animations->length; i++) {
		RGS_AddUIOption(spriteAnimations, ((RGS_Animation*)workingSprite->animations->data[i])->name, i, NULL, 0xffffff);
	}
}

void Editor_SpriteSave(void *filename) {
	RGS_SaveSprite(workingSprite, (char*)filename);
}

void refreshSpriteEditor() {
	
	reloadAnimList();

	spriteImage->object.onRelease = Editor_AddFrame;

	frameSelector->object.w = workingSprite->frameWidth;
	frameSelector->object.h = workingSprite->frameHeight;
	
	playSpriteAnimation(0, NULL);
	workingSprite->object.x = EDITOR_LEFT_MARGIN+5;
	workingSprite->object.y = EDITOR_TOP_MARGIN+25;
	
	spriteSheetLabel->object.y = workingSprite->object.y + workingSprite->currentFrame->surface->h+10;
	spriteImage->object.y = spriteSheetLabel->object.y + 25;	
}


void playSpriteAnimation(int id, void *data) {
	RGS_PlaySpriteAnimationById(workingSprite, id,0);
	RGS_Animation *anim = RGS_GetSpriteAnimationById(workingSprite, id);
	RGS_SetLabelText(animName->label, anim->name);

	char tmp[16];
	
	sprintf(tmp, "%d", anim->speed);
	RGS_SetLabelText(spriteSpeed->label, tmp);

	sprintf(tmp, "%d", anim->bBox.x);
	RGS_SetLabelText(animBboxX->label, tmp);

	sprintf(tmp, "%d", anim->bBox.y);
	RGS_SetLabelText(animBboxY->label, tmp);
	
	sprintf(tmp, "%d", anim->bBox.w);
	RGS_SetLabelText(animBboxW->label, tmp);
	
	sprintf(tmp, "%d", anim->bBox.h);
	RGS_SetLabelText(animBboxH->label, tmp);
	
	workingAnimation = anim;
}

void Editor_AddFrame(void *target) {
	RGS_AddAnimationFrame(workingAnimation, selectedFrameIndex);
}
