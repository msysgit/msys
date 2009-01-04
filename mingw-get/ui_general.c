/** \file ui.cpp
 *
 * Created: JohnE, 2008-10-11
 */


#include "ui_general.hh"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <limits.h>
#include <stdio.h>
#include "progressthread.hh"
#include "mainwnd.hh"
#include "error.hh"
#include "getbindir.hh"
#include "download.hh"
#include "pkgindex.hh"
#include "categorytree.hh"


ProgressThreadInfo* g_dlthreadinfo = 0;


void LastError_MsgBox(const char* title)
{
	MessageBox(g_hmainwnd, MGGetErrors()[0], title, MB_OK | MB_ICONERROR);
	MGClearErrors();
}


int ListDownloadCallback(size_t total, size_t current)
{
	return ProgressThread_IsCancelled(g_dlthreadinfo) ? 1 : 0;
}

int UpdateListThread(ProgressThreadInfo* info, void* param)
{
	g_dlthreadinfo = info;

	ProgressThread_NewStatus(info, "Downloading updated manifest...");

	char localpath[PATH_MAX + 1];
	snprintf(localpath, PATH_MAX + 1, "%s\\mingw_avail.mft", GetBinDir());
	int dlresult = DownloadFile(
	 "http://localhost:1330/mingwinst/mingw_avail.mft", localpath,
	 ListDownloadCallback
	 );

	g_dlthreadinfo = 0;

	if (dlresult > 0 && dlresult != 2)
		dlresult = -dlresult;
	return dlresult;
}

void UI_UpdateLists()
{
	if (ProgressThread("Download Updated Lists", UpdateListThread, 0) != 0)
		return;
	if (!PkgIndex_Load())
		return;
	CategoryTree_Reload();
	return;
}
