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

#include "map_editor.h"
#include "rgs.h"
#include "globals.h"
#include "entity_gfx.h"

RGS_Screen *mapEditorScreen;

RGS_Screen *gfxScreen;
RGS_Screen *geomScreen;
RGS_Screen *attScreen;
RGS_Screen *entityScreen;

RGS_Tilemap *workingTilemap = NULL;
RGS_Tilemap *selectorTilemap = NULL;

RGS_SurfaceImage *tilemapImage;

RGS_UIOptionList *mapLayers;

RGS_UICheckBox *gfxLayerCheckBox;
RGS_UICheckBox *geomLayerCheckBox;
RGS_UICheckBox *att1LayerCheckBox;
RGS_UICheckBox *entityLayerCheckBox;

RGS_UICheckBox *applyToAll;
RGS_UICheckBox *applyToAllAtts;

RGS_UIOptionList *geomTiles;
RGS_UIOptionList *attributeTiles;
RGS_UIOptionList *brushList;
RGS_UIOptionList *gfxPaintModeList;
RGS_UIOptionList *entityList;

int gfxPaintMode = GFX_PAINT_SINGLETILE;

RGS_UIHorizontalScroller *mapScrollerH;
RGS_UIVerticalScroller *mapScrollerV;
RGS_UIVerticalScroller *tileSelectorScroller;

RGS_UITextInput *brushName;
RGS_UIButton *saveBrush;
RGS_UIButton *saveBrushesToFile;

RGS_UIButton *entityPlaceModeButton;
RGS_UIButton *entitySelectModeButton;
RGS_UIButton *entityRemoveModeButton;
RGS_Shape *entityModeSelector;


char entity_icon[] = {0x53, 0x50, 0x52, 0x32, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x08, 0x00, 0x08, 0x00, 0x08, 0x00, 0x01, 0x07, 0x64, 0x65, 0x66, 0x61, 0x75, 0x6C, 0x74, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x08, 0x00, 0x01, 0x00, 0x00, 0x00, 0x08, 0x00, 0x08, 0x00, 0x03, 0x00, 0xBB, 0xFF, 0x00, 0xFF, 0x55, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x01, 0x02, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x01, 0x02, 0x00, 0x00, 0x00, 0x02, 0x02, 0x02, 0x01, 0x02, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02};


RGS_UITextInput *entName;
RGS_UITextInput *entCode;
RGS_UIButton *saveEntity;
RGS_UIButton *setEntitySprite;
RGS_UIButton *saveEntitiesToFile;

RGS_Sprite *previewSprite = NULL;

RGS_UITextInput *attName;
RGS_UITextInput *attCode;
RGS_UIButton *saveAtt;
RGS_UIButton *saveAttsToFile;

RGS_Shape *tileSelector;

RGS_UIButton *createEntityButton;
RGS_Screen *entityOverlayScreen;

void reloadFileBrowser(char *dirname);
extern RGS_Label *errorMessageLabel;
extern RGS_UITextInput *fileInput;
extern char curdir[4096];

char *currentTile = "";

int showGeomTiles = 1;
int showAttTiles = 1;
int showEntities = 1;

int curTileIndex = 0;;
int painting = 0;
int erasing = 0;

Uint16 curGFXTile = 0;
Uint8 curGeomTile = TILE_SOLID;

int paintMode = PAINT_MODE_GFX;

Uint16 newBrushStartTile = 0;
Uint16 newBrushEndTile = 0;
RGS_UIButton *createBrushButton;
RGS_UIButton *removeBrushButton;

Uint16 oldPaintMode = 0;

char *curTilemapTilePath = NULL;

RGS_Screen *newTilemapScreen = NULL;
RGS_UITextInput *newTilemapWidth;
RGS_UITextInput *newTilemapHeight;
RGS_UITextInput *newTilemapTileSize;
RGS_UIButton *newTilemapGo;

ISArray *entitySprites;

int doApplyToAll = 0;
int doApplyToAllAtts = 0;

ISArray *brushes;
ISArray *attributes;
ISArray *entities;

Editor_Brush *curBrush = NULL;
RGS_Tilemap *brushTilemap = NULL;
RGS_Tilemap *brushPreviewTilemap = NULL;

RGS_UIButton *createAttButton = NULL;
RGS_UIButton *removeAttButton = NULL;

int numAttColors = 16;
int attColors[16] = {0xFF96C9, 0x00ffff,0xff00ff,0x0000ff,0xc0c0c0,0x008080,0x800080,0x000080,0xffff00,0x808000,0x008000,0xff0000,0x800000,0xff6000,0x0078ff,0x00ffb4};
int curAttTile = 0;
Editor_Att *curAtt = NULL;
Editor_Entity *curEntity = NULL;

int entityMode = ENTITY_MODE_PLACE;
RGS_Entity *selectedEntity = NULL;

int eDragOffsetX = 0;
int eDragOffsetY = 0;

void initMapEditor() {
	
	mapEditorScreen = RGS_CreateScreen();
	mapEditorScreen->visible = 0;
	
	gfxScreen = RGS_CreateScreen();
	gfxScreen->visible = 0;
	geomScreen = RGS_CreateScreen();
	geomScreen->visible = 0;
	attScreen = RGS_CreateScreen();
	attScreen->visible = 0;
	entityScreen = RGS_CreateScreen();
	entityScreen->visible = 0;
	entityOverlayScreen = RGS_CreateScreen();
	
	entitySprites = ISArray_Create(sizeof(RGS_Sprite));
	
	RGS_CreateLabel(mapEditorScreen, "MAP LAYERS", RGS_FONT_BIG, EDITOR_LEFT_MARGIN+5,24,0x79CA00);
	RGS_CreateLabel(geomScreen, "COLLISION TILES", RGS_FONT_BIG, EDITOR_LEFT_MARGIN+170,24,0x6F7FF8);
	RGS_CreateLabel(attScreen, "ATTRIBUTE TILES", RGS_FONT_BIG, EDITOR_LEFT_MARGIN+170,24,0xFF8C49);
	
	RGS_CreateLabel(gfxScreen, "PAINT MODE", RGS_FONT_BIG, EDITOR_LEFT_MARGIN+160,24,0xFF931F);
	RGS_CreateLabel(gfxScreen, "GFX TILES", RGS_FONT_BIG, EDITOR_LEFT_MARGIN+250,24,0xEA68FC);
	RGS_CreateLabel(gfxScreen, "BRUSHES", RGS_FONT_BIG, EDITOR_LEFT_MARGIN+400,24,0x86FFE7);
	RGS_CreateLabel(entityScreen, "ENTITIES", RGS_FONT_BIG, EDITOR_LEFT_MARGIN+170,24,0xFF5E8B);
	
	mapLayers = RGS_CreateUIOptionList(mapEditorScreen, 15, EDITOR_LEFT_MARGIN+5, 40, 100, 100);
	RGS_AddUIOption(mapLayers, "GFX Tiles", PAINT_MODE_GFX, NULL, 0xffffff);
	RGS_AddUIOption(mapLayers, "Collision Tiles", PAINT_MODE_GEOM, NULL, 0xffffff);
	RGS_AddUIOption(mapLayers, "Attribute Tiles", PAINT_MODE_ATT1, NULL, 0xffffff);
	RGS_AddUIOption(mapLayers, "Entities", PAINT_MODE_ENTITY, NULL, 0xffffff);
	
	RGS_SelectButtonByID(mapLayers, PAINT_MODE_GFX);
	mapLayers->onSelect = choosePaintMode;
	
	gfxLayerCheckBox = RGS_CreateUICheckBox(mapEditorScreen, "visible", EDITOR_LEFT_MARGIN+110, 43);
	gfxLayerCheckBox->checked = 1;
	gfxLayerCheckBox->onChange = changeLayerVis;
	geomLayerCheckBox = RGS_CreateUICheckBox(mapEditorScreen, "visible", EDITOR_LEFT_MARGIN+110, 58);
	geomLayerCheckBox->checked = 1;
	geomLayerCheckBox->onChange = changeLayerVis;
	att1LayerCheckBox = RGS_CreateUICheckBox(mapEditorScreen, "visible", EDITOR_LEFT_MARGIN+110, 73);
	att1LayerCheckBox->checked = 1;
	att1LayerCheckBox->onChange = changeLayerVis;
	entityLayerCheckBox = RGS_CreateUICheckBox(mapEditorScreen, "visible", EDITOR_LEFT_MARGIN+110, 88);
	entityLayerCheckBox->checked = 1;
	entityLayerCheckBox->onChange = changeLayerVis;
	
	
	geomTiles = RGS_CreateUIOptionList(geomScreen, 15, EDITOR_LEFT_MARGIN+170, 40, 100, 100);
	RGS_AddUIOption(geomTiles, "TILE_SOLID", TILE_SOLID, NULL, 0x00ff00);
	RGS_AddUIOption(geomTiles, "TILE_EMPTY", TILE_EMPTY, NULL, 0x00ff00);
	geomTiles->onSelect = chooseGeomBrush;
	
	RGS_SelectButtonByID(geomTiles, 1);
	
	applyToAll = RGS_CreateUICheckBox(geomScreen, "Apply to all with same gfx.", EDITOR_LEFT_MARGIN+170, 150);
	applyToAll->onChange = changeApplyToAll;
	
	////////////////////////////////////////////////////////////////////////////////////
	// ATTRIBUTE SCREEN
	////////////////////////////////////////////////////////////////////////////////////
	
	attributeTiles = RGS_CreateUIOptionList(attScreen, 15, EDITOR_LEFT_MARGIN+170, 40, 100, 77);
	attributeTiles->onSelect = chooseAttTile;
	
	applyToAllAtts = RGS_CreateUICheckBox(attScreen, "Apply to all with same gfx.", EDITOR_LEFT_MARGIN+170, 150);
	applyToAllAtts->onChange = changeApplyToAll;
	
	createAttButton = RGS_CreateUIButton(attScreen, "New Attr.", EDITOR_LEFT_MARGIN+170, 122, 55, 18);
	createAttButton->uiObject.onRelease = createNewAtt;
	removeAttButton = RGS_CreateUIButton(attScreen, "Remove", EDITOR_LEFT_MARGIN+231, 122, 40, 18);
	removeAttButton->uiObject.onRelease = removeAtt;
	
	attName = RGS_CreateUITextInput(attScreen, EDITOR_LEFT_MARGIN+280, 40, 100, 18);
	attCode = RGS_CreateUITextInput(attScreen, EDITOR_LEFT_MARGIN+280, 65, 100, 18);
	
	saveAtt = RGS_CreateUIButton(attScreen, "Update", EDITOR_LEFT_MARGIN+280, 90, 50, 18);
	saveAtt->uiObject.onRelease = updateAttName;
	saveAttsToFile = RGS_CreateUIButton(attScreen, "Save Attributes To File", EDITOR_LEFT_MARGIN+280, 122, 130, 18);
	saveAttsToFile->uiObject.onRelease = saveAttributes;
	
	////////////////////////////////////////////////////////////////////////////////////	
	// ENTITY SCREEN 
	////////////////////////////////////////////////////////////////////////////////////
	
	entityList = RGS_CreateUIOptionList(entityScreen, 15, EDITOR_LEFT_MARGIN+170, 40, 100, 77);
	entityList->onSelect = chooseEntity;	
	
	createEntityButton = RGS_CreateUIButton(entityScreen, "New Entity", EDITOR_LEFT_MARGIN+170, 122, 55, 18);
	createEntityButton->uiObject.onRelease = createNewEntity;
	
	entName = RGS_CreateUITextInput(entityScreen, EDITOR_LEFT_MARGIN+280, 40, 100, 18);
	entCode = RGS_CreateUITextInput(entityScreen, EDITOR_LEFT_MARGIN+280, 65, 100, 18);
	setEntitySprite = RGS_CreateUIButton(entityScreen, "Set Sprite", EDITOR_LEFT_MARGIN+280, 90, 100, 18);
	setEntitySprite->uiObject.onRelease = setEntitySpriteGO;
	saveEntity = RGS_CreateUIButton(entityScreen, "Update", EDITOR_LEFT_MARGIN+280, 115, 50, 18);
	saveEntity->uiObject.onRelease = updateEntityName;
	saveEntitiesToFile = RGS_CreateUIButton(entityScreen, "Save Entities To File", EDITOR_LEFT_MARGIN+480, 122, 130, 18);
	saveEntitiesToFile->uiObject.onRelease = saveEntities;
	
	entityModeSelector = RGS_CreateShape(entityScreen, RGS_SHAPE_RECT, 0xffffff, EDITOR_LEFT_MARGIN+208, 145, 54, 22);
	
	RGS_CreateLabel(entityScreen, "MODE:", RGS_FONT_BIG, EDITOR_LEFT_MARGIN+170,152,0xFF8C49);
	entityPlaceModeButton = RGS_CreateUIButton(entityScreen, "Place", EDITOR_LEFT_MARGIN+210, 147, 50,18);
	entityPlaceModeButton->uiObject.onRelease = chooseEntityMode;
	entitySelectModeButton = RGS_CreateUIButton(entityScreen, "Select", EDITOR_LEFT_MARGIN+265, 147, 50,18);
	entitySelectModeButton->uiObject.onRelease = chooseEntityMode;
//	entityRemoveModeButton = RGS_CreateUIButton(entityScreen, "Remove", EDITOR_LEFT_MARGIN+320, 147, 50,18);
//	entityRemoveModeButton->uiObject.onRelease = chooseEntityMode;	
	
	////////////////////////////////////////////////////////////////////////////////////	
	// GFX SCREEN
	////////////////////////////////////////////////////////////////////////////////////
	
	gfxPaintModeList = RGS_CreateUIOptionList(gfxScreen, 15, EDITOR_LEFT_MARGIN+160, 40, 80, 100);
	RGS_AddUIOption(gfxPaintModeList, "Single tile", GFX_PAINT_SINGLETILE, NULL, 0xffffff);
	RGS_AddUIOption(gfxPaintModeList, "Brush", GFX_PAINT_BRUSH, NULL, 0xffffff);
	RGS_SelectButtonByID(gfxPaintModeList, GFX_PAINT_SINGLETILE);
	gfxPaintModeList->onSelect = chooseGfxPaintMode;
	
	brushList = RGS_CreateUIOptionList(gfxScreen, 15, EDITOR_LEFT_MARGIN+400, 40, 100, 77);
	brushList->onSelect = chooseBrush;
	
	createBrushButton = RGS_CreateUIButton(gfxScreen, "New Brush", EDITOR_LEFT_MARGIN+400, 122, 55, 18);
	createBrushButton->uiObject.onRelease = startCreatingBrush;
	removeBrushButton = RGS_CreateUIButton(gfxScreen, "Remove", EDITOR_LEFT_MARGIN+460, 122, 40, 18);
	removeBrushButton->uiObject.onRelease = removeBrush;
	
	brushName = RGS_CreateUITextInput(gfxScreen, EDITOR_LEFT_MARGIN+505, 40, 100, 18);
	saveBrush = RGS_CreateUIButton(gfxScreen, "Update", EDITOR_LEFT_MARGIN+610, 40, 50, 18);
	saveBrush->uiObject.onRelease = updateBrushName;
	saveBrushesToFile = RGS_CreateUIButton(gfxScreen, "Save Brushes To File", EDITOR_LEFT_MARGIN+695, 40, 120, 18);
	saveBrushesToFile->uiObject.onRelease = saveBrushes;
	
	mapScrollerH = RGS_CreateUIHorizontalScroller(mapEditorScreen, EDITOR_LEFT_MARGIN+5, 768-25, 1024 - EDITOR_LEFT_MARGIN - 30, 20);
	mapScrollerV = RGS_CreateUIVerticalScroller(mapEditorScreen, 1024-20, EDITOR_TOP_MARGIN, 20, 768 - EDITOR_TOP_MARGIN);
	tileSelectorScroller = RGS_CreateUIVerticalScroller(gfxScreen, EDITOR_LEFT_MARGIN+378, 40, 10, 100);
	
	newTilemapScreen = RGS_CreateScreen();
	
	RGS_CreateLabel(newTilemapScreen, "CREATE NEW TILEMAP", RGS_FONT_BIG, EDITOR_LEFT_MARGIN+5,EDITOR_TOP_MARGIN-130,0x79CA00);
	RGS_CreateLabel(newTilemapScreen, "Tile size (8-256):", RGS_FONT_SMALL, EDITOR_LEFT_MARGIN+5,EDITOR_TOP_MARGIN-97,0x4c4c4c);
	RGS_CreateLabel(newTilemapScreen, "Map width:", RGS_FONT_SMALL, EDITOR_LEFT_MARGIN+5,EDITOR_TOP_MARGIN-72,0x4c4c4c);
	RGS_CreateLabel(newTilemapScreen, "Map height:", RGS_FONT_SMALL, EDITOR_LEFT_MARGIN+5,EDITOR_TOP_MARGIN-47,0x4c4c4c);
	
	newTilemapTileSize = RGS_CreateUITextInput(newTilemapScreen, EDITOR_LEFT_MARGIN+85, EDITOR_TOP_MARGIN-100, 40, 20); 
	RGS_SetLabelText(newTilemapTileSize->label, "8");
	
	newTilemapWidth = RGS_CreateUITextInput(newTilemapScreen, EDITOR_LEFT_MARGIN+85, EDITOR_TOP_MARGIN-75, 40, 20); 
	RGS_SetLabelText(newTilemapWidth->label, "64");
	newTilemapHeight = RGS_CreateUITextInput(newTilemapScreen, EDITOR_LEFT_MARGIN+85, EDITOR_TOP_MARGIN-50, 40, 20); 
	RGS_SetLabelText(newTilemapHeight->label, "64");
	
	newTilemapGo = RGS_CreateUIButton(newTilemapScreen, "Create", EDITOR_LEFT_MARGIN+5, EDITOR_TOP_MARGIN-25, 120, 18);
	newTilemapGo->uiObject.onRelease = CreateNewTilemapGO;
	newTilemapScreen->visible = 0;
	
	brushes = ISArray_Create(sizeof(Editor_Brush));
	attributes = ISArray_Create(sizeof(Editor_Att));
	entities = ISArray_Create(sizeof(Editor_Entity));
	
	RGS_RegisterDefaultEntity(entityOverlayScreen, "", DefEntity_Init, NULL, NULL, 1, entity_icon);
	
	refreshAtts();
	
	//// temporary stuff
	
}

void chooseEntityMode(void *target) {
	//	entityModeSelector = RGS_CreateShape(entityScreen, RGS_SHAPE_RECT, 0xffffff, EDITOR_LEFT_MARGIN+208, 145, 54, 22);
	if(target == entityPlaceModeButton) {
		entityModeSelector->object.x = EDITOR_LEFT_MARGIN+208;
		entityMode = ENTITY_MODE_PLACE;
	} else if(target == entitySelectModeButton) {
		entityModeSelector->object.x = EDITOR_LEFT_MARGIN+263;
		entityMode = ENTITY_MODE_SELECT;
	} else if(target == entityRemoveModeButton) {
		entityModeSelector->object.x = EDITOR_LEFT_MARGIN+318;	
		entityMode = ENTITY_MODE_REMOVE;
	}
}

void DefEntity_Init(RGS_Entity *entity) {
	entity->noGravityOn = 1;
	entity->noClipOn = 1;
}

void refreshEntitiesInMap() {
	int i,j;
	Editor_Entity *curEdEnt;
	RGS_Entity *ent;
	
	for(j =0; j < entities->length;j++) {
		curEdEnt = (Editor_Entity*)entities->data[j];
		
		for(i =0; i < workingTilemap->entities->length;i++) {
			ent = (RGS_Entity*)workingTilemap->entities->data[i];
			if(ent->entityID == curEdEnt->code) {
				if(curEdEnt->spritePath->length > 0) {
					RGS_DestroyObject(ent->object);
					ent->object = (RGS_Object*)RGS_CreateSprite(entityOverlayScreen, curEdEnt->spritePath->text);
				}
			}
		}
	}
}


void updateEntityName(void *target) {
	ISString_SetText(curEntity->name, entName->label->text);
	curEntity->code = atoi(entCode->label->text);
	
	printf("id: %d\n", curEntity->code);
	
	int i;
	RGS_Entity *ent;
	for(i =0; i < workingTilemap->entities->length;i++) {
		ent = (RGS_Entity*)workingTilemap->entities->data[i];
		if(ent->entityID == curEntity->code) {
			if(curEntity->spritePath->length > 0) {
				RGS_DestroyObject(ent->object);
				ent->object = (RGS_Object*)RGS_CreateSprite(entityOverlayScreen, curEntity->spritePath->text);
			}
		}
	}
	
	refreshEntities();
}


void setEntitySpriteGO(void *target) {
	char tmp[4096];
	strcpy(tmp, curdir);
	strcat(tmp, "/");
	strcat(tmp, fileInput->label->text);
	ISString_SetText(curEntity->spritePath, tmp);
}

void refreshEntities() {
	int i;
	Editor_Entity *ent;
	
	RGS_ClearUIOptionList(entityList);
	//	RGS_AddUIOption(entityList, "CLEAR  ATTRIBUTE", 0, NULL, 0xffffff);
	
	for(i=0;i<entities->length;i++) {
		ent = (Editor_Entity*) entities->data[i];
		RGS_AddUIOption(entityList, ent->name->text, i, NULL, 0xffffff);
	}
	
	//if(entities->length > 0)
	//	chooseEntity(0, NULL);
}

void createNewEntity(void *target) {
	Editor_Entity *newEntity = (Editor_Entity*)ISArray_CreateElement(entities);
	newEntity->name = ISString_Create("new entity");
	newEntity->spritePath = ISString_Create("");
	newEntity->code = 1;
	refreshEntities();
}

void chooseEntity(int id, void *data) {
	curEntity = (Editor_Entity*) entities->data[id];
	RGS_SetLabelText(entName->label, curEntity->name->text);
	char code[16];
	sprintf(code, "%d", curEntity->code);
	RGS_SetLabelText(entCode->label, code);	
	if(previewSprite != NULL) {
		RGS_DestroyObject((RGS_Object*)previewSprite);
	}
	
	if(strlen(curEntity->spritePath->text) > 1) {
		previewSprite = RGS_CreateSprite(entityScreen, curEntity->spritePath->text);
		previewSprite->object.x = EDITOR_LEFT_MARGIN+390;
		previewSprite->object.y = 40;
	} else {
		previewSprite = NULL;
	}
}

void updateAttName(void *target) {
	ISString_SetText(curAtt->name, attName->label->text);
	curAtt->code = atoi(attCode->label->text);
	refreshAtts();
}

void saveEntities(void *target) {
	char spritePath[256];
	char tmp[4096];
	strcpy(tmp, curdir);
	strcat(tmp, "/");
	strcat(tmp, fileInput->label->text);
	
	int i;
	Uint16 sval = 0;
	
	FILE *outPtr = fopen(tmp, "wb");
	sval = entities->length;
	fwrite(&sval, 2, 1, outPtr);
	
	Editor_Entity *ent;
	
	for(i=0; i < entities->length; i++) {
		ent = (Editor_Entity*)entities->data[i];
		sval = ent->name->length;
		fwrite(&sval, 2, 1, outPtr);
		fwrite(ent->name->text, 1, ent->name->length, outPtr);
		
		if(ent->spritePath->length > 0) {
			strcpy(spritePath, ent->spritePath->text+strlen(curdir)+1);
			sval = strlen(spritePath);
			fwrite(&sval, 2, 1, outPtr);
			fwrite(spritePath, 1, strlen(spritePath), outPtr);
		} else {
			sval = 0;
			fwrite(&sval, 2, 1, outPtr);
		}
		fwrite(&ent->code, 1, 1, outPtr);
	}
	
	fclose(outPtr);
	reloadFileBrowser(curdir);
}

void saveAttributes(void *target) {
	char tmp[4096];
	strcpy(tmp, curdir);
	strcat(tmp, "/");
	strcat(tmp, fileInput->label->text);
	
	int i;
	Uint16 sval = 0;
	
	FILE *outPtr = fopen(tmp, "wb");
	sval = attributes->length;
	fwrite(&sval, 2, 1, outPtr);
	
	Editor_Att *att;
	
	for(i=0; i < attributes->length; i++) {
		att = (Editor_Att*)attributes->data[i];
		sval = att->name->length;
		fwrite(&sval, 2, 1, outPtr);
		fwrite(att->name->text, 1, att->name->length, outPtr);
		fwrite(&att->code, 1, 1, outPtr);
	}
	
	fclose(outPtr);
	reloadFileBrowser(curdir);
}

void chooseAttTile(int id, void *data) {
	if(id == 0) {
		curAtt = NULL;
		curAttTile = 0;
	} else {
		curAtt = (Editor_Att*) attributes->data[id-1];
		RGS_SetLabelText(attName->label, curAtt->name->text);
		char code[16];
		sprintf(code, "%d", curAtt->code);
		RGS_SetLabelText(attCode->label, code);
		curAttTile = curAtt->code;
	}	
}

void refreshAtts() {
	int i;
	Editor_Att *att;
	
	RGS_ClearUIOptionList(attributeTiles);
	RGS_AddUIOption(attributeTiles, "CLEAR  ATTRIBUTE", 0, NULL, 0xffffff);
	for(i=0;i<attributes->length;i++) {
		att = (Editor_Att*) attributes->data[i];
		RGS_AddUIOption(attributeTiles, att->name->text, i+1, NULL, attColors[att->code % 16]);
	}
	RGS_SelectButtonByID(attributeTiles, 0);
}

void createNewAtt(void *target) {
	Editor_Att *newAtt = (Editor_Att*)ISArray_CreateElement(attributes);
	newAtt->name = ISString_Create("new attribute");
	newAtt->code = 1;
	refreshAtts();
}

void removeAtt(void *target) {
	if(attributeTiles->selectedID == -1)
		return;
	
	if(attributeTiles->selectedID == 0)	{
		RGS_SetLabelText(errorMessageLabel, "ERROR: You cannot remove that! 'Ta fuck!");
		return;
	}
	
	Editor_Att *att = (Editor_Att*)attributes->data[attributeTiles->selectedID-1];
	
	ISArray_RemoveElement(attributes, (void*)att);
	refreshAtts();
}

void MapEditor_LoadEntities(char *fileName) {
	Editor_Entity *ent;
	int i;
	
	char tmp[4096];
	
	for(i=0;i<entities->length;i++) {
		ent = (Editor_Entity*)entities->data[i];
		ISString_Destroy(ent->name);
		ISString_Destroy(ent->spritePath);
		//		ISArray_RemoveElement(entities, (void*)ent);
		//		free((void*)ent);
	}
	if(entities->length > 0)
		ISArray_Clear(entities);
	
	FILE *inPtr = fopen(fileName, "rb");
	
	Uint16 numEnts, sval;
	fread(&numEnts, 2, 1, inPtr);
	
	char *tmpString = NULL;
	
	for(i=0;i<numEnts;i++) {
		ent = (Editor_Entity*)ISArray_CreateElement(entities);
		fread(&sval, 2, 1, inPtr);
		if(tmpString != NULL)
			free(tmpString);
		tmpString = (char*)malloc(sval+1);
		fread(tmpString, 1, sval, inPtr);
		memset(tmpString+sval, 0, 1);
		ent->name = ISString_Create(tmpString);
		free(tmpString);
		tmpString = NULL;
		
		fread(&sval, 2, 1, inPtr);
		if(sval > 0) {
			tmpString = (char*)malloc(sval+1);
			fread(tmpString, 1, sval, inPtr);
			memset(tmpString+sval, 0, 1);
			strcpy(tmp, curdir);
			strcat(tmp, "/");
			strcat(tmp, tmpString);	
			ent->spritePath = ISString_Create(tmp);
			free(tmpString);
			tmpString = NULL;
		} else {
			ent->spritePath = ISString_Create("");		
		}
		
		fread(&ent->code, 1, 1, inPtr);
	}
	
	fclose(inPtr);	
	
	refreshEntities();
	refreshEntitiesInMap();
}

void MapEditor_LoadAttributes(char *fileName) {
	
	int i;
	Editor_Att *att;
	
	for(i=0;i<attributes->length;i++) {
		att = (Editor_Att*)attributes->data[i];
		ISString_Destroy(att->name);
		ISArray_RemoveElement(attributes, (void*)att);
		free((void*)att);
	}
	
	FILE *inPtr = fopen(fileName, "rb");
	
	Uint16 numAtts, sval;
	fread(&numAtts, 2, 1, inPtr);
	
	char *tmpString = NULL;
	
	for(i=0;i<numAtts;i++) {
		att = (Editor_Att*)ISArray_CreateElement(attributes);
		fread(&sval, 2, 1, inPtr);
		if(tmpString != NULL)
			free(tmpString);
		tmpString = (char*)malloc(sval+1);
		fread(tmpString, 1, sval, inPtr);
		memset(tmpString+sval, 0, 1);
		att->name = ISString_Create(tmpString);
		fread(&att->code, 1, 1, inPtr);
	}
	
	fclose(inPtr);
	refreshAtts();
	
}

void MapEditor_LoadBrushes(char *fileName) {
	
	destroyBrushes();
	
	FILE *inPtr = fopen(fileName, "rb");
	
	Uint16 numBrushes, sval;
	fread(&numBrushes, 2, 1, inPtr);
	int i;
	
	char *tmpString = NULL;
	
	for(i=0;i<numBrushes;i++) {
		Editor_Brush *newBrush = (Editor_Brush*)ISArray_CreateElement(brushes);
		fread(&sval, 2, 1, inPtr);
		
		if(tmpString != NULL)
			free(tmpString);
		tmpString = (char*)malloc(sval+1);
		fread(tmpString, 1, sval, inPtr);
		memset(tmpString+sval, 0, 1);
		newBrush->name = ISString_Create(tmpString);
		fread(&newBrush->id, 2, 1, inPtr);
		fread(&newBrush->brushRect, sizeof(RGS_Rect), 1, inPtr);
		newBrush->data = (Uint16*)malloc(2*(newBrush->brushRect.w*newBrush->brushRect.h));
		fread(newBrush->data, 2, newBrush->brushRect.w*newBrush->brushRect.h, inPtr);
	}
	
	fclose(inPtr);
	refreshBrushList();
}

void saveBrushes(void *target) {
	char tmp[4096];
	strcpy(tmp, curdir);
	strcat(tmp, "/");
	strcat(tmp, fileInput->label->text);
	
	int i;
	
	Uint16 sval = 0;
	
	FILE *outPtr = fopen(tmp, "wb");
	sval = brushes->length;
	fwrite(&sval, 2, 1, outPtr);
	
	Editor_Brush *brush;
	
	for(i=0; i < brushes->length; i++) {
		brush = (Editor_Brush*)brushes->data[i];
		sval = brush->name->length;
		fwrite(&sval, 2, 1, outPtr);
		fwrite(brush->name->text, 1, brush->name->length, outPtr);
		fwrite(&brush->id, 2, 1, outPtr);
		fwrite(&brush->brushRect, sizeof(RGS_Rect), 1, outPtr);
		fwrite(brush->data, 2, brush->brushRect.w*brush->brushRect.h, outPtr);
	}
	
	fclose(outPtr);
	reloadFileBrowser(curdir);
}

void updateBrushName(void *target) {
	if(curBrush != NULL)
		ISString_SetText(curBrush->name, brushName->label->text);
	refreshBrushList();
}

void chooseGfxPaintMode(int id, void *data) {
	if(id == GFX_PAINT_BRUSH) {
		if(brushes->length == 0) {
			RGS_SetLabelText(errorMessageLabel, "There are no brushes defined!");
			RGS_SelectButtonByID(gfxPaintModeList, gfxPaintMode);
			return;
		}
		if(curBrush == NULL) {
			RGS_SetLabelText(errorMessageLabel, "There is no brush selected!");
			RGS_SelectButtonByID(gfxPaintModeList, gfxPaintMode);
			return;
		}
		brushTilemap->object.visible = 1;
	} else {
		brushTilemap->object.visible = 0;
	}
	gfxPaintMode = id;
}

void chooseGeomBrush(int id, void *data) {
	curGeomTile = id;
}

void choosePaintMode(int id, void *data) {
	paintMode = id;
	
	gfxScreen->visible = 0;
	geomScreen->visible = 0;
	attScreen->visible = 0;
	entityScreen->visible = 0;
	
	switch(paintMode) {
		case PAINT_MODE_GFX:
			gfxScreen->visible = 1;
			break;
		case PAINT_MODE_GEOM:
			geomScreen->visible = 1;		
			break;
		case PAINT_MODE_ATT1:
			attScreen->visible = 1;
			break;
		case PAINT_MODE_ENTITY:
			entityScreen->visible = 1;
			break;
	}
}

void MapEditor_SaveTilemap(char *fileName) {
	RGS_SaveTilemap(workingTilemap, fileName);
}

void CreateNewTilemapGO(void *target) {
	if(curTilemapTilePath == NULL) {
		RGS_SetLabelText(errorMessageLabel, "ERROR: You need to choose a tileset first!");
		return;
	}	
	
	if(workingTilemap != NULL) {
		RGS_DestroyObject((RGS_Object*)workingTilemap);
	}
	
	RGS_ScreenDestroyObjects(entityOverlayScreen);	
	workingTilemap = RGS_CreateEmptyTilemap(mapEditorScreen, 1024-EDITOR_LEFT_MARGIN-30, 768-EDITOR_TOP_MARGIN-30, atoi(newTilemapWidth->label->text), atoi(newTilemapHeight->label->text),  atoi(newTilemapTileSize->label->text), curTilemapTilePath);
	setupNewTilemap();
	choosePaintMode(PAINT_MODE_GFX, NULL);
	newTilemapScreen->visible = 0;
}

void startCreatingBrush(void *target) {
	if(paintMode != PAINT_MODE_BRUSHDEFSTART)
		oldPaintMode = paintMode;
	paintMode = PAINT_MODE_BRUSHDEFSTART;
}

void refreshBrushList() {
	RGS_ClearUIOptionList(brushList);
	int i;
	Editor_Brush *brush;
	for(i=0;i<brushes->length;i++){
		brush = (Editor_Brush*)brushes->data[i];
		RGS_AddUIOption(brushList, brush->name->text, i, NULL, 0xffffff);
	}
}

void removeBrush(void *target) {
	if(brushList->selectedID == -1)
		return;
	
	Editor_Brush *brush = (Editor_Brush*)brushes->data[brushList->selectedID];
	ISArray_RemoveElement(brushes, (void*)brush);
	DestroyBrush(brush);
	refreshBrushList();
}

void destroyBrushes() {
	int i;
	Editor_Brush *brush;
	int stLen = brushes->length;
	for(i=stLen-1;i >= 0; i--) {
		brush = (Editor_Brush*)brushes->data[i];
		DestroyBrush(brush);
		ISArray_RemoveElement(brushes, brush);
	}
}

void DestroyBrush(Editor_Brush *brush) {
	free(brush->data);
	ISString_Destroy(brush->name);
	free(brush);
}

void changeApplyToAll(void *target) {
	RGS_UICheckBox *targetBox = (RGS_UICheckBox*)target;
	if(targetBox == applyToAll) {
		doApplyToAll = targetBox->checked;	
	} else if (targetBox == applyToAllAtts) {
		doApplyToAllAtts = targetBox->checked;	
	}
}

void changeLayerVis(void *target) {
	RGS_UICheckBox *targetBox = (RGS_UICheckBox*)target;
	if(targetBox == gfxLayerCheckBox) {
		workingTilemap->object.visible = targetBox->checked;
	} else if(targetBox == geomLayerCheckBox) {
		showGeomTiles = targetBox->checked;
	} else if(targetBox == att1LayerCheckBox) {
		showAttTiles = att1LayerCheckBox->checked;
	} else if(targetBox == entityLayerCheckBox) {
		showEntities = entityLayerCheckBox->checked;
	}
} 

void applyBrush(Editor_Brush *brush, Uint16 tIndex) {
	int i,j;
	
	int x = tIndex % workingTilemap->mapWidth;
	int y = (tIndex-x) / workingTilemap->mapWidth;
	
	int k = 0;
	
	for(i=y;i<y+brush->brushRect.h;i++) {
		for(j=x;j<x+brush->brushRect.w;j++) {
			workingTilemap->mapTileData[(i*workingTilemap->mapWidth)+j] = brush->data[k];
			k++;
		}
	}
}

void paintTile(int tileIndex) {
	int i;
	
	switch(paintMode) {
		case PAINT_MODE_GEOM:
			if(doApplyToAll == 1) {
				for(i=0;i<workingTilemap->mapWidth*workingTilemap->mapHeight;i++) {
					if(workingTilemap->mapTileData[i] == workingTilemap->mapTileData[tileIndex])
						workingTilemap->mapAttributeData[i] = curGeomTile;
				}
			} else {
				workingTilemap->mapAttributeData[tileIndex] = curGeomTile;
			}
			break;
			case PAINT_MODE_GFX:
			switch(gfxPaintMode) {
				case GFX_PAINT_SINGLETILE:
					workingTilemap->mapTileData[tileIndex] = curGFXTile;
					break;
				case GFX_PAINT_BRUSH:
					applyBrush(curBrush, tileIndex);
					break;
			}
			RGS_RedrawTilemapBuffer(workingTilemap);
			break;
			case PAINT_MODE_BRUSHDEFSTART:
			paintMode = PAINT_MODE_BRUSHDEFEND;
			newBrushStartTile = tileIndex;
			break;
			case PAINT_MODE_BRUSHDEFEND:
			newBrushEndTile = tileIndex;
			break;
			case PAINT_MODE_ATT1:
			if(doApplyToAllAtts == 1) {
				for(i=0;i<workingTilemap->mapWidth*workingTilemap->mapHeight;i++) {
					if(workingTilemap->mapTileData[i] == workingTilemap->mapTileData[tileIndex])
						workingTilemap->userAttributeData[i] = curAttTile;
				}
			} else {			
				workingTilemap->userAttributeData[tileIndex] = curAttTile;
			}
			break;
			case PAINT_MODE_ENTITY: {
				switch(entityMode) {
					case ENTITY_MODE_PLACE: {
						RGS_EntitySprite *newSprite;
						if(curEntity != NULL) {
						if(curEntity->spritePath->length > 0) {
							newSprite = RGS_CreateEntitySprite(entityOverlayScreen, workingTilemap, curEntity->spritePath->text);
						} else {
							newSprite = RGS_CreateEntitySpriteFromMemory(entityOverlayScreen, workingTilemap, entity_icon);
						}
						
						newSprite->entity->entityID = curEntity->code;
						newSprite->entity->noClipOn = 1;
						newSprite->entity->noGravityOn = 1;
						newSprite->entity->x = (tileIndex % workingTilemap->mapWidth) * workingTilemap->tileSize;
						newSprite->entity->y = ((tileIndex / workingTilemap->mapWidth)+1) * workingTilemap->tileSize;
						}
						}						
						break;
					case ENTITY_MODE_SELECT: {
						selectedEntity = RGS_TilemapGetEntityAt(workingTilemap, RGS_GetMouseX()-workingTilemap->object.x+workingTilemap->cameraX, RGS_GetMouseY()-workingTilemap->object.y+workingTilemap->cameraY);
						if(selectedEntity != NULL) {
							eDragOffsetX = RGS_GetMouseX()-workingTilemap->object.x+workingTilemap->cameraX-selectedEntity->x;
							eDragOffsetY = RGS_GetMouseY()-workingTilemap->object.y+workingTilemap->cameraY-selectedEntity->y;
						}
					}
					break;
						default:
						break;
				}
			}
			break;
	}
}

void MapEditor_KeyPress(int keyCode) {
	switch(keyCode) {
		case RGS_DELETE:
			if(selectedEntity != NULL) {
				RGS_ScreenRemoveObject(entityOverlayScreen, selectedEntity->object);
				selectedEntity = NULL;
			}
		break;
	}
}


void changeTile(void *target) {
	painting = 1;
	paintTile(curTileIndex);
}

void selectTile(void *target) {
	int startY = workingTilemap->cameraY/workingTilemap->tileSize;
	if(startY > 0)
		startY += 1;
	
	int offsetY = selectorTilemap->cameraY - (startY*workingTilemap->tileSize);
	curGFXTile = (RGS_GetMouseX() - selectorTilemap->object.x) / workingTilemap->tileSize + (((RGS_GetMouseY() - selectorTilemap->object.y+offsetY) / workingTilemap->tileSize)*selectorTilemap->mapWidth);
}

void endChangeTile(void *target) {
	painting = 0;
	
	if(selectedEntity != NULL) {
		selectedEntity->object->onRelease = NULL;
	}
	
	if(paintMode == PAINT_MODE_BRUSHDEFEND) {
		paintMode = oldPaintMode;
		createNewBrush();
	}
}

void copyBrushToTilemap(Editor_Brush *brush, RGS_Tilemap *tilemap) {
	int i,j;
	int k =0;
	for(i =0; i < brush->brushRect.h; i++) {
		for(j=0; j < brush->brushRect.w; j++) {
			tilemap->mapTileData[(i*brush->brushRect.w) + j] = brush->data[k];
			k++;
		}
	}
}

void chooseBrush(int id, void *data) {
	curBrush = (Editor_Brush*)brushes->data[id];
	
	if(brushTilemap != NULL) {
		RGS_DestroyObject((RGS_Object*)brushTilemap);
	}
	
	if(brushPreviewTilemap != NULL) {
		RGS_DestroyObject((RGS_Object*)brushPreviewTilemap);
	}
	
	brushTilemap = RGS_CreateEmptyTilemap(gfxScreen, curBrush->brushRect.w*workingTilemap->tileSize, curBrush->brushRect.h*workingTilemap->tileSize, curBrush->brushRect.w,curBrush->brushRect.h, workingTilemap->tileSize, curTilemapTilePath);
	brushPreviewTilemap = RGS_CreateEmptyTilemap(gfxScreen, curBrush->brushRect.w*workingTilemap->tileSize, curBrush->brushRect.h*workingTilemap->tileSize, curBrush->brushRect.w,curBrush->brushRect.h, workingTilemap->tileSize, curTilemapTilePath);
	brushPreviewTilemap->object.x = EDITOR_LEFT_MARGIN+505;
	brushPreviewTilemap->object.y = 62;
	
	copyBrushToTilemap(curBrush, brushTilemap);
	copyBrushToTilemap(curBrush, brushPreviewTilemap);
	
	RGS_SetLabelText(brushName->label, curBrush->name->text);
	brushTilemap->object.visible = 0;
	
}

void createNewBrush() {
	Editor_Brush *newBrush = (Editor_Brush*)ISArray_CreateElement(brushes);
	
	if(workingTilemap->cameraX > 0)
		newBrush->brushRect.x =	(workingTilemap->cameraX/workingTilemap->tileSize) +1+ ((tileSelector->object.x - workingTilemap->object.x)/workingTilemap->tileSize);
	else
		newBrush->brushRect.x =	(tileSelector->object.x - workingTilemap->object.x)/workingTilemap->tileSize;
	
	if(workingTilemap->cameraY > 0)
		newBrush->brushRect.y =	(workingTilemap->cameraY/workingTilemap->tileSize) + ((tileSelector->object.y - workingTilemap->object.y)/workingTilemap->tileSize);
	else
		newBrush->brushRect.y =	(tileSelector->object.y - workingTilemap->object.y)/workingTilemap->tileSize;
	
	newBrush->brushRect.w = tileSelector->object.w / workingTilemap->tileSize;
	newBrush->brushRect.h = tileSelector->object.h / workingTilemap->tileSize;
	
	newBrush->data = (Uint16*)malloc(sizeof(Uint16) * newBrush->brushRect.w * newBrush->brushRect.h);
	int i,j;
	int k=0;
	
	for(i=newBrush->brushRect.y; i < newBrush->brushRect.y+newBrush->brushRect.h; i++) {
		for(j=newBrush->brushRect.x; j < newBrush->brushRect.x+newBrush->brushRect.w; j++) {
			memcpy(newBrush->data+k, workingTilemap->mapTileData + (i*workingTilemap->mapWidth) + j, sizeof(Uint16));
			k++;
		}
	}
	
	newBrush->id = brushes->length-1;
	newBrush->name = ISString_Create("new brush");
	refreshBrushList();
	//RGS_AddUIOption(brushList, "new brush", brushes->length-1, NULL, 0xffffff);
}

void scrollMap(void *target) {
	workingTilemap->cameraX = ((float)((workingTilemap->mapWidth * workingTilemap->tileSize)-1024+EDITOR_LEFT_MARGIN+30)) * mapScrollerH->scrollValue;
	workingTilemap->cameraY = ((float)((workingTilemap->mapHeight * workingTilemap->tileSize)-768+EDITOR_TOP_MARGIN+30)) * mapScrollerV->scrollValue;
}

void scrollTileSelector(void *target) {
	selectorTilemap->cameraY = ((float)((selectorTilemap->mapHeight * selectorTilemap->tileSize)-100)) * tileSelectorScroller->scrollValue;
}

void CreateTileSelector() {
	
	if(selectorTilemap != NULL) {
		RGS_DestroyObject((RGS_Object*)selectorTilemap);
	}
	
	// create tile selector	
	int tWidth = 128/workingTilemap->tileSize;
	int tHeight = ceil((float)((workingTilemap->tilesSurface->surface->w/workingTilemap->tileSize) * (workingTilemap->tilesSurface->surface->h/workingTilemap->tileSize))/tWidth);
	
	selectorTilemap = RGS_CreateEmptyTilemap(gfxScreen, 128, 100, tWidth,tHeight, workingTilemap->tileSize, curTilemapTilePath);
	selectorTilemap->object.x = EDITOR_LEFT_MARGIN+250;
	selectorTilemap->object.y = 40;
	
	selectorTilemap->object.onPress = selectTile;
	
	int i,j;
	int k=0;
	for(i=0;i < tHeight; i++) {
		for(j=0;j < tWidth; j++) {
			selectorTilemap->mapTileData[i*tWidth+j] = k;
			k++;
		}
	}
	
	float tmp = ((float)(100))/((float)(selectorTilemap->mapHeight*workingTilemap->tileSize));
	tileSelectorScroller->percentage = tmp*100.0f;
	tileSelectorScroller->onChange = scrollTileSelector;		
	tileSelector = RGS_CreateShape(mapEditorScreen, RGS_SHAPE_RECT_OUTLINE, 0xffffff, 0,0, workingTilemap->tileSize, workingTilemap->tileSize);
	
}

void MapEditor_SetTilesetPath(char *newPath) {
	if(curTilemapTilePath == NULL)
		curTilemapTilePath = (char*)malloc(4096);
	
	strcpy(curTilemapTilePath, newPath);
	
	if(workingTilemap != NULL) {
		RGS_LoadNewTilemapTiles(workingTilemap, newPath);
		CreateTileSelector();
	}
}


void MapEditor_CreateNewTilemap() {
	mapEditorScreen->visible = 0;
	newTilemapScreen->visible = 1;
}

void openMap(char *mapFile) {
	
	if(curTilemapTilePath == NULL) {
		RGS_SetLabelText(errorMessageLabel, "ERROR: You need to choose a tileset first!");
		return;
	}
	
	RGS_ScreenDestroyObjects(entityOverlayScreen);	
	
	workingTilemap = RGS_CreateTilemap(mapEditorScreen, 1024-EDITOR_LEFT_MARGIN-30, 768-EDITOR_TOP_MARGIN-30, mapFile, curTilemapTilePath);
	
	setupNewTilemap();
	choosePaintMode(PAINT_MODE_GFX, NULL);
}

void setupNewTilemap() {


	workingTilemap->object.x = EDITOR_LEFT_MARGIN+5;
	workingTilemap->object.y = EDITOR_TOP_MARGIN+5;
	//tilemapImage = RGS_CreateSurfaceImage(mapEditorScreen, workingTilemap->tilesSurface, EDITOR_LEFT_MARGIN+5, 5);
	workingTilemap->object.onPress = changeTile;
	workingTilemap->object.onRelease = endChangeTile;
	workingTilemap->object.onReleaseOutside = endChangeTile;
	mapEditorScreen->visible =1 ;
	float tmp = ((float)(1024-EDITOR_LEFT_MARGIN-30))/((float)(workingTilemap->mapWidth*workingTilemap->tileSize));
	mapScrollerH->percentage = tmp*100.0f;
	mapScrollerH->onChange = scrollMap;
	
	tmp = ((float)(768-EDITOR_TOP_MARGIN-30))/((float)(workingTilemap->mapHeight*workingTilemap->tileSize));
	mapScrollerV->percentage = tmp*100.0f;
	mapScrollerV->onChange = scrollMap;
	
	CreateTileSelector();
}

void updateMapEditor() {
	if(mapEditorScreen->visible == 0)
		return;
	
	if(workingTilemap == NULL)
		return;
	
	entityOverlayScreen->visible = showEntities;
	
	int i,j;
	RGS_Rect rect;
	rect.w = rect.h = workingTilemap->tileSize;
	
	int startX = workingTilemap->cameraX/workingTilemap->tileSize;
	if(startX > 0)
		startX += 1;
	int offsetX = workingTilemap->cameraX - (startX*workingTilemap->tileSize);
	
	int	startY = workingTilemap->cameraY/workingTilemap->tileSize;
	int offsetY = workingTilemap->cameraY - (startY*workingTilemap->tileSize);
	
	for(i=startX; i < (startX + workingTilemap->renderWidth/workingTilemap->tileSize); i++) {
		for(j=startY; j < (startY + workingTilemap->renderHeight/workingTilemap->tileSize); j++) {
			rect.x = workingTilemap->object.x + (i-startX) * workingTilemap->tileSize - offsetX;
			rect.y = workingTilemap->object.y + (j-startY) * workingTilemap->tileSize - offsetY;
			if(RGS_GetMouseX() > rect.x && RGS_GetMouseX() < rect.x + rect.w && RGS_GetMouseY() > rect.y && RGS_GetMouseY() < rect.y + rect.h) {
				if(i <= workingTilemap->mapWidth && j < workingTilemap->mapHeight) {
					if(paintMode == PAINT_MODE_GFX && gfxPaintMode == GFX_PAINT_BRUSH && brushTilemap != NULL) {
						brushTilemap->object.x = rect.x;
						brushTilemap->object.y = rect.y;
						brushTilemap->object.visible = 1;						
					}
					if(paintMode == PAINT_MODE_BRUSHDEFSTART) {
						tileSelector->object.x = rect.x;
						tileSelector->object.y = rect.y;
						tileSelector->object.w = workingTilemap->tileSize;
						tileSelector->object.h = workingTilemap->tileSize;
						tileSelector->object.visible = 1;
					} else if(paintMode == PAINT_MODE_BRUSHDEFEND) {
						tileSelector->object.w = RGS_GetMouseX() - tileSelector->object.x;
						tileSelector->object.w = tileSelector->object.w - (tileSelector->object.w % workingTilemap->tileSize) + workingTilemap->tileSize;
						tileSelector->object.h = RGS_GetMouseY() - tileSelector->object.y;				
						tileSelector->object.h = tileSelector->object.h - (tileSelector->object.h % workingTilemap->tileSize) + workingTilemap->tileSize;
						tileSelector->object.visible = 1;
					} else {
						tileSelector->object.visible = 0;
					}	
					curTileIndex = i+j*workingTilemap->mapWidth;
					if(painting == 1 && paintMode != PAINT_MODE_ENTITY)
						paintTile(curTileIndex);
				} else {
					if(brushTilemap != NULL)
						brushTilemap->object.visible = 0;
				}
			}
			
			if(selectedEntity != NULL && paintMode == PAINT_MODE_ENTITY && entityMode == ENTITY_MODE_SELECT) {

						if(painting == 1) {
							selectedEntity->x = RGS_GetMouseX()-workingTilemap->object.x+workingTilemap->cameraX-eDragOffsetX - (( RGS_GetMouseX()-workingTilemap->object.x+workingTilemap->cameraX-eDragOffsetX) % workingTilemap->tileSize);
							selectedEntity->y = RGS_GetMouseY()-workingTilemap->object.y+workingTilemap->cameraY-eDragOffsetY - ((RGS_GetMouseY()-workingTilemap->object.y+workingTilemap->cameraY-eDragOffsetY) % workingTilemap->tileSize);
							selectedEntity->object->onRelease = endChangeTile;
						}
						
						tileSelector->object.x = selectedEntity->object->x;
						tileSelector->object.y = selectedEntity->object->y;
						tileSelector->object.w = selectedEntity->object->w;
						tileSelector->object.h = selectedEntity->object->h;
						tileSelector->object.visible = 1;		
						

			}
			
			if(i <= workingTilemap->mapWidth && j <= workingTilemap->mapHeight && showGeomTiles == 1) {
				if(workingTilemap->mapAttributeData[i+j*workingTilemap->mapWidth] == TILE_SOLID)
					RGS_FillRect(&rect, 0x00ff00);
			}
			
			if(i <= workingTilemap->mapWidth && j <= workingTilemap->mapHeight && showAttTiles == 1) {
				if(workingTilemap->userAttributeData[i+j*workingTilemap->mapWidth] != 0)
					RGS_FillRect(&rect, attColors[workingTilemap->userAttributeData[i+j*workingTilemap->mapWidth]]);
			}
			
		}
	}
}

void MapEditor_Hide() {
	mapEditorScreen->visible = 0;
	gfxScreen->visible = 0;
	geomScreen->visible = 0;
	attScreen->visible = 0;
	entityOverlayScreen->visible = 0;
	entityScreen->visible = 0;
}

void MapEditor_Show() {
	mapEditorScreen->visible = 1;
	choosePaintMode(PAINT_MODE_GFX, NULL);
}


void MapEditor_ImportTilemap(RGS_Surface *srcSurface, int tileSize) {
	if(curTilemapTilePath == NULL) {
		RGS_SetLabelText(errorMessageLabel, "ERROR: You need to choose a tileset first!");
		return;
	}	
	
	if(workingTilemap != NULL) {
		RGS_DestroyObject((RGS_Object*)workingTilemap);
	}
	
	RGS_ScreenDestroyObjects(entityOverlayScreen);	
	workingTilemap = RGS_CreateEmptyTilemap(mapEditorScreen, 1024-EDITOR_LEFT_MARGIN-30, 768-EDITOR_TOP_MARGIN-30, srcSurface->surface->w/tileSize, srcSurface->surface->h/tileSize,  tileSize, curTilemapTilePath);
	setupNewTilemap();
	
	int i,j,l,m,n;
	int bufferSize = tileSize*tileSize;
	Uint8 *tempBuffer = (Uint8*)malloc(bufferSize);
	Uint8 *tempTileBuffer = (Uint8*)malloc(bufferSize);
	int missingByte = 0;
	
	Uint8 *pixPtr = (Uint8*)srcSurface->surface->pixels;
	
	Uint8 *tilePixPtr = (Uint8*)workingTilemap->tilesSurface->surface->pixels;
	Uint16 newTileMatch;
	
	RGS_Color c1,c2;
	
	for(i=0;i<workingTilemap->mapHeight;i++) {
		for(j=0;j<workingTilemap->mapWidth;j++) {
			for(l=0;l<tileSize;l++) {
				memcpy(tempBuffer+(tileSize*l), pixPtr+((((i*workingTilemap->mapWidth*tileSize)*tileSize)+(j*tileSize))+(srcSurface->surface->w*l)), tileSize);
			}
			newTileMatch = 0;
			for(m=0;m<workingTilemap->tilesSurface->surface->h/tileSize;m++) {
				for(n=0;n<workingTilemap->tilesSurface->surface->w/tileSize;n++) {
					for(l=0;l<tileSize;l++) {
						memcpy(tempTileBuffer+(tileSize*l), tilePixPtr+((((m*(workingTilemap->tilesSurface->surface->w/tileSize)*tileSize)*tileSize)+(n*tileSize))+(workingTilemap->tilesSurface->surface->w*l)), tileSize);
					}
					missingByte = 0;
					for(l=0; l < bufferSize;l++) {
						c1 = RGS_GetSurfaceColorByIndex(srcSurface, tempBuffer[l]);
						c2 =  RGS_GetSurfaceColorByIndex(workingTilemap->tilesSurface, tempTileBuffer[l]);
						if(c1.r != c2.r || c1.g != c2.g || c1.b != c2.b) {
							missingByte = 1;
						}
					}
					if(missingByte == 0)
						newTileMatch = m*(workingTilemap->tilesSurface->surface->w/tileSize)+n;
				}	
			}
			
			workingTilemap->mapTileData[(i*workingTilemap->mapWidth)+j] = newTileMatch;
		}
	}
	MapEditor_Show();
}

void MapEditor_CreateTiles(RGS_Surface *srcSurface, char *destFile, int tileSize) {
	int i,j,k,l;
	int tileWidth = srcSurface->surface->w / tileSize;
	int tileHeight = srcSurface->surface->h / tileSize;
	int bufferSize = tileSize*tileSize;
	int hasTile = 0;
	int missingByte = 0;
	
	Uint8 *pixPtr = (Uint8*)srcSurface->surface->pixels;
	
	Uint8 *tempBuffer = (Uint8*)malloc(bufferSize);
	int numTiles=0;
	Uint8 **tileBuffer = NULL;
	
	for(i=0; i < tileHeight; i++) {
		for(j=0; j < tileWidth; j++) {
			// copy tile to buffer
			for(l=0;l<tileSize;l++) {
				memcpy(tempBuffer+(tileSize*l), pixPtr+((((i*tileWidth*tileSize)*tileSize)+(j*tileSize))+(srcSurface->surface->w*l)), tileSize);
			}
			
			hasTile = 0;
			for(k=0;k<numTiles;k++) {
				missingByte = 0;
				for(l=0; l < bufferSize;l++) {
					if(tileBuffer[k][l] != tempBuffer[l]) {
						missingByte = 1;
					}
				}
				if(missingByte == 0) {
					hasTile = 1;
					break;
				}
			}
			
			if(hasTile == 0) {
				tileBuffer = (Uint8**)realloc(tileBuffer, (sizeof(Uint8*)*(numTiles+1)));
				tileBuffer[numTiles] = NULL;
				tileBuffer[numTiles] = (Uint8*)malloc(bufferSize);
				memcpy(tileBuffer[numTiles], tempBuffer, bufferSize);
				numTiles++;
			}
		}
	}
	
	int optimalSize = ceil(pow(numTiles, 0.5));
	RGS_Surface *newSurface = RGS_CreateSurfaceFromSurface(srcSurface, optimalSize*tileSize, optimalSize*tileSize);			
	pixPtr = (Uint8*)newSurface->surface->pixels;
	
	int yOffset = 0;
	for(i=0;i<numTiles;i++) {
		if(i%optimalSize == 0 && i != 0)
			yOffset+=tileSize;
		for(j=0;j<tileSize;j++) {
			memcpy(pixPtr+((yOffset*optimalSize*tileSize)+(j*optimalSize*tileSize)+((i*tileSize) % (optimalSize*tileSize))), tileBuffer[i]+(j*tileSize), tileSize);
		}
	}
	
	RGS_WriteSurface(newSurface, destFile, 0);
	
	if(numTiles > 0) {
		free(tileBuffer);
	}
	free(tempBuffer);
}

