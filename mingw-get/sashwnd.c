
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "resource.h"
#include "mainwnd.hh"


static LRESULT CALLBACK SashWndProc
 (HWND hwnd,
  UINT uMsg,
  WPARAM wParam,
  LPARAM lParam)
{
	static PAINTSTRUCT ps;
	static HDC hdc;
	static HPEN pen, oldpen;
	static RECT tbrc, statrc, rc;
	static POINT tl, br;
	static int drag_offset;

	switch (uMsg)
	{
	case WM_LBUTTONDOWN:
		GetWindowRect(hwnd, &rc);
		tl.x = rc.left; tl.y = rc.top; ScreenToClient(hwnd, &tl);
		br.x = rc.right; br.y = rc.bottom; ScreenToClient(hwnd, &br);
		if (br.x - tl.x < br.y - tl.y)
			drag_offset = LOWORD(lParam) - tl.x;
		else
			drag_offset = HIWORD(lParam) - tl.y;

		SetCapture(hwnd);

		GetClientRect(GetParent(hwnd), &rc);
		GetWindowRect(GetDlgItem(GetParent(hwnd), IDC_MAINTOOLBAR), &tbrc);
		TransToClient(GetParent(hwnd), &tbrc);
		GetWindowRect(GetDlgItem(GetParent(hwnd), IDC_STATBAR), &statrc);
		TransToClient(GetParent(hwnd), &statrc);
		if (br.x - tl.x < br.y - tl.y)
		{
			tl.x = rc.left + drag_offset;
			tl.y = rc.top + (tbrc.bottom - tbrc.top) + 6;
			br.x = rc.right + 1 + drag_offset - 8;
			br.y = rc.bottom + 1 - (statrc.bottom - statrc.top);
		}
		else
		{
			tl.x = rc.left;
			tl.y = rc.top + drag_offset  + (tbrc.bottom - tbrc.top) + 6;
			br.x = rc.right + 1;
			br.y = rc.bottom + 1 - (statrc.bottom - statrc.top) + drag_offset - 8;
		}
		ClientToScreen(GetParent(hwnd), &tl);
		ClientToScreen(GetParent(hwnd), &br);
		SetRect(&rc, tl.x, tl.y, br.x, br.y);
		ClipCursor(&rc);

		return 0;

	case WM_MOUSEMOVE:
		if (wParam & MK_LBUTTON)
		{
			GetWindowRect(hwnd, &rc);
			tl.x = rc.left; tl.y = rc.top; ScreenToClient(GetParent(hwnd), &tl);
			br.x = rc.right; br.y = rc.bottom; ScreenToClient(GetParent(hwnd), &br);
			DWORD dwpos = GetMessagePos();
			POINT mpt; mpt.x = LOWORD(dwpos); mpt.y = HIWORD(dwpos);
			ScreenToClient(GetParent(hwnd), &mpt);
			if (br.x - tl.x < br.y - tl.y)
				NewVertSashPos(mpt.x - drag_offset);
			else
				NewHorzSashPos(mpt.y - drag_offset);
		}
		break;

	case WM_LBUTTONUP:
		ClipCursor(0);
		ReleaseCapture();
		return 0;

	case WM_PAINT:
		{
			GetWindowRect(hwnd, &rc);
			tl.x = rc.left; tl.y = rc.top; ScreenToClient(hwnd, &tl);
			br.x = rc.right; br.y = rc.bottom; ScreenToClient(hwnd, &br);

			hdc = BeginPaint(hwnd, &ps);

			pen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_3DLIGHT));
			oldpen = SelectObject(hdc, pen);
			MoveToEx(hdc, tl.x, tl.y, 0);
			if (br.x - tl.x < br.y - tl.y)
				LineTo(hdc, tl.x, br.y);
			else
				LineTo(hdc, br.x, tl.y);
			SelectObject(hdc, oldpen);
			DeleteObject(pen);

			pen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_3DSHADOW));
			oldpen = SelectObject(hdc, pen);
			MoveToEx(hdc, br.x - 1, br.y - 1, 0);
			if (br.x - tl.x < br.y - tl.y)
				LineTo(hdc, br.x - 1, tl.y - 1);
			else
				LineTo(hdc, tl.x - 1, br.y - 1);
			SelectObject(hdc, oldpen);
			DeleteObject(pen);

			EndPaint(hwnd, &ps);
		}
		return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


int SashWnd_RegisterClasses(HINSTANCE hinstance)
{
	WNDCLASS wc;
	wc.style = 0;
	wc.lpfnWndProc = SashWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hinstance;
	wc.hIcon = 0;
	wc.hCursor = LoadCursor(0, IDC_SIZEWE);
	wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
	wc.lpszMenuName = 0;
	wc.lpszClassName = SASHWND_VERT_CLASSNAME;
	if (!RegisterClass(&wc))
		return 0;
	wc.hCursor = LoadCursor(0, IDC_SIZENS);
	wc.lpszClassName = SASHWND_HORZ_CLASSNAME;
	if (!RegisterClass(&wc))
		return 0;
	return 1;
}
