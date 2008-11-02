/** \file getbindir.c
 *
 * Created: JohnE, 2008-10-09
 */


#define WIN32_LEAN_AND_MEAN
#include <windows.h>


const char* GetBinDir()
{
	static char path[1024] = {0};
	if (!path[0])
	{
		GetModuleFileName(0, path, 1024);
		int i = strlen(path);
		for (; i >= 0; --i)
		{
			if (path[i] == '/' || path[i] == '\\')
			{
				path[i] = '\0';
				break;
			}
			path[i] = '\0';
		}
	}
	return path;
}
