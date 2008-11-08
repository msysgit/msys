/** \file mainwnd.hh
 *
 * Created: JohnE, 2008-11-08
 */
#ifndef MAINWND_HH_INC
#define MAINWND_HH_INC


#define WIN32_LEAN_AND_MEAN
#include <windows.h>


#ifdef __cplusplus
extern "C" {
#endif


void TransToClient(HWND hwnd, RECT* rc);
void NewVertSashPos(int pos);
void NewHorzSashPos(int pos);
int CreateMainWnd();
int MainMessageLoop();


#ifdef __cplusplus
} //extern "C"
#endif


extern HWND g_hmainwnd;


#endif // MAINWND_HH_INC
