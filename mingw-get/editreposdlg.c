/** \file editreposdlg.c
 *
 * Created: JohnE, 2008-12-28
 */


#include "editreposdlg.hh"

#include "winmain.hh"
#include "mainwnd.hh"
#include "resource.h"


static BOOL CALLBACK EditReposProc
 (HWND hwndDlg,
  UINT uMsg,
  WPARAM wParam,
  LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
		}
		return FALSE;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hwndDlg, 1);
			return TRUE;
		}
		break;
	}

    return FALSE;
}


void EditReposDlg()
{
	DialogBox(g_hinstance, MAKEINTRESOURCE(IDD_EDITREPOS), g_hmainwnd,
	 EditReposProc);
}
