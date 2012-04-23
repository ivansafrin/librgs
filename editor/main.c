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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef _WINDOWS
	#include <windows.h>
#else
	#include <dirent.h> 
#endif

#include "SDL.h"
#include "rgs.h"

#include "globals.h"
#include "sprite_editor.h"
#include "map_editor.h"

#define	EDMODE_NONE		0
#define	EDMODE_MAP		1
#define	EDMODE_SPRITE	2
#define	EDMODE_SURFACE	3

RGS_Screen *mainScreen;

RGS_Screen *bmpScreen;
RGS_Surface *bmpSurface;

RGS_Screen *browserScreen;
RGS_UIOptionList *browserList;
RGS_SurfaceImage *bmpImage;
RGS_UIButton *bmpImportSprite;
RGS_UIButton *bmpImportSurface;

RGS_UITextInput *bmpImportSpriteWidth;
RGS_UITextInput *bmpImportSpriteHeight;

RGS_UITextInput *tileImportTileSize;
RGS_UIButton *tileImportTile;

RGS_UITextInput *mapImportTileSize;
RGS_UIButton *mapImportTiles;

RGS_UITextInput *fileInput;
RGS_UIButton *fileOpen;
RGS_UIButton *fileSave;
RGS_UIButton *fileSetTiles;
RGS_UIButton *newTilemapBtn;

RGS_Label *errorMessageLabel;
RGS_Label *tilesetLabel;

int exitVar = 0;
char curdir[4096];

int currentMode = EDMODE_NONE;

extern RGS_Screen *spriteEditorScreen;
extern RGS_Screen *mapEditorScreen;

#ifdef _WINDOWS
void wtoc(char* Dest, const WCHAR* Source)
{
	int i = 0;
	while(Source[i] != '\0') {
		Dest[i] = (char)Source[i];
		++i;
	}
}
void ctow(WCHAR* Dest, const char* Source)
{
	int i = 0;
	while(Source[i] != '\0') {
		Dest[i] = (WCHAR)Source[i];
		++i;
	}
}


#endif

void reloadFileBrowser(char *dirname) {
#ifdef _WINDOWS

 	strcpy(curdir, dirname);
	RGS_ClearUIOptionList(browserList);

	WIN32_FIND_DATA findFileData;

	WCHAR tmp[4096];
	memset(tmp, 0, sizeof(WCHAR)*4096);
	ctow(tmp, curdir);
	SetCurrentDirectory(tmp);

	HANDLE hFind = FindFirstFile((LPCWSTR)"*", &findFileData);
	if(hFind  == INVALID_HANDLE_VALUE) {
		return;
	}

	int fileNumber = 0;
	char fileName[260];
	char xt[4];

	do {
		memset(fileName, 0, 260);
		wtoc(fileName, findFileData.cFileName);
		if( findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
			RGS_AddUIOption(browserList, fileName, fileNumber, NULL, 0xffffff);
		} else {
			strcpy(xt, fileName+strlen(fileName)-3);
			if(strcmp(xt, "bmp") == 0 || strcmp(xt, "spr") == 0 || strcmp(xt, "sur") == 0 || strcmp(xt, "map") == 0 || strcmp(xt, "bru") == 0 || strcmp(xt, "att") == 0 || strcmp(xt, "ent") == 0)
				RGS_AddUIOption(browserList, fileName, fileNumber, NULL, 0x79CA00);
		}
		
		fileNumber++;
    } while(FindNextFile(hFind, &findFileData));

    FindClose(hFind);
#else
	DIR           *d;
	struct dirent *dir;
	
	d = opendir(dirname);
	int i = 0;
	if(d) {
		strcpy(curdir, dirname);
		RGS_ClearUIOptionList(browserList);
		
		while ((dir = readdir(d)) != NULL) {
//			printf("%s\n", dir->d_name);
			if(dir->d_type == DT_DIR) {
				RGS_AddUIOption(browserList, dir->d_name, i, NULL, 0xffffff);
			} else {
				char xt[4];
				strcpy(xt, dir->d_name+strlen(dir->d_name)-3);
				if(strcmp(xt, "bmp") == 0 || strcmp(xt, "spr") == 0 || strcmp(xt, "sur") == 0 || strcmp(xt, "map") == 0 || strcmp(xt, "bru") == 0 || strcmp(xt, "att") == 0 || strcmp(xt, "ent") == 0)
					RGS_AddUIOption(browserList, dir->d_name, i, NULL, 0x79CA00);
			}
			i++;
		}
		closedir(d);
	}
#endif
}

void Editor_HideAll() {
	MapEditor_Hide();
	spriteEditorScreen->visible = 0;
	bmpScreen->visible = 0;
}

void Editor_ImportBMPSurface(void *target) {
	char tmp[2048];
	strcpy(tmp, curdir);
	strcat(tmp, "/");
	strcat(tmp, fileInput->label->text);

	RGS_WriteSurface(bmpSurface, tmp, 0);
	reloadFileBrowser(curdir);
}

void Editor_ImportBMPSprite(void *target) {
	Sprite_CreateFromBMP(atoi(bmpImportSpriteWidth->label->text), atoi(bmpImportSpriteHeight->label->text));
	currentMode = EDMODE_SPRITE;
}

void Editor_ImportTilemap(void *target) {
	Editor_HideAll();
	MapEditor_ImportTilemap(bmpSurface, atoi(tileImportTileSize->label->text));
	currentMode = EDMODE_MAP;
}

void Editor_CreateTiles(void *target) {
	char tmp[2048];
	strcpy(tmp, curdir);
	strcat(tmp, "/");
	strcat(tmp, fileInput->label->text);

	MapEditor_CreateTiles(bmpSurface, tmp, atoi(tileImportTileSize->label->text));
	reloadFileBrowser(curdir);
}

void initEditor() {
	RGS_Init(1024,768,0,0, RGS_NORMAL);
	RGS_SetClearColor(30,30,30);
	
	mainScreen = RGS_CreateScreen();
	RGS_CreateShape(mainScreen, RGS_SHAPE_RECT, 0x000000, 0,0,1024, 14);
	RGS_CreateShape(mainScreen, RGS_SHAPE_RECT, 0x121212, EDITOR_LEFT_MARGIN,14,1024-EDITOR_LEFT_MARGIN, EDITOR_TOP_MARGIN-14);
	RGS_CreateShape(mainScreen, RGS_SHAPE_RECT, 0x141414, 0,14,EDITOR_LEFT_MARGIN, 768-14);
	RGS_CreateLabel(mainScreen, "RETRO GAME SYSTEM EDITOR", RGS_FONT_BIG, 3,2,0x4c4c4c);
	
	errorMessageLabel = RGS_CreateLabel(mainScreen, "", RGS_FONT_BIG, 200, 2, 0xff0000);
	tilesetLabel = RGS_CreateLabel(mainScreen, "Current Tileset:   None", RGS_FONT_SMALL, 600, 2, 0x00ff00);
	
	bmpScreen = RGS_CreateScreen();
	bmpScreen->visible = 0;

	RGS_CreateLabel(bmpScreen, "IMPORT TO SPRITE", RGS_FONT_BIG, EDITOR_LEFT_MARGIN+5,EDITOR_TOP_MARGIN-100,0x79CA00);
	RGS_CreateLabel(bmpScreen, "Frame width:", RGS_FONT_SMALL, EDITOR_LEFT_MARGIN+5,EDITOR_TOP_MARGIN-72,0x4c4c4c);
	RGS_CreateLabel(bmpScreen, "Frame height:", RGS_FONT_SMALL, EDITOR_LEFT_MARGIN+5,EDITOR_TOP_MARGIN-47,0x4c4c4c);
	
	bmpImportSpriteWidth = RGS_CreateUITextInput(bmpScreen, EDITOR_LEFT_MARGIN+85, EDITOR_TOP_MARGIN-75, 40, 20); 
	RGS_SetLabelText(bmpImportSpriteWidth->label, "32");
	bmpImportSpriteHeight = RGS_CreateUITextInput(bmpScreen, EDITOR_LEFT_MARGIN+85, EDITOR_TOP_MARGIN-50, 40, 20); 
	RGS_SetLabelText(bmpImportSpriteHeight->label, "32");

	bmpImportSprite = RGS_CreateUIButton(bmpScreen, "Import", EDITOR_LEFT_MARGIN+5, EDITOR_TOP_MARGIN-25, 120, 18);
	bmpImportSprite->uiObject.onRelease = Editor_ImportBMPSprite;

	bmpImportSurface = RGS_CreateUIButton(bmpScreen, "Save as surface", EDITOR_LEFT_MARGIN+140, EDITOR_TOP_MARGIN-25, 120, 18);
	bmpImportSurface->uiObject.onRelease = Editor_ImportBMPSurface;

	RGS_CreateLabel(bmpScreen, "CONVERT TO TILES", RGS_FONT_BIG, EDITOR_LEFT_MARGIN+285,EDITOR_TOP_MARGIN-72,0xDF75FF);
	RGS_CreateLabel(bmpScreen, "Tile size:", RGS_FONT_SMALL, EDITOR_LEFT_MARGIN+285,EDITOR_TOP_MARGIN-47,0x4c4c4c);
	tileImportTileSize = RGS_CreateUITextInput(bmpScreen, EDITOR_LEFT_MARGIN+340, EDITOR_TOP_MARGIN-50, 40, 20); 
	RGS_SetLabelText(tileImportTileSize->label, "8");
	tileImportTile = RGS_CreateUIButton(bmpScreen, "Create Tiles", EDITOR_LEFT_MARGIN+285, EDITOR_TOP_MARGIN-25, 120, 18);
	tileImportTile->uiObject.onRelease = Editor_CreateTiles;

	RGS_CreateLabel(bmpScreen, "IMPORT AS TILEMAP", RGS_FONT_BIG, EDITOR_LEFT_MARGIN+415,EDITOR_TOP_MARGIN-72,0x66FAA6);
	RGS_CreateLabel(bmpScreen, "Tile size:", RGS_FONT_SMALL, EDITOR_LEFT_MARGIN+415,EDITOR_TOP_MARGIN-47,0x4c4c4c);
	mapImportTileSize = RGS_CreateUITextInput(bmpScreen, EDITOR_LEFT_MARGIN+470, EDITOR_TOP_MARGIN-50, 40, 20); 
	RGS_SetLabelText(mapImportTileSize->label, "8");
	mapImportTiles = RGS_CreateUIButton(bmpScreen, "Import tilemap", EDITOR_LEFT_MARGIN+415, EDITOR_TOP_MARGIN-25, 120, 18);
	mapImportTiles->uiObject.onRelease = Editor_ImportTilemap;

	
}

void Editor_Exit(void *target) {
	exitVar = 1;
}

void loadBMPFile(char *filename) {
	Editor_HideAll();
	bmpSurface = RGS_LoadBMPSurface(filename);
	bmpImage = RGS_CreateSurfaceImage(bmpScreen, bmpSurface, EDITOR_LEFT_MARGIN, EDITOR_TOP_MARGIN+5+50);
	bmpScreen->visible = 1;
	currentMode = EDMODE_SURFACE;
}

void loadSURFile(char *filename) {
	Editor_HideAll();
	bmpSurface = RGS_LoadSurface(filename);
	bmpImage = RGS_CreateSurfaceImage(bmpScreen, bmpSurface, EDITOR_LEFT_MARGIN, EDITOR_TOP_MARGIN+5+50);
	bmpScreen->visible = 1;
	currentMode = EDMODE_SURFACE;
}

void selectFile(char *filename) {
	RGS_SetLabelText(fileInput->label, filename);
}

void openFile(void *target) {
	char tmp[1024];
	strcpy(tmp, curdir);
	strcat(tmp, "/");
	strcat(tmp, fileInput->label->text);
	char xt[4];
	strcpy(xt, fileInput->label->text+strlen(fileInput->label->text)-3);
	
	if(strcmp(xt, "bmp") == 0) {
		Editor_HideAll();
		loadBMPFile(tmp);
		currentMode = EDMODE_SURFACE;		
	}
	else if(strcmp(xt, "sur") == 0) {
		Editor_HideAll();
		loadSURFile(tmp);
		currentMode = EDMODE_SURFACE;
	}
	else if(strcmp(xt, "map") == 0) {
		Editor_HideAll();
		openMap(tmp);
		currentMode = EDMODE_MAP;
	}
	else if(strcmp(xt, "spr") == 0) {
		Editor_HideAll();
		openSprite(tmp);
		currentMode = EDMODE_SPRITE;
	}
	else if(strcmp(xt, "bru") == 0) {
		MapEditor_LoadBrushes(tmp);
	}
	else if(strcmp(xt, "att") == 0) {
		MapEditor_LoadAttributes(tmp);
	}
	else if(strcmp(xt, "ent") == 0) {
		MapEditor_LoadEntities(tmp);
	}
}

void saveFile(void *target) {
	char tmp[1024];
	strcpy(tmp, curdir);
	strcat(tmp, "/");
	strcat(tmp, fileInput->label->text);
	
	switch(currentMode) {
		case EDMODE_MAP:
			MapEditor_SaveTilemap(tmp);
		break;
		case EDMODE_SPRITE:
			Editor_SpriteSave(tmp);	
		break;
	}
	
	reloadFileBrowser(curdir);
}

void setTiles(void *target) {
	char tmp[1024];
	strcpy(tmp, curdir);
	strcat(tmp, "/");
	strcat(tmp, fileInput->label->text);
	
	char tmp2[1024];
	sprintf(tmp2, "Current Tileset:   %s", fileInput->label->text);
	RGS_SetLabelText(tilesetLabel, tmp2);
	MapEditor_SetTilesetPath(tmp);
}

void createNewTilemap(void *target) {
	MapEditor_CreateNewTilemap();	
	Editor_HideAll();
	//MapEditor_Show();
}

void chooseFile(int id, void *data) {
#ifdef _WINDOWS

	char tmp[4096];

	WIN32_FIND_DATA findFileData;

	HANDLE hFind = FindFirstFile((LPCWSTR)"*", &findFileData);
	if(hFind  == INVALID_HANDLE_VALUE) {
		return;
	}
	int fileNumber = 0;
	char fileName[260];
	char xt[4];

	do {
		memset(fileName, 0, 260);
		wtoc(fileName, findFileData.cFileName);
		if(fileNumber == id) {
			strcpy(tmp, curdir);
			strcat(tmp, "\\");
			strcat(tmp, fileName);
			if( findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
				reloadFileBrowser(tmp);
			} else {
				selectFile(fileName);
			}
			FindClose(hFind);
			return;
		}
		fileNumber++;
    } while(FindNextFile(hFind, &findFileData));

	FindClose(hFind);
#else
	DIR *d;
	struct dirent *dir;
	char tmp[1024];
	
	d = opendir(curdir);
	int i = 0;
	if(d) {
		while ((dir = readdir(d)) != NULL) {
			if(i == id) {
				strcpy(tmp, curdir);
				strcat(tmp, "/");
				strcat(tmp, dir->d_name);
				if(dir->d_type == DT_DIR) {
					reloadFileBrowser(tmp);
				} else {
					selectFile(dir->d_name);
				}
				closedir(d);
				return;
			}
			i++;
		}
		closedir(d);
	}
#endif
}

void initFileBrowser() {
	browserScreen = RGS_CreateScreen();
	browserList = RGS_CreateUIOptionList(browserScreen, 15, 5, EDITOR_TOP_MARGIN+25, EDITOR_LEFT_MARGIN-10, 500);
	browserList->onSelect = chooseFile;
	fileInput = RGS_CreateUITextInput(mainScreen, 5, EDITOR_TOP_MARGIN, EDITOR_LEFT_MARGIN-10, 20);
	fileOpen = RGS_CreateUIButton(mainScreen, "Load File", 5, EDITOR_TOP_MARGIN-25, EDITOR_LEFT_MARGIN-10, 20);
	fileOpen->uiObject.onRelease = openFile;
	fileSave = RGS_CreateUIButton(mainScreen, "Save File", 5, EDITOR_TOP_MARGIN-50, EDITOR_LEFT_MARGIN-10, 20);
	fileSave->uiObject.onRelease = saveFile;
	fileSetTiles = RGS_CreateUIButton(mainScreen, "Set As Tileset", 5, EDITOR_TOP_MARGIN-75, EDITOR_LEFT_MARGIN-10, 20);
	fileSetTiles->uiObject.onRelease = setTiles;
	newTilemapBtn = RGS_CreateUIButton(mainScreen, "New Tilemap", 5, EDITOR_TOP_MARGIN-100, EDITOR_LEFT_MARGIN-10, 20);
	newTilemapBtn->uiObject.onRelease = createNewTilemap;
		
#ifdef _WINDOWS
	WCHAR tmp[4096];
	GetCurrentDirectory(4096, (LPWSTR)tmp);
	wtoc(curdir, tmp);
	reloadFileBrowser(curdir);
#else
	reloadFileBrowser(".");
#endif
}

void updateEditor() {
	Editor_UpdateSpriteEditor();
	updateMapEditor();
}

void onKeyPress(int keyCode) {
	MapEditor_KeyPress(keyCode);
}

int main(int argc, char *argv[]) {

	initEditor();
	initFileBrowser();
	initSpriteEditor();
	initMapEditor();
	RGS_SetCustomUpdateFunction(updateEditor);
	RGS_FocusScreen(mainScreen);
	RGS_SetKeyCallback(onKeyPress);
	while(RGS_Update() != 0 && exitVar == 0){}
	RGS_Shutdown();
	return 1;
}
