/** \file categorytree.c
 *
 * Created: JohnE, 2009-01-03
 */


#include "categorytree.hh"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include "mainwnd.hh"
#include "packagelist.hh"
#include "pkgindex.hh"
#include "resource.h"


void CategoryTree_Reload()
{
	TreeView_DeleteAllItems(GetDlgItem(g_hmainwnd, IDC_CATLIST));

	TVINSERTSTRUCT tvins;
	tvins.item.mask = TVIF_TEXT | TVIF_PARAM;
	tvins.item.pszText = (CHAR*)"All Packages";
	tvins.item.cchTextMax = 199;
	tvins.item.lParam = 0;
	tvins.hInsertAfter = TVI_LAST;
	tvins.hParent = TVI_ROOT;
	SendMessage(
	 GetDlgItem(g_hmainwnd, IDC_CATLIST),
	 TVM_INSERTITEM,
	 0,
	 (LPARAM)(LPTVINSERTSTRUCT)&tvins
	 );

	int i;
	for (i = 0; i < PkgIndex_NumHeadings(); ++i)
	{
		tvins.item.pszText = (CHAR*)PkgIndex_GetHeading(i);
		tvins.item.cchTextMax = 199;
		tvins.item.lParam = -1;
		tvins.hInsertAfter = TVI_LAST;
		tvins.hParent = TVI_ROOT;
		HTREEITEM hitem = (HTREEITEM)SendMessage(
		 GetDlgItem(g_hmainwnd, IDC_CATLIST), TVM_INSERTITEM, 0,
		 (LPARAM)(LPTVINSERTSTRUCT)&tvins
		 );
		const int* j;
		for (j = PkgIndex_GetHeadingChildren(i); *j != -1; ++j)
		{
			tvins.item.pszText = (CHAR*)PkgIndex_GetCategory(*j);
			tvins.item.cchTextMax = 199;
			tvins.item.lParam = *j + 1;
			tvins.hInsertAfter = TVI_LAST;
			tvins.hParent = hitem;
			SendMessage(GetDlgItem(g_hmainwnd, IDC_CATLIST), TVM_INSERTITEM,
			 0, (LPARAM)(LPTVINSERTSTRUCT)&tvins);
		}
	}
	int cats = -1;
	PackageList_SetCategories(&cats);
}


int CategoryTree_WM_NOTIFY
 (HWND hwndDlg,
  WPARAM wParam,
  LPARAM lParam,
  int* return_value_out __attribute__((unused)))
{
	switch (((LPNMHDR)lParam)->code)
	{
	case TVN_SELCHANGED:
		if (((LPNMTREEVIEW)lParam)->itemNew.lParam < 0)
		{
			HWND htv = GetDlgItem(hwndDlg, IDC_CATLIST);
			int childct = 0;
			HTREEITEM child = TreeView_GetChild(htv,
			 ((LPNMTREEVIEW)lParam)->itemNew.hItem);
			for (; child; child = TreeView_GetNextSibling(htv, child))
				++childct;
			int cats[childct + 1];
			cats[childct] = -1;
			for (child = TreeView_GetChild(htv,
			 ((LPNMTREEVIEW)lParam)->itemNew.hItem);
			 child;
			 child = TreeView_GetNextSibling(htv, child))
			{
				TVITEM tvitem;
				tvitem.hItem = child;
				tvitem.mask = TVIF_PARAM;
				if (TreeView_GetItem(htv, &tvitem))
					cats[childct - 1] = tvitem.lParam - 1;
				else
					cats[childct - 1] = -1;
				--childct;
			}
			PackageList_SetCategories(cats);
		}
		else if (((LPNMTREEVIEW)lParam)->itemNew.lParam == 0)
		{
			int cats = -1;
			PackageList_SetCategories(&cats);
		}
		else
		{
			int cats[] = {((LPNMTREEVIEW)lParam)->itemNew.lParam - 1, -1};
			PackageList_SetCategories(cats);
		}
		break;
	}
	return 0;
}
