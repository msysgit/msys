/********************************************************************
*	Module:	winui.cpp. This is part of WinUI.
*
*	Purpose:	WinUI main module. Contains procedures relative to User Interface 
*			and general purpose procedures.
*
*	Authors:	Manu B.
*
*	License:	Public domain.
*			2002 Manu B.
*
*	Revision:	1.0a
*
*
********************************************************************/
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <ctype.h>
#include "winui.h"


/********************************************************************
*	Functions:	debugf, ClientToWindow.
*
*	Purpose:	A few handy procedures.
*
*	Revisions:	
*
********************************************************************/
const char *_dbg_caption = "Debug Message !";
char _mb_buffer[256];

int MsgBox(HWND hWnd, UINT uType, char *lpCaption, const char *format, ...){
	/*	A MessageBox-like procedure that can output formatted text to a
		Windows message box.
		Apparently, under Win9x, if lpCaption is NULL, MessageBoxA displays
		a default 'Error' caption in the title bar.						*/

	if (format){
		char buffer[256];
		va_list argptr;
		va_start(argptr, format);
		int result = _vsnprintf(buffer, sizeof(buffer), format, argptr);
		va_end(argptr);
		if (result != -1)
			return MessageBox(hWnd, buffer, lpCaption, uType);
	}
return -1;
}

int debugf(const char *format, ...){
	/*	A printf-like procedure that can output
		formatted text to a Windows message box.	*/

	if (format){
		va_list argptr;
		va_start(argptr, format);
		int result = _vsnprintf(_mb_buffer, sizeof(_mb_buffer), format, argptr);
		va_end(argptr);
		if (result != -1)
			return MessageBox(0, _mb_buffer, _dbg_caption, MB_ICONERROR);
	}
return -1;
}

bool ClientToWindow(HWND hWnd, LPPOINT lpPoint){
	/* 	The ClientToWindow function converts the client coordinates
		of a specified point to window coordinates.				*/

	// Convert given point to screen coordinates.
	if (!ClientToScreen(hWnd, lpPoint))
		return false;

	// Get the window rect in screen coordinates.
	RECT rect;
	if (!GetWindowRect(hWnd, &rect))
		return false;

	// Offset coordinates relative to the top-left of the window.
	lpPoint->x -= rect.left;
	lpPoint->y -= rect.top;
return true;
}


/********************************************************************
*	Class:	CChrono.
*
*	Purpose:	
*
*	Revisions:	
*
********************************************************************/
CChrono::CChrono(){
	_time = 0;
}

void CChrono::Start(void){
	_time = ::GetTickCount();
}

DWORD CChrono::Stop(void){
	DWORD diff = ::GetTickCount() - _time;
	_time = 0;
return diff;
}


/********************************************************************
*	Section:
*		CWindow based classes for common controls.
*
*	Content:
*		CWindow
*		CToolBar
*		CStatusBar
*		CTreeView
*		CListView
*		CTabCtrl
*		CScintilla
*		CSplitter (Dummy window)
*
*	Revisions:	
*
********************************************************************/
/********************************************************************
*	Class:	Base Window class.
*
*	Purpose:	
*
*	Revisions:	
*
********************************************************************/
LRESULT CALLBACK WndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);

CWindow::CWindow(){
	_pParent	= NULL;
	_hWnd 	= 0;
	_hInst 	= 0;
	_lParam	= NULL;
	_WndProc	= NULL;
}

HWND CWindow::CreateEx(CWindow *pWindow, DWORD dwExStyle, LPCTSTR lpClassName, 
		LPCTSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, 
		HMENU hMenu, LPVOID lpParam){
	/*	CreateEx procedures prevent creating the same window twice.
		If pWindow is null, parent window is the desktop.				*/

	if (!_hWnd){
		HWND hParent;
		if(pWindow){
			hParent = pWindow->_hWnd;
			_hInst = pWindow->_hInst;
		}else{
			hParent = 0;
			_hInst = ::GetModuleHandle(NULL);
		}
	
		_pParent = pWindow;
		_lParam = lpParam;
	
		_hWnd = CreateWindowEx(
			dwExStyle, 
			lpClassName, 
			lpWindowName, 
			dwStyle,
			x, 
			y, 
			nWidth, 
			nHeight, 
			hParent, 
			(HMENU) hMenu,
			_hInst, 
			this);
		// Note: We can get lpParam using this->_lParam.
		return _hWnd;
	}
return 0;
}

bool CWindow::SubClass(void){
	if (_hWnd && !_WndProc){
		_WndProc = (WNDPROC) ::SetWindowLong(_hWnd, GWL_WNDPROC, (LONG) WndProc);
		
		// Store a pointer to this window class.
		::SetWindowLong(_hWnd, GWL_USERDATA, (LONG) this);
		return true;
	}
return false;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam){
	/*	The window callback procedure redirects messages to the owner class.		*/

	// Get a pointer to the owner CWindow class.
	CWindow *pWindow = (CWindow *) ::GetWindowLong(hWnd, GWL_USERDATA);

	if (pWindow == NULL)
		return 0;
	else
		return pWindow->CWndProc(Message, wParam, lParam);
}


/********************************************************************
*	Class:	CToolBar.
*
*	Purpose:	
*
*	Revisions:	
*
********************************************************************/
HWND CToolBar::CreateEx(CWindow *pWindow, DWORD dwExStyle, LPCTSTR lpWindowName, 
		DWORD dwStyle, int x, int y, int nWidth, int nHeight, HMENU hMenu, LPVOID lpParam){

	if (pWindow && !_hWnd){
		// Set window parameters and create the control.
		_pParent = pWindow;
		_hInst = _pParent->GetHinstance();
	
		_hWnd = CreateWindowEx(
			dwExStyle, 
			TOOLBARCLASSNAME, 
			lpWindowName, 
			dwStyle,
			x, 
			y, 
			nWidth, 
			nHeight, 
			_pParent->GetHwnd(), 
			hMenu,
			_hInst, 
			lpParam); 
	
		// For backward compatibility. 
		if(_hWnd)
			::SendMessage(_hWnd, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0); 
		return _hWnd; 
	}
return 0;
} 
 
LRESULT CToolBar::AddBitmap(UINT resId, int nBmp, HINSTANCE hInstance){
	/* Add the bitmap containing button images to the toolbar. */ 

	TBADDBITMAP tbab; 
	tbab.hInst	= hInstance; 
	tbab.nID	= resId;
return ::SendMessage(_hWnd, TB_ADDBITMAP, (WPARAM) nBmp, (LPARAM) &tbab); 
}


/********************************************************************
*	Class:	CStatusBar.
*
*	Purpose:	
*
*	Revisions:	
*
********************************************************************/
HWND CStatusBar::CreateEx(CWindow *pWindow, DWORD dwExStyle, LPCTSTR lpWindowName, 
		DWORD dwStyle, int x, int y, int nWidth, int nHeight, HMENU hMenu, LPVOID lpParam){

	if (pWindow && !_hWnd){
		// Set window parameters and create the control.
		_pParent = pWindow;
		_hInst = pWindow->GetHinstance();
		_lParam = lpParam;
	
		_hWnd = CreateWindowEx(
			dwExStyle,
			STATUSCLASSNAME, 
			lpWindowName,
			dwStyle,
			x, 
			y, 
			nWidth, 
			nHeight, 
			_pParent->GetHwnd(),
			hMenu,
			_hInst,
			lpParam);
	
		return _hWnd;
	}
return 0;
}


/********************************************************************
*	Class:	CTreeView.
*
*	Purpose:	
*
*	Revisions:	
*
********************************************************************/
HWND CTreeView::CreateEx(CWindow *pWindow, DWORD dwExStyle, LPCTSTR lpWindowName, 
		DWORD dwStyle, int x, int y, int nWidth, int nHeight, HMENU hMenu, LPVOID lpParam){

	if (pWindow && !_hWnd){
		// Set window parameters and create the control.
		_pParent = pWindow;
		_hInst = pWindow->GetHinstance();
		_lParam = lpParam;
	
		_hWnd = CreateWindowEx(
			dwExStyle,
			WC_TREEVIEW, 
			lpWindowName,
			dwStyle,
			x, 
			y, 
			nWidth, 
			nHeight, 
			_pParent->GetHwnd(),
			hMenu,
			_hInst,
			lpParam);
	
		return _hWnd;
	}
return 0;
}

bool CTreeView::GetItemText(HTREEITEM hitem, char *pszText, int cchTextMax){
	_tvi.hItem = hitem;
	_tvi.mask = TVIF_HANDLE | TVIF_TEXT;
	_tvi.pszText = pszText;
	_tvi.cchTextMax = cchTextMax;
return TreeView_GetItem(_hWnd, &_tvi);
}

LPARAM CTreeView::GetItemParam(HTREEITEM hitem){
	_tvi.hItem = hitem;
	_tvi.mask = TVIF_HANDLE | TVIF_PARAM;
	_tvi.lParam = 0;
	TreeView_GetItem(_hWnd, &_tvi);
	// FIXME: should return TRUE or FALSE.
	// See CListView::GetItemParam().
return _tvi.lParam;
}


/********************************************************************
*	Class:	CListView.
*
*	Purpose:	
*
*	Revisions:	
*
********************************************************************/
CListView::CListView(){
	_lastRow = 0;
}

HWND CListView::CreateEx(CWindow *pWindow, DWORD dwExStyle, LPCTSTR lpWindowName, 
		DWORD dwStyle, int x, int y, int nWidth, int nHeight, HMENU hMenu, LPVOID lpParam){

	if (pWindow && !_hWnd){
		// Set window parameters and create the control.
		_pParent = pWindow;
		_hInst = pWindow->GetHinstance();
		_lParam = lpParam;
	
		_hWnd = CreateWindowEx(
			dwExStyle, 
			WC_LISTVIEW, 
			lpWindowName, 
			dwStyle,
			x, 
			y, 
			nWidth, 
			nHeight, 
			_pParent->GetHwnd(), 
			hMenu,
			_hInst, 
			lpParam);
	
		return _hWnd;
	}
return 0;
}

BOOL CListView::GetItemParam(int iItem, LPARAM *lParam){
	_lvi.mask		= LVIF_PARAM;
	_lvi.iItem		= iItem;
	_lvi.lParam		= 0;
	BOOL result = ListView_GetItem(_hWnd, &_lvi);
	*lParam = _lvi.lParam;
return result;
}

void CListView::DeleteAllItems(void){
	// Win9x LVM_DELETEALLITEMS is very slow.
	int numitems = ListView_GetItemCount(_hWnd);
	for (int n = 0; n < numitems; n++){
		ListView_DeleteItem(_hWnd, 0);
	}
	_lastRow = 0;
	// FIXME: should return TRUE or FALSE.
}


/********************************************************************
*	Class:	CTabCtrl.
*
*	Purpose:	Tab control.
*
*	Revisions:	
*
********************************************************************/
HWND CTabCtrl::CreateEx(CWindow *pWindow, DWORD dwExStyle, LPCTSTR lpWindowName, 
		DWORD dwStyle, int x, int y, int nWidth, int nHeight, HMENU hMenu, LPVOID lpParam){

	if (pWindow && !_hWnd){
		// Set window parameters and create the control.
		_pParent = pWindow;
		_hInst = pWindow->GetHinstance();
		_lParam = lpParam;
	
		_hWnd = CreateWindowEx(
			dwExStyle,
			WC_TABCONTROL, 
			lpWindowName,
			dwStyle,
			x, 
			y, 
			nWidth, 
			nHeight, 
			_pParent->GetHwnd(),
			hMenu,
			_hInst,
			lpParam);
	
		return _hWnd;
	}
return 0;
}

int CTabCtrl::InsertItem(int iItem, UINT mask, LPTSTR pszText, int cchTextMax, int iImage, LPARAM lParam){
	_tie.mask 				= mask;
	#if (_WIN32_IE >= 0x0300)
		// PSDK says that dwState and dwStateMask members
		// are ignored when inserting an item.
		_tie.dwState		= 0;
		_tie.dwStateMask	= 0;
	#else
		_tie.lpReserved1		= 0;
		_tie.lpReserved2		= 0;
	#endif
	_tie.pszText 			= pszText;
	_tie.cchTextMax 			= cchTextMax;
	_tie.iImage 			= iImage;
	_tie.lParam 			= lParam;
return TabCtrl_InsertItem(_hWnd, iItem, &_tie);
}

BOOL CTabCtrl::SetItemParam(int iItem, LPARAM lParam){
	_tie.mask = TCIF_PARAM;
	_tie.lParam = lParam;
return TabCtrl_SetItem(_hWnd, iItem, &_tie);
}

LPARAM CTabCtrl::GetItemParam(int iItem){
	_tie.mask = TCIF_PARAM;
	BOOL result = TabCtrl_GetItem(_hWnd, iItem, &_tie);
	if (result)
		return _tie.lParam;
	// FIXME: should return TRUE or FALSE.
return 0;
}


/********************************************************************
*	Class:	CScintilla.
*
*	Purpose:	
*
*	Revisions:	
*
********************************************************************/
HWND CScintilla::CreateEx(CWindow *pWindow, DWORD dwExStyle, LPCTSTR lpWindowName, 
		DWORD dwStyle, int x, int y, int nWidth, int nHeight, HMENU hMenu, LPVOID lpParam){

	if (pWindow && !_hWnd){
		// Set window parameters and create the control.
		_pParent = pWindow;
		_hInst = pWindow->GetHinstance();
		_lParam = lpParam;
	
		_hWnd = CreateWindowEx(
			dwExStyle,
			"Scintilla", 
			lpWindowName,
			dwStyle,
			x, 
			y, 
			nWidth, 
			nHeight, 
			_pParent->GetHwnd(),
			hMenu,
			_hInst,
			lpParam);
	
		return _hWnd;
	}
return 0;
}


/********************************************************************
*	Class:	CSplitter.
*
*	Purpose:	Provides a simple splitter class. This work is based on the
*			wonderful "Splitter Windows" tutorial by James Brown.
*			http://freespace.virgin.net/james.brown7/tuts/splitter.htm
*			Catch22 Productions home page:
*			http://freespace.virgin.net/james.brown7/index.htm
*
*	Revision:	1.0
*
********************************************************************/
CSplitter::CSplitter(){
	/*	The CSplitter is a particular kind of CWindow since its not really a window,
		as a matter of fact, _hWnd is always null.						*/

	_WndOne 		= NULL;
	_WndTwo 		= NULL;
	_splitFrame		= true;
	_wndOneVisible	= true;
	_wndTwoVisible	= true;
	_paneSize		= -1;
	_minPaneSize 	= 4;
	_barSize		= 4;
	_hCursor 		= NULL;
	_frameMarginOne	= 10;
	_frameMarginTwo	= 10;
	_mouseCaptured	= false;
	_initialized		= false;
}

bool CSplitter::CreateEx(CWindow *PaneOne, CWindow *PaneTwo, bool vertical, int splitMode, 
				int x, int y, int nWidth, int nHeight, int paneSize, bool splitFrame){

	if(!_initialized && PaneOne && PaneTwo){
		_WndOne = PaneOne;
		_WndTwo = PaneTwo;
		_isVertical = vertical;
		if (_isVertical)
			_hCursor = ::LoadCursor(NULL, IDC_SIZEWE);
		else
			_hCursor = ::LoadCursor(NULL, IDC_SIZENS);
		_splitMode = splitMode;
		// Set the frame rect.
		_frameRectLeft	= x; 
		_frameRectTop	= y; 
		_frameRectRight	= x+nWidth; 
		_frameRectBottom = y+nHeight; 
		if (_paneSize == -1)
			_paneSize = paneSize;
		_splitFrame = splitFrame;
		_initialized = true;
	}
return _initialized;
}

BOOL CSplitter::SetFrameMargins(int frameMarginOne, int frameMarginTwo){
	_frameMarginOne	= frameMarginOne;
	_frameMarginTwo	= frameMarginTwo;
return true;
}

BOOL CSplitter::SetPosition(HWND, int x, int y, int width, int height, UINT){
	// Set the frame rect.
	_frameRectLeft	= x; 
	_frameRectTop	= y; 
	_frameRectRight	= x+width; 
	_frameRectBottom = y+height; 
	// Calculate the bar rect.
	SetBarRect();
	// Size child windows.
	SizeContent();
return true;
}

void CSplitter::SplitFrame(bool split){
	_splitFrame = split;
	SizeContent();
}

bool CSplitter::OnLButtonDown(HWND hWnd, short xPos, short yPos){
	/* Save mouse position */
	_startDragPos.x = xPos;
	_startDragPos.y = yPos;
	if (PtInRect(&_barRect, _startDragPos)){
		/* Begin of mouse capture */
		_mouseCaptured = true;
		::SetCapture(hWnd);

		/* Save the Xor bar position */
		_xorBarPos.x = _barRect.left;
		_xorBarPos.y = _barRect.top;
		::ClientToWindow(hWnd, &_xorBarPos);

		/* Draw the Xor bar */
		HDC hdc = ::GetWindowDC(hWnd);
		DrawXorBar(hdc, _xorBarPos.x, _xorBarPos.y, (_barRect.right-_barRect.left), (_barRect.bottom-_barRect.top));
		::ReleaseDC(hWnd, hdc);
	}
return _mouseCaptured;
}

void CSplitter::OnLButtonUp(HWND hWnd, short xPos, short yPos){
	if (_mouseCaptured){
		/* Erase previous Xor bar */
		HDC hdc = ::GetWindowDC(hWnd);
		DrawXorBar(hdc, _xorBarPos.x, _xorBarPos.y, (_barRect.right-_barRect.left), (_barRect.bottom-_barRect.top));
		::ReleaseDC(hWnd, hdc);

		/* Calculate the position difference */
		int deltaPos;
		if(_isVertical)
			deltaPos = xPos - _startDragPos.x;
		else
			deltaPos = yPos - _startDragPos.y;

		/* Move the splitter bar to a new position */
		if (deltaPos != 0){
			// Calculate the pane size.
			if (_splitMode == SPLMODE_1)
				_paneSize += deltaPos;
			else if (_splitMode == SPLMODE_2)
				_paneSize -= deltaPos;
			// Set the bar position and size the child windows.
			MoveBar(_paneSize);
			SizeContent();
		}

		/* End of mouse capture */
		::ReleaseCapture();
		_mouseCaptured = false;
	}
}

void CSplitter::OnMouseMove(HWND hWnd, short xPos, short yPos){
	if (_mouseCaptured){
		POINT newPos;
		int deltaPos;
		int barLimit;
		/* Calculate the new Xor bar position */
		if (_isVertical){
			deltaPos = xPos - _startDragPos.x;
			newPos.x = _barRect.left + deltaPos;
			newPos.y = _barRect.top;
			// Apply limits.
			barLimit = _frameRectLeft+_frameMarginOne;
			if(newPos.x < barLimit) 
				newPos.x = barLimit;
			barLimit = _frameRectRight-_barSize-_frameMarginTwo;
			if(newPos.x > barLimit) 
				newPos.x = barLimit;
		}else{
			deltaPos = yPos - _startDragPos.y;
			newPos.x = _barRect.left;
			newPos.y = _barRect.top + deltaPos;
			// Apply limits.
			barLimit = _frameRectTop+_frameMarginOne;
			if(newPos.y < barLimit) 
				newPos.y = barLimit;
			barLimit = _frameRectBottom-_barSize-_frameMarginTwo;
			if(newPos.y > barLimit) 
				newPos.y = barLimit;
		}

		/* Erase previous Xor bar and draw another one to the new position */
		::ClientToWindow(hWnd, &newPos);
		HDC hdc = ::GetWindowDC(hWnd);
		int width = _barRect.right-_barRect.left;
		int height = _barRect.bottom-_barRect.top;
		DrawXorBar(hdc, _xorBarPos.x, _xorBarPos.y, width, height);
		DrawXorBar(hdc, newPos.x, newPos.y, width, height);
		// Save the bar position to erase it later.
		_xorBarPos.x = newPos.x;
		_xorBarPos.y = newPos.y;
		::ReleaseDC(hWnd, hdc);
	}
}

bool CSplitter::OnSetCursor(HWND hWnd, LPARAM){
	if (_wndOneVisible && _wndTwoVisible){
		POINT pt;
		::GetCursorPos(&pt);
		::ScreenToClient(hWnd, &pt);
	
		if(::PtInRect(&_barRect, pt)){
			::SetCursor(_hCursor);
			return true;
		}
	}
return false;
}

void CSplitter::GetFrameRect(LPRECT frameRect){
	frameRect->left = _frameRectLeft;
	frameRect->top = _frameRectTop;
	frameRect->right = _frameRectRight;
	frameRect->bottom = _frameRectBottom;
}

void CSplitter::SetBarRect(void){
	/* Calculate the splitter bar rect */
	if(_isVertical){
		if (_splitMode == SPLMODE_1)
			_barRect.left = _frameRectLeft+_paneSize;
		else if (_splitMode == SPLMODE_2)
			_barRect.left = _frameRectRight-_paneSize-_barSize;
		_barRect.top	= _frameRectTop; 
		_barRect.right	= _barRect.left+_barSize; 
		_barRect.bottom 	= _frameRectBottom;
	}else{
		_barRect.left	= _frameRectLeft; 
		if (_splitMode == SPLMODE_1)
			_barRect.top = _frameRectTop+_paneSize;
		else if (_splitMode == SPLMODE_2)
			_barRect.top = _frameRectBottom-_paneSize-_barSize;
		_barRect.right	= _frameRectRight; 
		_barRect.bottom 	= _barRect.top+_barSize;
	}
}

void CSplitter::SizeContent(void){
	/*	Once the frame rect is set, the left/top of wnd1 and the right/bottom of
		wnd2 are the frame rect. Then, SizeContent needs to set the right/bottom
		of wnd1 and the left/top of wnd2.								*/

	int wndOneRectRight = 0;
	int wndOneRectBottom = 0;
	int wndTwoRectLeft = 0;
	int wndTwoRectTop = 0;
	bool showWndOne;
	bool showWndTwo;

	int variableSize;

	if(_isVertical){
		// A vertical splitter, we can set the top/bottom coords.
		wndOneRectBottom = _frameRectBottom;
		wndTwoRectTop = _frameRectTop;
		// From here, we only have to set wndOneRectRight / wndTwoRectLeft.

		/*------------------ Vertical splitter - mode 1 ------------------*/
		if (_splitMode == SPLMODE_1){
			if (_splitFrame){
				variableSize = (_frameRectRight - _barRect.right);
				if (variableSize < _minPaneSize){
					// Hide the window that is too small.
					wndOneRectRight = _frameRectRight;
					showWndOne = true;
					showWndTwo = false;
				}else{
					// Normal sizing.
					wndOneRectRight = _barRect.left;
					wndTwoRectLeft = _barRect.right;
					showWndOne = true;
					showWndTwo = true;
				}
			}else{
				// Don't split the frame window.
				wndTwoRectLeft = _frameRectLeft;
				showWndOne = false;
				showWndTwo = true;
			}

		/*------------------ Vertical splitter - mode 2 ------------------*/
		}else if (_splitMode == SPLMODE_2){
			if (_splitFrame){
				variableSize = (_barRect.left-_frameRectLeft);
				if (variableSize < _minPaneSize){
					// Hide the window that is too small.
					wndTwoRectLeft = _frameRectLeft;
					showWndOne = false;
					showWndTwo = true;
				}else{
					// Normal sizing.
					wndOneRectRight = _barRect.left;
					wndTwoRectLeft = _barRect.right;
					showWndOne = true;
					showWndTwo = true;
				}
			}else{
				// Don't split the frame window.
				wndOneRectRight = _frameRectRight;
				showWndOne = true;
				showWndTwo = false;
			}
		}else{
			return;
		}
	}else{
		// An horizontal splitter, we can set the left/right coords.
		wndOneRectRight = _frameRectRight;
		wndTwoRectLeft = _frameRectLeft;
		// From here, we only have to set wndOneRectBottom / wndTwoRectTop.

		/*------------------ Horizontal splitter - mode 1 ------------------*/
		if (_splitMode == SPLMODE_1){
			// TODO: this mode isn't fully written.
			variableSize = (_frameRectBottom-_barRect.bottom);
			showWndOne = true;
			showWndTwo = true;
			if (variableSize < _minPaneSize){
					wndOneRectBottom = _frameRectBottom-_frameRectTop;
			}else{
					wndOneRectBottom = _barRect.top-_frameRectTop;
			}

		/*------------------ Horizontal splitter - mode 2 ------------------*/
		}else if (_splitMode == SPLMODE_2){
			if (_splitFrame){
				variableSize = (_barRect.top-_frameRectTop);
				if (variableSize < _minPaneSize){
					// Hide the window that is too small.
					wndTwoRectTop = _frameRectTop;
					showWndOne = false;
					showWndTwo = true;
				}else{
					// Normal sizing.
					wndOneRectBottom = _barRect.top;
					wndTwoRectTop = _barRect.bottom;
					showWndOne = true;
					showWndTwo = true;
				}
			}else{
				// Don't split the frame window.
				wndOneRectBottom = _frameRectBottom;
				showWndOne = true;
				showWndTwo = false;
			}
		}else{
			return;
		}
	}

	if (showWndOne){
		_WndOne->SetPosition(0,
			_frameRectLeft,
			_frameRectTop,
			(wndOneRectRight - _frameRectLeft),
			(wndOneRectBottom - _frameRectTop),
			0);
		if (!_wndOneVisible){
			_WndOne->Show();
			_wndOneVisible = true;
		}
	}else if (_wndOneVisible){
		_WndOne->Show(SW_HIDE);
		_wndOneVisible = false;
	}

	if (showWndTwo){
		_WndTwo->SetPosition(0,
			wndTwoRectLeft,
			wndTwoRectTop, 
			(_frameRectRight - wndTwoRectLeft),
			(_frameRectBottom - wndTwoRectTop),
			0);
		if (!_wndTwoVisible){
			_WndTwo->Show();
			_wndTwoVisible = true;
		}
	}else if (_wndTwoVisible){
		_WndTwo->Show(SW_HIDE);
		_wndTwoVisible = false;
	}
}

bool CSplitter::MoveBar(int paneSize){
	int barLimit;
	_paneSize = paneSize;
	if(_isVertical){
		/* Vertical splitter bar */
		if (_splitMode == SPLMODE_1){
			if (_paneSize < _frameMarginOne)
				_paneSize = _frameMarginOne;
			barLimit = _frameRectRight - _frameRectLeft - _frameMarginTwo;
			if (_paneSize > barLimit)
				_paneSize = barLimit;
		}else if (_splitMode == SPLMODE_2){
		}
	}else{
		// Apply limits.
		if (_splitMode == SPLMODE_1){
		}else if (_splitMode == SPLMODE_2){
			if (_paneSize < _frameMarginTwo)
				_paneSize = _frameMarginTwo;
			barLimit = _frameRectBottom - _frameRectTop - _frameMarginOne - _barSize;
			if (_paneSize > barLimit)
				_paneSize = barLimit;
		}
	}

	SetBarRect();
return true;
}

void CSplitter::DrawXorBar(HDC hdc, int x1, int y1, int width, int height){
	static WORD _dotPatternBmp[8] = 
	{ 
		0x00aa, 0x0055, 0x00aa, 0x0055, 
		0x00aa, 0x0055, 0x00aa, 0x0055
	};

	HBITMAP hbm;
	HBRUSH  hbr, hbrushOld;

	hbm = ::CreateBitmap(8, 8, 1, 1, _dotPatternBmp);
	hbr = ::CreatePatternBrush(hbm);
	
	::SetBrushOrgEx(hdc, x1, y1, 0);
	hbrushOld = (HBRUSH)::SelectObject(hdc, hbr);
	
	::PatBlt(hdc, x1, y1, width, height, PATINVERT);
	
	::SelectObject(hdc, hbrushOld);
	
	::DeleteObject(hbr);
	::DeleteObject(hbm);
}


/********************************************************************
*	Section:
*		Dialog box classes.
*
*	Content:
*		CFileDlgBase
*		CDlgBase
*		CTabbedDlg
*		CShellDlg
*
*	Revisions:	
*
********************************************************************/
/********************************************************************
*	Class:	CFileDlgBase.
*
*	Purpose:	Open/Save Dlg.
*
*	Revisions:	
*
********************************************************************/
CFileDlgBase::CFileDlgBase(){
	_ofn.lStructSize = sizeof(OPENFILENAME);	// DWORD
	Reset();
}

CFileDlgBase::~CFileDlgBase(){
	FreeBuffer();
}

void CFileDlgBase::Reset(void){
	_nextFileName		= NULL;

	// Set methods.
	_ofn.lpstrTitle 		= 0;					// LPCTSTR
	_ofn.nFilterIndex 		= 0;					// DWORD
	_ofn.lpstrFilter 		= 0;					// LPCTSTR
	_ofn.lpstrDefExt 		= 0;					// LPCTSTR
	_ofn.Flags 			= 0;					// DWORD

	_ofn.lpstrInitialDir 	= 0;					// LPCTSTR

	// Get methods.
	_ofn.nFileOffset 		= 0;					// WORD
	_ofn.nFileExtension 	= 0;					// WORD

	// Unused.
	_ofn.hInstance 		= 0;					// HINSTANCE
	_ofn.lpstrCustomFilter 	= 0;					// LPTSTR
	_ofn.nMaxCustFilter 	= 0;					// DWORD
	_ofn.lpstrFileTitle 	= 0;					// LPTSTR
	_ofn.nMaxFileTitle 	= 0;					// DWORD
	_ofn.lCustData 		= 0;					// DWORD
	_ofn.lpfnHook 		= 0;					// LPOFNHOOKPROC
	_ofn.lpTemplateName	= 0;					// LPCTSTR
}

void CFileDlgBase::SetData(char *filter, char *defExt, DWORD flags){
	SetFilter(filter);
	SetDefExt(defExt);
	SetFlags(flags);
}

void CFileDlgBase::SetTitle(char *title){
	_ofn.lpstrTitle = title;
}

void CFileDlgBase::SetFilterIndex(DWORD filterIndex){
	_ofn.nFilterIndex	= filterIndex;
}

void CFileDlgBase::SetFilter(char *filter){
	_ofn.lpstrFilter = filter;
}

void CFileDlgBase::SetDefExt(char *defExt){
	_ofn.lpstrDefExt = defExt;
}

void CFileDlgBase::SetFlags(DWORD flags){
	_ofn.Flags = flags;
}

void CFileDlgBase::SetInitialDir(char *lpstrInitialDir){
	_ofn.lpstrInitialDir = lpstrInitialDir;
}

bool CFileDlgBase::GetDirectory(char *dirBuffer){
	char *fileName = _ofn.lpstrFile;
	WORD fileOffset = _ofn.nFileOffset;
	if (fileName && fileOffset > 1 && fileOffset < _MAX_DIR){
		strncpy(dirBuffer, fileName, fileOffset);
		dirBuffer[fileOffset-1] = '\0';
		return true;
	}
return false;
}

WORD CFileDlgBase::GetFileOffset(void){
return _ofn.nFileOffset;
}

WORD CFileDlgBase::GetFileExtension(void){
return _ofn.nFileExtension;
}

const char *CFileDlgBase::GetFileName(void){
	const char *pathName = _ofn.lpstrFile;
	if (pathName){
		if ((_ofn.Flags & OFN_ALLOWMULTISELECT) == OFN_ALLOWMULTISELECT)
			_nextFileName = pathName + _ofn.nFileOffset;
		return pathName + _ofn.nFileOffset;
	}
return NULL;
}

const char *CFileDlgBase::GetNextFileName(void){
	// Analyses a "path\0file1\0file2\0file[...]\0\0" string returned by Open/SaveDlg.
	const char *pathName = _ofn.lpstrFile;
	if (pathName && *pathName && (_ofn.Flags & OFN_ALLOWMULTISELECT)){
		// Initialize first call.
		if (_nextFileName){
			// Parse the string.
			while (*_nextFileName)
				_nextFileName++;
			// Get next char.
			_nextFileName++;
			// End of string ?
			if (*_nextFileName == '\0')
				_nextFileName = NULL;
			return _nextFileName;
		}
	}
return NULL;
}

bool CFileDlgBase::OpenFileName(CWindow *pWindow, DWORD nMaxFile){
	if (pWindow){
		_nextFileName		= NULL;
		_ofn.hwndOwner 		= pWindow->GetHwnd();	// HWND
		_ofn.lpstrFile 		= AllocBuffer(nMaxFile);
		_ofn.nMaxFile 		= nMaxFile;				// DWORD
		if (_ofn.lpstrFile){
			*_ofn.lpstrFile = '\0';
			return ::GetOpenFileName(&_ofn);
		}
	}
return false;
}

bool CFileDlgBase::SaveFileName(CWindow *pWindow, DWORD nMaxFile){
	if (pWindow){
		_nextFileName		= NULL;
		_ofn.hwndOwner 		= pWindow->GetHwnd();	// HWND
		_ofn.lpstrFile 		= AllocBuffer(nMaxFile);
		_ofn.nMaxFile 		= nMaxFile;				// DWORD
		if (_ofn.lpstrFile){
			*_ofn.lpstrFile = '\0';
			return ::GetSaveFileName(&_ofn);
		}
	}
return false;
}

void CFileDlgBase::FreeBuffer(void){
	if (_ofn.lpstrFile){
		free(_ofn.lpstrFile);
		_ofn.lpstrFile = NULL;
	}
}

char *CFileDlgBase::AllocBuffer(DWORD nMaxFile){
	FreeBuffer();
	_ofn.lpstrFile = (char *) malloc(nMaxFile);
return _ofn.lpstrFile;
}


/********************************************************************
*	Class:	CDlgBase.
*
*	Purpose:
*
*	Revisions:	
*
********************************************************************/
// Forward declaration of the dialog procedure.
BOOL CALLBACK DlgProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);

int CDlgBase::DialogBoxParam(CWindow *pWindow, WORD wResId, LPARAM lParam){
	/*	Creates a modal dialog using DialogBoxParam.	
		This procedure prevents creating a modal dialog if a modeless
		one already exists.								*/

	if (pWindow && !_hWnd){
		// Store CWindow parameters.
		_pParent = pWindow;
		_hInst = _pParent->GetHinstance();
		_lParam = (LPVOID) lParam;
	
		_modal = true;
		return ::DialogBoxParam(_hInst,
				MAKEINTRESOURCE(wResId),
				_pParent->GetHwnd(),
				(DLGPROC) DlgProc,
				(LPARAM) this);
	}
return -1;
}

HWND CDlgBase::CreateParam(CWindow *pWindow, WORD wResId, LPARAM lParam){
	/*	Creates a modeless dialog using CreateDialogParam.	
		This procedure prevents creating the same dialog twice.		*/

	if (pWindow){
		if (!_hWnd){
			// Store CWindow parameters.
			_pParent = pWindow;
			_hInst = _pParent->GetHinstance();
			_lParam = (LPVOID) lParam;
		
			_modal = false;
			return ::CreateDialogParam(_hInst,
					   MAKEINTRESOURCE(wResId),
					   _pParent->GetHwnd(),
					   (DLGPROC) DlgProc,
					   (LPARAM) this);
		}else{
			return _hWnd;
		}
	
	}
return 0;
}

BOOL CDlgBase::EndDlg(int nResult){
	/*	This function can	destroy both modal or modeless dialogs by calling
		respectivelly EndDialog or DestroyWindow. Then the dialog callback
		procedure will null _hWnd.								*/

	if (_hWnd){
		if (_modal)
			return ::EndDialog(_hWnd, nResult);
		else
			return ::DestroyWindow(_hWnd);
	}
return false;
}

HWND CDlgBase::GetItem(int nIDDlgItem){
return ::GetDlgItem(_hWnd, nIDDlgItem);
}

BOOL CDlgBase::SetItemText(HWND hItem, LPCTSTR lpString){
return ::SendMessage(hItem, WM_SETTEXT, 0, (long)lpString);
}

UINT CDlgBase::GetItemText(HWND hItem, LPTSTR lpString, int nMaxCount){
return ::SendMessage(hItem, WM_GETTEXT, nMaxCount, (long)lpString);
}

BOOL CDlgBase::CDlgProc(UINT Message, WPARAM, LPARAM){
	switch(Message){
		case WM_INITDIALOG:
			return TRUE;
	
		case WM_CLOSE:
			EndDlg(0); 
			break;
	}
return FALSE;
}


BOOL CALLBACK DlgProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam){
	/*	The dialog callback procedure redirects messages to the owner class.
		The procedure also initializes the owner class and nulls the window handle
		when the window is destroyed.								*/

	// 	FIXME: Returning FALSE while processing WM_INITDIALOG doesn't cancel the
	//	dialog creation.

	// Get a pointer to the owner CDlgBase class.
	CDlgBase *pDlgBase = (CDlgBase *) ::GetWindowLong(hWnd, DWL_USER);

	/* WM_INITDIALOG */
	if (pDlgBase == NULL){
		if (Message == WM_INITDIALOG){
			// Attach a pointer to the owner class and set the window handle.
			pDlgBase = (CDlgBase *) lParam;
			if (pDlgBase == NULL)
				return FALSE;	// Creation failed !
			::SetWindowLong(hWnd, DWL_USER, (LONG) pDlgBase);
			pDlgBase->SetHwnd(hWnd);

			// Process WM_INITDIALOG.
			return pDlgBase->CDlgProc(Message, wParam, (LONG) pDlgBase->GetParam());
		}else{
			return FALSE; // Can't process that message.
		}

	/* WM_NCDESTROY */
	}else if (Message == WM_NCDESTROY){
		pDlgBase->SetHwnd(0);
	}

return pDlgBase->CDlgProc(Message, wParam, lParam);
}


/********************************************************************
*	Class:	CTabbedDlg.
*
*	Purpose:
*
*	Revisions:	
*
********************************************************************/
CTabbedDlg::CTabbedDlg(){
	_hWndTab = 0;
	_pgRect.left 	= 0;
	_pgRect.top 	= 0;
	_pgRect.right 	= 100;
	_pgRect.bottom	= 100;
}

CTabbedDlg::~CTabbedDlg(){
}

bool CTabbedDlg::SetPagePosition(CDlgBase *pDlgBase, bool calcPos){
	if (_hWndTab && pDlgBase){
		// Get child dialog handle.
		HWND hChild = pDlgBase->GetHwnd();
		if (!hChild)
			return false;
		else if (calcPos){
			// Get tab's display area.
			RECT area;
			::GetWindowRect(_hWndTab, &area);
			::ScreenToClient(_hWnd, (POINT *) &area.left);
			::ScreenToClient(_hWnd, (POINT *) &area.right);
			TabCtrl_AdjustRect(_hWndTab, FALSE, &area);
			::CopyRect(&_pgRect, &area);
		
			// Get child dialog's rect.
			RECT child;
			::GetWindowRect(hChild, &child);
			::ScreenToClient(_hWnd, (POINT *) &child.left);
			::ScreenToClient(_hWnd, (POINT *) &child.right);
		
			// Center child dialog.
			int childWidth = child.right-child.left;
			int childHeight = child.bottom-child.top;
			int hMargin = ((area.right-area.left)-childWidth)/2;
			int vMargin = ((area.bottom-area.top)-childHeight)/2;
			_pgRect.left += hMargin;
			_pgRect.top += vMargin;
			_pgRect.right = childWidth;
			_pgRect.bottom = childHeight;
		}
		return ::SetWindowPos(hChild, 0, _pgRect.left, _pgRect.top, _pgRect.right, _pgRect.bottom, 0);
	}
return false;
}

void CTabbedDlg::OnNotify(int, LPNMHDR notify){
	// Dispatch tab control messages.
	switch (notify->code){
		case TCN_SELCHANGING:
			OnSelChanging(notify);
		break;

		case TCN_SELCHANGE:
			OnSelChange(notify);
		break;
	}
}

void CTabbedDlg::OnSelChanging(LPNMHDR notify){
	// Hide child dialog that is deselected.
	if (notify->hwndFrom == _hWndTab){
		CWindow * pPaneDlg = (CWindow *) GetTabItemParam();
		if (pPaneDlg){
			if (pPaneDlg->GetHwnd())
				pPaneDlg->Show(SW_HIDE);
		}
	}
}

void CTabbedDlg::OnSelChange(LPNMHDR notify){
	// Show child dialog that is selected.
	if (notify->hwndFrom == _hWndTab){
		CWindow * pPaneDlg = (CWindow *) GetTabItemParam();
		if (pPaneDlg){
			if (pPaneDlg->GetHwnd())
				pPaneDlg->Show();
				pPaneDlg->SetFocus();
		}
	}
}

LPARAM CTabbedDlg::GetTabItemParam(void){
	if (!_hWndTab)
		return false;
	int iItem = TabCtrl_GetCurSel(_hWndTab);

	_tcitem.mask = TCIF_PARAM;
	BOOL result = TabCtrl_GetItem(_hWndTab, iItem, &_tcitem);
	if (result)
		return _tcitem.lParam;
return 0;
}


/********************************************************************
*	Class:	CShellDlg.
*
*	Purpose:	
*
*	Revisions:	
*
********************************************************************/
CShellDlg::CShellDlg(){
	// Get shell task allocator.
	if(SHGetMalloc(&_pMalloc) == (HRESULT) E_FAIL)
		_pMalloc = NULL;
}

CShellDlg::~CShellDlg(){
	// Decrements the reference count.
	if (_pMalloc)
		_pMalloc->Release();
}

bool CShellDlg::BrowseForFolder(CWindow * pWindow, LPSTR pszDisplayName, 
		LPCSTR lpszTitle, UINT ulFlags, BFFCALLBACK lpfn, LPARAM lParam, int iImage){

	// Initialize output buffer.
	*pszDisplayName = '\0';
	// BROWSEINFO.
	if (!pWindow)
		_bi.hwndOwner = 0;
	else
		_bi.hwndOwner = pWindow->GetHwnd();
	_bi.pidlRoot = NULL;
	_bi.pszDisplayName = pszDisplayName;
	_bi.lpszTitle = lpszTitle;
	_bi.ulFlags = ulFlags;
	_bi.lpfn = lpfn;
	_bi.lParam = lParam;
	_bi.iImage = iImage;

	LPITEMIDLIST pidl = SHBrowseForFolder(&_bi);
	if(!pidl)
		return false;
	
	if(!SHGetPathFromIDList(pidl, pszDisplayName))
		return false;

	_pMalloc->Free(pidl);
return true;
}


/********************************************************************
*	Section:
*		Application classes.
*
*	Content:
*		CWinBase
*		CSDIBase
*		CMDIBase
*		CMDIClient
*		CMDIChild
*
*	Revisions:	
*
********************************************************************/
/********************************************************************
*	Class:	CWinBase.
*
*	Purpose:	Base Application class.
*
*	Revisions:	
*
********************************************************************/
CWinBase::CWinBase(){
	hPrevInst	= 0;
	lpCmdLine	= NULL;
	nCmdShow	= SW_SHOW;

	_bWinNT 	= false;
	strcpy(	appName, 	"CWinBase");
}

CWinBase::~CWinBase(){
}

bool CWinBase::Init(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, 
												int nCmdShow){
	_hInst 	= hInstance;
	hPrevInst	= hPrevInstance;
	lpCmdLine	= lpCmdLine;
	nCmdShow	= nCmdShow;
return true;
}	

bool CWinBase::SetName(char * name, char * version){
	strcpy(appName, name);
	strcat(appName, " ");

	if(version)
		strcat(appName, version);
return true;
}	

bool CWinBase::IsWinNT(void){
	OSVERSIONINFO osv = {sizeof(OSVERSIONINFO), 0, 0, 0, 0, ""};
	::GetVersionEx(&osv);

	_bWinNT = (osv.dwPlatformId == VER_PLATFORM_WIN32_NT);
return _bWinNT;
}	


/********************************************************************
*	Class:	CSDIBase.
*
*	Purpose:	Base SDI class.
*
*	Revisions:	
*
********************************************************************/
// Forward declaration of main window procedure.
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);

CSDIBase::CSDIBase(){
	_mainClass[0] 	= 0;
	_hAccel 		= NULL;
}

int CSDIBase::Run(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, 
												int nCmdShow){
	Init(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
	if (!CustomInit()){
		Release();
		return 1;
	}

	MSG Msg;
	if (CreateUI()){
		while(::GetMessage(&Msg, NULL, 0, 0)){
			if (!::TranslateAccelerator(_hWnd, _hAccel, &Msg)){
				::TranslateMessage(&Msg);
				::DispatchMessage(&Msg);
			}
		}
	}else{
		debugf("CSDIBase::Run: CreateUI() failed !");
	}

	Release();
return Msg.wParam;
}

bool CSDIBase::CustomInit(void){
return true;
}

bool CSDIBase::Release(void){
return true;
}

bool CSDIBase::CreateUI(void){
return false;
}

bool CSDIBase::MainRegisterEx(const char * className){
	strcpy(_mainClass, className);

	// Default values.
	_wc.cbSize			= sizeof(WNDCLASSEX);
	_wc.lpfnWndProc		= MainWndProc;
	_wc.hInstance		= _hInst;
	_wc.lpszClassName	= _mainClass;
	_wc.cbClsExtra		= 0;
	_wc.cbWndExtra		= sizeof(CSDIBase *);
return RegisterClassEx(&_wc);
}

LRESULT CALLBACK CSDIBase::CMainWndProc(UINT Message, WPARAM wParam, LPARAM lParam){

	switch (Message){
		case WM_DESTROY:
		PostQuitMessage (0);
		return 0;
	}
	
return DefWindowProc(_hWnd, Message, wParam, lParam);
}

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam){
	/*	The window callback procedure redirects messages to the owner class.
		The procedure also initializes the owner class and nulls the window handle
		when the window is destroyed.								*/

	// Get a pointer to the owner CSDIBase class.
	CSDIBase *pSdiBase = (CSDIBase *) ::GetWindowLong(hWnd, 0);

	/* WM_NCCREATE */
	if (pSdiBase == NULL){
		if (Message == WM_NCCREATE){
			// Attach a pointer to the owner class and set the window handle.
			pSdiBase = (CSDIBase *) ((CREATESTRUCT *)lParam)->lpCreateParams;
			if (pSdiBase == NULL)
				return FALSE;	// Creation failed !
			::SetWindowLong(hWnd, 0, (LONG) pSdiBase);
			pSdiBase->SetHwnd(hWnd);

			// Process WM_NCCREATE.
			return pSdiBase->CMainWndProc(Message, wParam, lParam);
		}else{
			return ::DefWindowProc(hWnd, Message, wParam, lParam);
		}

	/* WM_NCDESTROY */
	}else if (Message == WM_NCDESTROY){
		pSdiBase->SetHwnd(0);
	}

return pSdiBase->CMainWndProc(Message, wParam, lParam);
}


/********************************************************************
*	Class:	CMDIBase.
*
*	Purpose:	Base MDI class.
*
*	Revisions:	
*
********************************************************************/
// Forward declaration of the child window procedure.
LRESULT CALLBACK ChildWndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);

CMDIBase::CMDIBase(){
	_pMdiClient = NULL;
}

bool CMDIBase::SetMDIClient(CMDIClient *MdiClient){
	if (!_pMdiClient && MdiClient){
		_pMdiClient = MdiClient;
		return true;
	}
return false;
}

bool CMDIBase::ChildRegisterEx(const char *className){
	if (_pMdiClient){
		strcpy(_pMdiClient->childClass, className);
	
		// Default values.
		_wc.cbSize			= sizeof(WNDCLASSEX);
		_wc.lpfnWndProc		= ChildWndProc;
		_wc.hInstance		= _hInst;
		_wc.lpszClassName	= className;
		_wc.cbClsExtra		= 0;
		_wc.cbWndExtra		= sizeof(CMDIBase *);
		return RegisterClassEx(&_wc);
	}
return false;
}

int CMDIBase::Run(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, 
												int nCmdShow){
	if (!_pMdiClient)
		return 1;

	Init(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
	if (!CustomInit()){
		Release();
		return 1;
	}

	MSG Msg;
	if (CreateUI()){
		while(::GetMessage(&Msg, NULL, 0, 0)){
			if (!::TranslateAccelerator(_hWnd, _hAccel, &Msg)){
				if (!_pMdiClient->TranslateMDISysAccel(&Msg)){
					::TranslateMessage(&Msg);
					::DispatchMessage(&Msg);
				}
			}
		}
	}else{
		debugf("CMDIBase::Run: CreateUI() failed !");
	}

	Release();
return Msg.wParam;
}

bool CMDIBase::CustomInit(void){
return true;
}

bool CMDIBase::Release(void){
return true;
}

bool CMDIBase::CreateUI(void){
return false;
}

LRESULT CALLBACK CMDIBase::CMainWndProc(UINT, WPARAM, LPARAM){
return 0;
}


/********************************************************************
*	Class:	CMDIClient.
*
*	Purpose:	
*
*	Revisions:	
*
********************************************************************/
CMDIClient::CMDIClient(){
	childClass[0] = 0;
}

HWND CMDIClient::CreateEx(CWindow *pWindow, DWORD dwExStyle, LPCTSTR lpWindowName, 
		DWORD dwStyle, int x, int y, int nWidth, int nHeight, HMENU hMenu, LPCLIENTCREATESTRUCT ccs){

	if (pWindow && !_hWnd){
		// Store a pointer to parent class.
		_pParent = pWindow;

		// Get parent class handles.
		_hInst = _pParent->GetHinstance();
	
		_hWnd = CreateWindowEx(
			dwExStyle,
			"mdiclient", 
			lpWindowName,
			dwStyle,
			x, 
			y, 
			nWidth, 
			nHeight, 
			_pParent->GetHwnd(),
			hMenu,
			_hInst, 
			(LPVOID) ccs);
		
		return _hWnd;
	}
return 0;
}


/********************************************************************
*	Class:	CMDIChild.
*
*	Purpose:	Base Windows Class.
*
*	Revisions:	
*
********************************************************************/
CMDIChild::CMDIChild(){
	_pFrame = NULL;
}

HWND CMDIChild::CreateEx(CMDIClient *pMdiClient, DWORD dwExStyle, LPCTSTR lpWindowName, 
	DWORD dwStyle, int x, int y, int nWidth, int nHeight, HMENU hMenu, LPVOID lpParam){

	if (pMdiClient && pMdiClient->GetHwnd() && !_hWnd){
		// Store pointers, lParam and _hInst.
		_pFrame = (CMDIBase *) pMdiClient->GetParent(); // Owner of CChildWndProc.
		_pParent = pMdiClient;
		_hInst = _pParent->GetHinstance();
		_lParam = lpParam;
	
		HWND hWnd = CreateWindowEx(
					dwExStyle,
					pMdiClient->childClass,
					lpWindowName,
					dwStyle,
					x,
					y,
					nWidth,
					nHeight,
					_pParent->GetHwnd(),
					hMenu,
					_hInst,
					this);
		return hWnd;
	}
return 0;
}

LRESULT CALLBACK CMDIChild::CChildWndProc(UINT, WPARAM, LPARAM){
return 0;
}

LRESULT CALLBACK ChildWndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam){
	/*	The child window procedure redirects messages to the owner class.
		The procedure also initializes the owner class and nulls the window handle
		when the window is destroyed.
		A child list is used to free MdiChild objects.							*/

	// Get a pointer to the owner CMDIChild class.
	CMDIChild *pMdiChild = (CMDIChild *) GetWindowLong(hWnd, 0);

	/* WM_NCCREATE */
	if (pMdiChild == NULL){
		if (Message == WM_NCCREATE){
			// Attach a pointer to the owner class and set the window handle.
			pMdiChild = (CMDIChild *) ((MDICREATESTRUCT *) ((CREATESTRUCT *)lParam)->lpCreateParams)->lParam;
			if (pMdiChild == NULL)
				return FALSE;	// Creation failed !
			::SetWindowLong(hWnd, 0, (LONG) pMdiChild);
			pMdiChild->SetHwnd(hWnd);

			// Insert MdiChild to the child list.
			((CMDIClient *) pMdiChild->GetParent())->childList.InsertFirst(pMdiChild);

			// Process WM_NCCREATE.
			return pMdiChild->CChildWndProc(Message, wParam, lParam);
		}else{
			return ::DefMDIChildProc(hWnd, Message, wParam, lParam);
		}

	/* WM_NCDESTROY */
	}else if (Message == WM_NCDESTROY){
//		::MessageBox(0, "destroy mdi child", "msg", MB_OK);
		// Free MdiChild object.
		((CMDIClient *) pMdiChild->GetParent())->childList.Destroy(pMdiChild);
		return ::DefMDIChildProc(hWnd, Message, wParam, lParam);
	}

return pMdiChild->CChildWndProc(Message, wParam, lParam);
}
