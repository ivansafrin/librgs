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

#include "rgs_sprite.h"
#include "isutil.h"


RGS_Sprite *RGS_CreateSprite(RGS_Screen *screen, char *filename) {
	RGS_Sprite *newSprite;
	newSprite = (RGS_Sprite*)malloc(sizeof(RGS_Sprite));
	memset(newSprite, 0, sizeof(RGS_Sprite));
	
	RGS_InitializeObject(&newSprite->object, 0, 0);
	RGS_AddScreenObject(screen, (RGS_Object*)newSprite);
	newSprite->object.Render = RGS_RenderSprite;
	newSprite->object.Update = RGS_UpdateSprite;
	
	char *data = (char*)RGS_OpenDataPointer(filename);
	if(data != NULL) {
		RGS_SpriteFromMemory(newSprite, data);
		free(data);
	}
	RGS_ResetTimer(&newSprite->timer);
	
	return newSprite;
}

RGS_Sprite *RGS_CreateEmptySprite(RGS_Screen *screen, RGS_Surface *allFrames, int frameWidth, int frameHeight) {
	RGS_Sprite *newSprite;
	newSprite = (RGS_Sprite*)malloc(sizeof(RGS_Sprite));
	memset(newSprite, 0, sizeof(RGS_Sprite));
	
	RGS_InitializeObject(&newSprite->object, 0, 0);
	RGS_AddScreenObject(screen, (RGS_Object*)newSprite);
	newSprite->object.Render = RGS_RenderSprite;
	newSprite->object.Update = RGS_UpdateSprite;

	newSprite->frameWidth = frameWidth;
	newSprite->frameHeight = frameHeight;
	
	newSprite->totalFrames = (allFrames->surface->w/frameWidth)*(allFrames->surface->h/frameHeight);
	newSprite->allFrames = allFrames;
	newSprite->animations = ISArray_Create(sizeof(RGS_Animation));
		
	newSprite->currentFrame = (RGS_Surface*)malloc(sizeof(RGS_Surface));
	memset(newSprite->currentFrame, 0, sizeof(RGS_Surface));
	
	newSprite->currentFrame->surface = SDL_CreateRGBSurface(SDL_SWSURFACE|SDL_ANYFORMAT,newSprite->frameWidth, newSprite->frameHeight,8,RMASK,GMASK,BMASK,AMASK);
	SDL_LockSurface(newSprite->currentFrame->surface);
	SDL_SetPalette(newSprite->currentFrame->surface,SDL_LOGPAL|SDL_PHYSPAL,newSprite->allFrames->surface->format->palette->colors,0,newSprite->allFrames->surface->format->palette->ncolors);
	SDL_SetColorKey(newSprite->currentFrame->surface,SDL_SRCCOLORKEY,SDL_MapRGB(newSprite->currentFrame->surface->format,255,0,255));
	SDL_UnlockSurface(newSprite->currentFrame->surface);

	newSprite->isAnimated = 1;
	RGS_Animation *defaultAnim = RGS_AddSpriteAnimation(newSprite, "default", 10);

	int i;
	for(i = 0; i < newSprite->totalFrames; i++) {
		RGS_AddAnimationFrame(defaultAnim, i);
	}
		
	RGS_PlaySpriteAnimation(newSprite, "default",0);
	
	RGS_ResetTimer(&newSprite->timer);
	
	return newSprite;	
}

RGS_EntitySprite *RGS_CreateEntitySprite(RGS_Screen *screen, RGS_Tilemap *tilemap, char *filename) {
	RGS_EntitySprite *newSprite = (RGS_EntitySprite*)malloc(sizeof(RGS_EntitySprite));
	memset(newSprite, 0, sizeof(RGS_EntitySprite));
	newSprite->sprite = RGS_CreateSprite(screen, filename);
	newSprite->entity = RGS_CreateEntity(tilemap, &newSprite->sprite->bBox, &newSprite->sprite->object);
	return newSprite;
}

RGS_EntitySprite *RGS_CreateEntitySpriteFromMemory(RGS_Screen *screen, RGS_Tilemap *tilemap, char *data) {
	RGS_EntitySprite *newSprite = (RGS_EntitySprite*)malloc(sizeof(RGS_EntitySprite));
	memset(newSprite, 0, sizeof(RGS_EntitySprite));
	newSprite->sprite = RGS_CreateSpriteFromMemory(screen, data);
	newSprite->entity = RGS_CreateEntity(tilemap, &newSprite->sprite->bBox, &newSprite->sprite->object);
	return newSprite;
}

void RGS_UpdateSprite(void *object) {
	RGS_Sprite *sprite = (RGS_Sprite*)object;
	
	if(sprite->currentAnimation->numFrames < 1)
		return;
	
	RGS_UpdateTimer(&sprite->timer);
	
	if(sprite->currentAnimation == 0)
		return;
		
	if(RGS_TimerHasElapsed(&sprite->timer, sprite->currentAnimation->speed) == 0)
		return;
	
	sprite->currentFrameIndex++;
	if(sprite->currentFrameIndex >= sprite->currentAnimation->numFrames) {
			if(sprite->playingOnce == 0)
				sprite->currentFrameIndex = 0;
			else
				sprite->currentFrameIndex = sprite->currentAnimation->numFrames-1;
	}
	
	int frameID = sprite->currentAnimation->frames[sprite->currentFrameIndex];	
	
	RGS_Rect srcRGSRect;
	srcRGSRect.x = (frameID*sprite->frameWidth) % (sprite->allFrames->surface->w);
	srcRGSRect.y = ((frameID*sprite->frameWidth) / (sprite->allFrames->surface->w)) * sprite->frameHeight;
	srcRGSRect.w =  sprite->frameWidth;
	srcRGSRect.h =  sprite->currentFrame->surface->h;

	RGS_Rect dstRGSRect;
	dstRGSRect.x = 0;
	dstRGSRect.y = 0;
	dstRGSRect.w =  sprite->frameWidth;
	dstRGSRect.h =  sprite->currentFrame->surface->h;

	RGS_CopySurface(sprite->allFrames, &srcRGSRect, sprite->currentFrame, &dstRGSRect);

	if(sprite->flipX == 1) {
		RGS_MirrorSurfaceX(sprite->currentFrame);
	}
	if(sprite->flipY == 1) {
		RGS_MirrorSurfaceY(sprite->currentFrame);
	}
}

void RGS_RenderSprite(SDL_Surface *target, void *object) {
	if(object == 0)
		return;
		
	SDL_Rect srcRect, destRect;
	
	RGS_Sprite *sprite = (RGS_Sprite*)object;

	srcRect.x = 0;
	srcRect.y = 0;
	srcRect.w = sprite->currentFrame->surface->w;
	srcRect.h = sprite->currentFrame->surface->h;
	
	destRect.x = ((RGS_Object*)object)->x;
	destRect.y = ((RGS_Object*)object)->y;
	destRect.w = srcRect.w;
	destRect.h = srcRect.h;
	
	SDL_BlitSurface(sprite->currentFrame->surface, &srcRect, target, &destRect);
	
	RGS_Rect rect = sprite->bBox;
	rect.x += ((RGS_Object*)object)->x;
	rect.y += ((RGS_Object*)object)->y;
	
	if(sprite->showBBox == 1) {
		RGS_FillRect(&rect, 0x00ff00);
	}
}

void RGS_SpriteSetBBox(RGS_Sprite *sprite, short x, short y, short w, short h) {
		sprite->bBox.x = x;
		sprite->bBox.y = y;
		sprite->bBox.w = w;
		sprite->bBox.h = h;
}

void RGS_SaveSprite(RGS_Sprite *sprite, char *filename) {
	int i,j;
	char tByte;

	FILE *outPtr = fopen(filename, "wb");
	char header[4] = {'S', 'P', 'R', '2'};
	fwrite(header, 4, 1, outPtr);
	fwrite(&sprite->bBox, sizeof(RGS_Rect), 1, outPtr);
		
	fwrite(&sprite->frameWidth, 2, 1, outPtr);
	fwrite(&sprite->frameHeight, 2, 1, outPtr);
	
	tByte = sprite->animations->length;
	fwrite(&tByte, 1, 1, outPtr);
	
	for(i=0; i <sprite->animations->length; i++) {
		tByte = (char)strlen(((RGS_Animation*)sprite->animations->data[i])->name);
		fwrite(&tByte, 1, 1, outPtr);
		fwrite(((RGS_Animation*)sprite->animations->data[i])->name, 1 , tByte, outPtr);
		fwrite(&((RGS_Animation*)sprite->animations->data[i])->speed, 2, 1, outPtr);
		fwrite(&((RGS_Animation*)sprite->animations->data[i])->bBox, sizeof(RGS_Rect), 1, outPtr);
		fwrite(&((RGS_Animation*)sprite->animations->data[i])->numFrames, 2, 1, outPtr);
		for(j=0;j<((RGS_Animation*)sprite->animations->data[i])->numFrames;j++) {
			fwrite(&((RGS_Animation*)sprite->animations->data[i])->frames[j], 2, 1, outPtr);
		}
	}

	fclose(outPtr);
	RGS_WriteSurface(sprite->allFrames, filename, 1);
}

RGS_Sprite *RGS_CreateSpriteFromMemory(RGS_Screen *screen, char *data) {
	RGS_Sprite *newSprite = (RGS_Sprite*)malloc(sizeof(RGS_Sprite));
	memset(newSprite,0,sizeof(RGS_Sprite));
	
	RGS_InitializeObject(&newSprite->object, 0, 0);
	RGS_AddScreenObject(screen, (RGS_Object*)newSprite);
	newSprite->object.Render = RGS_RenderSprite;
	newSprite->object.Update = RGS_UpdateSprite;
	
	RGS_SpriteFromMemory(newSprite, data);
	return newSprite;
}

void RGS_SpriteFromMemory(RGS_Sprite *sprite, char *data) {
	
	int i,j;
	char animName[9];
	short numFrames;
	RGS_Animation *newAnim;
	
	data += 4;
	RGS_Rect *box = (RGS_Rect*)data;
	RGS_SpriteSetBBox(sprite,box->x,box->y,box->w,box->h);
	data += sizeof(RGS_Rect);
	
	sprite->animations = ISArray_Create(sizeof(RGS_Animation));
		
//	sprite->frameWidth = (short)*data;
	memcpy(&sprite->frameWidth, data,2);
	data+=2;
	memcpy(&sprite->frameHeight, data,2);	
//	sprite->frameHeight = (short)*data;
	data+=2;
	sprite->numAnimations = *data;
	data++;
	
	short speed;
	for(i = 0; i < sprite->numAnimations; i++) {
		memset(animName, 0, 9);
		memcpy(animName, data+1, *data);
		data += (*data)+1;
		memcpy(&speed, data, 2);
		newAnim = RGS_AddSpriteAnimation(sprite, animName, speed);
		data += 2;
		memcpy(&newAnim->bBox, data, sizeof(RGS_Rect));
		data += sizeof(RGS_Rect);
		
		numFrames = *data;
		data += 2;
		for(j=0;j<numFrames;j++) {
			RGS_AddAnimationFrame(newAnim, *data);
			data += 2;
		}
	}

	sprite->allFrames = (RGS_Surface*)RGS_SurfaceFromMemory(data);

	sprite->currentFrame = (RGS_Surface*)malloc(sizeof(RGS_Surface));
	memset(sprite->currentFrame, 0, sizeof(RGS_Surface));
	
	sprite->object.w = sprite->frameWidth;
	sprite->object.h = sprite->frameHeight;
		
	sprite->currentFrame->surface = SDL_CreateRGBSurface(SDL_SWSURFACE|SDL_ANYFORMAT,sprite->frameWidth, sprite->frameHeight,8,RMASK,GMASK,BMASK,AMASK);
	
	SDL_LockSurface(sprite->currentFrame->surface);
	SDL_SetPalette(sprite->currentFrame->surface,SDL_LOGPAL|SDL_PHYSPAL,sprite->allFrames->surface->format->palette->colors,0,sprite->allFrames->surface->format->palette->ncolors);
	SDL_SetColorKey(sprite->currentFrame->surface,SDL_SRCCOLORKEY,SDL_MapRGB(sprite->currentFrame->surface->format,255,0,255));
	SDL_UnlockSurface(sprite->currentFrame->surface);

	sprite->isAnimated = 1;
	RGS_PlaySpriteAnimation(sprite, "default", 0);
}

RGS_Animation *RGS_AddSpriteAnimation(RGS_Sprite *sprite, char *name, short speed) {
	RGS_Animation *newAnimation = (RGS_Animation*)ISArray_CreateElement(sprite->animations);
	newAnimation->speed = speed;
	newAnimation->numFrames = 0;
	strcpy(newAnimation->name, name);
	return newAnimation;
}


void RGS_RemoveAnimation(RGS_Sprite *sprite, RGS_Animation *animation) {
	ISArray_RemoveElement(sprite->animations, animation);
}

void RGS_AddAnimationFrame(RGS_Animation *anim, short frameID) {
	anim->numFrames++;
	anim->frames = (short*)realloc(anim->frames, sizeof(short)*anim->numFrames);
	anim->frames[anim->numFrames-1] = frameID;
}

RGS_Animation *RGS_GetSpriteAnimationById(RGS_Sprite *sprite, int id) {
	return (RGS_Animation*)sprite->animations->data[id];
}

void RGS_PlaySpriteAnimationById(RGS_Sprite *sprite, int id, Uint8 playOnce) {
	sprite->playingOnce = playOnce;
	RGS_Animation *anim = (RGS_Animation*)sprite->animations->data[id];
	sprite->currentAnimation = anim;
	sprite->currentFrameIndex = 0;
	sprite->bBox = anim->bBox;
}

void RGS_PlaySpriteAnimation(RGS_Sprite *sprite, char *name, Uint8 playOnce) {
	int i;
	sprite->playingOnce = playOnce;
	
	if(playOnce == 1)
		sprite->currentFrameIndex = 0;
	
	RGS_Animation *anim;
	for(i=0; i < sprite->animations->length; i++) {
		anim = (RGS_Animation*)sprite->animations->data[i];
		if(strcmp(name, anim->name) == 0 && anim != sprite->currentAnimation) {
			sprite->currentAnimation = anim;
			sprite->currentFrameIndex = 0;
			return;
		}
	}
}
