
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <shlobj.h>
#include <sys/stat.h>
#include <string>
#include <map>
#include "resource.h"
#include "error.hh"
#include "instmanager.hpp"
#include "previnstalls.hh"
#include "ui.hh"
#include "winmain.hh"
#include "mainwnd.hh"


char g_inst_loc[MAX_PATH] = {0};


static HWND g_hdlg = 0;
static BOOL g_createinst = FALSE;

struct CmpStrI
{
	bool operator () (const std::string& s1, const std::string& s2)
	{
		return (stricmp(s1.c_str(), s2.c_str()) < 0);
	}
};
typedef std::map< std::string, int, CmpStrI > InstIndexMap;
static InstIndexMap prev_insts;


static void ValidateInstPath(HWND hdlg, const std::string& path)
{
	int valid = 0;
	if (path.length() > 0)
	{
		size_t i = path.find_last_of("/\\", path.length() - 2);
		if (path[1] == ':' && i != std::string::npos)
		{
			std::string pdir = (i >= 4) ? path.substr(0, i) : path.substr(0, i + 1);
			struct _stat st;
			if (_stat(pdir.c_str(), &st) == 0
			 && _S_ISDIR(st.st_mode))
			{
				std::string dpath = path;
				if (dpath[dpath.length() - 1] == '/'
				 || dpath[dpath.length() - 1] == '\\')
					dpath.erase(dpath.length() - 1);
				if (_stat(dpath.c_str(), &st) == 0)
				{
					if (_S_ISDIR(st.st_mode))
					{
						if (InstManager::InstExistsAt(dpath))
						{
							valid = 1;
							g_createinst = FALSE;
							Static_SetText(GetDlgItem(hdlg, IDC_VALTEXT),
							 "This is an existing installation");
							Button_SetCheck(GetDlgItem(hdlg, IDC_EXISTMANAGE),
							 BST_CHECKED);
							EnableWindow(GetDlgItem(hdlg, IDC_PREVINSTLIST),
							 TRUE);
							InstIndexMap::iterator found =
							 prev_insts.find(dpath);
							if (found != prev_insts.end())
							{
								(void)ListBox_SetCurSel(
								 GetDlgItem(hdlg, IDC_PREVINSTLIST),
								 found->second
								 );
							}
							else
							{
								(void)ListBox_SetCurSel(
								 GetDlgItem(hdlg, IDC_PREVINSTLIST), -1
								 );
							}
						}
						else
						{
							valid = 1;
							g_createinst = TRUE;
							Button_SetCheck(GetDlgItem(hdlg, IDC_EXISTMANAGE),
							 BST_UNCHECKED);
							EnableWindow(GetDlgItem(hdlg, IDC_PREVINSTLIST),
							 FALSE);
							Static_SetText(GetDlgItem(hdlg, IDC_VALTEXT),
							 "This path exists");
						}
					}
					else
					{
						Static_SetText(GetDlgItem(hdlg, IDC_VALTEXT),
						 "This path is not a directory");
					}
				}
				else
				{
					valid = 1;
					g_createinst = TRUE;
					Button_SetCheck(GetDlgItem(hdlg, IDC_EXISTMANAGE),
					 BST_UNCHECKED);
					EnableWindow(GetDlgItem(hdlg, IDC_PREVINSTLIST), FALSE);
					Static_SetText(GetDlgItem(hdlg, IDC_VALTEXT),
					 "This directory will be created");
				}
			}
			else
			{
				Static_SetText(GetDlgItem(hdlg, IDC_VALTEXT),
				 "The parent directory doesn't exist");
			}
		}
		else
		{
			Static_SetText(GetDlgItem(hdlg, IDC_VALTEXT),
			 "Not a valid installation path");
		}
	}
	else
	{
		Static_SetText(GetDlgItem(hdlg, IDC_VALTEXT),
		 "Please enter an installation path");
	}
	EnableWindow(GetDlgItem(hdlg, IDOK), valid ? TRUE : FALSE);
}


extern "C" void PrevInstCallback(const char* path)
{
	int index = ListBox_AddString(GetDlgItem(g_hdlg, IDC_PREVINSTLIST), path);
	prev_insts[path] = index;
}

static int CALLBACK InstBrowseProc
 (HWND hwnd,
  UINT uMsg,
  LPARAM lParam,
  LPARAM lpData)
{
	if (uMsg == BFFM_INITIALIZED)
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
	return 0;
}

static BOOL CALLBACK InstSelProc
 (HWND hwndDlg,
  UINT uMsg,
  WPARAM wParam,
  LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			g_hdlg = hwndDlg;
			if (!g_inst_loc[0])
				strcpy(g_inst_loc, "C:\\mingwgettest");
			if (GetPrevInstalls(PrevInstCallback) > 0)
			{
				SendMessage(GetDlgItem(hwndDlg, IDC_PREVINSTLIST), LB_SETCURSEL,
				 0, 0);
				SendMessage(hwndDlg, WM_COMMAND,
				 MAKEWPARAM(IDC_PREVINSTLIST, LBN_SELCHANGE),
				 (LPARAM)GetDlgItem(hwndDlg, IDC_PREVINSTLIST));
				EnableWindow(GetDlgItem(hwndDlg, IDC_PREVINSTLIST), TRUE);
			}
			else
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_PREVINSTLIST), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);
				Edit_SetText(GetDlgItem(hwndDlg, IDC_INSTPATH), g_inst_loc);
			}
			SetFocus(GetDlgItem(hwndDlg, IDC_INSTPATH));
		}
		return FALSE;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			{
				g_inst_loc[0] = 0;
				Edit_GetText(GetDlgItem(hwndDlg, IDC_INSTPATH), g_inst_loc,
				 MAX_PATH);
				EndDialog(hwndDlg, 1);
			}
			return TRUE;
		case IDCANCEL:
			EndDialog(hwndDlg, 0);
			return TRUE;
		case IDC_EXISTMANAGE:
			EnableWindow(GetDlgItem(hwndDlg, IDC_PREVINSTLIST),
			 Button_GetCheck((HWND)lParam) == BST_CHECKED);
			return TRUE;
		case IDC_BROWSENEWINST:
			{
				char path[MAX_PATH];
				path[0] = 0;
				Edit_GetText(GetDlgItem(hwndDlg, IDC_INSTPATH), path, MAX_PATH);
				if (!path[0])
					strcpy(path, "C:\\");
				BROWSEINFO bi;
				bi.hwndOwner = hwndDlg;
				bi.pidlRoot = 0;
				bi.pszDisplayName = 0;
				bi.lpszTitle = "Select a Folder";
				bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
				bi.lpfn = InstBrowseProc;
				bi.lParam = reinterpret_cast< LPARAM >(path);
				bi.iImage = 0;
				LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
				if (pidl)
				{
					char cpath[MAX_PATH];
					if (SHGetPathFromIDList(pidl, cpath))
					{
						std::string path = cpath;
						if (path.length() < 5
						 || stricmp(path.c_str() + path.length() - 5, "mingw")
						  != 0)
						{
							if (path[path.length() - 1] != '\\')
								path += '\\';
							Edit_SetText(GetDlgItem(hwndDlg, IDC_INSTPATH),
							 (path + "MinGW").c_str());
						}
						else
						{
							Edit_SetText(GetDlgItem(hwndDlg, IDC_INSTPATH),
							 path.c_str());
						}
					}
					CoTaskMemFree(pidl);
				}
			}
			return TRUE;
		case IDC_INSTPATH:
			if (HIWORD(wParam) == EN_CHANGE)
			{
				char path[MAX_PATH];
				path[0] = 0;
				Edit_GetText((HWND)lParam, path, MAX_PATH);
				ValidateInstPath(hwndDlg, path);
				return TRUE;
			}
			break;
		case IDC_PREVINSTLIST:
			if (HIWORD(wParam) == LBN_SELCHANGE)
			{
				int sel = ListBox_GetCurSel((HWND)lParam);
				int len = ListBox_GetTextLen((HWND)lParam, sel);
				char* txt = (char*)malloc(len + 1);
				if (ListBox_GetText((HWND)lParam, sel, txt) > 0)
					Edit_SetText(GetDlgItem(hwndDlg, IDC_INSTPATH), txt);
				free(txt);
				return TRUE;
			}
			break;
		}
		break;

	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		return TRUE;
	}

    return FALSE;
}


extern "C" void SelectInstallation()
{
	if (!DialogBox(g_hinstance, MAKEINTRESOURCE(IDD_INSTSEL), g_hmainwnd,
	 InstSelProc) || !g_inst_loc[0])
		return;
	if (!InstManager::Load(g_inst_loc))
	{
		LastError_MsgBox("Installation Failure");
		return;
	}
	char title[1024];
	snprintf(title, 1024, "mingw-get: %s", g_inst_loc);
	SetWindowText(g_hmainwnd, title);
}
