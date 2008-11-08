
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#include <stdio.h>
#include "multiuser.hh"


static int GetPrevInstByFolder(int folder, void (*callback)(const char*))
{
	LPITEMIDLIST pidl = 0;
	if (SHGetSpecialFolderLocation(0, folder, &pidl) != S_OK)
		return 0;
	char fpath[MAX_PATH];
	fpath[0] = 0;
	SHGetPathFromIDList(pidl, fpath);
	CoTaskMemFree(pidl);
	if (!fpath[0])
		return 0;
	int plen = strlen(fpath);
	char* mpath = malloc(plen + 25);
	sprintf(mpath, "%s\\MinGW\\installations.txt", fpath);
	FILE* infile = fopen(mpath, "r");
	free(mpath);
	if (!infile)
		return 0;
	int icount = 0;
	while (fgets(fpath, MAX_PATH, infile))
	{
		int i;
		for (i = strlen(fpath) - 1; i >= 0; --i)
		{
			if (fpath[i] != '\r' && fpath[i] != '\n')
				break;
			fpath[i] = 0;
		}
		callback(fpath);
		++icount;
	}
	fclose(infile);
	return icount;
}


int GetPrevInstalls(void (*callback)(const char*))
{
	int icount = GetPrevInstByFolder(CSIDL_APPDATA, callback);
	if (GetAccountTypeHelper() >= MU_POWER)
		icount += GetPrevInstByFolder(CSIDL_COMMON_APPDATA, callback);
	return icount;
}
