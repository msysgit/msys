/********************************************************************
*	Module:	main.h. This is part of MSYS Here.
*
*	License:	Public domain.
*			2002 Manu B.
*
********************************************************************/
#ifndef MAIN_H
#define MAIN_H

#include "CFile.h"

#define unused(x) (x) = (x)

class CWinApp : public CSDIBase {
	public:
	CWinApp() : _here(NULL) {};
	~CWinApp() {};

	int Run(void);

	private:
	bool	_GetMsysDir(char *dest, size_t maxlen);
	bool	_ReadIniFile(void);
	bool	_ShellFound(const char *directory);
	bool	_WriteIniFile(void);
	bool	_FileExist(const char *directory, const char *filename);
	bool	_GetOpt(void);
	bool	_Msys(void);
	bool	_Unixify(char *src);
	bool	_RunCmd(LPTSTR lpCommandLine, LPCTSTR lpCurrentDirectory, bool show);

	LRESULT CALLBACK CMainWndProc(UINT Message, WPARAM wParam, LPARAM lParam);

	CIniFile IniFile;
	char _msysDir[_MAX_PATH];
	char *_here;
	bool _startRxvt;
};

#endif
