
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include "pkgindex.hpp"
#include <curl/curl.h>


extern "C" int CreateMainWnd();
extern "C" int MainMessageLoop();
extern "C" void SelectInstallation();
void UI_RefreshCategoryList();


HINSTANCE g_hinstance;


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	g_hinstance = hInstance;

	curl_global_init(CURL_GLOBAL_ALL);

	if (!CreateMainWnd())
		return 1;

	PkgIndex::LoadIndex();
	UI_RefreshCategoryList();

	SelectInstallation();

	int ret = MainMessageLoop();

	curl_global_cleanup();

	return ret;
}
