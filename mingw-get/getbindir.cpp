/** \file getbindir.cpp
 *
 * Created: JohnE, 2008-10-09
 */


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>


std::string GetBinDir()
{
	char path[1024];
	GetModuleFileName(0, path, 1024);
	for (int i = strlen(path) - 1; i >= 0; --i)
	{
		if (path[i] == '/' || path[i] == '\\')
		{
			path[i] = '\0';
			break;
		}
		path[i] = '\0';
	}
	return path;
}
