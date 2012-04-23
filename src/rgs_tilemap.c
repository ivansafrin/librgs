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

#include "rgs_tilemap.h"
#include "rgs_sprite.h"

RGS_Tilemap *RGS_CreateTilemap(RGS_Screen *screen, int renderWidth, int renderHeight, char *mapFile, char *tileFile) {

	RGS_Tilemap *newTilemap;
	newTilemap = (RGS_Tilemap*)malloc(sizeof(RGS_Tilemap));
	memset(newTilemap, 0, sizeof(RGS_Tilemap));
	
	RGS_InitializeObject(&newTilemap->object, 0, 0);
	RGS_AddScreenObject(screen, (RGS_Object*)newTilemap);
	newTilemap->object.Render = RGS_RenderTilemap;
	newTilemap->object.Update = RGS_UpdateTilemap;
	newTilemap->renderWidth = renderWidth;
	newTilemap->renderHeight = renderHeight;
	newTilemap->cameraX = 0;
	newTilemap->cameraY = 0;
	newTilemap->paused = 0;
	newTilemap->entities = ISArray_Create(sizeof(RGS_Entity));
	RGS_LoadTilemap(newTilemap, mapFile, tileFile);
	
	newTilemap->object.w  = renderWidth;
	newTilemap->object.h  = renderHeight;
	
	RGS_ResetTimer(&newTilemap->timer);
	
	return newTilemap;
}

RGS_Tilemap *RGS_CreateEmptyTilemap(RGS_Screen *screen, int renderWidth, int renderHeight, short mapWidth, short mapHeight, short tileSize, char *tileFile) {
	RGS_Tilemap *newTilemap;
	newTilemap = (RGS_Tilemap*)malloc(sizeof(RGS_Tilemap));
	memset(newTilemap, 0, sizeof(RGS_Tilemap));
	
	RGS_InitializeObject(&newTilemap->object, 0, 0);
	RGS_AddScreenObject(screen, (RGS_Object*)newTilemap);
	newTilemap->object.Render = RGS_RenderTilemap;
	newTilemap->object.Update = RGS_UpdateTilemap;
	newTilemap->renderWidth = renderWidth;
	newTilemap->renderHeight = renderHeight;
	newTilemap->cameraX = 0;
	newTilemap->cameraY = 0;
	newTilemap->paused = 0;

	newTilemap->tileSize = tileSize;

	strcpy(newTilemap->object.typeString, "tilemap");

	// replace with RGS_NewSurface
	char *tileData = (char*)RGS_OpenDataPointer(tileFile);
	newTilemap->tilesSurface = (RGS_Surface*)RGS_SurfaceFromMemory(tileData);
	free(tileData);

	newTilemap->mapWidth = mapWidth;
	newTilemap->mapHeight = mapHeight;
	newTilemap->mapSize = newTilemap->mapWidth*newTilemap->mapHeight;

	newTilemap->mapTileData = (Uint16*)malloc(newTilemap->mapSize*2);
	memset(newTilemap->mapTileData, 0, newTilemap->mapSize*2);
	newTilemap->mapAttributeData = (Uint8*)malloc(newTilemap->mapSize);
	memset(newTilemap->mapAttributeData, 0, newTilemap->mapSize);
	newTilemap->userAttributeData = (Uint8*)malloc(newTilemap->mapSize);
	memset(newTilemap->userAttributeData, 0, newTilemap->mapSize);
	
	// replace with RGS_NewSurface
	newTilemap->drawBuffer = (RGS_Surface*)malloc(sizeof(RGS_Surface));
	memset(newTilemap->drawBuffer, 0, sizeof(RGS_Surface));
	newTilemap->drawBuffer->surface = SDL_CreateRGBSurface(SDL_SWSURFACE|SDL_ANYFORMAT, newTilemap->mapWidth*newTilemap->tileSize, newTilemap->mapHeight*newTilemap->tileSize,8,RMASK,GMASK,BMASK,AMASK);
	SDL_LockSurface(newTilemap->drawBuffer->surface);
	SDL_SetPalette(newTilemap->drawBuffer->surface,SDL_LOGPAL|SDL_PHYSPAL,newTilemap->tilesSurface->surface->format->palette->colors,0,newTilemap->tilesSurface->surface->format->palette->ncolors);
	//SDL_SetColorKey(newTilemap->drawBuffer->surface,SDL_SRCCOLORKEY,SDL_MapRGB(newTilemap->drawBuffer->surface->format,255,0,255));
	SDL_UnlockSurface(newTilemap->drawBuffer->surface);
	
	newTilemap->loaded = 1;
	newTilemap->refresh = 1;
	
	newTilemap->object.w  = renderWidth;
	newTilemap->object.h  = renderHeight;
		
	newTilemap->entities = ISArray_Create(sizeof(RGS_Entity));
	
	RGS_ResetTimer(&newTilemap->timer);
	return newTilemap;
}

void RGS_TilemapAddEntity(RGS_Tilemap *tilemap, RGS_Entity *entity) {
	ISArray_PushElement(tilemap->entities, (void*)entity);
}

void RGS_LoadTilemap(RGS_Tilemap *tilemap, char *mapFile, char *tileFile) {
	char *mapData = (char*)RGS_OpenDataPointer(mapFile);
	char *tileData = (char*)RGS_OpenDataPointer(tileFile);
	if(tileData != NULL && mapData != NULL) {
		RGS_LoadTilemapFromMemory(tilemap, mapData, tileData);
		free(mapData);
		free(tileData);
	}
}

void RGS_LoadNewTilemapTiles(RGS_Tilemap *tilemap, char *newTilesFile) {
	tilemap->tilesSurface = RGS_LoadSurface(newTilesFile);
	tilemap->refresh = 1;
	RGS_CopySurfacePalette(tilemap->drawBuffer, tilemap->tilesSurface);
}

void RGS_SaveTilemap(RGS_Tilemap *tilemap, char *mapFile) {
	Uint16 val = 0;
	Uint8 bval = 0;
		
	FILE *outPtr = fopen(mapFile, "wb");
	bval = tilemap->tileSize;
	fwrite(&bval, 1, 1, outPtr);	
	val = tilemap->mapWidth;
	fwrite(&val, 2, 1, outPtr);
	val = tilemap->mapHeight;
	fwrite(&val, 2, 1, outPtr);
	
	fwrite(tilemap->mapTileData, 2, tilemap->mapWidth*tilemap->mapHeight, outPtr);
	fwrite(tilemap->mapAttributeData, 1, tilemap->mapWidth*tilemap->mapHeight, outPtr);
	fwrite(tilemap->userAttributeData, 1, tilemap->mapWidth*tilemap->mapHeight, outPtr);
	
	val = tilemap->entities->length;
	fwrite(&val, 2, 1, outPtr);
	RGS_Entity *ent;
	int i;
	for(i = 0; i< tilemap->entities->length; i++) {
		ent = (RGS_Entity*) tilemap->entities->data[i];
		fwrite(&ent->entityID, 2, 1, outPtr);
		fwrite(&ent->tileX, 2, 1, outPtr);
		fwrite(&ent->tileY, 2, 1, outPtr);
	}
	
	fclose(outPtr);
}

void RGS_LoadTilemapFromMemory(RGS_Tilemap *tilemap, char *mapData, char *tileData) {

	memcpy(&tilemap->tileSize, mapData, 1);
	mapData++;
	
	// add surfcae removal here!
	tilemap->tilesSurface = (RGS_Surface*)RGS_SurfaceFromMemory(tileData);
	memcpy(&tilemap->mapWidth, mapData, 2);
	memcpy(&tilemap->mapHeight, mapData+2, 2);
	mapData += 4;
	
	tilemap->mapSize = tilemap->mapWidth*tilemap->mapHeight;
	
	tilemap->mapTileData = (Uint16*)malloc(tilemap->mapSize*2);
	tilemap->mapAttributeData = (Uint8*)malloc(tilemap->mapSize);
	tilemap->userAttributeData = (Uint8*)malloc(tilemap->mapSize);
	
	memcpy(tilemap->mapTileData, mapData, tilemap->mapSize*2);
	mapData += tilemap->mapSize*2;
	memcpy(tilemap->mapAttributeData, mapData, tilemap->mapSize);
	mapData += tilemap->mapSize;
	memcpy(tilemap->userAttributeData, mapData, tilemap->mapSize);
	mapData += tilemap->mapSize;
	
	Uint16 entCount, entCode, entX, entY = 0;
	memcpy(&entCount, mapData, 2);
	mapData += 2;
	int i;
	
	for(i = 0; i< entCount; i++) {
		memcpy(&entCode, mapData, 2);
		mapData += 2;
		memcpy(&entX, mapData, 2);
		mapData += 2;
		memcpy(&entY, mapData, 2);
		mapData += 2;
		RGS_CreateEntitySpriteByCode(tilemap, entCode, entX, entY);
	}
	
	tilemap->drawBuffer = (RGS_Surface*)malloc(sizeof(RGS_Surface));
	memset(tilemap->drawBuffer, 0, sizeof(RGS_Surface));
	
	tilemap->drawBuffer->surface = SDL_CreateRGBSurface(SDL_SWSURFACE|SDL_ANYFORMAT, tilemap->mapWidth*tilemap->tileSize, tilemap->mapHeight*tilemap->tileSize,8,RMASK,GMASK,BMASK,AMASK);
	SDL_LockSurface(tilemap->drawBuffer->surface);
	SDL_SetPalette(tilemap->drawBuffer->surface,SDL_LOGPAL|SDL_PHYSPAL,tilemap->tilesSurface->surface->format->palette->colors,0,tilemap->tilesSurface->surface->format->palette->ncolors);
	SDL_SetColorKey(tilemap->drawBuffer->surface,SDL_SRCCOLORKEY,SDL_MapRGB(tilemap->drawBuffer->surface->format,255,0,255));
	SDL_UnlockSurface(tilemap->drawBuffer->surface);
	
	tilemap->loaded = 1;
	tilemap->refresh = 1;
}

void RGS_RedrawTilemapBuffer(RGS_Tilemap *tilemap) {
	int i,j;
	RGS_Rect srcRect;
	RGS_Rect destRect;
	
	SDL_LockSurface(tilemap->tilesSurface->surface);
	SDL_LockSurface(tilemap->drawBuffer->surface);
	
	short index;
	int indexPitch = tilemap->tilesSurface->surface->w/tilemap->tileSize;
	
	for(i=0;i<tilemap->mapHeight;i++) {
		for(j=0;j<tilemap->mapWidth;j++) {
			index = tilemap->mapTileData[(i*tilemap->mapWidth)+j];
			srcRect.x = tilemap->tileSize*( index % indexPitch);
			srcRect.y = tilemap->tileSize*( index / indexPitch);
			srcRect.w = tilemap->tileSize;
			srcRect.h = tilemap->tileSize;
			
			if(srcRect.x > tilemap->tilesSurface->surface->w - tilemap->tileSize)
				srcRect.x = tilemap->tilesSurface->surface->w - tilemap->tileSize;
		
			if(srcRect.y > tilemap->tilesSurface->surface->h - tilemap->tileSize)
				srcRect.y = tilemap->tilesSurface->surface->h - tilemap->tileSize;
				 
			
			destRect.x = j*tilemap->tileSize;
			destRect.y = i*tilemap->tileSize;
			destRect.w = tilemap->tileSize;
			destRect.h = tilemap->tileSize;
	//		if(tilemap->mapAttributeData[(i*tilemap->mapWidth)+j] != TILE_SOLID)
			RGS_CopySurface(tilemap->tilesSurface, &srcRect, tilemap->drawBuffer, &destRect);
		}
	}
	
	SDL_UnlockSurface(tilemap->tilesSurface->surface);
	SDL_UnlockSurface(tilemap->drawBuffer->surface);
}

int RGS_CheckEntityCollisionsX(RGS_Tilemap *tilemap, RGS_Entity *entity) {
	
	if(entity->xActualVelocity < 1.0)
		return entity->x+entity->bBox->x;	
	
	int i;
	RGS_Entity *ent;
	for(i=0;i<tilemap->entities->length;i++) {
		ent = (RGS_Entity*)tilemap->entities->data[i];
		if(ent->entityCollide ==1  && ent != entity) {
			if(RGS_CheckEntityCollision(entity, ent) == 1) {
				if(entity->xDirection == 1)
					return ent->x + ent->bBox->x - entity->bBox->w;
				else
					return ent->x + ent->bBox->x + ent->bBox->w;
			}
		}
	}
	return ((int)entity->x)+entity->bBox->x;
}

int RGS_CheckEntityCollisionsY(RGS_Tilemap *tilemap, RGS_Entity *entity) {
	int i;
	RGS_Entity *ent;
	for(i=0;i<tilemap->entities->length;i++) {	
		ent = (RGS_Entity*)tilemap->entities->data[i];
		if(ent->entityCollide == 1 && ent != entity) {
			if(RGS_CheckEntityCollision(entity, ent) ==1) {
				if(entity->falling == 1) {
					entity->standingOn = (void*)ent;
					return ent->y - ent->object->h + ent->bBox->y;
				} else {
					entity->standingOn = NULL;
					return ent->y - ent->object->h + ent->bBox->y +  ent->bBox->h-entity->bBox->y+entity->bBox->h;
				}
			}
		}
	}
	entity->standingOn = NULL;
	return entity->y+entity->bBox->y;
}

int UpdateEntityX(RGS_Entity *entity, int newx) {
	if(newx != ((int)entity->x)+entity->bBox->x && entity->xActualVelocity > 0){
		entity->xVelocity = 0;
		entity->xActualVelocity = 0;
		entity->x = newx-entity->bBox->x;
		return 1;
	}
	return 0;
}

int UpdateEntityY(RGS_Entity *entity, int newy) {
	if(newy != (int)entity->y+entity->bBox->y){
		entity->yVelocity = 0;
		entity->y = newy;
		if(entity->falling == 1)
			entity->isTouchingGround = 1;
		return 1;
	} else {
		entity->isTouchingGround = 0;
		return 0;
	}
}

void RGS_UpdateEntityInTilemap(RGS_Tilemap *tilemap, RGS_Entity *entity, float elapsed) {
	
		float lastY = entity->y;
	
		if(entity->noGravityOn != 1) {
			if(elapsed < 0.3)
				entity->yVelocity += entity->gravity*elapsed;
		}
		
		if(entity->yVelocity > entity->yVelocityLimit)
			entity->yVelocity = entity->yVelocityLimit;
			
		if(entity->yVelocity*elapsed < tilemap->tileSize-1)
			entity->y += entity->yVelocity*elapsed;
		else
			entity->y += tilemap->tileSize-1;
				
		if(entity->y > lastY)
			entity->falling = 1;
		else
			entity->falling = 0;
				
		int newy;
		if(entity->noClipOn != 1) {
			newy = RGS_CheckEntityCollisionsY(tilemap, entity);
			if(newy != (int)entity->y+entity->bBox->y){
				entity->yVelocity = 0;
				entity->y = newy;
				if(entity->falling == 1)
					entity->isTouchingGround = 1;
			} else {
				newy =  RGS_CheckTilemapCollisionRectY(tilemap, entity->x+entity->bBox->x, entity->y+entity->bBox->y, entity->bBox->w, entity->bBox->h, entity->falling);
				UpdateEntityY(entity, newy);
			}
		}

		//DO X COLLISION SECOND!

		if(entity->xActualVelocity < entity->xVelocity) {
			entity->xActualVelocity += entity->acceleration*elapsed;
		} else {
			entity->xActualVelocity -= entity->acceleration*elapsed;
			entity->xVelocity = 0;
		}
		
		if(entity->xActualVelocity < 0)
			entity->xActualVelocity = 0;
		//NOTE: Fixed for half-tile limit, no more alt-tab issues afaik

		if(entity->xVelocity*elapsed < (tilemap->tileSize>>1)-1)
			entity->x += (entity->xActualVelocity*elapsed)*(float)entity->xDirection;
		else
			entity->x += ((tilemap->tileSize>>1)-1)*(float)entity->xDirection;

		float newx;
		if(entity->noClipOn != 1) {
			newx = RGS_CheckEntityCollisionsX(tilemap, entity);
			if(newx != ((int)entity->x)+entity->bBox->x && entity->xActualVelocity > 0){
				entity->xVelocity = 0;
				entity->xActualVelocity = 0;
				if(entity->xDirection == 1)
					entity->x = newx-entity->bBox->x;
				else
					entity->x = newx-entity->bBox->x;
			} else {
				newx = RGS_CheckTilemapCollisionRectX(tilemap, entity->x+entity->bBox->x, entity->y+entity->bBox->y-1, entity->bBox->w, entity->bBox->h, entity->xDirection);
				UpdateEntityX(entity, newx);
			}
		}
		
		if(entity->x < 0)
			entity->x = 0;
		
		if(entity->Update != NULL)
			entity->Update(entity);
			
		entity->object->x = tilemap->object.x+floor(entity->x)-tilemap->cameraX;
		entity->object->y = tilemap->object.y+floor(entity->y)-entity->object->h-tilemap->cameraY;
}

void RGS_UpdateTilemap(void *object) {
	RGS_Tilemap *tilemap = (RGS_Tilemap*)object;
	RGS_UpdateTimer(&tilemap->timer);
	if(tilemap->paused == 1)
		return;	
	float elapsed = ((float)RGS_TimerElapsed(&tilemap->timer))/1000;

	if(elapsed > 0.05)
		elapsed = 0.05;
	
	if(tilemap->followTarget != 0) {
		int cameraX = floor(tilemap->followTarget->x)-(tilemap->renderWidth/2) + tilemap->followAdjustX;
		int cameraY = floor(tilemap->followTarget->y)-tilemap->followTarget->object->h+tilemap->followTarget->bBox->y+(tilemap->followTarget->bBox->h/2)-(tilemap->renderHeight/2) + tilemap->followAdjustY;
		
		if(cameraX< 0) cameraX = 0;
		if(cameraY< 0) cameraY = 0;
	
		if(cameraY > (tilemap->mapHeight*tilemap->tileSize) - tilemap->renderHeight)
			cameraY = (tilemap->mapHeight*tilemap->tileSize) - tilemap->renderHeight;
		if(cameraX > (tilemap->mapWidth*tilemap->tileSize) - tilemap->renderWidth)
			cameraX = (tilemap->mapWidth*tilemap->tileSize) - tilemap->renderWidth;
		RGS_TilemapLookAt(tilemap, cameraX, cameraY);
	}		
	
	// update the entities
	int i;
	RGS_Entity *entity;
	for(i=0; i < tilemap->entities->length; i++) {
		entity = (RGS_Entity*)tilemap->entities->data[i];
		if(entity->isVirtual == 0) {
			RGS_UpdateEntityInTilemap(tilemap, entity, elapsed);
			entity->tileX = entity->x/tilemap->tileSize;
			entity->tileY = entity->y/tilemap->tileSize;
		}
	}
	
	if(tilemap->refresh == 1) {
		RGS_RedrawTilemapBuffer(tilemap);
		tilemap->refresh = 0;
	}
		
}

int RGS_CheckEntityPointCollision(RGS_Entity *entity, int x, int y) {
			 
	if(x > entity->x + entity->bBox->x && x < entity->x + entity->bBox->x + entity->bBox->w &&
		y > entity->y - entity->object->h + entity->bBox->y && y < entity->y - entity->object->h + entity->bBox->y + entity->bBox->h)
		return 1;
	else
		return 0;
}

RGS_Entity *RGS_TilemapGetEntityAt(RGS_Tilemap *tilemap, int x, int y) {
	RGS_Entity *entity = NULL;
	int i;
	
	for(i=0; i < tilemap->entities->length; i++) {
		entity = (RGS_Entity*)tilemap->entities->data[i];
		if(RGS_CheckEntityPointCollision(entity, x, y) == 1)
			return entity;
	}
	
	return NULL;
}

int RGS_GetEntityDistance(RGS_Entity *ent1, RGS_Entity *ent2) {
	float dist = sqrt((ent2->x-ent1->x) * (ent2->x-ent1->x) + (ent2->y-ent1->y) * (ent2->y-ent1->y));
	return (int)dist;
}

Uint8 RGS_CheckEntityCollision(RGS_Entity *ent1, RGS_Entity *ent2) {

    Uint16 left1, left2;
    Uint16 right1, right2;
    Uint16 top1, top2;
    Uint16 bottom1, bottom2;

	left1 = floor(ent1->x)+ent1->bBox->x;
	right1 = left1+ent1->bBox->w;
	left2 = floor(ent2->x)+ent2->bBox->x;
	right2 = left2+ent2->bBox->w;
	top1 = floor(ent1->y)-ent1->object->h+ent1->bBox->y;
	top2 = floor(ent2->y)-ent2->object->h+ent2->bBox->y;
	bottom1 = top1+ent1->bBox->h;
	bottom2 = top2+ent2->bBox->h;

    if (bottom1 <= top2) return(0);
    if (top1 >= bottom2) return(0);
    if (right1 <= left2) return(0);
    if (left1 >= right2) return(0);
	return 1;
}

int RGS_CheckTilemapCollisionRectY(RGS_Tilemap *tilemap, int x, int y, int w, int h, int falling) {
	int nx;
	int ny = y;
	int yrow = 0;
	if(falling != 1)
		yrow = h-1;
	
	for(nx = x; nx < x+w; nx++) {
		ny = RGS_CheckTilemapSlantY(tilemap, nx ,y-yrow-1, falling);
		if(ny != y-yrow-1)
			return ny+yrow;		
		ny = RGS_CheckTilemapCollisionY(tilemap, nx, y-yrow, falling);
		if(ny != y-yrow)
			return ny+yrow;
	}
	
	ny = RGS_CheckTilemapSlantY(tilemap, x ,y-yrow, falling);
	if(ny != y-yrow)
		return ny+yrow;
	
	ny = RGS_CheckTilemapSlantY(tilemap, x+w,y-yrow, falling);
	if(ny != y-yrow)
		return ny+yrow;
	
	return y;
}

int RGS_CheckTilemapCollisionRectX(RGS_Tilemap *tilemap, int x, int y, int w, int h, int direction) {
	int ny;
	int nx = x;
	int xcol = 0;
	if(direction > 0)
		xcol = w-1;

	for(ny = y; ny > y-h; ny--) {
		nx = RGS_CheckTilemapCollisionX(tilemap, x+xcol, ny, direction);
		if(nx != x+xcol)
			return nx-xcol;
	}
	return x;
}


int RGS_CheckTilemapSlantY(RGS_Tilemap *tilemap, int x, int y, int falling)  {
	int xTile = (int)(ceil((float)(x/tilemap->tileSize)));
	int yTile = (int)(ceil((float)(y/tilemap->tileSize)));
	
	if(xTile < 0 || yTile < 0)
		return y;
	
	if(RGS_GetTileIndex(tilemap, xTile, yTile) == TILE_LSLANT || RGS_GetTileIndex(tilemap, xTile, yTile) == TILE_RSLANT) {
		/*
		switch(RGS_GetTileIndex(xTile, yTile)%64) {
			case 3:
				if(y%8 > (8 - (x%8))-1)
					return (floor((float)(y/8))*8) + (8-(x%8)-1);
				break;
			case 4:
				if(y%8 > (x%8))
					return (floor((float)(y/8))*8) + (x%8);
				break;
			default:
				return y;
				break;
		}
		*/
	} else {
		return y;
	}
	return y;
}


int RGS_CheckTilemapCollisionX(RGS_Tilemap *tilemap, int x, int y, int direction) {
	int xTile = (int)(ceil((float)(x/tilemap->tileSize)));	
	int yTile = (int)(ceil((float)(y/tilemap->tileSize)));
	

	if(xTile <= 0 || yTile <= 0)
		return x;
	

	if(RGS_GetTileIndex(tilemap, xTile, yTile) == TILE_SOLID){
		if(direction > 0)
			return ((x/tilemap->tileSize)*tilemap->tileSize)-1;
		else
			return ((x/tilemap->tileSize)*tilemap->tileSize)+tilemap->tileSize;
	} else {
		return x;
	}
}

int RGS_CheckTilemapCollisionY(RGS_Tilemap *tilemap, int  x, int y, int falling)
{
	int xTile = (int)(ceil((float)(x/tilemap->tileSize)));	
	int yTile = (int)(ceil((float)(y/tilemap->tileSize)));
	

	if(xTile < 0 || yTile < 0)
		return y;
	
	if(RGS_GetTileIndex(tilemap,xTile, yTile) == TILE_SOLID){
		if(falling == 1)
			return (y/tilemap->tileSize)*tilemap->tileSize;
		else
			return ((y/tilemap->tileSize)*tilemap->tileSize)+tilemap->tileSize;
	}
	else {
		return y;
	}
	return y;
}

Uint8 RGS_GetAttributeAtPoint(RGS_Tilemap *tilemap, Uint16 x, Uint16 y) {
	int xTile = (int)(ceil((float)(x/tilemap->tileSize)));	
	int yTile = (int)(ceil((float)(y/tilemap->tileSize)));
	
	return tilemap->userAttributeData[(yTile*tilemap->mapWidth)+xTile];
}

int RGS_EntityInAttribute(RGS_Tilemap *tilemap, RGS_Entity *entity, Uint8 attribute) {
	int i;
	
	for(i=entity->x + entity->bBox->x;i <= entity->x + entity->bBox->x+entity->bBox->w;i++) {
		if(RGS_GetAttributeAtPoint(tilemap, i, entity->y + entity->bBox->y) == attribute)
			return 1;
	}

	return 0;
}

Uint8 RGS_GetTileIndex(RGS_Tilemap *tilemap, int x, int y) {
	return tilemap->mapAttributeData[(y*tilemap->mapWidth)+x];
}


void RGS_TilemapFollowEntity(RGS_Tilemap *tilemap, RGS_Entity *entity, int ax, int ay) {
	tilemap->followTarget = entity;
	tilemap->followAdjustX = ax;
	tilemap->followAdjustY = ay;
}

void RGS_TilemapLookAt(RGS_Tilemap *tilemap, int x, int y) {
	tilemap->cameraX = x;
	tilemap->cameraY = y;
}

void RGS_TilemapLookAtTile(RGS_Tilemap *tilemap, int x, int y) {
	RGS_TilemapLookAt(tilemap, x*tilemap->tileSize, y*tilemap->tileSize);
}

void RGS_RenderTilemap(SDL_Surface *target, void *object) {
	RGS_Tilemap *tilemap = (RGS_Tilemap*)object;
	SDL_Rect srcRect, destRect;
	srcRect.x = tilemap->cameraX;
	srcRect.y = tilemap->cameraY;
	srcRect.w = tilemap->renderWidth;
	srcRect.h = tilemap->renderHeight;

	destRect.x = tilemap->object.x;
	destRect.y = tilemap->object.y;
	destRect.w = tilemap->renderWidth;
	destRect.h = tilemap->renderHeight;	
	
	SDL_BlitSurface(tilemap->drawBuffer->surface, &srcRect, target, &destRect);
}

RGS_Entity *RGS_CreateEntity(RGS_Tilemap *tilemap, RGS_Rect *bBox, RGS_Object *object) {
	RGS_Entity *newEntity = (RGS_Entity*) ISArray_CreateElement(tilemap->entities);
	newEntity->boundTilemap = (void*)tilemap;
	newEntity->bBox = bBox;
	newEntity->object = object;
	newEntity->acceleration = 800;
	newEntity->gravity = 800;
	newEntity->yVelocityLimit = 300;
	newEntity->xDirection = 1;
	return newEntity;
}

void RGS_RegisterEntity(Uint16 entityCode, RGS_Screen *spriteScreen, char *spritePath, 	void (*Init)(RGS_Entity *entity), void (*Update)(RGS_Entity *entity), void (*Destroy)(RGS_Entity *entity), Uint8 memorySprite, char *memData) {
	RGS_EntityRegistration *newEntReg = (RGS_EntityRegistration*)ISArray_CreateElement(g_rgs->entityRegistrations);
	newEntReg->entityCode = entityCode;
	newEntReg->spritePath = ISString_Create(spritePath);
	newEntReg->spriteScreen = spriteScreen;

#ifdef _WINDOWS
	newEntReg->Init = (void (__cdecl *)(void *))Init;
	newEntReg->Update = (void (__cdecl *)(void *))Update;
	newEntReg->Destroy = (void (__cdecl *)(void *))Destroy;
#else
	newEntReg->Init = Init;
	newEntReg->Update = Update;
	newEntReg->Destroy = Destroy;
#endif

	newEntReg->memorySprite = memorySprite;
	newEntReg->memoryData = memData;
}

void RGS_RegisterDefaultEntity(RGS_Screen *spriteScreen, char *spritePath, 	void (*Init)(RGS_Entity *entity), void (*Update)(RGS_Entity *entity), void (*Destroy)(RGS_Entity *entity), Uint8 memorySprite, char *memData)  {

	RGS_EntityRegistration *entReg;

	if(g_rgs->defaultEntity != NULL) {
		entReg = (RGS_EntityRegistration*)g_rgs->defaultEntity;
		ISString_Destroy(entReg->spritePath);
		free(g_rgs->defaultEntity);
	}
	
	entReg = (RGS_EntityRegistration*)malloc(sizeof(RGS_EntityRegistration));
	
	entReg->entityCode = 0;

	entReg->spritePath = ISString_Create(spritePath);
	entReg->spriteScreen = spriteScreen;
	entReg->memorySprite = memorySprite;
	entReg->memoryData = memData;

#ifdef _WINDOWS
	entReg->Init = (void (__cdecl *)(void *))Init;
	entReg->Update = (void (__cdecl *)(void *))Update;
	entReg->Destroy = (void (__cdecl *)(void *))Destroy;
#else
	entReg->Init = Init;
	entReg->Update = Update;
	entReg->Destroy = Destroy;
#endif	

	g_rgs->defaultEntity = (void*)entReg;

}


void RGS_CreateEntityFromReg(RGS_Entity *entity,  RGS_EntityRegistration *entReg) {

	RGS_Sprite *newSprite;
	
	if(entReg->memorySprite == 1)
		newSprite = RGS_CreateSpriteFromMemory(entReg->spriteScreen, entReg->memoryData);
	else
		newSprite = RGS_CreateSprite(entReg->spriteScreen, entReg->spritePath->text);
		
	entity->object = &newSprite->object;
	entity->bBox = &newSprite->bBox;
	entity->Init = entReg->Init;
	entity->Update = entReg->Update;
	entity->Destroy = entReg->Destroy;
	if(entity->Init != NULL)
		entity->Init(entity);
}

void RGS_CreateEntitySpriteByCode(RGS_Tilemap *tilemap, Uint16 entityCode, Uint16 x, Uint16 y) {
	int i;
	RGS_Entity *newEntity = RGS_CreateEntity(tilemap, NULL, NULL);
	newEntity->tileX = x;
	newEntity->tileY = y;
	newEntity->x = x*tilemap->tileSize;
	newEntity->y = y*tilemap->tileSize;
	newEntity->entityID = entityCode;
	
	RGS_EntityRegistration *entReg;
	for(i = 0; i < 	g_rgs->entityRegistrations->length; i++) {
		entReg = (RGS_EntityRegistration*)g_rgs->entityRegistrations->data[i];
		if(entReg->entityCode == entityCode) {
			RGS_CreateEntityFromReg(newEntity, entReg);
			return;
		}
	}
	
	if(g_rgs->defaultEntity != NULL) {
		RGS_CreateEntityFromReg(newEntity, (RGS_EntityRegistration*)g_rgs->defaultEntity);
		return;
	}
	
	newEntity->isVirtual = 1;
}
