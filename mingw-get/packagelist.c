/** \file packagelist.c
 *
 * Created: JohnE, 2009-01-03
 */


#include "packagelist.hh"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <malloc.h>
#include <limits.h>
#include "pkgindex.hh"
#include "packageview.hh"
#include "mainwnd.hh"
#include "winmain.hh"
#include "versioncompare.hh"
#include "pkg_const.h"
#include "resource.h"


static HWND g_hpackagelist = 0;


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


void PackageList_Create()
{
	g_hpackagelist = GetDlgItem(g_hmainwnd, IDC_COMPLIST);
	ListView_SetExtendedListViewStyle(g_hpackagelist,
	 LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP);
	HIMAGELIST il = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 10, 0);
	HBITMAP buttonsbmp =
	 LoadBitmap(g_hinstance, MAKEINTRESOURCE(IDB_STATES));
	ImageList_AddMasked(il, buttonsbmp, RGB(255, 0, 255));
	DeleteObject(buttonsbmp);
	(void)ListView_SetImageList(g_hpackagelist, il, LVSIL_SMALL);
	SIZE sz;
	sz.cx = 100;
	HDC lvdc = GetDC(g_hpackagelist);
	GetTextExtentPoint32(lvdc, "sample string", 13, &sz);
	ReleaseDC(g_hpackagelist, lvdc);
	InsertColumn(g_hpackagelist, "", 0, LVCFMT_LEFT, 25);
	InsertColumn(g_hpackagelist, "Package", 1, LVCFMT_LEFT, sz.cx);
	InsertColumn(g_hpackagelist, "Installed Version", 2, LVCFMT_LEFT, 0);
	InsertColumn(g_hpackagelist, "Latest Version", 3, LVCFMT_LEFT, 0);
	InsertColumn(g_hpackagelist, "Size", 4, LVCFMT_RIGHT, 0);
	InsertColumn(g_hpackagelist, "Description", 5, LVCFMT_LEFT, sz.cx * 1.8);
}


struct PackageListSorter
{
	short field;
	short reverse;
};

int CALLBACK PackageListSortCompare(LPARAM lp1, LPARAM lp2, LPARAM lpsort)
{
	struct PackageListSorter* sort = (struct PackageListSorter*)lpsort;
	int ret = 0;
	switch (sort->field)
	{
	case 1:
		ret = strcmp(PkgIndex_PackageGetID(lp1), PkgIndex_PackageGetID(lp2));
		break;
	case 2:
		ret = VersionCompare(PkgIndex_PackageGetInstalledVersion(lp1),
		 PkgIndex_PackageGetInstalledVersion(lp2));
		break;
	case 3:
		ret = VersionCompare(PkgIndex_PackageGetLatestVersion(lp1),
		 PkgIndex_PackageGetLatestVersion(lp2));
		break;
	case 5:
		ret = stricmp(PkgIndex_PackageGetTitle(lp1),
		 PkgIndex_PackageGetTitle(lp2));
		break;
	};
	return ret * sort->reverse;
};

void PackageList_Sort(int column)
{
	static int cur_column = 0;

	if (cur_column == column)
		cur_column = column + 6;
	else
		cur_column = column;

	struct PackageListSorter sort;
	sort.field = cur_column % 6;
	sort.reverse = (cur_column >= 6) ? -1 : 1;
	ListView_SortItems(g_hpackagelist, PackageListSortCompare, (LPARAM)&sort);
}


void PackageList_OnSelect(int sel)
{
	LVITEM lvitem;
	lvitem.iItem = sel;
	lvitem.iSubItem = 0;
	lvitem.mask = LVIF_PARAM;
	ListView_GetItem(g_hpackagelist, &lvitem);
	PackageView_SetPackage(lvitem.lParam);
}


void PackageList_OnStateCycle(int sel)
{
	LVITEM lvitem;
	lvitem.iItem = sel;
	lvitem.iSubItem = 0;
	lvitem.mask = LVIF_PARAM;
	ListView_GetItem(g_hpackagelist, &lvitem);
	if (PkgIndex_PackageGetSelectedAction(lvitem.lParam) == ACT_INSTALL_VERSION)
		PkgIndex_PackageSelectAction(lvitem.lParam, ACT_NO_CHANGE);
	else
		PkgIndex_PackageSelectAction(lvitem.lParam, ACT_INSTALL_VERSION);
	lvitem.mask = LVIF_IMAGE;
	lvitem.iImage = PkgIndex_PackageGetStateImage(lvitem.lParam);
	ListView_SetItem(g_hpackagelist, &lvitem);
}


static void AddPackage(int package)
{
	LVITEM lvi;
	lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
	lvi.iItem = INT_MAX;
	lvi.iSubItem = 0;
	char emptystr = 0;
	lvi.pszText = &emptystr;
	lvi.iImage = PkgIndex_PackageGetStateImage(package);
	lvi.lParam = package;
	lvi.iItem = ListView_InsertItem(g_hpackagelist, &lvi);
	lvi.mask = LVIF_TEXT;
	lvi.pszText = LPSTR_TEXTCALLBACK;
	int i;
	for (i = 1; i <= 5; ++i)
	{
		lvi.iSubItem = i;
		ListView_SetItem(g_hpackagelist, &lvi);
	}
	if (lvi.iItem == 0)
	{
		ListView_SetItemState(g_hpackagelist, 0,
		 LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	}
}


void PackageList_SetCategories(const int* categories)
{
	ListView_DeleteAllItems(g_hpackagelist);

	int have_item = 0;
	if (*categories == -1)
	{
		int i = 0;
		for (; i < PkgIndex_NumPackages(); ++i)
			AddPackage(i);
		have_item = (i > 0);
	}
	else
	{
		int i = 0;
		for (; i < PkgIndex_NumPackages(); ++i)
		{
			if (PkgIndex_PackageInAnyOf(i, categories))
			{
				AddPackage(i);
				if (!have_item)
					have_item = 1;
			}
		}
	}

	if (have_item)
	{
		ListView_SetItemState(g_hpackagelist, 0, LVIS_SELECTED | LVIS_FOCUSED,
		 LVIS_SELECTED | LVIS_FOCUSED);
	}
	else
		PackageView_SetPackage(-1);
}
