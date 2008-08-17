
#include "ui.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <climits>
#include <list>
#include "resource.h"
#include "package.hpp"
#include "inst_manager.hpp"


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


static void LVAddPackage(const Package& pkg)
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


extern "C" void UI_NotifyCategoryChange(int sel)
{
	ListView_DeleteAllItems(GetDlgItem(g_hmainwnd, IDC_COMPLIST));
	if (sel == 0)
	{
		for (InstManager::PackageIter it = InstManager::Packages_Begin();
		 it != InstManager::Packages_End();
		 ++it)
			LVAddPackage(*(it->second));
	}
	else
	{
		for (InstManager::PackageIter it = InstManager::Packages_Begin();
		 it != InstManager::Packages_End();
		 ++it)
		{
			if (it->second->m_category == sel - 1)
				LVAddPackage(*(it->second));
		}
	}
}


struct LVSortType
{
	int m_field;
	int m_reverse;

	LVSortType(int field, bool reverse)
	 : m_field(field),
	 m_reverse(reverse ? -1 : 1)
	{
	}
};

extern "C" int VersionCompare(const char*, const char*);

int CALLBACK LVSortCompare(LPARAM lp1, LPARAM lp2, LPARAM lpsort)
{
	LVSortType* st = reinterpret_cast< LVSortType* >(lpsort);
	int ret = 0;
	switch (st->m_field)
	{
	case 1:
		ret = strcmp(reinterpret_cast< Package* >(lp1)->m_id.c_str(),
		 reinterpret_cast< Package* >(lp2)->m_id.c_str());
		break;
	case 3:
		ret = VersionCompare(
		 reinterpret_cast< Package* >(lp1)->m_latest_version.c_str(),
		 reinterpret_cast< Package* >(lp2)->m_latest_version.c_str()
		 );
		break;
	};
	//Ensure an exact reverse sort
	if (ret == 0 && st->m_reverse == -1)
		return 1;
	return ret * st->m_reverse;
};


extern "C" void UI_SortListView(int column)
{
	static int cur_column = 0;

	if (cur_column == column)
		cur_column = column + 6;
	else
		cur_column = column;

	LVSortType st(cur_column % 6, (cur_column >= 6));
	ListView_SortItems(GetDlgItem(g_hmainwnd, IDC_COMPLIST), LVSortCompare,
	 reinterpret_cast< LPARAM >(&st));
}


void UI::NotifyNewPackages()
{
	int ct = ListBox_GetCount(GetDlgItem(g_hmainwnd, IDC_CATLIST));
	for (; ct > 1; --ct)
		ListBox_DeleteString(GetDlgItem(g_hmainwnd, IDC_CATLIST), ct - 1);
	ListView_DeleteAllItems(GetDlgItem(g_hmainwnd, IDC_COMPLIST));

	for (ct = 0; ct < InstManager::NumCategories(); ++ct)
	{
		ListBox_AddString(GetDlgItem(g_hmainwnd, IDC_CATLIST),
		 InstManager::GetCategory(ct));
	}

	for (InstManager::PackageIter it = InstManager::Packages_Begin();
	 it != InstManager::Packages_End();
	 ++it)
		LVAddPackage(*(it->second));
}
