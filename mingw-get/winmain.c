
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <curl/curl.h>
#include "mainwnd.hh"
#include "selectinst.hh"
#include "categorytree.hh"
#include "pkgindex.hh"


HINSTANCE g_hinstance;


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	g_hinstance = hInstance;

	curl_global_init(CURL_GLOBAL_ALL);

	if (!CreateMainWnd())
		return 1;

	PkgIndex_Load();
	CategoryTree_Reload();

	SelectInstallation();

	int ret = MainMessageLoop();

	curl_global_cleanup();

	return ret;
}
