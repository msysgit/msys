
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <malloc.h>
#include <ole2.h>
#include "resource.h"


static int g_vert_grip_x = 150;
static float g_horz_grip_prop = 0.5f;
static HACCEL g_haccel;
static HINSTANCE g_instance = 0;

static const char* state_items[] = {
	"Installed",
	"Not Installed",
	0
};

static void InsertColumn(HWND hlv, const char* txt, int index, int fmt)
{
	int tlen = strlen(txt);
	if (tlen >= 200)
		return;
	char* tcopy = malloc(tlen + 1);
	strcpy(tcopy, txt);
	SIZE sz;
	if (!GetTextExtentPoint32(GetDC(hlv), tcopy, tlen, &sz))
		return;
	LVCOLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
	lvc.fmt = fmt;
	lvc.cx = sz.cx + 15;
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
const char* PackageGetSubItemText(LPARAM, int);

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
			InsertColumn(hlv, "S", 0, LVCFMT_LEFT);
			InsertColumn(hlv, "Package", 1, LVCFMT_LEFT);
			InsertColumn(hlv, "Installed Version", 2, LVCFMT_LEFT);
			InsertColumn(hlv, "Latest Version", 3, LVCFMT_LEFT);
			InsertColumn(hlv, "Size", 4, LVCFMT_RIGHT);
			InsertColumn(hlv, "Description", 5, LVCFMT_LEFT);
		}
		return TRUE;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		PostQuitMessage(0);
		return TRUE;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDI_FILE_EXIT:
			DestroyWindow(hwndDlg);
			PostQuitMessage(0);
			return TRUE;
		case IDI_FILE_CHANGEINST:
			SelectInst(g_instance, hwndDlg);
			return TRUE;
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == LVN_GETDISPINFO)
		{
			((NMLVDISPINFO*)lParam)->item.pszText =
			 (char*)PackageGetSubItemText(((NMLVDISPINFO*)lParam)->item.lParam,
			  ((NMLVDISPINFO*)lParam)->item.iSubItem);
			return 0;
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


static LRESULT CALLBACK DescWndProc
 (HWND hwnd,
  UINT uMsg,
  WPARAM wParam,
  LPARAM lParam)
{
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
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


HWND CreateMainWnd(HINSTANCE hInstance)
{
	g_instance = hInstance;

	InitCommonControls();
	OleInitialize(0);

	WNDCLASS wc;
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = DescWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = 0;
	wc.hCursor = 0;
	wc.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
	wc.lpszMenuName = 0;
	wc.lpszClassName = FULLDESCCLASSNAME;
	if (!RegisterClass(&wc))
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
