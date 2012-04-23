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

#ifndef _IS_UTIL_H_
#define _IS_UTIL_H_

#ifdef _WINDOWS
	#define DECLSPEC __declspec(dllexport)
#else
	#define DECLSPEC
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* @package Utils */
/* @beginmodule Array */

typedef struct {
	long length;
	long itemSize;
	void **data;
} ISArray;

extern DECLSPEC void ISArray_PushElement(ISArray *array, void *newItem);
extern DECLSPEC void *ISArray_CreateElement(ISArray *array);
extern DECLSPEC int ISArray_HasElement(ISArray *array, void *element);
extern DECLSPEC int ISArray_RemoveElement(ISArray *array, void *element);
extern DECLSPEC ISArray *ISArray_Create(long itemSize);
extern DECLSPEC void ISArray_Clear(ISArray *array);

/* @beginmodule String */

typedef struct {
	char *text;
	long length;
} ISString;

extern DECLSPEC ISString *ISString_Create(char *data);
extern DECLSPEC void ISString_SetText(ISString *string, char *newText);
extern DECLSPEC void ISString_Destroy(ISString *string);

/* @endmodule */

#ifdef __cplusplus
}
#endif


#endif

