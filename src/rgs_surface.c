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

#include "rgs_surface.h"


RGS_Surface *RGS_SurfaceFromMemory(char *imageData) {
	
	int hasColorKey = 0;

	RGS_Surface *newSurface;
	newSurface = (RGS_Surface*) malloc(sizeof(RGS_Surface));
	memset(newSurface, 0, sizeof(RGS_Surface));
	
	short width, height, numColors;
	int offset = 0;
	memcpy(&width, imageData,2);
	offset += 2;
	memcpy(&height, imageData+offset,2);
	newSurface->surface = SDL_CreateRGBSurface(SDL_SWSURFACE|SDL_ANYFORMAT,width, height,8,RMASK,GMASK,BMASK,AMASK);

	offset += 2;
	memcpy(&numColors, imageData+offset,2);
	offset += 2;

	SDL_Palette *palTest = (SDL_Palette*)malloc(sizeof(SDL_Palette));
	palTest->ncolors = numColors;
	palTest->colors = (SDL_Color*)malloc(sizeof(SDL_Color)*numColors);
	SDL_Color *palPtr = palTest->colors;
	SDL_Color tmpColor;
	char cbuf;

	newSurface->palette.numcolors = numColors;
	newSurface->palette.colors = (RGS_Color*)malloc(sizeof(RGS_Color)*numColors);
	RGS_Color *rgsColor = newSurface->palette.colors;
	
	int i;
	for(i=0; i<numColors;i++) {
		memcpy(&cbuf, imageData+offset,1);
		tmpColor.r = (Uint8)cbuf;
		rgsColor->r = (Uint8)cbuf;
		offset++;
		memcpy(&cbuf, imageData+offset,1);
		tmpColor.g = (Uint8)cbuf;
		rgsColor->g = (Uint8)cbuf;
		offset++;
		memcpy(&cbuf, imageData+offset,1);
		tmpColor.b = (Uint8)cbuf;
		rgsColor->b = (Uint8)cbuf;
		tmpColor.unused = (Uint8)cbuf;
		offset++;
		
		if(tmpColor.r == 255 && tmpColor.g == 0 && tmpColor.b == 255)
			hasColorKey = 1;
		
		memcpy(palPtr, &tmpColor, sizeof(SDL_Color));
		palPtr++;
		rgsColor++;
	}

	char value;
	memcpy(&value, imageData+offset, 1);

	SDL_LockSurface(newSurface->surface);
	SDL_SetPalette(newSurface->surface,SDL_LOGPAL|SDL_PHYSPAL,palTest->colors,0,numColors);
	newSurface->surface->format->palette = palTest;
	SDL_UnlockSurface(newSurface->surface);

	char *surfacePtr = (char*)newSurface->surface->pixels;

	for(i=0; i<width*height;i++) {
		memcpy(surfacePtr+i, imageData+offset, 1);
		offset++;
	}
	
	if(hasColorKey)
		SDL_SetColorKey(newSurface->surface,SDL_SRCCOLORKEY,SDL_MapRGB(newSurface->surface->format,255,0,255));

	return newSurface;
}

void RGS_CopySurface(RGS_Surface *srcSurface, RGS_Rect *srcRect, RGS_Surface *destSurface, RGS_Rect *destRect) {
	Uint8 *pSrc = (Uint8*)srcSurface->surface->pixels;
	pSrc += srcRect->y*srcSurface->surface->pitch+srcRect->x;
	Uint8 *pDst = (Uint8*)destSurface->surface->pixels;
	pDst += destRect->y*destSurface->surface->pitch+destRect->x;
	int i;
	for(i = 0; i < srcRect->h; i++)
	{
		memcpy(pDst,pSrc,srcRect->w);
		pSrc += srcSurface->surface->pitch;
		pDst += destSurface->surface->pitch;
	}
}

RGS_Surface *RGS_LoadBMPSurface(char *filename) {
	RGS_Surface *newSurface = NULL;
	char *data = (char*)RGS_OpenDataPointer(filename);
	if(data != NULL) {
		newSurface = RGS_SurfaceFromBMPMemory(data);
		free(data);
	}
	return newSurface;
}

void RGS_FreeSurface(RGS_Surface *surface) {
	SDL_FreeSurface(surface->surface);
}

void RGS_CopySurfacePalette(RGS_Surface *destSurface, RGS_Surface *srcSurface) {
	SDL_Palette *palTest = (SDL_Palette*)malloc(sizeof(SDL_Palette));	
	palTest->ncolors = srcSurface->surface->format->palette->ncolors;
	palTest->colors = (SDL_Color*)malloc(sizeof(SDL_Color)*srcSurface->surface->format->palette->ncolors);
	memcpy(palTest->colors, srcSurface->surface->format->palette->colors, sizeof(SDL_Color)*palTest->ncolors);
	
	SDL_LockSurface(destSurface->surface);
	SDL_SetPalette(destSurface->surface,SDL_LOGPAL|SDL_PHYSPAL,palTest->colors,0,palTest->ncolors);
	SDL_UnlockSurface(destSurface->surface);
}

RGS_Surface *RGS_CreateSurfaceFromSurface(RGS_Surface *srcSurface, Uint16 width, Uint16 height) {
	RGS_Surface *newSurface = (RGS_Surface*)malloc(sizeof(RGS_Surface));
	newSurface->curPaletteIndex = srcSurface->curPaletteIndex;
	newSurface->loaded = srcSurface->loaded;
	newSurface->numPals = srcSurface->numPals;
	
	newSurface->surface = SDL_CreateRGBSurface(SDL_SWSURFACE|SDL_ANYFORMAT,width, height,8,RMASK,GMASK,BMASK,AMASK);
	
	RGS_CopySurfacePalette(newSurface, srcSurface);
	
	memset(newSurface->surface->pixels, 0, width*height);
	return newSurface;
}

RGS_Surface *RGS_LoadSurface(char *filename) {
	RGS_Surface *newSurface = NULL;
	char *data = (char*)RGS_OpenDataPointer(filename);
	if(data != NULL) {
		newSurface = RGS_SurfaceFromMemory(data);
		free(data);
	} else {
		printf("Error opening data source!\n");	
	}
	return newSurface;
}

RGS_Color RGS_GetSurfaceColorByIndex(RGS_Surface *surface, Uint8 index) {
	if(index < surface->palette.numcolors)
		return surface->palette.colors[index];
	else
		return surface->palette.colors[surface->palette.numcolors-1];
}

RGS_Surface *RGS_SurfaceFromBMPMemory(char *imageData) {

	int hasColorKey=0;

	RGS_Surface *newSurface = (RGS_Surface*)malloc(sizeof(RGS_Surface));
	memset(newSurface, 0, sizeof(RGS_Surface));
	
	int width, height, numColors;
	int offset = 0;
	offset += 18;
	memcpy(&width, imageData+offset,4);
	offset += 4;
	memcpy(&height, imageData+offset,4);
	if(height < 0)
		height *= -1;
	
	newSurface->surface = SDL_CreateRGBSurface(SDL_SWSURFACE|SDL_ANYFORMAT,width, height,8,RMASK,GMASK,BMASK,AMASK);

	offset += 24;
	memcpy(&numColors, imageData+offset,4);
	offset += 8;
	
	printf("%d !\n", numColors);

	SDL_Palette *palTest = (SDL_Palette*)malloc(sizeof(SDL_Palette));
	palTest->ncolors = numColors;
	palTest->colors = (SDL_Color*)malloc(sizeof(SDL_Color)*numColors);
	SDL_Color *palPtr = palTest->colors;
	SDL_Color tmpColor;
	
	char cbuf;
	
	newSurface->palette.numcolors = numColors;
	newSurface->palette.colors = (RGS_Color*)malloc(sizeof(RGS_Color)*numColors);
	RGS_Color *rgsColor = newSurface->palette.colors;

	int i;
	for(i=0; i<numColors;i++) {
		memcpy(&cbuf, imageData+offset,1);
		tmpColor.b = (Uint8)(cbuf);
		rgsColor->b = (Uint8)(cbuf);
		tmpColor.unused = cbuf;
		offset++;
		memcpy(&cbuf, imageData+offset,1);
		tmpColor.g = (Uint8)(cbuf);
		rgsColor->g = (Uint8)(cbuf);
		offset++;
		memcpy(&cbuf, imageData+offset,1);
		tmpColor.r = (Uint8)(cbuf);
		rgsColor->r = (Uint8)(cbuf);		
		offset+=2;
		memcpy(palPtr, &tmpColor, sizeof(SDL_Color));
		if(tmpColor.r == 255 && tmpColor.g == 0 && tmpColor.b == 255)
			hasColorKey = 1;
		palPtr++;
		rgsColor++;
	}

	char value;
	memcpy(&value, imageData+offset, 1);

	SDL_LockSurface(newSurface->surface);
	SDL_SetPalette(newSurface->surface,SDL_LOGPAL|SDL_PHYSPAL,palTest->colors,0,numColors);
	newSurface->surface->format->palette = palTest;
	SDL_UnlockSurface(newSurface->surface);

	char *surfacePtr = (char*)newSurface->surface->pixels;

	for(i=0; i<width*height;i++) {
		memcpy(surfacePtr+i, imageData+offset, 1);
		offset++;
	}
	
	if(hasColorKey)
		SDL_SetColorKey(newSurface->surface,SDL_SRCCOLORKEY,SDL_MapRGB(newSurface->surface->format,255,0,255));

	return newSurface;
}

void RGS_MirrorSurfaceX(RGS_Surface *surface) {
	int i,j;
	
	char *pSrc = (char*)surface->surface->pixels;
	char *pTmp = (char*) malloc(surface->surface->pitch*surface->surface->h);
	char *pTmpStart = pTmp;
	char *pSrcStart = pSrc;

	memcpy(pTmp,pSrc,surface->surface->pitch*surface->surface->h);

	pTmp = pTmpStart;
	pSrc = pSrcStart;

	for(i = 0; i < surface->surface->h; i++)
	{
		for(j = 0; j < surface->surface->pitch; j++)
		{
			memcpy(pSrc+j, pTmp+surface->surface->pitch-j-1, 1);
		}
		
		pSrc += surface->surface->pitch;
		pTmp += surface->surface->pitch;
	}
	free(pTmpStart);

}

void RGS_MirrorSurfaceY(RGS_Surface *surface) {

	int i;
	char *pSrc = (char*)surface->surface->pixels;
	char *pTmp = (char*) malloc((surface->surface->w*sizeof(char))*surface->surface->h);
	char *pTmpStart = pTmp;
	char *pSrcStart = pSrc;

	for(i = 0; i < surface->surface->h; i++)
	{
		memcpy(pTmp,pSrc,surface->surface->w*sizeof(char));
		pSrc += surface->surface->w;
		pTmp += surface->surface->w;
	}
	
	pSrc = pSrcStart;
	pTmp -= surface->surface->w;
	
	for(i = 0; i < surface->surface->h; i++)
	{
		memcpy(pSrc,pTmp,surface->surface->w*sizeof(char));
		pSrc += surface->surface->w;
		pTmp -= surface->surface->w;
	}

	free(pTmpStart);
}

RGS_SurfaceImage *RGS_CreateSurfaceImageFromFile(RGS_Screen *screen, char *filename, short x, short y) {
	RGS_Surface *sur = RGS_LoadSurface(filename);
	return RGS_CreateSurfaceImage(screen, sur, x, y);
}

RGS_SurfaceImage *RGS_CreateSurfaceImage(RGS_Screen *screen, RGS_Surface *surface, short x, short y) {
	RGS_SurfaceImage *newImage;
	newImage = (RGS_SurfaceImage*)malloc(sizeof(RGS_SurfaceImage));
	memset(newImage, 0, sizeof(RGS_SurfaceImage));
	RGS_InitializeObject(&newImage->object, x, y);
	newImage->object.Render = RGS_RenderSurfaceImage;
	RGS_AddScreenObject(screen, (RGS_Object*)newImage);
	newImage->surface = surface;
	
	newImage->object.w = surface->surface->w;
	newImage->object.h = surface->surface->h;
		
	return newImage;

}

void RGS_LoadSurfacePalette(RGS_Surface *surface, char *palette) {		
	FILE *pFile;
	pFile = fopen(palette, "rb");
		
	Uint32 sig=0;
	Uint32 fileSize = 0;
	Uint32 riffType = 0;
		
	if(!pFile) {
		return;
	}
		
	fread(&sig, sizeof(Uint32), 1, pFile);
	fread(&fileSize, sizeof(Uint32), 1, pFile);
	fread(&riffType, sizeof(Uint32), 1, pFile);
		
	Uint32 chunkSig = 0;
	Uint32 chunkSize = 0;
	fread(&chunkSig, sizeof(Uint32), 1, pFile);
	fread(&chunkSize, sizeof(Uint32), 1, pFile);
		
	// read in number of colors
	Uint16 numColors = 0;
	Uint16 palVersion = 0;
		
	fread(&palVersion, sizeof(Uint16), 1, pFile);
	fread(&numColors, sizeof(Uint16), 1, pFile);
		
	SDL_Palette *palTest = malloc(sizeof(SDL_Palette));
		
	palTest->ncolors = numColors;
	palTest->colors = (SDL_Color*)malloc(sizeof(SDL_Color)*numColors);
	SDL_Color *palPtr = palTest->colors;
		
	SDL_Color tmpColor;
		
	int i;
	for(i=0; i<numColors;i++) {
		fread(&tmpColor.r, sizeof(Uint8), 1, pFile);
		fread(&tmpColor.g, sizeof(Uint8), 1, pFile);
		fread(&tmpColor.b, sizeof(Uint8), 1, pFile);
		fread(&tmpColor.unused, sizeof(Uint8), 1, pFile);
		memcpy(palPtr, &tmpColor, sizeof(SDL_Color));
		palPtr++;
	}
		
//	pals.push_back(palTest);
	fclose(pFile);
	SDL_SetPalette(surface->surface,SDL_LOGPAL|SDL_PHYSPAL,palTest->colors,0,palTest->ncolors);
	surface->surface->format->palette = palTest;
//	numPals++;
//	curPaletteIndex = numPals;
//	SwapPalette(numPals);
	
}

void RGS_RenderSurfaceImage(SDL_Surface *target, void *object) {
	RGS_SurfaceImage *image = (RGS_SurfaceImage*)object;
	if(image->surface == NULL)
		return;
		
	SDL_Rect srcRect, destRect;
	srcRect.x = 0;
	srcRect.y = 0;
	srcRect.w = image->surface->surface->w;
	srcRect.h = image->surface->surface->h;
	
	destRect.x = ((RGS_Object*)object)->x;
	destRect.y = ((RGS_Object*)object)->y;
	destRect.w = srcRect.w;
	destRect.h = srcRect.h;
	
	SDL_BlitSurface(image->surface->surface, &srcRect, target, &destRect);
}

void RGS_WriteSurface(RGS_Surface *surface, char *filename, int append) {
	short sval = 0;
	char bval = 0;
	FILE *outPtr;
	
	if(append == 1)
		outPtr = fopen(filename, "ab");
	else
		outPtr = fopen(filename, "wb");
		
	sval = (short) surface->surface->w;
	fwrite(&sval, 2, 1, outPtr);
	sval = (short) surface->surface->h;
	fwrite(&sval, 2, 1, outPtr);
	sval = surface->surface->format->palette->ncolors;
	fwrite(&sval, 2 , 1, outPtr);

	int i=0;
	for(i=0; i< surface->surface->format->palette->ncolors;i++){
		bval  = surface->surface->format->palette->colors[i].r;
		fwrite(&bval, 1, 1, outPtr);
		bval  = surface->surface->format->palette->colors[i].g;
		fwrite(&bval, 1, 1, outPtr);
		bval  = surface->surface->format->palette->colors[i].b;
		fwrite(&bval, 1, 1, outPtr);
	}

	fwrite(surface->surface->pixels, 1 , surface->surface->w*surface->surface->h, outPtr);
	fclose(outPtr);
}
