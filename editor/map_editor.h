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

#ifndef _MAP_EDITOR_H_
#define _MAP_EDITOR_H_

#include "isutil.h"
#include "rgs.h"
#include "entity_gfx.h"

#define PAINT_MODE_GFX					0
#define PAINT_MODE_GEOM					1
#define PAINT_MODE_ATT1					2
#define PAINT_MODE_BRUSHDEFSTART		3
#define PAINT_MODE_BRUSHDEFEND			4
#define PAINT_MODE_ENTITY				5

#define GFX_PAINT_SINGLETILE			0
#define GFX_PAINT_BRUSH					1

#define ENTITY_MODE_PLACE				0
#define ENTITY_MODE_SELECT				1
#define ENTITY_MODE_REMOVE				2

typedef struct {
	RGS_Rect brushRect;
	Uint16 *data;
	Uint16 id;
	ISString *name;
} Editor_Brush;

typedef struct {
	ISString *name;
	Uint8 code;
} Editor_Att;

typedef struct {
	ISString *name;
	Uint8 code;
	ISString *spritePath;
} Editor_Entity;

typedef struct {
	ISArray *brushes;
} Editor_LevelDef;

typedef struct {
	ISArray *levelDefs;
} Editor_GameDef;


void initMapEditor();
void openMap(char *mapFile);
void updateMapEditor();
void changeLayerVis(void *target);
void changeTile(void *target);
void selectTile(void *target);
void endChangeTile(void *target);
void scrollMap(void *target);
void scrollTileSelector(void *target);

void changeApplyToAll(void *target);
void startCreatingBrush(void *target);
void createNewBrush();

void CreateNewTilemapGO(void *target);
void setupNewTilemap();

void MapEditor_CreateTiles(RGS_Surface *srcSurface, char *destFile, int tileSize);
void MapEditor_ImportTilemap(RGS_Surface *srcSurface, int tileSize);

void MapEditor_SetTilesetPath(char *newPath);
void MapEditor_CreateNewTilemap();

void MapEditor_SaveTilemap(char *fileName);
void MapEditor_Hide();
void MapEditor_Show();

void MapEditor_LoadBrushes(char *fileName);
void MapEditor_LoadAttributes(char *fileName);
void MapEditor_LoadEntities(char *fileName);

void choosePaintMode(int id, void *data);
void chooseGeomBrush(int id, void *data);
void chooseBrush(int id, void *data);
void chooseGfxPaintMode(int id, void *data);
void chooseAttTile(int id, void *data);
void chooseEntity(int id, void *data);
void chooseEntityMode(void *target);

void createNewEntity(void *target);

void removeBrush(void *target);

void updateAttName(void *target);
void saveAttributes(void *target);
void updateBrushName(void *target);
void saveBrushes(void *target);
void updateEntityName(void *target);
void setEntitySpriteGO(void *target);
void saveEntities(void *target);

void refreshAtts();
void createNewAtt(void *target);
void removeAtt(void *target);

void MapEditor_KeyPress(int keyCode);

void DestroyBrush(Editor_Brush *brush);

void destroyBrushes();

void refreshBrushList();

void refreshEntitiesInMap();
void refreshEntities();

void DefEntity_Init(RGS_Entity *entity);


#endif