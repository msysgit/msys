
#define WIN32_LEAN_AND_MEAN
#include <windows.h>


int UI_Windowed(HINSTANCE);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	return UI_Windowed(hInstance);
}
