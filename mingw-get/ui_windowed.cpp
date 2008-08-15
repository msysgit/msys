
#include "ui.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <climits>
#include "resource.h"
#include "package.hpp"


static HWND g_hmainwnd = 0;


extern "C" HWND CreateMainWnd(HINSTANCE);
extern "C" int MainMessageLoop(HWND);
extern "C" void SelectInst(HINSTANCE, HWND);

extern "C" int UI_Windowed(HINSTANCE hinstance)
{
	g_hmainwnd = CreateMainWnd(hinstance);
	if (!g_hmainwnd)
		return 1;

	SelectInst(hinstance, g_hmainwnd);

	return MainMessageLoop(g_hmainwnd);
}


void UI::NotifyNewPackages()
{
	int ct = ListBox_GetCount(GetDlgItem(g_hmainwnd, IDC_CATLIST));
	for (; ct > 1; --ct)
		ListBox_DeleteString(GetDlgItem(g_hmainwnd, IDC_CATLIST), ct - 1);
	ListView_DeleteAllItems(GetDlgItem(g_hmainwnd, IDC_COMPLIST));
}


void UI::NotifyNewCategory(const char* cname)
{
	ListBox_AddString(GetDlgItem(g_hmainwnd, IDC_CATLIST), cname);
}


void UI::NotifyNewPackage(const Package& pkg)
{
	HWND hlist = GetDlgItem(g_hmainwnd, IDC_COMPLIST);

	LVITEM lvi;
	lvi.mask = LVIF_TEXT | LVIF_PARAM;
	lvi.iItem = INT_MAX;
	lvi.iSubItem = 0;
	char emptystr = 0;
	lvi.pszText = &emptystr;
	lvi.lParam = reinterpret_cast< LPARAM >(&pkg);
	lvi.iItem = ListView_InsertItem(hlist, &lvi);

	lvi.pszText = LPSTR_TEXTCALLBACK;
	for (int i = 1; i <= 5; ++i)
	{
		lvi.iSubItem = i;
		ListView_SetItem(hlist, &lvi);
	}
}
