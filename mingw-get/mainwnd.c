
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <malloc.h>
#include <ole2.h>
#include "resource.h"


#define ID_SHOW_STABLE 10001
#define ID_SHOW_UNSTABLE 10002


static int g_vert_grip_x = 150;
static float g_horz_grip_prop = 0.5f;
static HACCEL g_haccel;
static HINSTANCE g_instance = 0;


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


static void TransToClient(HWND hwnd, RECT* rc)
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


void SelectInst(HINSTANCE, HWND);
const char* Pkg_GetSubItemText(LPARAM, int);
void UI_NotifyCategoryChange(int);
void UI_SortListView(int);
void UI_OnListViewSelect(int);
void DescWnd_SetHWND(HWND);
int Pkg_UnstableShown(LPARAM);
void Pkg_SetUnstableShown(LPARAM, int);
void InstMgr_SetAllPkgsShowUnstable(int);

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
			HIMAGELIST hImageList = ImageList_Create(16, 16,
			 ILC_COLOR16 | ILC_MASK, 3, 0);
			SendMessage(htb, TB_SETIMAGELIST, (WPARAM)0, (LPARAM)hImageList);
			SendMessage(htb, TB_LOADIMAGES, (WPARAM)IDB_STD_SMALL_COLOR,
			 (LPARAM)HINST_COMMCTRL);
			TBBUTTON tbButtons[3] = {
			 { MAKELONG(STD_FILENEW, 0), -1, TBSTATE_ENABLED, 
			  TBSTYLE_BUTTON|TBSTYLE_AUTOSIZE, {0}, 0, (INT_PTR)"Update Lists" },
			 { MAKELONG(STD_FILEOPEN, 0), -1, TBSTATE_ENABLED, 
			  TBSTYLE_BUTTON|TBSTYLE_AUTOSIZE, {0}, 0, (INT_PTR)"Mark All Upgrades"},
			 { MAKELONG(STD_FILESAVE, 0), -1, 0, 
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
			SendMessage(hcb, CB_SETCURSEL, 0, 0);

			HWND hcl = GetDlgItem(hwndDlg, IDC_CATLIST);
			SendMessage(hcl, LB_ADDSTRING, 0, (LPARAM)"All");
			SendMessage(hcl, LB_SETCURSEL, 0, 0);

			HWND hlv = GetDlgItem(hwndDlg, IDC_COMPLIST);
			ListView_SetExtendedListViewStyle(hlv,
			 LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP);
			HIMAGELIST il =
			 ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 10, 0);
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
			InsertColumn(hlv, "S", 0, LVCFMT_LEFT, 0);
			InsertColumn(hlv, "Package", 1, LVCFMT_LEFT, sz.cx);
			InsertColumn(hlv, "Installed Version", 2, LVCFMT_LEFT, 0);
			InsertColumn(hlv, "Latest Version", 3, LVCFMT_LEFT, 0);
			InsertColumn(hlv, "Size (DL)", 4, LVCFMT_RIGHT, 0);
			InsertColumn(hlv, "Description", 5, LVCFMT_LEFT, sz.cx);
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
		case IDM_VIEW_SHOWSTABLE:
			InstMgr_SetAllPkgsShowUnstable(0);
			if (ListView_GetItemCount(GetDlgItem(hwndDlg, IDC_COMPLIST)) > 0)
			{
				ListView_RedrawItems(GetDlgItem(hwndDlg, IDC_COMPLIST), 0,
				 ListView_GetItemCount(GetDlgItem(hwndDlg, IDC_COMPLIST)) - 1);
				UpdateWindow(GetDlgItem(hwndDlg, IDC_COMPLIST));
			}
			return TRUE;
		case IDM_VIEW_SHOWUNSTABLE:
			InstMgr_SetAllPkgsShowUnstable(1);
			if (ListView_GetItemCount(GetDlgItem(hwndDlg, IDC_COMPLIST)) > 0)
			{
				ListView_RedrawItems(GetDlgItem(hwndDlg, IDC_COMPLIST), 0,
				 ListView_GetItemCount(GetDlgItem(hwndDlg, IDC_COMPLIST)) - 1);
				UpdateWindow(GetDlgItem(hwndDlg, IDC_COMPLIST));
			}
			return TRUE;
		case IDC_CATLIST:
			if (HIWORD(wParam) == LBN_SELCHANGE)
			{
				UI_NotifyCategoryChange(ListBox_GetCurSel((HWND)lParam));
				return TRUE;
			}
			break;
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == NM_RCLICK &&
		 ((LPNMHDR)lParam)->hwndFrom ==
		  ListView_GetHeader(GetDlgItem(hwndDlg, IDC_COMPLIST)))
		{
			DWORD dwpos = GetMessagePos();
			HD_HITTESTINFO hdhti;
			hdhti.pt.x = GET_X_LPARAM(dwpos);
			hdhti.pt.y = GET_Y_LPARAM(dwpos);
			ScreenToClient(((LPNMHDR)lParam)->hwndFrom, &hdhti.pt);
			int hit = SendMessage(((LPNMHDR)lParam)->hwndFrom, HDM_HITTEST, 0,
			 (LPARAM)&hdhti);
			if ((hdhti.flags & HHT_ONHEADER) && hit == 3)
			{
				HMENU menu = CreatePopupMenu();
				AppendMenu(menu, MF_STRING, IDM_VIEW_SHOWSTABLE,
				 "Show only &stable versions\tCtrl+Shift+S");
				AppendMenu(menu, MF_STRING, IDM_VIEW_SHOWUNSTABLE,
				 "Show &unstable versions\tCtrl+Shift+U");
				TrackPopupMenuEx(menu,
				 TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON,
				 GET_X_LPARAM(dwpos), GET_Y_LPARAM(dwpos), hwndDlg, 0);
				DestroyMenu(menu);
				return 0;
			}
			break;
		}
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
				UI_SortListView(((LPNMLISTVIEW)lParam)->iSubItem);
				return 0;
			case LVN_ITEMCHANGED:
				if ((((LPNMLISTVIEW)lParam)->uChanged & LVIF_STATE)
				 && (((LPNMLISTVIEW)lParam)->uNewState & LVIS_SELECTED))
					UI_OnListViewSelect(((LPNMLISTVIEW)lParam)->iItem);
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
						lvitem.mask = LVIF_PARAM;
						ListView_GetItem(((LPNMHDR)lParam)->hwndFrom, &lvitem);
						if (Pkg_UnstableShown(lvitem.lParam))
						{
							AppendMenu(menu, MF_STRING, ID_SHOW_STABLE,
							 "Show &stable version");
						}
						else
						{
							AppendMenu(menu, MF_STRING, ID_SHOW_UNSTABLE,
							 "Show &unstable version");
						}
						int ret = TrackPopupMenuEx(menu,
						 TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NONOTIFY |
						  TPM_RETURNCMD | TPM_RIGHTBUTTON,
						 GET_X_LPARAM(dwpos), GET_Y_LPARAM(dwpos),
						 ((LPNMHDR)lParam)->hwndFrom, 0);
						switch (ret)
						{
						case ID_SHOW_STABLE:
							Pkg_SetUnstableShown(lvitem.lParam, 0);
							ListView_RedrawItems(((LPNMHDR)lParam)->hwndFrom,
							 hit, hit);
							UpdateWindow(((LPNMHDR)lParam)->hwndFrom);
							break;
						case ID_SHOW_UNSTABLE:
							Pkg_SetUnstableShown(lvitem.lParam, 1);
							ListView_RedrawItems(((LPNMHDR)lParam)->hwndFrom,
							 hit, hit);
							UpdateWindow(((LPNMHDR)lParam)->hwndFrom);
							break;
						}
						DestroyMenu(menu);
					}
				}
				return 0;
			case NM_CUSTOMDRAW:
				{
					static COLORREF old_color;
					LRESULT ret = CDRF_DODEFAULT;
					switch (((LPNMCUSTOMDRAW)lParam)->dwDrawStage)
					{
					case CDDS_PREPAINT:
						ret = CDRF_NOTIFYITEMDRAW;
						break;
					case CDDS_ITEMPREPAINT:
						if (Pkg_UnstableShown(
						 ((LPNMCUSTOMDRAW)lParam)->lItemlParam
						 ))
						{
							old_color = ((LPNMLVCUSTOMDRAW)lParam)->clrText;
							((LPNMLVCUSTOMDRAW)lParam)->clrText =
							 RGB(255, 128, 0);
							HFONT hFont =
							 (HFONT)SendMessage(((LPNMHDR)lParam)->hwndFrom,
							  WM_GETFONT, 0, 0);
							LOGFONT lf = {0};
							GetObject(hFont, sizeof(LOGFONT), &lf);
							lf.lfWeight |= FW_BOLD;
							HFONT hFontBold = CreateFontIndirect(&lf);
							SelectObject(((LPNMCUSTOMDRAW)lParam)->hdc,
							 hFontBold);
							ret = CDRF_NEWFONT | CDRF_NOTIFYPOSTPAINT;
						}
						else
							ret = CDRF_DODEFAULT;
						break;
					case CDDS_ITEMPOSTPAINT:
						((LPNMLVCUSTOMDRAW)lParam)->clrText = old_color;
						ret = CDRF_DODEFAULT;
					}
					SetWindowLong(hwndDlg, DWL_MSGRESULT, ret);
					return TRUE;
				}
			}
			break;
		}
		break;

	case WM_SIZE:
		{
			int cli_width = LOWORD(lParam);
			int cli_height = HIWORD(lParam);

			RECT tbrc, statrc, ctyperc, rc;

			GetWindowRect(GetDlgItem(hwndDlg, IDC_MAINTOOLBAR), &tbrc);
			TransToClient(hwndDlg, &tbrc);
			MoveWindow(GetDlgItem(hwndDlg, IDC_MAINTOOLBAR),
			 tbrc.left, tbrc.top, cli_width - tbrc.left * 2,
			 tbrc.bottom - tbrc.top, TRUE);

			GetWindowRect(GetDlgItem(hwndDlg, IDC_STATBAR), &statrc);
			TransToClient(hwndDlg, &statrc);
			MoveWindow(GetDlgItem(hwndDlg, IDC_STATBAR),
			 statrc.left, cli_height - (statrc.bottom - statrc.top),
			 cli_width - statrc.left * 2,
			 statrc.bottom - statrc.top, TRUE);

			int t = tbrc.bottom + 6;

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
			  (statrc.bottom - statrc.top) - 4, TRUE);

			int h = (cli_height - t - (statrc.bottom - statrc.top)) *
			 g_horz_grip_prop;

			MoveWindow(GetDlgItem(hwndDlg, IDC_COMPLIST),
			 g_vert_grip_x + 8, t, cli_width - g_vert_grip_x - 10, h, TRUE);

			MoveWindow(GetDlgItem(hwndDlg, IDC_FULLDESC),
			 g_vert_grip_x + 8, t + h + 8, cli_width - g_vert_grip_x - 10,
			 cli_height - t - h - (statrc.bottom - statrc.top) - 10, TRUE);
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
		if (TranslateAccelerator(wnd, g_haccel, &msg))
		{
		}
		else if (IsDialogMessage(wnd, &msg))
		{
		}
		else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return 0;
}


int DescWnd_RegisterClass(HINSTANCE);

HWND CreateMainWnd(HINSTANCE hInstance)
{
	g_instance = hInstance;

	InitCommonControls();
	OleInitialize(0);

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
		if (TranslateAccelerator(wnd, g_haccel, &msg))
		{
		}
		else if (IsDialogMessage(wnd, &msg))
		{
		}
		else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return msg.wParam;
}
