
/************************************************************/
/*                                                          */
/*   BFONT.h v. 1.1.0 - Billi Font Library by Diego Billi   */
/*                                                          */
/*   mail: dbilli@cs.unibo.it                               */
/*   home: http://www.cs.unibo.it/~dbilli (ITALIAN)         */
/*                                                          */
/************************************************************/


#ifndef __BFONT_H_
#define __BFONT_H_


#include <SDL.h>

#ifdef __cplusplus
extern "C" {
#endif


/* Max number of characters allowed in a BFont font */
#define BFONT_NUM_CHARS 256


/* BFont font structure */
typedef struct _BFont_Info BFont_Info;

/* BFont_Info structure */
struct _BFont_Info {
	int          h;                      /* font height */
	SDL_Surface *Surface;                /* font surface */
    SDL_Rect     Chars[BFONT_NUM_CHARS]; /* characters width */
};


/* Load and store the font in the BFont_Info structure */
BFont_Info * BFont_LoadFont (void *fontPtr);

/* Free memory */
void BFont_FreeFont(BFont_Info *Font);

/* Returns a pointer to the current font structure */
BFont_Info * BFont_GetCurrentFont(void);

/* Set the current font */
void BFont_SetCurrentFont (BFont_Info *Font);

/* Returns the font height */
int BFont_FontHeight (BFont_Info *Font);

/* Change the font height */
void BFont_SetFontHeight (BFont_Info *Font, int height);

/* Returns the character width of the specified font */
int BFont_CharWidth (BFont_Info *Font,int c);

/* Write a single character on the "Surface" with the current font */
int BFont_PutChar (SDL_Surface *Surface, int x, int y, int c);

/* Write a single character on the "Surface" with the specified font */
int BFont_PutCharFont (SDL_Surface *Surface, BFont_Info *Font,int x, int y, int c);

/* Returns the width, in pixels, of the text calculated with the current font*/
int BFont_TextWidth (const char *text);

/* Returns the width, in pixels, of the text calculated with the specified font*/
int BFont_TextWidthFont (BFont_Info *Font, const char *text);

/* Write a string on the "Surface" with the current font */
void BFont_PutString (SDL_Surface *Surface, int x, int y, const char *text);

/* Write a string on the "Surface" with the specified font */
void BFont_PutStringFont (SDL_Surface *Surface, BFont_Info *Font, int x, int y, const char *text);

/* Write a left-aligned string on the "Surface" with the current font */
void BFont_LeftPutString (SDL_Surface *Surface, int y, const char *text);

/* Write a left-aligned string on the "Surface" with the specified font */
void BFont_LeftPutStringFont (SDL_Surface *Surface, BFont_Info *Font, int y, const char *text);

/* Write a center-aligned string on the "Surface" with the current font */
void BFont_CenteredPutString (SDL_Surface *Surface, int y, const char *text);

/* Write a center-aligned string on the "Surface" with the specified font */
void BFont_CenteredPutStringFont (SDL_Surface *Surface, BFont_Info *Font, int y, const char *text);

/* Write a right-aligned string on the "Surface" with the specified font */
void BFont_RightPutString (SDL_Surface *Surface, int y, const char *text);

/* Write a right-aligned string on the "Surface" with the specified font */
void BFont_RightPutStringFont (SDL_Surface *Surface, BFont_Info *Font, int y, const char *text);

/* Write a justify-aligned string on the "Surface" with the specified font */
void BFont_JustifiedPutString (SDL_Surface *Surface, int y, const char *text);

/* Write a justify-aligned string on the "Surface" with the specified font */
void BFont_JustifiedPutStringFont (SDL_Surface *Surface, BFont_Info *Font, int y, const char *text);


/* The following functions do the same task but have the classic "printf" sintax */

void BFont_PrintString              (SDL_Surface *Surface, int x, int y, const char *fmt, ...);
void BFont_PrintStringFont          (SDL_Surface *Surface, BFont_Info *Font, int x, int y, const char *fmt, ...);

void BFont_CenteredPrintString      (SDL_Surface *Surface, int y,  const char *fmt, ...);
void BFont_CenteredPrintStringFont  (SDL_Surface *Surface, BFont_Info *Font, int y,  const char *fmt, ...);

void BFont_RightPrintString         (SDL_Surface *Surface, int y, const char *fmt, ...);
void BFont_RightPrintStringFont     (SDL_Surface *Surface, BFont_Info *Font, int y,  const char *fmt, ...);

void BFont_LeftPrintString          (SDL_Surface *Surface, int y, const char *fmt, ...);
void BFont_LeftPrintStringFont      (SDL_Surface *Surface, BFont_Info *Font, int y,  const char *fmt, ...);

void BFont_JustifiedPrintString     (SDL_Surface *Surface, int y, const char *fmt, ...);
void BFont_JustifiedPrintStringFont (SDL_Surface *Surface, BFont_Info *Font,  int y, const char *fmt, ...);


/* Returns a new font colored with the color (r,g,b) */
BFont_Info * BFont_SetFontColor (BFont_Info *Font,Uint8 r, Uint8 g, Uint8 b);


/* Load and store the font int the BFont_Info structure from a SDL surface */
BFont_Info * BFont_LoadFontFromSurface (SDL_Surface *Surface);


/* Return a SDL Surface containing the string "text" */
SDL_Surface * BFont_CreateSurface (const char *text);
SDL_Surface * BFont_CreateSurfaceFont (BFont_Info *Font, const char *text);


/* This is for compatibility with old versions 1.0.X */

#ifdef BFONT_KEEP_COMPATIBILITY

	/* BFont v. 1.0.2 */

	#define LoadFont                 BFont_LoadFont
	#define FreeFont                 BFont_FreeFont
	#define GetCurrentFont           BFont_GetCurrentFont
	#define SetCurrentFont           BFont_SetCurrentFont
	#define FontHeight               BFont_FontHeight
	#define SetFontHeight            BFont_SetFontHeight
	#define CharWidth                BFont_CharWidth
	#define PutChar                  BFont_PutChar
	#define PutCharFont              BFont_PutCharFont
	#define TextWidth                BFont_TextWidth
	#define TextWidthFont            BFont_TextWidthFont
	#define PutString                BFont_PutString 
	#define PutStringFont            BFont_PutStringFont
	#define LeftPutString            BFont_LeftPutString
	#define LeftPutStringFont        BFont_LeftPutStringFont
	#define CenteredPutString        BFont_CenteredPutString
	#define CenteredPutStringFont    BFont_CenteredPutStringFont
	#define RightPutString           BFont_RightPutString 
	#define RightPutStringFont       BFont_RightPutStringFont
	#define JustifiedPutString       BFont_JustifiedPutString
	#define JustifiedPutStringFont   BFont_JustifiedPutStringFont

	#define PrintString              BFont_PrintString 
	#define PrintStringFont          BFont_PrintStringFont
	#define CenteredPrintString      BFont_CenteredPrintString 
	#define CenteredPrintStringFont  BFont_CenteredPrintStringFont  
	#define RightPrintString         BFont_RightPrintString
	#define RightPrintStringFont     BFont_RightPrintStringFont
	#define LeftPrintString          BFont_LeftPrintString
	#define LeftPrintStringFont      BFont_LeftPrintStringFont
	#define JustifiedPrintString     BFont_JustifiedPrintString
	#define JustifiedPrintStringFont BFont_JustifiedPrintStringFont

	/* BFont v. 1.0.3 */

	#define SetFontColor             BFont_SetFontColor

	/* BFont v. 1.0.4 */

	#define LoadFontFromSurface      BFont_LoadFontFromSurface

	/* BFont v. 1.0.5 */

	#define CreateSurface            BFont_CreateSurface
	#define CreateSurfaceFont        BFont_CreateSurfaceFont

#endif /* end compatibility */



#ifdef __cplusplus
}
#endif

#endif
