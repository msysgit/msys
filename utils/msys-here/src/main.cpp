/********************************************************************
*	Module:	main.cpp. This is part of MSYS Here.
*
*	Purpose:	Main module.
*
*	License:	Public domain.
*			2002 Manu B.
*
*	Revisions:	0.1a
*
********************************************************************/
#include "winui.h"
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include "main.h"
#include "rsrc.h"
#include "getopt.h"

#define APP_NAME "MSYS Here"
#define INI_FILE_NAME "\\msys.ini"
#define MSYS_DVLPR "MSYSTEM=MSYS"

/* Globals */
CWinApp winApp;
extern int _argc;
extern char **_argv;
const char *rxvt = "bin\\rxvt";
const char *rxvtopts = "-sl 2500 -sr -fn Courier-12 -tn msys";
const char *rxvtexec = "/bin/sh --login -i";


/********************************************************************
*	Function:	WinMain procedure.
*
*	Purpose:	Runs the application.
*
*	Revisions:	
*
********************************************************************/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){
	unused(hInstance);
	unused(hPrevInstance);
	unused(lpCmdLine);
	unused(nCmdShow);
return winApp.Run();
}


/********************************************************************
*	Class:	CWinApp.
*
*	Purpose:	The application object.
*
*	Revisions:	
*
********************************************************************/
int CWinApp::Run(void){
	/*	Get user parameters, and run MSYS.			*/
	
	// get MSYS directory from argv and read user parameters.
	if (!_GetMsysDir(_msysDir, sizeof(_msysDir))){
		debugf("CWinApp::Run(): cannot retrieve MSYS directory!");
		return 1;
	}
	_ReadIniFile();
	

	/*	1. Check if a valid shell lives in /bin.	*/
	if (!_ShellFound(_msysDir)){
		MsgBox(0, MB_ICONERROR, APP_NAME,
			"Cannot find a shell in your MSYS environment:\n%s", _msysDir);
		return 1;
	}

	/*	2. Get option switches						*/
	_GetOpt();


	/*	3. What does the end user wish?
			- Create a dialog box to edit user parameters.
			- Run rxvt to test new parameters.
			- Exit without saving parameters.
			- Save parameters.
			- Destroy the dialog box and continue.		*/


	/*	Writes the application's data to its ini file.	*/

	_WriteIniFile();
	/*	4. Set environment variables?		*/
	
	/*	if "x%MSYSTEM%" == "x" set MSYSTEM=MINGW32
		if "%1" == "MSYS" set MSYSTEM=MSYS
		
		if NOT "x%DISPLAY%" == "x" set DISPLAY=	*/
		
	/*	5. Run MSYS?
			- Msys DVLPR?
			- Msys-here and terminate.
			- Run rxvt/sh and terminate.				*/

return (_Msys() ? 0 : 1);
}

bool CWinApp::_GetMsysDir(char *dest, size_t maxlen){
	/*	Get MSYS directory from argv.	*/
	
	char *lastsep = strrchr(_argv[0], '\\');
	if (lastsep){
		size_t len = lastsep-_argv[0];
		if ((len+20+1) > maxlen)
			return false;
		strncpy(dest, _argv[0], len);
		dest[len] = '\0';
		return true;
	}
return false;
}

bool CWinApp::_ReadIniFile(void){
	/*	Read user parameters from msys.ini.
		This procedure initializes the application's variables,
		with default values if there's no ini file yet.		*/

	char iniFileName[_MAX_PATH];
	strcpy(iniFileName, _msysDir);
	strcat(iniFileName, INI_FILE_NAME);
	
	if (!IniFile.Open(iniFileName, "rb")){
		// msys.ini doesn't exist, create an empty one.
		FILE *file = fopen(iniFileName, "wb");
		if (!file)
			return false;
		else
			fclose(file);

		// now load it.
		if (!IniFile.Open(iniFileName, "rb")){
			debugf("CWinApp::_ReadIniFile(): cannot load %s !", iniFileName);
			return false;
		}

	}

	// [Msys] section.
	IniFile.GetSection(	"Msys");
	_startRxvt = IniFile.GetInt(	"StartRxvt", 1);

	// [RxvtColors] section.
	IniFile.GetSection(	"RxvtColors");
	IniFile.GetString(_mingw32BgColor, 	"Mingw32BgColor",	"LightYellow",	sizeof(_mingw32BgColor));
	IniFile.GetString(_mingw32FgColor, 	"Mingw32FgColor",	"Navy",		sizeof(_mingw32FgColor));
	IniFile.GetString(_msysBgColor, 	"MsysBgColor",		"White",		sizeof(_msysBgColor));
	IniFile.GetString(_msysFgColor, 	"MsysFgColor",		"Black",		sizeof(_msysFgColor));
	IniFile.Close();
return true;
}

bool CWinApp::_ShellFound(const char *directory){
	/*	Try to find a shell in "c:\msys_dir\bin".	*/
	
	if (!directory || !*directory)
		return false;

	// we need at least sh.
	if (_FileExist(directory, "\\bin\\sh.exe")){
		if (_startRxvt && !_FileExist(directory, "\\bin\\rxvt.exe")){
			// no rxvt, run sh.
			_startRxvt = false;
		}
		return true;
	}
return false;
}

bool CWinApp::_WriteIniFile(void){
	/*	Update msys.ini.			*/
	
	IniFile.FOpen(NULL, "wb");
	FILE *file = IniFile.GetFile();
	if (!file){
		IniFile.Close();
		return false;
	}

	fprintf(file, ";\n;\tMSYS-here %s.\n", FULL_APP_VERSION);
	fprintf(file, ";\tInitialization file.\n;\n");
	fprintf(file, ";\tMSYS home:\n;\thttp://www.mingw.org/\n;\n\n");
	
	// [Msys]
	fprintf (file, "[Msys]\n");
	fprintf (file, "StartRxvt = %d\n\n",		_startRxvt);
	
	fprintf (file, "[RxvtColors]\n\n");
	fprintf (file, "Mingw32BgColor = %s\n",	_mingw32BgColor);
	fprintf (file, "Mingw32FgColor = %s\n\n",	_mingw32FgColor);
	fprintf (file, "MsysBgColor = %s\n",		_msysBgColor);
	fprintf (file, "MsysFgColor = %s\n",		_msysFgColor);

return true;
}

bool CWinApp::_FileExist(const char *directory, const char *filename){
	/*	Could be included to CPath objects.	*/
	
	if (!directory || !*directory || !filename || !*filename)
		return false;

	char filepath[_MAX_PATH];
	strcpy(filepath, directory);
	strcat(filepath, filename);
return (access(filepath, 0) ? false : true);
}

bool CWinApp::_GetOpt(void){
	/*	Get command line options.
		- MSYS Here: msys -h"c:\change_dir".
		- MSYS DVLPR: msys -d.			*/
	
	while (1){
		int option_index = 0;
		static struct option long_options[] = {
			{0, 0, 0, 0}
		};
		
		int c = getopt_long(
			_argc,
			_argv,
			"dh:",
			long_options,
			&option_index);
	
		if (c == -1)
			break;
		
		switch (c){
			case 0:
			break;
			
			case 'd':
			_dvlpr = true;
			break;
			
			case 'h':
			_here = optarg;
			break;
			
			case '?':
			break;
			
			default:
			debugf("?? getopt returned character code 0%o ??\n", c);
		}
	}
return true;
}

bool CWinApp::_Msys(void){
	/*	Runs Msys.		*/
	
	char cmdline[512];
	char *_bgColor = _mingw32BgColor;
	char *_fgColor = _mingw32FgColor;

	if (_dvlpr){
		putenv(MSYS_DVLPR);
		_bgColor = _msysBgColor;
		_fgColor = _msysFgColor;
	}
	
	if (_here && *_here){
		_Unixify(_here);
		if (_startRxvt){
			if (1 > _snprintf(cmdline, sizeof(cmdline), "%s -fg %s -bg %s %s -e %s -c \"cd '%s' ; exec /bin/sh\"", rxvt, _fgColor, _bgColor, rxvtopts, rxvtexec, _here))
				return false;
		}else{
			if (1 > _snprintf(cmdline, sizeof(cmdline), "bin\\sh --login -i -c \"cd '%s' ; exec /bin/sh\"", _here))
				return false;
		}
	}else{
		if (_startRxvt){
			if (1 > _snprintf(cmdline, sizeof(cmdline), "%s -fg %s -bg %s %s -e %s", rxvt, _fgColor, _bgColor, rxvtopts, rxvtexec))
				return false;
		}else{
			if (1 > _snprintf(cmdline, sizeof(cmdline), "bin\\sh --login -i"))
				return false;
		}
	}
return _RunCmd(cmdline, NULL, !_startRxvt);
}

bool CWinApp::_RunCmd(LPTSTR lpCommandLine, LPCTSTR lpCurrentDirectory, bool show){
	/*	A simple procedure to create a child process.	*/
	
	STARTUPINFO si = {sizeof(STARTUPINFO), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	PROCESS_INFORMATION pi = {0, 0, 0, 0};

	if (!show){
		si.dwFlags 		= STARTF_USESHOWWINDOW;
		si.wShowWindow	= SW_HIDE;
	}

	return CreateProcess(
		NULL,
		lpCommandLine,
		NULL,
		NULL,
		false,
		0,
		NULL,
		lpCurrentDirectory,
		&si,
		&pi);
}

bool CWinApp::_Unixify(char *src){
	while (*src){
		if (*src == '\\')
			*src = '/';
		src++;
	}
return true;
}

LRESULT CALLBACK CWinApp::CMainWndProc(UINT Message, WPARAM wParam, LPARAM lParam){
	// TODO: use CWinApp::Run(...) to create dialogs,
	// then this will be the application's window proc.

	switch (Message){
		case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	
return ::DefWindowProc(_hWnd, Message, wParam, lParam);
}
