
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include "pkg_index.hpp"


extern "C" HWND CreateMainWnd(HINSTANCE);
extern "C" int MainMessageLoop(HWND);
std::string GetBinDir();
extern "C" void SelectInst(HINSTANCE, HWND);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	HWND mainwnd = CreateMainWnd(hInstance);
	if (!mainwnd)
		return 1;

	PkgIndex::LoadManifest(GetBinDir() + "\\mingw_avail.mft");

	SelectInst(hInstance, mainwnd);

	return MainMessageLoop(mainwnd);
}
