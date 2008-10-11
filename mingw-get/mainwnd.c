
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <malloc.h>
#include <ole2.h>
#include "resource.h"
#include "pkg_const.h"


static int g_vert_grip_x = 150;
static float g_horz_grip_prop = 0.5f;
static HACCEL g_haccel;
static HINSTANCE g_instance = 0;


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


void NewVertSashPos(int pos, HWND mainwnd)
{
	g_vert_grip_x = pos;
	RECT rc;
	GetClientRect(mainwnd, &rc);
	SendMessage(mainwnd, WM_SIZE, SIZE_RESTORED,
	 MAKELPARAM(rc.right - rc.left, rc.bottom - rc.top));
}


void NewHorzSashPos(int pos, HWND mainwnd)
{
	RECT tbrc, statrc, rc;
	GetWindowRect(GetDlgItem(mainwnd, IDC_MAINTOOLBAR), &tbrc);
	TransToClient(mainwnd, &tbrc);
	GetWindowRect(GetDlgItem(mainwnd, IDC_STATBAR), &statrc);
	TransToClient(mainwnd, &statrc);
	GetClientRect(mainwnd, &rc);
	g_horz_grip_prop = (float)(pos - (tbrc.bottom - tbrc.top) - 6) /
	 ((rc.bottom - rc.top) - (tbrc.bottom - tbrc.top) - (statrc.bottom - statrc.top) - 6);
	SendMessage(mainwnd, WM_SIZE, SIZE_RESTORED,
	 MAKELPARAM(rc.right - rc.left, rc.bottom - rc.top));
}


static void InsertColumn
 (HWND hlv,
  const char* txt,
  int index,
  int fmt,
  int width)
{
	int tlen = strlen(txt);
	if (tlen >= 200)
		return;
	char* tcopy = malloc(tlen + 1);
	strcpy(tcopy, txt);
	LVCOLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
	lvc.fmt = fmt;
	if (width > 0)
		lvc.cx = width;
	else
	{
		SIZE sz;
		if (GetTextExtentPoint32(GetDC(hlv), tcopy, tlen, &sz))
			lvc.cx = sz.cx + 15;
		else
			lvc.cx = 100;
	}
	lvc.pszText = tcopy;
	lvc.cchTextMax = tlen;
	ListView_InsertColumn(hlv, index, &lvc);
	free(tcopy);
}


void SelectInst(HINSTANCE, HWND);
const char* Pkg_GetSubItemText(LPARAM, int);
void UI_OnCategoryChange(int, HWND);
void UI_SortListView(int, HWND);
void UI_OnListViewSelect(int, HWND);
void DescWnd_SetHWND(HWND);
void UI_OnStateCycle(int, HWND);
const char* Pkg_GetInstalledVersion(LPARAM);
int Pkg_GetSelectedAction(LPARAM);
void Pkg_SelectAction(LPARAM, int);
int Pkg_GetStateImage(LPARAM);

static BOOL CALLBACK MainWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static int old_cli_width = 0, old_cli_height = 0;

	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			RECT rc;
			GetClientRect(hwndDlg, &rc);
			old_cli_width = rc.right;
			old_cli_height = rc.bottom;

			HWND htb = GetDlgItem(hwndDlg, IDC_MAINTOOLBAR);
			HIMAGELIST il =
			 ImageList_Create(24, 24, ILC_COLOR32 | ILC_MASK, 6, 0);
			HBITMAP tbbuttons =
			 LoadBitmap(g_instance, MAKEINTRESOURCE(IDB_TBBUTTONS));
			ImageList_AddMasked(il, tbbuttons, RGB(255, 0, 255));
			DeleteObject(tbbuttons);
			SendMessage(htb, TB_SETIMAGELIST, (WPARAM)0, (LPARAM)il);
			TBBUTTON tbButtons[3] = {
			 { 0, -1, TBSTATE_ENABLED, 
			  TBSTYLE_BUTTON|TBSTYLE_AUTOSIZE, {0}, 0, (INT_PTR)"Update Lists" },
			 { 1, -1, TBSTATE_ENABLED, 
			  TBSTYLE_BUTTON|TBSTYLE_AUTOSIZE, {0}, 0, (INT_PTR)"Mark All Upgrades"},
			 { 2, -1, 0, 
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

			HWND hcl = GetDlgItem(hwndDlg, IDC_CATLIST);
			SendMessage(hcl, LB_ADDSTRING, 0, (LPARAM)"All");
			SendMessage(hcl, LB_SETCURSEL, 0, 0);

			HWND hlv = GetDlgItem(hwndDlg, IDC_COMPLIST);
			ListView_SetExtendedListViewStyle(hlv,
			 LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP);
			il = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 10, 0);
			HBITMAP buttonsbmp =
			 LoadBitmap(g_instance, MAKEINTRESOURCE(IDB_STATES));
			ImageList_AddMasked(il, buttonsbmp, RGB(255, 0, 255));
			DeleteObject(buttonsbmp);
			(void)ListView_SetImageList(hlv, il, LVSIL_SMALL);
			SIZE sz;
			sz.cx = 100;
			HDC lvdc = GetDC(hlv);
			GetTextExtentPoint32(lvdc, "sample string", 13, &sz);
			ReleaseDC(hlv, lvdc);
			InsertColumn(hlv, "", 0, LVCFMT_LEFT, 25);
			InsertColumn(hlv, "Package", 1, LVCFMT_LEFT, sz.cx);
			InsertColumn(hlv, "Installed Version", 2, LVCFMT_LEFT, 0);
			InsertColumn(hlv, "Latest Version", 3, LVCFMT_LEFT, 0);
			InsertColumn(hlv, "Size", 4, LVCFMT_RIGHT, 0);
			InsertColumn(hlv, "Description", 5, LVCFMT_LEFT, sz.cx * 1.8);

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
			SelectInst(g_instance, hwndDlg);
			return TRUE;
		case IDC_CATLIST:
			if (HIWORD(wParam) == LBN_SELCHANGE)
			{
				UI_OnCategoryChange(ListBox_GetCurSel((HWND)lParam), hwndDlg);
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
					 (char*)Pkg_GetSubItemText(((NMLVDISPINFO*)lParam)->item.lParam,
					  ((NMLVDISPINFO*)lParam)->item.iSubItem);
				}
				return 0;
			case LVN_COLUMNCLICK:
				UI_SortListView(((LPNMLISTVIEW)lParam)->iSubItem, hwndDlg);
				return 0;
			case LVN_ITEMCHANGED:
				if ((((LPNMLISTVIEW)lParam)->uChanged & LVIF_STATE)
				 && (((LPNMLISTVIEW)lParam)->uNewState & LVIS_SELECTED))
					UI_OnListViewSelect(((LPNMLISTVIEW)lParam)->iItem, hwndDlg);
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
						UI_OnStateCycle(lvhti.iItem, hwndDlg);
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
						 Pkg_GetInstalledVersion(lvitem.lParam);
						if (instv)
							AppendMenu(menu, MF_SEPARATOR, -1, 0);
						else
						{
							int act = Pkg_GetSelectedAction(lvitem.lParam);
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
							Pkg_SelectAction(lvitem.lParam,
							 ACT_INSTALL_VERSION);
							break;
						case 10000 + ACT_NO_CHANGE:
							Pkg_SelectAction(lvitem.lParam, ACT_NO_CHANGE);
							break;
						}
						lvitem.mask = LVIF_IMAGE;
						lvitem.iImage = Pkg_GetStateImage(lvitem.lParam);
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


int SashWnd_RegisterClasses(HINSTANCE);

HWND CreateMainWnd(HINSTANCE hInstance)
{
	g_instance = hInstance;

	InitCommonControls();
	OleInitialize(0);

	if (!SashWnd_RegisterClasses(hInstance))
		return 0;

	g_haccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDA_MAINACCEL));

	HWND wnd = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_MAINDLG), 0,
     MainWndProc);
	if (!wnd)
		return 0;
	ShowWindow(wnd, SW_SHOW);
	if (ProcessQueuedMessages(wnd))
		return 0;

	return wnd;
}


int MainMessageLoop(HWND wnd)
{
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(wnd, g_haccel, &msg))
		{
			if (!IsDialogMessage(wnd, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
	return msg.wParam;
}
