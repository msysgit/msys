/** \file packageview.c
 *
 * Created: JohnE, 2009-01-03
 */


#include "packageview.hh"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include "mainwnd.hh"
#include "pkgindex.hh"
#include "pkg_const.h"
#include "resource.h"


void PackageView_SetPackage(int package)
{
	if (package < 0)
	{
		Static_SetText(GetDlgItem(g_hmainwnd, IDC_DESCTITLE), "");
		Edit_SetText(GetDlgItem(g_hmainwnd, IDC_FULLDESC), "");
		(void)ComboBox_ResetContent(GetDlgItem(g_hmainwnd, IDC_INSTVERSION));
		EnableWindow(GetDlgItem(g_hmainwnd, IDC_INSTVERSION), FALSE);
		EnableWindow(GetDlgItem(g_hmainwnd, IDC_FULLDESC), FALSE);
		return;
	}

	Edit_SetText(GetDlgItem(g_hmainwnd, IDC_FULLDESC),
	 PkgIndex_PackageGetDescription(package));
	EnableWindow(GetDlgItem(g_hmainwnd, IDC_FULLDESC), TRUE);
	Static_SetText(GetDlgItem(g_hmainwnd, IDC_DESCTITLE),
	 PkgIndex_PackageGetTitle(package));
	(void)ComboBox_ResetContent(GetDlgItem(g_hmainwnd, IDC_INSTVERSION));
	char vstr[1024];
	int i;
	for (i = 0; i < PkgIndex_PackageNumVersions(package); ++i)
	{
		if (PkgIndex_PackageVersionGetStatus(package, i) == PSTATUS_ALPHA)
		{
			snprintf(vstr, 1024, "Alpha: %s",
			 PkgIndex_PackageVersionGetString(package, i));
		}
		else
		{
			snprintf(vstr, 1024, "Stable: %s",
			 PkgIndex_PackageVersionGetString(package, i));
		}
		(void)ComboBox_AddString(GetDlgItem(g_hmainwnd, IDC_INSTVERSION), vstr);
	}
	if (i > 0)
	{
		EnableWindow(GetDlgItem(g_hmainwnd, IDC_INSTVERSION), TRUE);
		(void)ComboBox_SetCurSel(GetDlgItem(g_hmainwnd, IDC_INSTVERSION), 0);
		PkgIndex_PackageSetSelectedVersion(package, 0);
	}
	else
	{
		EnableWindow(GetDlgItem(g_hmainwnd, IDC_INSTVERSION), FALSE);
		PkgIndex_PackageSetSelectedVersion(package, -1);
	}
}
