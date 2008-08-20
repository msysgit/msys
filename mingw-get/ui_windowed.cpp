
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
	lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
	lvi.iItem = INT_MAX;
	lvi.iSubItem = 0;
	char emptystr = 0;
	lvi.pszText = &emptystr;
	lvi.iImage = 1;
	lvi.lParam = reinterpret_cast< LPARAM >(&pkg);
	lvi.iItem = ListView_InsertItem(hlist, &lvi);
	lvi.pszText = LPSTR_TEXTCALLBACK;
	for (int i = 1; i <= 5; ++i)
	{
		lvi.iSubItem = i;
		ListView_SetItem(hlist, &lvi);
	}
	if (lvi.iItem == 0)
	{
		ListView_SetItemState(GetDlgItem(g_hmainwnd, IDC_COMPLIST), 0,
		 LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	}
}


extern "C" void UI_NotifyCategoryChange(int sel)
{
	ListView_DeleteAllItems(GetDlgItem(g_hmainwnd, IDC_COMPLIST));
	bool have_item = false;
	if (sel == 0)
	{
		for (InstManager::PackageIter it = InstManager::Packages_Begin();
		 it != InstManager::Packages_End();
		 ++it)
		{
			LVAddPackage(*(it->second));
			have_item = true;
		}
	}
	else
	{
		for (InstManager::PackageIter it = InstManager::Packages_Begin();
		 it != InstManager::Packages_End();
		 ++it)
		{
			if (it->second->m_categories.count(sel - 1) > 0)
			{
				LVAddPackage(*(it->second));
				have_item = true;
			}
		}
	}
	if (have_item)
	{
		ListView_SetItemState(GetDlgItem(g_hmainwnd, IDC_COMPLIST), 0,
		 LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	}
	else
		Edit_SetText(GetDlgItem(g_hmainwnd, IDC_FULLDESC), "");
}


void DescWnd_SetDescription(const std::string&);

extern "C" void UI_OnListViewSelect(int sel)
{
	LVITEM lvitem;
	lvitem.iItem = sel;
	lvitem.mask = LVIF_PARAM;
	ListView_GetItem(GetDlgItem(g_hmainwnd, IDC_COMPLIST), &lvitem);
	Edit_SetText(GetDlgItem(g_hmainwnd, IDC_FULLDESC),
	 reinterpret_cast< Package* >(lvitem.lParam)->m_description.c_str());
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
	case 2:
		ret = VersionCompare(
		 reinterpret_cast< Package* >(lp1)->m_installed_version.c_str(),
		 reinterpret_cast< Package* >(lp2)->m_installed_version.c_str()
		 );
		break;
	case 3:
		ret = VersionCompare(
		 reinterpret_cast< Package* >(lp1)->m_stable_version.c_str(),
		 reinterpret_cast< Package* >(lp2)->m_stable_version.c_str()
		 );
		break;
	};
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


void UI::ResetLists()
{
	int ct = ListBox_GetCount(GetDlgItem(g_hmainwnd, IDC_CATLIST));
	for (; ct > 1; --ct)
		ListBox_DeleteString(GetDlgItem(g_hmainwnd, IDC_CATLIST), ct - 1);
	ListView_DeleteAllItems(GetDlgItem(g_hmainwnd, IDC_COMPLIST));
}


void UI::NotifyNewCategory(const char* name)
{
	ListBox_AddString(GetDlgItem(g_hmainwnd, IDC_CATLIST), name);
}


void UI::NotifyNewPackage(const Package& pkg)
{
	int sel = ListBox_GetCurSel(GetDlgItem(g_hmainwnd, IDC_CATLIST));
	if (sel <= 0 || pkg.m_categories.count(sel - 1) > 0)
		LVAddPackage(pkg);
}
