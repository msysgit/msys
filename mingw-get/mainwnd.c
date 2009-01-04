
#include "mainwnd.hh"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <malloc.h>
#include <ole2.h>
#include "resource.h"
#include "pkg_const.h"
#include "selectinst.hh"
#include "sashwnd.hh"
#include "winmain.hh"
#include "editreposdlg.hh"
#include "ui_general.hh"
#include "pkgindex.hh"
#include "packagelist.hh"


HWND g_hmainwnd = 0;


static int g_vert_grip_x = 175;
static float g_horz_grip_prop = 0.5f;
static HACCEL g_haccel;


void TransToClient(HWND hwnd, RECT* rc)
{
	POINT p;
	p.x = rc->left;
	p.y = rc->top;
	ScreenToClient(hwnd, &p);
	rc->left = p.x;
	rc->top = p.y;
	p.x = rc->right;
	p.y = rc->bottom;
	ScreenToClient(hwnd, &p);
	rc->right = p.x;
	rc->bottom = p.y;
}


void NewVertSashPos(int pos)
{
	g_vert_grip_x = pos;
	RECT rc;
	GetClientRect(g_hmainwnd, &rc);
	SendMessage(g_hmainwnd, WM_SIZE, SIZE_RESTORED,
	 MAKELPARAM(rc.right - rc.left, rc.bottom - rc.top));
}


void NewHorzSashPos(int pos)
{
	RECT tbrc, statrc, rc;
	GetWindowRect(GetDlgItem(g_hmainwnd, IDC_MAINTOOLBAR), &tbrc);
	TransToClient(g_hmainwnd, &tbrc);
	GetWindowRect(GetDlgItem(g_hmainwnd, IDC_STATBAR), &statrc);
	TransToClient(g_hmainwnd, &statrc);
	GetClientRect(g_hmainwnd, &rc);
	g_horz_grip_prop = (float)(pos - (tbrc.bottom - tbrc.top) - 6) /
	 ((rc.bottom - rc.top) - (tbrc.bottom - tbrc.top) - (statrc.bottom - statrc.top) - 6);
	SendMessage(g_hmainwnd, WM_SIZE, SIZE_RESTORED,
	 MAKELPARAM(rc.right - rc.left, rc.bottom - rc.top));
}


BOOL CALLBACK MainWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			g_hmainwnd = hwndDlg;

			HWND htb = GetDlgItem(hwndDlg, IDC_MAINTOOLBAR);
			HIMAGELIST il =
			 ImageList_Create(24, 24, ILC_COLOR32 | ILC_MASK, 6, 0);
			HBITMAP tbbuttons =
			 LoadBitmap(g_hinstance, MAKEINTRESOURCE(IDB_TBBUTTONS));
			ImageList_AddMasked(il, tbbuttons, RGB(255, 0, 255));
			DeleteObject(tbbuttons);
			SendMessage(htb, TB_SETIMAGELIST, (WPARAM)0, (LPARAM)il);
			TBBUTTON tbButtons[3] = {
			 { 0, IDM_SOURCES_UPDATELISTS, TBSTATE_ENABLED,
			  TBSTYLE_BUTTON|TBSTYLE_AUTOSIZE, {0}, 0, (INT_PTR)"Update Lists" },
			 { 1, -1, TBSTATE_ENABLED,
			  TBSTYLE_BUTTON|TBSTYLE_AUTOSIZE, {0}, 0, (INT_PTR)"Mark All Upgrades"},
			 { 2, IDM_EDIT_APPLY, TBSTATE_ENABLED,
			  TBSTYLE_BUTTON|TBSTYLE_AUTOSIZE, {0}, 0, (INT_PTR)"Apply"}
			};
			SendMessage(htb, TB_BUTTONSTRUCTSIZE,
			 (WPARAM)sizeof(TBBUTTON), 0);
			SendMessage(htb, TB_ADDBUTTONS, (WPARAM)3, 
			 (LPARAM)&tbButtons);
			SendMessage(htb, TB_AUTOSIZE, 0, 0);
			ShowWindow(htb, TRUE);

			HWND hcb = GetDlgItem(hwndDlg, IDC_CATTYPE);
			SendMessage(hcb, CB_ADDSTRING, 0, (LPARAM)"Category");
			SendMessage(hcb, CB_ADDSTRING, 0, (LPARAM)"State");
			SendMessage(hcb, CB_ADDSTRING, 0, (LPARAM)"Release Status");
			SendMessage(hcb, CB_SETCURSEL, 0, 0);

			TVINSERTSTRUCT tvins;
			tvins.item.mask = TVIF_TEXT | TVIF_PARAM;
			tvins.item.pszText = (CHAR*)"All Packages";
			tvins.item.cchTextMax = 199;
			tvins.item.lParam = 1;
			tvins.hInsertAfter = TVI_LAST;
			tvins.hParent = TVI_ROOT;
			SendMessage(
			 GetDlgItem(hwndDlg, IDC_CATLIST),
			 TVM_INSERTITEM,
			 0,
			 (LPARAM)(LPTVINSERTSTRUCT)&tvins
			 );

			PackageList_Create();

			HFONT hFont = (HFONT)SendMessage(hwndDlg, WM_GETFONT, 0, 0);
			LOGFONT lf = {0};
			GetObject(hFont, sizeof(LOGFONT), &lf);
			lf.lfWeight |= FW_BOLD;
			lf.lfHeight += 2;
			HFONT hFontTitle = CreateFontIndirect(&lf);
			SendMessage(GetDlgItem(hwndDlg, IDC_DESCTITLE), WM_SETFONT,
			 (WPARAM)hFontTitle, MAKELPARAM(FALSE, 0));
			DeleteObject(hFontTitle);
		}
		return TRUE;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		PostQuitMessage(0);
		return TRUE;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDM_FILE_EXIT:
			DestroyWindow(hwndDlg);
			PostQuitMessage(0);
			return TRUE;
		case IDM_FILE_CHANGEINST:
			SelectInstallation();
			return TRUE;
		case IDM_SOURCES_UPDATELISTS:
			UI_UpdateLists();
			return TRUE;
		case IDM_SOURCES_REPOSITORIES:
			EditReposDlg();
			return TRUE;
		case IDC_CATLIST:
			if (HIWORD(wParam) == LBN_SELCHANGE)
			{
				//UI_OnCategoryChange(ListBox_GetCurSel((HWND)lParam));
				return TRUE;
			}
			break;
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom)
		{
		case IDC_COMPLIST:
			switch (((LPNMHDR)lParam)->code)
			{
			case LVN_GETDISPINFO:
				{
					((NMLVDISPINFO*)lParam)->item.pszText =
					 (char*)PkgIndex_PackageGetSubItemText(
					  ((NMLVDISPINFO*)lParam)->item.lParam,
					  ((NMLVDISPINFO*)lParam)->item.iSubItem
					  );
				}
				return 0;
			case LVN_COLUMNCLICK:
				PackageList_Sort(((LPNMLISTVIEW)lParam)->iSubItem);
				return 0;
			case LVN_ITEMCHANGED:
				if ((((LPNMLISTVIEW)lParam)->uChanged & LVIF_STATE)
				 && (((LPNMLISTVIEW)lParam)->uNewState & LVIS_SELECTED))
					PackageList_OnSelect(((LPNMLISTVIEW)lParam)->iItem);
				return 0;
			case NM_CLICK:
				{
					DWORD dwpos = GetMessagePos();
					LVHITTESTINFO lvhti;
					memset(&lvhti, 0, sizeof(lvhti));
					lvhti.pt.x = GET_X_LPARAM(dwpos);
					lvhti.pt.y = GET_Y_LPARAM(dwpos);
					ScreenToClient(((LPNMHDR)lParam)->hwndFrom, &lvhti.pt);
					ListView_HitTest(((LPNMHDR)lParam)->hwndFrom, &lvhti);
					if ((lvhti.flags & LVHT_ONITEMICON)
					 && !(lvhti.flags & LVHT_ONITEMLABEL)
					 && lvhti.iItem >= 0)
						PackageList_OnStateCycle(lvhti.iItem);
				}
				return 0;
			case NM_RCLICK:
				{
					DWORD dwpos = GetMessagePos();
					LVHITTESTINFO lvhti;
					lvhti.pt.x = GET_X_LPARAM(dwpos);
					lvhti.pt.y = GET_Y_LPARAM(dwpos);
					ScreenToClient(((LPNMHDR)lParam)->hwndFrom, &lvhti.pt);
					int hit = ListView_HitTest(((LPNMHDR)lParam)->hwndFrom,
					 &lvhti);
					if (hit >= 0)
					{
						HMENU menu = CreatePopupMenu();
						LVITEM lvitem;
						lvitem.iItem = hit;
						lvitem.iSubItem = 0;
						lvitem.mask = LVIF_PARAM;
						ListView_GetItem(((LPNMHDR)lParam)->hwndFrom, &lvitem);
						const char* instv =
						 PkgIndex_PackageGetInstalledVersion(lvitem.lParam);
						if (instv)
							AppendMenu(menu, MF_SEPARATOR, -1, 0);
						else
						{
							int act =
							 PkgIndex_PackageGetSelectedAction(lvitem.lParam);
							if (act != ACT_NO_CHANGE)
							{
								AppendMenu(menu, MF_STRING,
								 10000 + ACT_NO_CHANGE,
								 "&Deselect this package");
							}
							if (act != ACT_INSTALL_VERSION)
							{
								AppendMenu(menu, MF_STRING,
								 10000 + ACT_INSTALL_VERSION,
								 "&Select this package");
							}
						}
						int ret = TrackPopupMenuEx(menu,
						 TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NONOTIFY |
						  TPM_RETURNCMD | TPM_RIGHTBUTTON,
						 GET_X_LPARAM(dwpos), GET_Y_LPARAM(dwpos),
						 ((LPNMHDR)lParam)->hwndFrom, 0);
						DestroyMenu(menu);
						switch (ret)
						{
						case 10000 + ACT_INSTALL_VERSION:
							PkgIndex_PackageSelectAction(lvitem.lParam,
							 ACT_INSTALL_VERSION);
							break;
						case 10000 + ACT_NO_CHANGE:
							PkgIndex_PackageSelectAction(lvitem.lParam,
							 ACT_NO_CHANGE);
							break;
						}
						lvitem.mask = LVIF_IMAGE;
						lvitem.iImage = PkgIndex_PackageGetStateImage(lvitem.lParam);
						ListView_SetItem(((LPNMHDR)lParam)->hwndFrom, &lvitem);
					}
				}
				return 0;
			}
			break;
		}
		break;

	case WM_SIZE:
		{
			int cli_width = LOWORD(lParam);
			int cli_height = HIWORD(lParam);

			RECT tbrc, statrc, ctyperc, instvrc, vtirc, rc;

			GetWindowRect(GetDlgItem(hwndDlg, IDC_MAINTOOLBAR), &tbrc);
			TransToClient(hwndDlg, &tbrc);

			GetWindowRect(GetDlgItem(hwndDlg, IDC_STATBAR), &statrc);
			TransToClient(hwndDlg, &statrc);

			int t = tbrc.bottom + 6;

			MoveWindow(GetDlgItem(hwndDlg, IDC_VERTSASH), g_vert_grip_x, t,
			 8, cli_height - (statrc.bottom - statrc.top) - t, TRUE);

			GetWindowRect(GetDlgItem(hwndDlg, IDC_CATTYPE), &ctyperc);
			TransToClient(hwndDlg, &ctyperc);
			MoveWindow(GetDlgItem(hwndDlg, IDC_CATTYPE),
			 ctyperc.left, t, g_vert_grip_x,
			 ctyperc.bottom - ctyperc.top, TRUE);

			GetWindowRect(GetDlgItem(hwndDlg, IDC_CATLIST), &rc);
			TransToClient(hwndDlg, &rc);
			MoveWindow(GetDlgItem(hwndDlg, IDC_CATLIST),
			 rc.left, t + (ctyperc.bottom - ctyperc.top) + 2,
			 g_vert_grip_x,
			 cli_height - t - (ctyperc.bottom - ctyperc.top) -
			  (statrc.bottom - statrc.top) - 2, TRUE);

			int h = (cli_height - t - (statrc.bottom - statrc.top)) *
			 g_horz_grip_prop;

			MoveWindow(GetDlgItem(hwndDlg, IDC_COMPLIST),
			 g_vert_grip_x + 8, t, cli_width - g_vert_grip_x - 8, h, TRUE);
			InvalidateRect(GetDlgItem(hwndDlg, IDC_COMPLIST), 0, TRUE);
			UpdateWindow(GetDlgItem(hwndDlg, IDC_COMPLIST));

			MoveWindow(GetDlgItem(hwndDlg, IDC_HORZSASH), g_vert_grip_x + 8,
			 t + h, cli_width - g_vert_grip_x - 8, 8, TRUE);

			GetWindowRect(GetDlgItem(hwndDlg, IDC_INSTVERSION), &instvrc);
			TransToClient(hwndDlg, &instvrc);
			MoveWindow(GetDlgItem(hwndDlg, IDC_INSTVERSION),
			 cli_width - (instvrc.right - instvrc.left), t + h + 10,
			 (instvrc.right - instvrc.left), instvrc.bottom - instvrc.top,
			 TRUE);
			InvalidateRect(GetDlgItem(hwndDlg, IDC_INSTVERSION), 0, TRUE);
			UpdateWindow(GetDlgItem(hwndDlg, IDC_INSTVERSION));

			GetWindowRect(GetDlgItem(hwndDlg, IDC_VTITEXT), &vtirc);
			TransToClient(hwndDlg, &vtirc);
			MoveWindow(GetDlgItem(hwndDlg, IDC_VTITEXT),
			 cli_width - (instvrc.right - instvrc.left) - (vtirc.right - vtirc.left) - 2,
			 t + h + 14, vtirc.right - vtirc.left, vtirc.bottom - vtirc.top,
			 TRUE);
			InvalidateRect(GetDlgItem(hwndDlg, IDC_VTITEXT), 0, TRUE);
			UpdateWindow(GetDlgItem(hwndDlg, IDC_VTITEXT));

			GetWindowRect(GetDlgItem(hwndDlg, IDC_DESCTITLE), &rc);
			TransToClient(hwndDlg, &rc);
			MoveWindow(GetDlgItem(hwndDlg, IDC_DESCTITLE), g_vert_grip_x + 10,
			 t + h + 8 + (instvrc.bottom - instvrc.top) - (rc.bottom - rc.top),
			 cli_width - g_vert_grip_x - (instvrc.right - instvrc.left) - (vtirc.right - vtirc.left) - 14,
			 rc.bottom - rc.top, TRUE);
			InvalidateRect(GetDlgItem(hwndDlg, IDC_DESCTITLE), 0, TRUE);
			UpdateWindow(GetDlgItem(hwndDlg, IDC_DESCTITLE));

			MoveWindow(GetDlgItem(hwndDlg, IDC_FULLDESC), g_vert_grip_x + 8,
			 t + h + (instvrc.bottom - instvrc.top) + 10,
			 cli_width - g_vert_grip_x - 8,
			 cli_height - t - h - (instvrc.bottom - instvrc.top) - (statrc.bottom - statrc.top) - 10,
			 TRUE);

			MoveWindow(GetDlgItem(hwndDlg, IDC_MAINTOOLBAR),
			 tbrc.left, tbrc.top, cli_width - tbrc.left * 2,
			 tbrc.bottom - tbrc.top, TRUE);
			InvalidateRect(GetDlgItem(hwndDlg, IDC_MAINTOOLBAR), 0, TRUE);
			UpdateWindow(GetDlgItem(hwndDlg, IDC_MAINTOOLBAR));
			MoveWindow(GetDlgItem(hwndDlg, IDC_STATBAR),
			 statrc.left, cli_height - (statrc.bottom - statrc.top),
			 cli_width - statrc.left * 2,
			 statrc.bottom - statrc.top, TRUE);
			InvalidateRect(GetDlgItem(hwndDlg, IDC_STATBAR), 0, TRUE);
			UpdateWindow(GetDlgItem(hwndDlg, IDC_STATBAR));
		}
		return 0;
	}

    return FALSE;
}


static int ProcessQueuedMessages(HWND wnd)
{
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
			return 1;
		if (!TranslateAccelerator(wnd, g_haccel, &msg))
		{
			if (!IsDialogMessage(wnd, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
	return 0;
}


int CreateMainWnd()
{
	InitCommonControls();
	OleInitialize(0);

	if (!SashWnd_RegisterClasses(g_hinstance))
	{
		MessageBox(0, "Couldn't register window classes for sashes.",
		 "Windowing Failure", MB_OK | MB_ICONERROR);
		return 0;
	}

	g_haccel = LoadAccelerators(g_hinstance, MAKEINTRESOURCE(IDA_MAINACCEL));

	g_hmainwnd = CreateDialog(g_hinstance, MAKEINTRESOURCE(IDD_MAINDLG), 0,
     MainWndProc);
	if (!g_hmainwnd)
	{
		MessageBox(0, "Couldn't create the main window.", "Windowing Failure",
		 MB_OK | MB_ICONERROR);
		return 0;
	}
	ShowWindow(g_hmainwnd, SW_SHOW);
	if (ProcessQueuedMessages(g_hmainwnd))
		return 0;

	return 1;
}


int MainMessageLoop()
{
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(g_hmainwnd, g_haccel, &msg))
		{
			if (!IsDialogMessage(g_hmainwnd, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
	return msg.wParam;
}
