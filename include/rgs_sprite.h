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
 
#ifndef RGS_SPRITE_H_
#define RGS_SPRITE_H_

#include "rgs_data.h"
#include "rgs_tilemap.h"
#include "isutil.h"

#ifdef __cplusplus
extern "C" {
#endif


/* @package Graphics */
/* @beginmodule Sprite */

extern RGS_Core *g_rgs;

typedef struct {
	char name[9];
	short numFrames;
	short *frames;
	short speed;
	RGS_Rect bBox;
} RGS_Animation;

typedef struct {
	RGS_Object object;
	RGS_Surface *allFrames;
	RGS_Surface *currentFrame;
	RGS_Rect bBox;
	ISArray *animations;
	RGS_Animation *currentAnimation;
	Uint8 showBBox;
	Uint8 playingOnce;
	short frameWidth;
	short frameHeight;	
	char numAnimations;
	short totalFrames;
	short isAnimated;
	short flipX;
	short flipY;
	short currentFrameIndex;
	RGS_Timer timer;
} RGS_Sprite;

extern DECLSPEC RGS_Sprite *RGS_CreateSprite(RGS_Screen *screen, char *filename);
extern DECLSPEC RGS_Sprite *RGS_CreateEmptySprite(RGS_Screen *screen, RGS_Surface *allFrames, int frameWidth, int frameHeight);
extern DECLSPEC void RGS_RenderSprite(SDL_Surface *target, void *object);
extern DECLSPEC void RGS_UpdateSprite(void *object);
extern DECLSPEC void RGS_SpriteSetBBox(RGS_Sprite *sprite, short x, short y, short w, short h);
extern DECLSPEC void RGS_SpriteFromMemory(RGS_Sprite *sprite, char *data);
extern DECLSPEC RGS_Sprite *RGS_CreateSpriteFromMemory(RGS_Screen *screen, char *data);
extern DECLSPEC RGS_Animation *RGS_AddSpriteAnimation(RGS_Sprite *sprite, char *name, short speed);
extern DECLSPEC void RGS_RemoveAnimation(RGS_Sprite *sprite, RGS_Animation *animation);
extern DECLSPEC void RGS_AddAnimationFrame(RGS_Animation *anim, short frameID);
extern DECLSPEC void RGS_PlaySpriteAnimation(RGS_Sprite *sprite, char *name, Uint8 playOnce);
extern DECLSPEC void RGS_PlaySpriteAnimationById(RGS_Sprite *sprite, int id, Uint8 playOnce);
extern DECLSPEC RGS_Animation *RGS_GetSpriteAnimationById(RGS_Sprite *sprite, int id);
extern DECLSPEC void RGS_SaveSprite(RGS_Sprite *sprite, char *filename);

/* package Tilemap */
/* @beginmodule EntitySprite */

typedef struct {
	RGS_Sprite *sprite;
	RGS_Entity *entity;
} RGS_EntitySprite;


extern DECLSPEC RGS_EntitySprite *RGS_CreateEntitySpriteFromMemory(RGS_Screen *screen, RGS_Tilemap *tilemap, char *data);
extern DECLSPEC RGS_EntitySprite *RGS_CreateEntitySprite(RGS_Screen *screen, RGS_Tilemap *tilemap, char *filename);

/* @endmodule */

#ifdef __cplusplus
}
#endif

#endif
