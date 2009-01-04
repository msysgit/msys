/** \file progressthread.c
 *
 * Created: JohnE, 2009-01-03
 */


#include "progressthread.hh"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include "resource.h"
#include "error.hh"
#include "mainwnd.hh"
#include "winmain.hh"


struct ProgressThreadInfoStruct
{
	HWND hprogressdlg;
	const char* dlgtitle;
	int (*thread_func)(ProgressThreadInfo*, void*);
	void* thread_func_param;
	int finished;
	int result;
	int cancel_signal;
};


DWORD WINAPI ProgressThreadFunc(LPVOID param)
{
	int result = ((ProgressThreadInfo*)param)->thread_func(param,
	 ((ProgressThreadInfo*)param)->thread_func_param);
	PostMessage(((ProgressThreadInfo*)param)->hprogressdlg, WM_USER + 2012, 0,
	 result);
	return 0;
}


static BOOL CALLBACK ProgressThreadDlgProc
 (HWND hwndDlg,
  UINT uMsg,
  WPARAM wParam,
  LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			SetWindowLong(hwndDlg, GWL_USERDATA, lParam);
			ProgressThreadInfo* info = (ProgressThreadInfo*)lParam;
			SetWindowText(hwndDlg, info->dlgtitle);
			ShowWindow(GetDlgItem(hwndDlg, IDC_ACTIONLIST), FALSE);
			info->hprogressdlg = hwndDlg;
			info->finished = 0;
			info->cancel_signal = 0;
			HANDLE hthread = CreateThread(0, 0, ProgressThreadFunc, info, 0, 0);
			if (hthread)
				CloseHandle(hthread);
			else
			{
				MGError("Failed to create worker thread");
				EndDialog(hwndDlg, -1);
			}
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_SHOWDETAILS:
			{
				RECT wrc, crc;
				GetWindowRect(hwndDlg, &wrc);
				GetClientRect(hwndDlg, &crc);
				ShowWindow(GetDlgItem(hwndDlg, IDC_ACTIONTEXT), FALSE);
				ShowWindow(GetDlgItem(hwndDlg, IDC_ACTIONLIST), TRUE);
				ShowWindow(GetDlgItem(hwndDlg, IDC_SHOWDETAILS), FALSE);
				MoveWindow(hwndDlg, wrc.left, wrc.top, wrc.right - wrc.left,
				 ((float)(crc.bottom - crc.top) / 45.0f) * 150 + wrc.bottom - wrc.top - crc.bottom + crc.top,
				 TRUE);
				SetFocus(GetDlgItem(hwndDlg, IDC_PROGRESSEND));
			}
			return TRUE;
		case IDC_PROGRESSEND:
			{
				ProgressThreadInfo* info =
				 (ProgressThreadInfo*)GetWindowLong(hwndDlg, GWL_USERDATA);
				if (info->finished)
					EndDialog(hwndDlg, info->result);
				else
				{
					EnableWindow(GetDlgItem(hwndDlg, IDC_PROGRESSEND), FALSE);
					Button_SetText(GetDlgItem(hwndDlg, IDC_PROGRESSEND),
					 "Please wait...");
					info->cancel_signal = 1;
				}
			}
			return TRUE;
		}
		break;

	case WM_USER + 2012:
		{
			ProgressThreadInfo* info =
			 (ProgressThreadInfo*)GetWindowLong(hwndDlg, GWL_USERDATA);
			if (info->cancel_signal)
				EndDialog(hwndDlg, lParam);
			else
			{
				Button_SetText(GetDlgItem(hwndDlg, IDC_PROGRESSEND), "&OK");
				info->result = lParam;
				info->finished = 1;
				if (lParam == 0)
					ProgressThread_NewStatus(info, "Finished.");
				else
				{
					char status[1024];
					int i = 0;
					for (; MGGetErrors()[i]; ++i)
					{
						snprintf(status, 1024, "[Error] %s", MGGetErrors()[i]);
						ProgressThread_NewStatus(info, status);
					}
					MGClearErrors();
				}
			}
		}
		return TRUE;

	case WM_SIZE:
		{
			int cli_height = HIWORD(lParam);

			RECT btnrc;
			GetWindowRect(GetDlgItem(hwndDlg, IDC_PROGRESSEND), &btnrc);
			TransToClient(hwndDlg, &btnrc);
			MoveWindow(GetDlgItem(hwndDlg, IDC_PROGRESSEND), btnrc.left,
			 cli_height - (btnrc.bottom - btnrc.top) - 4,
			 btnrc.right - btnrc.left, btnrc.bottom - btnrc.top, TRUE);

			GetWindowRect(GetDlgItem(hwndDlg, IDC_SHOWDETAILS), &btnrc);
			TransToClient(hwndDlg, &btnrc);
			MoveWindow(GetDlgItem(hwndDlg, IDC_SHOWDETAILS), btnrc.left,
			 cli_height - (btnrc.bottom - btnrc.top) - 4,
			 btnrc.right - btnrc.left, btnrc.bottom - btnrc.top, TRUE);

			RECT rc;
			GetWindowRect(GetDlgItem(hwndDlg, IDC_ACTIONLIST), &rc);
			TransToClient(hwndDlg, &rc);
			MoveWindow(GetDlgItem(hwndDlg, IDC_ACTIONLIST), rc.left, rc.top,
			 rc.right - rc.left,
			 cli_height - (btnrc.bottom - btnrc.top) - rc.top - 16, TRUE);

			GetWindowRect(GetDlgItem(hwndDlg, IDC_STATUSGRP), &rc);
			TransToClient(hwndDlg, &rc);
			MoveWindow(GetDlgItem(hwndDlg, IDC_STATUSGRP), rc.left, rc.top,
			 rc.right - rc.left,
			 cli_height - (btnrc.bottom - btnrc.top) - rc.top - 10, TRUE);
		}
		return TRUE;
	}

	return FALSE;
}


int ProgressThread
 (const char* dialog_title,
  int (*thread_func)(ProgressThreadInfo*, void*),
  void* thread_func_param)
{
	ProgressThreadInfo info;
	info.dlgtitle = (dialog_title) ? dialog_title : "Caption";
	info.thread_func = thread_func;
	info.thread_func_param = thread_func_param;
	return DialogBoxParam(g_hinstance, MAKEINTRESOURCE(IDD_PROGRESSDLG),
	 g_hmainwnd, ProgressThreadDlgProc, (LPARAM)&info);
}


void ProgressThread_NewStatus(ProgressThreadInfo* info, const char* status)
{
	Static_SetText(GetDlgItem(info->hprogressdlg, IDC_ACTIONTEXT), status);
	(void)ListBox_AddString(GetDlgItem(info->hprogressdlg, IDC_ACTIONLIST),
	 status);
}


int ProgressThread_IsCancelled(ProgressThreadInfo* info)
{
	return info->cancel_signal;
}
