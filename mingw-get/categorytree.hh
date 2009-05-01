/** \file categorytree.hh
 *
 * Created: JohnE, 2009-01-03
 */
#ifndef CATEGORYTREE_HH_INC
#define CATEGORYTREE_HH_INC


#define WIN32_LEAN_AND_MEAN
#include <windows.h>


#ifdef __cplusplus
extern "C" {
#endif


void CategoryTree_Reload();
int CategoryTree_WM_NOTIFY
 (HWND hwndDlg,
  WPARAM wParam,
  LPARAM lParam,
  int* return_value_out);


#ifdef __cplusplus
} //extern "C"
#endif


#endif // CATEGORYTREE_HH_INC
