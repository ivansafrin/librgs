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

#include "isutil.h"
#include "stdlib.h"
#include "string.h"

void ISArray_PushElement(ISArray *array, void *newItem) {
	array->data = (void**)realloc(array->data, (sizeof(void*) * (array->length+1)));
	array->data[array->length] = newItem;
	array->length++;
}

ISArray *ISArray_Create(long itemSize) {
	ISArray *newArray = (ISArray*) malloc(sizeof(ISArray));
	newArray->data = 0;
	newArray->itemSize = itemSize;
	newArray->length = 0;
	return newArray;
}

void *ISArray_CreateElement(ISArray *array) {
	void *newElement = malloc(array->itemSize);
	memset(newElement, 0, array->itemSize);
	ISArray_PushElement(array, newElement);
	return newElement;
}


int ISArray_HasElement(ISArray *array, void *element) {
	int i;
	for(i=0;i<array->length;i++) {
		if(array->data[i] == element)
			return 1;
	}
	return 0;
}

int ISArray_RemoveElement(ISArray *array, void *element) {
	if(ISArray_HasElement(array, element) == 0)
		return 0;
	
	int i;
	int j =0;
	int removed = 0;
	void **newData;
	
	if(array->length > 1) {
		newData = (void**)malloc((array->length-1)*sizeof(void*));
	} else {
		newData = (void**)malloc(sizeof(void*));
		memset(newData, 0, sizeof(void*));
	}
	
	for(i=0;i<array->length;i++) {
		if(array->data[i] != element || removed == 1) {
			newData[j] = array->data[i];
			j++;
		} else {
			removed = 1;
		}
	}
	
	free(array->data);	
	array->length--;
	array->data = newData;
	return 1;
}

void ISArray_Clear(ISArray *array) {
	int i;
	for(i=0;i<array->length;i++) {
		free(array->data[i]);
	}
	array->length = 0;
}

ISString *ISString_Create(char *data) {
	ISString *newString = (ISString*) malloc(sizeof(ISString));
	if(data != NULL) {
		newString->text = (char*)malloc(strlen(data)+1);
		memcpy(newString->text, data, strlen(data)+1);
		newString->length = strlen(data); 
	} else {
		newString->text = (char*)malloc(1);
		memset(newString->text, '\0', 1);
		newString->length = 0; 	
	}
	return newString;
}

void ISString_SetText(ISString *string, char *newText) {
	free(string->text);
	string->text = (char*)malloc(strlen(newText)+1);
 	memcpy(string->text, newText, strlen(newText)+1);
	string->length = strlen(newText); 		
}

void ISString_Destroy(ISString *string) {
	free(string->text);
}
