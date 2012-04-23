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

#ifndef _RGS_TILEMAP_H_
#define _RGS_TILEMAP_H_

#include "rgs_data.h"
#include "rgs_surface.h"
#include "rgs_core.h"
#include "isutil.h"
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TILE_EMPTY	0x00
#define TILE_SOLID	0x01
#define TILE_LSLANT 0x02
#define TILE_RSLANT 0x03

/* @package Tilemap */
/* @beginmodule Entity */

typedef struct {
	RGS_Rect *bBox;
	RGS_Object *object;
	float x;
	float y;
	Uint16 tileX;
	Uint16 tileY;
	float yVelocity;
	float yVelocityLimit;
	float xVelocity;
	float xActualVelocity;
	float xVelocityLimit;
	int xDirection;
	float acceleration;
	float gravity;
	int isTouchingGround;
	Uint8 noGravityOn;
	Uint8 noClipOn;
	Uint8 entityCollide;
	Uint8	isVirtual;
	int falling;
	Uint16 entityID;
	void *boundTilemap;
	void *standingOn;
	void *customData;
	void (*Init)(void *entity);
	void (*Update)(void *entity);
	void (*Destroy)(void *entity);
} RGS_Entity;

typedef struct {
	void (*Init)(void *entity);
	void (*Update)(void *entity);
	void (*Destroy)(void *entity);
	Uint16 entityCode;
	RGS_Screen *spriteScreen;
	ISString *spritePath;
	Uint8 memorySprite;
	char *memoryData;
} RGS_EntityRegistration;

/* @beginmodule Tilemap */

typedef struct {
	RGS_Object object;
	RGS_Surface *tilesSurface;
	RGS_Surface *drawBuffer;
	ISArray *entities;
	RGS_Entity *followTarget;
	short mapWidth;
	short mapHeight;
	int renderWidth;
	int renderHeight;
	int mapSize;
	Uint16 *mapTileData;
	Uint8 *mapAttributeData;
	Uint8 *userAttributeData;	
	int loaded;
	int refresh;
	int cameraX;
	int cameraY;
	int followAdjustX;
	int followAdjustY;
	int showTileProp1;
	Uint8 tileSize;
	RGS_Timer timer;
	int paused;
} RGS_Tilemap;

extern DECLSPEC RGS_Tilemap *RGS_CreateTilemap(RGS_Screen *screen, int renderWidth, int renderHeight, char *mapFile, char *tileFile);
extern DECLSPEC RGS_Tilemap *RGS_CreateEmptyTilemap(RGS_Screen *screen, int renderWidth, int renderHeight, short mapWidth, short mapHeight, short tileSize, char *tileFile);
extern DECLSPEC void RGS_LoadTilemap(RGS_Tilemap *tilemap, char *mapFile, char *tileFile);
extern DECLSPEC void RGS_SaveTilemap(RGS_Tilemap *tilemap, char *mapFile);
extern DECLSPEC void RGS_LoadTilemapFromMemory(RGS_Tilemap *tilemap, char *mapData, char *tileData);
extern DECLSPEC void RGS_RedrawTilemapBuffer(RGS_Tilemap *tilemap);
extern DECLSPEC void RGS_UpdateTilemap(void *object);
extern DECLSPEC void RGS_UpdateEntityInTilemap(RGS_Tilemap *tilemap, RGS_Entity *entity, float elapsed);
extern DECLSPEC void RGS_RenderTilemap(SDL_Surface *target, void *object);
extern DECLSPEC void RGS_TilemapLookAt(RGS_Tilemap *tilemap, int x, int y);
extern DECLSPEC void RGS_TilemapLookAtTile(RGS_Tilemap *tilemap, int x, int y);
extern DECLSPEC void RGS_TilemapFollowEntity(RGS_Tilemap *tilemap, RGS_Entity *entity, int ax, int ay);
extern DECLSPEC int RGS_CheckTilemapCollisionRectY(RGS_Tilemap *tilemap, int x, int y, int w, int h, int falling);
extern DECLSPEC int RGS_CheckTilemapCollisionRectX(RGS_Tilemap *tilemap, int x, int y, int w, int h, int direction);
extern DECLSPEC int RGS_CheckTilemapSlantY(RGS_Tilemap *tilemap, int x, int y, int falling);
extern DECLSPEC int RGS_CheckTilemapCollisionY(RGS_Tilemap *tilemap, int  x, int y, int falling);
extern DECLSPEC int RGS_CheckTilemapCollisionX(RGS_Tilemap *tilemap, int x, int y, int direction);
extern DECLSPEC RGS_Entity *RGS_TilemapGetEntityAt(RGS_Tilemap *tilemap, int x, int y);
extern DECLSPEC Uint8 RGS_GetTileIndex(RGS_Tilemap *tilemap, int x, int y);
extern DECLSPEC void RGS_LoadNewTilemapTiles(RGS_Tilemap *tilemap, char *newTilesFile);
extern DECLSPEC Uint8 RGS_GetAttributeAtPoint(RGS_Tilemap *tilemap, Uint16 x, Uint16 y);

/* @beginmodule Entity */
extern DECLSPEC RGS_Entity *RGS_CreateEntity(RGS_Tilemap *tilemap, RGS_Rect *bBox, RGS_Object *object);
extern DECLSPEC int RGS_CheckEntityCollisionsX(RGS_Tilemap *tilemap, RGS_Entity *entity);
extern DECLSPEC int RGS_CheckEntityCollisionsY(RGS_Tilemap *tilemap, RGS_Entity *entity);
extern DECLSPEC int RGS_CheckEntityPointCollision(RGS_Entity *entity, int x, int y);
extern DECLSPEC int RGS_GetEntityDistance(RGS_Entity *ent1, RGS_Entity *ent2);
extern DECLSPEC int RGS_EntityInAttribute(RGS_Tilemap *tilemap, RGS_Entity *entity, Uint8 attribute);
extern DECLSPEC Uint8 RGS_CheckEntityCollision(RGS_Entity *ent1, RGS_Entity *ent2);
extern DECLSPEC void RGS_RegisterEntity(Uint16 entityCode, RGS_Screen *spriteScreen, char *spritePath, 	void (*Init)(RGS_Entity *entity), void (*Update)(RGS_Entity *entity), void (*Destroy)(RGS_Entity *entity), Uint8 memorySprite, char *memData);
extern DECLSPEC void RGS_RegisterDefaultEntity(RGS_Screen *spriteScreen, char *spritePath, 	void (*Init)(RGS_Entity *entity), void (*Update)(RGS_Entity *entity), void (*Destroy)(RGS_Entity *entity), Uint8 memorySprite, char *memData);
extern DECLSPEC void RGS_CreateEntitySpriteByCode(RGS_Tilemap *tilemap, Uint16 entityCode, Uint16 x, Uint16 y);

/* @endmodule */

#ifdef __cplusplus
}
#endif

#endif
