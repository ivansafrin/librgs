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
 
#ifndef RGS_SCREEN_H_
#define RGS_SCREEN_H_

#include "rgs_data.h"

#ifdef __cplusplus
extern "C" {
#endif

/* @package Graphics */
/* @beginmodule Screens */

extern RGS_Core *g_rgs;

/** @desc Creates a new screen 
 * @return Pointer to the created screen
*/
extern DECLSPEC RGS_Screen *RGS_CreateScreen();
/** @desc Adds screen to automatic render queue. This is called automatically by RGS_CreateScreen
 *  @param screen Screen to add.
 */
extern DECLSPEC void RGS_AddScreen(RGS_Screen *screen);
/** @desc Adds a child object to the screen's render queue
 *  @param Object to add
 */
extern DECLSPEC void RGS_AddScreenObject(RGS_Screen *screen, RGS_Object *object);
/** @desc Focuses input on a screen.
 *  @param screen Screen to focus input on
 */
extern DECLSPEC void RGS_FocusScreen(RGS_Screen *screen);
/** @desc Focuses the next object in the screen. Useful for tabbing.
 *  @param screen Target screen
 */
extern DECLSPEC void RGS_ScreenFocusNext(RGS_Screen *screen);
/** @desc Focuses a specific object in the target screen
 *  @param screen Target screen
 *  @param object Object to focus 
 */
extern DECLSPEC void RGS_ScreenFocusObject(RGS_Screen *screen, RGS_Object *object);
/** @desc Removes but not destroys a child object of a screen
 *  @param screen Target screen
 *  @param object Object to remove
 */
extern DECLSPEC int  RGS_ScreenRemoveObject(RGS_Screen *screen, RGS_Object *object);
/** @desc Destroys all the objects in the screen. The Destroy function in the object is called automatically.
 *  @param screen Target screen
 */
extern DECLSPEC void RGS_ScreenDestroyObjects(RGS_Screen *screen);
/** @endmodule */

extern DECLSPEC void RGS_ScreenKeyDownEvent(RGS_Screen *screen, int eventCode);
extern DECLSPEC void RGS_ScreenCharInputEvent(RGS_Screen *screen, int charCode);
extern DECLSPEC void RGS_RenderScreen(RGS_Screen *screen);

#ifdef __cplusplus
}
#endif

#endif