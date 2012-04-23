
/***********************************************************/
/*                                                         */
/*   BFONT.c v. 1.1.0 - Billi Font Library by Diego Billi  */
/*                                                         */
/*   mail: dbilli@cs.unibo.it                              */
/*   home: http://www.cs.unibo.it/~dbilli (ITALIAN)        */
/*                                                         */
/***********************************************************/

#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "stdarg.h"
#include "BFont.h"

#include "rgs.h"


/* ATTENTION: MS Visual C++ do not declarate vsnprintf in <stdio.h>  */
#ifdef WIN32
	#define vsnprintf _vsnprintf
#endif


/* The first character in BFont fonts is '!' */
#define BFONT_FIRST_FONT_CHAR  33

/* ASCII value for "space" */
#define SPACE  32


/* Current font */
static BFont_Info *CurrentFont;


/* buffer size  for buffered prints*/
#define BFONT_BUFFER_LEN 1024

/* Single global var for buffered prints */
static char bfont_buffer[BFONT_BUFFER_LEN];


/* utility functions */
static Uint32 BFontGetPixel(SDL_Surface *Surface, Sint32 X, Sint32 Y);
static void   PutPixel(SDL_Surface *surface, Sint32 X, Sint32 Y, Uint32 pixel);


/***************************** BFont Functions ********************************/

void InitFont(BFont_Info *Font)
{
    int x = 0, i = 0;
    Uint32 separator_color;

    i= BFONT_FIRST_FONT_CHAR;

    if (SDL_MUSTLOCK(Font->Surface)) SDL_LockSurface(Font->Surface);

    separator_color = BFontGetPixel(Font->Surface,0,0);
     
    x=0;
    while ( (x < (Font->Surface->w-1)) && (i < BFONT_NUM_CHARS) ) 
	{
	    if(BFontGetPixel(Font->Surface,x,0) != separator_color) {
            Font->Chars[i].x = x;
            Font->Chars[i].y = 1;
            Font->Chars[i].h = Font->Surface->h;
            for ( ; (BFontGetPixel(Font->Surface, x, 0) != separator_color) && (x < Font->Surface->w); ++x) 
				;
            Font->Chars[i].w = (x - Font->Chars[i].x);
            i++;
	    }
        else {
	        x++;
        }
    }

    Font->Chars[SPACE].x = 0;
    Font->Chars[SPACE].y = 0;
    Font->Chars[SPACE].h = Font->Surface->h;
    Font->Chars[SPACE].w = Font->Chars[BFONT_FIRST_FONT_CHAR].w;

    Font->h = Font->Surface->h;

    SDL_SetColorKey(Font->Surface, SDL_SRCCOLORKEY, BFontGetPixel(Font->Surface, 0, Font->Surface->h-1));

    if (SDL_MUSTLOCK(Font->Surface)) 
		SDL_UnlockSurface(Font->Surface);
}


BFont_Info * BFont_LoadFont (void *fontPtr){
    SDL_Surface *surface = NULL;
    int x;
	BFont_Info *Font = NULL;

	Font = (BFont_Info *) malloc(sizeof(BFont_Info));
	if (Font == NULL) 
		return NULL;
	
	RGS_Surface *surfacePlus = RGS_SurfaceFromMemory((char*)fontPtr);
	
	surface = surfacePlus->surface;	

	if (surface == NULL) {
		free(Font);
		return NULL;
	}

	Font->Surface = surface;
	for (x=0; x < BFONT_NUM_CHARS ; x++) {
		Font->Chars[x].x = 0;
		Font->Chars[x].y = 0;
		Font->Chars[x].h = 0;
		Font->Chars[x].w = 0;
	}

	InitFont(Font);
	BFont_SetCurrentFont(Font);

	return Font;
}


BFont_Info * BFont_LoadFontFromSurface (SDL_Surface *Surface)
{
    int i;
	BFont_Info *Font=NULL;

	Font = (BFont_Info *) malloc(sizeof(BFont_Info));

	if (Font == NULL) 
		return NULL;

	Font->Surface = Surface;
	for (i=0; i < BFONT_NUM_CHARS; i++) {
		Font->Chars[i].x = 0;
		Font->Chars[i].y = 0;
		Font->Chars[i].h = 0;
		Font->Chars[i].w = 0;
	}

	InitFont(Font);
	BFont_SetCurrentFont(Font);

	return Font;
}


void BFont_FreeFont(BFont_Info *Font)
{
    SDL_FreeSurface(Font->Surface);
	free(Font);
	Font = NULL;
}

BFont_Info * BFont_SetFontColor(BFont_Info *Font,Uint8 r, Uint8 g, Uint8 b)
{
    int x,y;

    BFont_Info *newfont;
    SDL_Surface *surface = NULL;

    Uint32 pixel;
    Uint8 old_r, old_g, old_b, old_a;
    Uint8 new_r, new_g, new_b, new_a;
    Uint32 color_key;

	newfont = (BFont_Info *) malloc(sizeof(BFont_Info));
	if (newfont == NULL) 
		return NULL;

	newfont->h = Font->h;
	for (x=0; x < BFONT_NUM_CHARS; x++) {
		newfont->Chars[x].x = Font->Chars[x].x;
		newfont->Chars[x].y = Font->Chars[x].y;
		newfont->Chars[x].h = Font->Chars[x].h;
		newfont->Chars[x].w = Font->Chars[x].w;
	}

	surface = SDL_ConvertSurface(Font->Surface, Font->Surface->format, Font->Surface->flags);
	if (surface == NULL) {
		free(newfont);
		return NULL;
	}

	if (SDL_MUSTLOCK(surface))       SDL_LockSurface(surface);
	if (SDL_MUSTLOCK(Font->Surface)) SDL_LockSurface(Font->Surface);

	color_key =  BFontGetPixel(surface, 0, surface->h-1);

	for( x=0; x < Font->Surface->w; x++) 
	{
		for( y=0; y < Font->Surface->h; y++) 
		{
			old_r = old_g = old_b = 0;

			pixel = BFontGetPixel(Font->Surface,x,y);

			if (pixel != color_key) 
			{
				SDL_GetRGBA(pixel, surface->format, &old_r, &old_g, &old_b, &old_a);

				new_r = (Uint8) ((old_r * r) / 255);
				new_g = (Uint8) ((old_g * g) / 255);
				new_b = (Uint8) ((old_b * b) / 255);

				/* Next line modified by  Antti Mannisto  */
				new_a = old_a;
				pixel = SDL_MapRGBA(surface->format, new_r, new_g, new_b, new_a);
				PutPixel(surface,x,y,pixel);
			}
		}
	}
	if (SDL_MUSTLOCK(surface))       SDL_UnlockSurface(surface);
	if (SDL_MUSTLOCK(Font->Surface)) SDL_UnlockSurface(Font->Surface);

 	SDL_SetColorKey(surface, SDL_SRCCOLORKEY, color_key);

	newfont->Surface = surface;

    return newfont;
}

void BFont_SetCurrentFont(BFont_Info *Font)
{
    CurrentFont = Font;
}

BFont_Info * BFont_GetCurrentFont(void)
{
    return CurrentFont;
}

int BFont_FontHeight (BFont_Info *Font)
{
    return (Font->h);
}

void BFont_SetFontHeight(BFont_Info *Font, int height)
{
	if (height >= 0)
		Font->h = height;
}


int BFont_CharWidth(BFont_Info *Font,int c)
{
    return Font->Chars[c].w;
}

int BFont_PutChar(SDL_Surface *Surface, int x, int y, int c)
{
    return BFont_PutCharFont(Surface, CurrentFont, x, y, c);
}

int BFont_PutCharFont(SDL_Surface *Surface, BFont_Info *Font,int x, int y, int c)
{
    SDL_Rect dest;
	SDL_Rect src;

	src = Font->Chars[c];
	if (src.h > Font->h)
		src.h = Font->h;

	dest = src;
	dest.x = x;
	dest.y = y;

    if (c != SPACE) {
        SDL_BlitSurface( Font->Surface, &src, Surface, &dest);
    }

	/* Next line modified by  Antti Mannisto  */
    return (Font->Chars[c].w);
}

void BFont_PutString(SDL_Surface *Surface, int x, int y, const char *text)
{
    BFont_PutStringFont(Surface, CurrentFont, x, y, text);
}

void BFont_PutStringFont(SDL_Surface *Surface, BFont_Info *Font, int x, int y, const char *text)
{
    while ( *text ) {
        x  += BFont_PutCharFont(Surface,Font,x,y, *text);
        text++;;
    }
}


int BFont_TextWidth(const char *text)
{
    return BFont_TextWidthFont( CurrentFont, text);
}

int BFont_TextWidthFont(BFont_Info *Font, const char *text)
{
    int x=0;

	while (*text) {
		x += BFont_CharWidth(Font,*text);
		text++;
	}
    return x;
}


/* counts the spaces of the strings */
static int count (const char *text)
{
    char *p = NULL;
    int pos = -1;
    int i   = 0;

    /* Calculate the space occupied by the text without spaces */
    while ((p=strchr((char*)&text[pos+1],SPACE)) != NULL) {
            i++;
            pos = p - text;
    }
    return i;
}

void BFont_JustifiedPutString(SDL_Surface *Surface, int y, const char *text)
{
    BFont_JustifiedPutStringFont( Surface, CurrentFont, y,text);
}

void BFont_JustifiedPutStringFont(SDL_Surface *Surface, BFont_Info *Font,  int y, const char *text)
{
    int spaces = 0;
    int gap;
    int single_gap;
    int dif;

    char *strtmp;
    char *p;
    int pos = -1;
    int xpos = 0;


    if (strchr(text,SPACE) == NULL) {
        BFont_PutStringFont(Surface, Font, 0, y, text);
    }
    else {
        gap = (Surface->w-1) - BFont_TextWidthFont(Font,text);

        if (gap <= 0) {
            BFont_PutStringFont(Surface, Font,0,y,text);
        } else {
            spaces = count(text);
            dif = gap % spaces;
            single_gap = (gap - dif) / spaces;
            xpos=0;
            pos = -1;
            while ( spaces > 0 ) {
                p = strstr((char*)&text[pos+1]," ");
                strtmp = NULL;
                strtmp = (char *) calloc ((p - &text[pos+1]) + 1,sizeof(char));
                if (strtmp != NULL) 
				{
                    strncpy (strtmp, &text[pos+1], (p - &text[pos+1]));
                    BFont_PutStringFont(Surface, Font, xpos, y, strtmp);
                    xpos = xpos + BFont_TextWidthFont(Font, strtmp) + single_gap + BFont_CharWidth(Font,SPACE);
                    if (dif >= 0) {
                        xpos ++;
                        dif--;
                    }
                    pos = p - text;
                    spaces--;
                    free(strtmp);
                }
            }
            strtmp = NULL;
            strtmp = (char *) calloc (strlen(&text[pos+1]) + 1,sizeof(char));

            if (strtmp != NULL) {
                strncpy (strtmp, &text[pos+1], strlen( &text[pos+1]));
                BFont_PutStringFont(Surface, Font,xpos, y, strtmp);
                free(strtmp);
            }
        }
    }
}

void BFont_CenteredPutString(SDL_Surface *Surface, int y, const char *text)
{
    BFont_CenteredPutStringFont(Surface, CurrentFont, y, text);
}

void BFont_CenteredPutStringFont(SDL_Surface *Surface, BFont_Info *Font, int y, const char *text)
{
    BFont_PutStringFont(Surface, Font, (Surface->w/2) - (BFont_TextWidthFont(Font,text)/2), y, text);
}

void BFont_RightPutString(SDL_Surface *Surface, int y, const char *text)
{
    BFont_RightPutStringFont(Surface, CurrentFont, y, text);
}

void BFont_RightPutStringFont(SDL_Surface *Surface, BFont_Info *Font, int y, const char *text)
{
    BFont_PutStringFont(Surface, Font, Surface->w - BFont_TextWidthFont(Font,text) - 1, y, text);
}

void BFont_LeftPutString(SDL_Surface *Surface, int y, const char *text)
{
    BFont_LeftPutStringFont(Surface, CurrentFont, y, text);
}

void BFont_LeftPutStringFont(SDL_Surface *Surface, BFont_Info *Font, int y, const char *text)
{
    BFont_PutStringFont(Surface, Font, 0, y, text);
}

/******/

void BFont_PrintString (SDL_Surface *Surface, int x, int y, const char *fmt, ...)
{
    va_list args;

    va_start (args,fmt);
    vsnprintf(bfont_buffer,BFONT_BUFFER_LEN,fmt,args);
    va_end(args);

    bfont_buffer[BFONT_BUFFER_LEN-1] = '\0';
    BFont_PutStringFont(Surface, CurrentFont, x, y, bfont_buffer);
}

void BFont_PrintStringFont(SDL_Surface *Surface, BFont_Info *Font, int x, int y, const char *fmt, ...)
{
    va_list args;
    
    va_start (args,fmt);
    vsnprintf(bfont_buffer,BFONT_BUFFER_LEN,fmt,args);
    va_end(args);

    bfont_buffer[BFONT_BUFFER_LEN-1] = '\0';
    BFont_PutStringFont(Surface, Font, x, y, bfont_buffer);
}

void BFont_CenteredPrintString(SDL_Surface *Surface, int y,  const char *fmt, ...)
{
    va_list args;
    
    va_start (args,fmt);
    vsnprintf(bfont_buffer,BFONT_BUFFER_LEN,fmt,args);
    va_end(args);

    bfont_buffer[BFONT_BUFFER_LEN-1] = '\0';
    BFont_CenteredPutString(Surface, y, bfont_buffer);
}

void BFont_CenteredPrintStringFont(SDL_Surface *Surface, BFont_Info *Font, int y,  const char *fmt, ...)
{
    va_list args;
    
    va_start (args,fmt);
    vsnprintf(bfont_buffer,BFONT_BUFFER_LEN,fmt,args);
    va_end(args);

    bfont_buffer[BFONT_BUFFER_LEN-1] = '\0';
    BFont_CenteredPutStringFont(Surface, Font, y, bfont_buffer);
}

void BFont_RightPrintString(SDL_Surface *Surface, int y, const char *fmt, ...)
{
    va_list args;
    
    va_start (args,fmt);
    vsnprintf(bfont_buffer,BFONT_BUFFER_LEN,fmt,args);
    va_end(args);

    bfont_buffer[BFONT_BUFFER_LEN-1] = '\0';
    BFont_RightPutString(Surface, y, bfont_buffer);
}

void BFont_RightPrintStringFont(SDL_Surface *Surface, BFont_Info *Font, int y,  const char *fmt, ...)
{
    va_list args;
    
    va_start (args,fmt);
    vsnprintf(bfont_buffer,BFONT_BUFFER_LEN,fmt,args);
    va_end(args);

    bfont_buffer[BFONT_BUFFER_LEN-1] = '\0';
    BFont_RightPutStringFont(Surface, Font, y, bfont_buffer);
}

void BFont_LeftPrintString(SDL_Surface *Surface, int y, const char *fmt, ...)
{
    va_list args;
    
    va_start (args,fmt);
    vsnprintf(bfont_buffer,BFONT_BUFFER_LEN,fmt,args);
    va_end(args);

    bfont_buffer[BFONT_BUFFER_LEN-1] = '\0';
    BFont_LeftPutString(Surface, y, bfont_buffer);
}

void BFont_LeftPrintStringFont(SDL_Surface *Surface, BFont_Info *Font, int y, const  char *fmt, ...)
{
    va_list args;
    
    va_start (args,fmt);
    vsnprintf(bfont_buffer,BFONT_BUFFER_LEN,fmt,args);
    va_end(args);

    bfont_buffer[BFONT_BUFFER_LEN-1] = '\0';
    BFont_LeftPutStringFont(Surface, Font, y, bfont_buffer);
}

void BFont_JustifiedPrintString(SDL_Surface *Surface, int y, const char *fmt, ...)
{
    va_list args;
    
    va_start (args,fmt);
    vsnprintf(bfont_buffer,BFONT_BUFFER_LEN,fmt,args);
    va_end(args);

    bfont_buffer[BFONT_BUFFER_LEN-1] = '\0';
    BFont_JustifiedPutString( Surface,  y,bfont_buffer);
}

void BFont_JustifiedPrintStringFont(SDL_Surface *Surface, BFont_Info *Font,  int y, const char *fmt, ...)
{
    va_list args;
    
    va_start (args,fmt);
    vsnprintf(bfont_buffer,BFONT_BUFFER_LEN,fmt,args);
    va_end(args);

    bfont_buffer[BFONT_BUFFER_LEN-1] = '\0';
    BFont_JustifiedPutStringFont( Surface, Font, y,bfont_buffer);
}

SDL_Surface * BFont_CreateSurfaceFont (BFont_Info *Font, const char *text)
{
    SDL_Surface *surface = NULL;
	Uint32 color_key;

    surface = SDL_CreateRGBSurface(Font->Surface->flags,
		BFont_TextWidthFont(Font, text), 
		BFont_FontHeight(Font), 
		Font->Surface->format->BitsPerPixel, 
		Font->Surface->format->Rmask, 
		Font->Surface->format->Gmask, 
		Font->Surface->format->Bmask, 
		0);

	if (surface == NULL)
		return NULL;

	if (SDL_MUSTLOCK(Font->Surface)) SDL_LockSurface(Font->Surface);
	
	color_key = BFontGetPixel(Font->Surface, 0, Font->Surface->h - 1);

	if (SDL_MUSTLOCK(Font->Surface)) SDL_UnlockSurface(Font->Surface);	

	SDL_FillRect(surface, NULL, color_key);

	BFont_PutStringFont(surface, Font, 0, 0, text);

	SDL_SetColorKey(surface, SDL_SRCCOLORKEY, color_key);

	return surface;
}


SDL_Surface * BFont_CreateSurface (const char *text)
{
	return BFont_CreateSurfaceFont(CurrentFont, text);
}


/*********************************************************************************************************/
/*********************************************************************************************************/
/*********************************************************************************************************/

static void   PutPixel(SDL_Surface *surface, Sint32 X, Sint32 Y, Uint32 pixel)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to set */
    Uint8 *p = (Uint8 *)surface->pixels + Y * surface->pitch + X * bpp;

    switch(bpp) {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16 *)p = pixel;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        } else {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32 *)p = pixel;
        break;
    }
}

static Uint32 BFontGetPixel(SDL_Surface *Surface, Sint32 X, Sint32 Y)
{

   Uint8  *bits;
   Uint32 Bpp;

   if (X<0) puts("x too small in GetPixel!");
   if (X>=Surface->w) puts("x too big in GetPixel!");

   Bpp = Surface->format->BytesPerPixel;

   bits = ((Uint8 *)Surface->pixels)+Y*Surface->pitch+X*Bpp;

   // Get the pixel
   switch(Bpp) {
      case 1:
         return *((Uint8 *)Surface->pixels + Y * Surface->pitch + X);
         break;
      case 2:
         return *((Uint16 *)Surface->pixels + Y * Surface->pitch/2 + X);
         break;
      case 3: { // Format/endian independent
         Uint8 r, g, b;
         r = *((bits)+Surface->format->Rshift/8);
         g = *((bits)+Surface->format->Gshift/8);
         b = *((bits)+Surface->format->Bshift/8);
         return SDL_MapRGB(Surface->format, r, g, b);
         }
         break;
      case 4:
         return *((Uint32 *)Surface->pixels + Y * Surface->pitch/4 + X);
         break;
   }

    return -1;
}

