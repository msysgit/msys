/********************************************************************
*	Module:	winui.h. This is part of WinUI.
*
*	License:	Public domain.
*			2002 Manu B.
*
********************************************************************/
#ifndef WINUI_H
#define WINUI_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <shlobj.h>
#include <commdlg.h>
#include <commctrl.h>
#include "CList.h"

/*	WinUI tools 1.0.
	TODO: should be moved to a dedicated module containing
	all tips and tricks.							*/

// message box functions.
int MsgBox(HWND hWnd, UINT uType, char *lpCaption, const char *format, ...);
int debugf(const char *format, ...);

// converts the client coordinates of a point to window coordinates.
bool ClientToWindow(HWND hWnd, LPPOINT lpPoint);

class CSingleInstance {
	/*	Good trick from Redmond guys, to know
		if another instance of an application already
		exists.
		
		Reference: M$ KB (Q243953).				
		Note: In order to use a local CSingleInstance,
		we don't call CloseHandle().				*/
	
	public:
	CSingleInstance(LPCTSTR uniqueID){
		_hMutex = ::CreateMutex(NULL, TRUE, uniqueID);
		_lastError = ::GetLastError();
	}
	~CSingleInstance(){
		if (_hMutex)
			::ReleaseMutex(_hMutex);
	}
	bool AlreadyExists(void){
		return (_lastError == ERROR_ALREADY_EXISTS);
	}
	
	private:
	HANDLE _hMutex;
	DWORD _lastError;
};

class CCriticalSection {
	public:
	CCriticalSection() {::InitializeCriticalSection(&_cs);}
	~CCriticalSection() {::DeleteCriticalSection(&_cs);}
	void Enter() {::EnterCriticalSection(&_cs);}
	void Leave() {::LeaveCriticalSection(&_cs);}

	private:
	CRITICAL_SECTION _cs; 
};

class UseOle {
	/*	Automatic OLE Initialization.		*/
	
	public:
	UseOle() {_oleError = ::OleInitialize(NULL);}
	~UseOle() {::OleUninitialize();}

	HRESULT GetInitializeError(void) {return _oleError;}

	protected:
	HRESULT _oleError;
};

class CChrono {
	/*	A simple chrono.		*/
	
	public:
	CChrono() : _time(0) {};
	~CChrono() {};

	void Start(void) {_time = ::GetTickCount();}
	DWORD Stop(void){
		DWORD diff = ::GetTickCount() - _time;
		_time = 0;
		return diff;
	}

	protected:
	DWORD _time;
};


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
class CWindow {
	public:
	CWindow();
	virtual ~CWindow() {}

	virtual HWND CreateEx(CWindow *pWindow, DWORD dwExStyle, 
		LPCTSTR lpClassName, LPCTSTR lpWindowName, DWORD dwStyle, int x, int y, 
		int nWidth, int nHeight, HMENU hMenu, LPVOID lpParam);

	bool SubClass(void);
	virtual LRESULT CALLBACK CWndProc(UINT Message, WPARAM wParam, LPARAM lParam){
		return ::CallWindowProc(_WndProc, _hWnd, Message, wParam, lParam);
	}

	// SetHwnd should be use only in callback procedures.
	void SetHwnd(HWND hWnd) {_hWnd = hWnd;}
	bool IsThis(HWND hWnd) {return (hWnd == _hWnd);}
	CWindow *GetParent(void) {return _pParent;}
	HWND GetHwnd(void) {return _hWnd;}
	HINSTANCE GetHinstance(void) {return _hInst;}
	LPVOID GetParam(void) {return _lParam;}
	LONG SetLong(int nIndex, LONG dwNewLong) {return ::SetWindowLong(_hWnd, nIndex, dwNewLong);}
	LONG GetLong(int nIndex) {return ::GetWindowLong(_hWnd, nIndex);}

	LRESULT SendMessage(UINT Msg, WPARAM wParam = 0, LPARAM lParam = 0){
		return ::SendMessage(_hWnd, Msg, wParam, lParam);
	}

	virtual BOOL SetPosition(HWND hInsertAfter, int x, int y, int width, int height, UINT uFlags){
		return ::SetWindowPos(_hWnd, hInsertAfter, x, y, width, height, uFlags);
	}

	bool Show(int nCmdShow = SW_SHOWNORMAL) {return ::ShowWindow(_hWnd, nCmdShow);}
	HWND SetFocus(void) {return ::SetFocus(_hWnd);}
	
	protected:
	CWindow	*_pParent;
	HWND 	_hWnd;
	HINSTANCE _hInst;
	LPVOID	_lParam;
	WNDPROC	_WndProc;
};

class CToolBar : public CWindow {
	public:
	HWND CreateEx(CWindow *pWindow, DWORD dwExStyle, LPCTSTR lpWindowName, 
		DWORD dwStyle, int x, int y, int nWidth, int nHeight, HMENU hMenu = 0, LPVOID lpParam = NULL);

	// These procedures are usefull since there is no ToolBar macros.
	LRESULT AddBitmap(UINT resId, int nBmp, HINSTANCE hInstance);
	BOOL AddButtons(TBBUTTON *tbButtons, UINT numButtons) {
		return ::SendMessage(_hWnd, TB_ADDBUTTONS, (WPARAM) numButtons, (LPARAM) tbButtons); 
	}
};

class CStatusBar : public CWindow {
	public:
	HWND CreateEx(CWindow *pWindow, DWORD dwExStyle, LPCTSTR lpWindowName, 
		DWORD dwStyle, int x, int y, int nWidth, int nHeight, HMENU hMenu = 0, LPVOID lpParam = NULL);
};

class CTreeView : public CWindow {
	public:
	HWND CreateEx(CWindow *pWindow, DWORD dwExStyle, LPCTSTR lpWindowName, 
		DWORD dwStyle, int x, int y, int nWidth, int nHeight, HMENU hMenu = 0, LPVOID lpParam = NULL);
	HTREEITEM GetSelection(void) {return TreeView_GetSelection(_hWnd);}
	bool GetItemText(HTREEITEM hitem, char *pszText, int cchTextMax);
	LPARAM GetItemParam(HTREEITEM hitem);

	protected:
	// Insert structures.
	TVINSERTSTRUCT _tvins; 
	TVITEM _tvi;
};

class CListView : public CWindow {
	public:
	CListView();

	HWND CreateEx(CWindow *pWindow, DWORD dwExStyle, LPCTSTR lpWindowName, 
		DWORD dwStyle, int x, int y, int nWidth, int nHeight, HMENU hMenu = 0, LPVOID lpParam = NULL);
	BOOL GetItemParam(int iItem, LPARAM *lParam);
	void DeleteAllItems(void);

	protected:
	DWORD _lastRow;
	// Insert structure.
	LV_ITEM _lvi;
};

class CTabCtrl : public CWindow {
	public:
	HWND CreateEx(CWindow *pWindow, DWORD dwExStyle, LPCTSTR lpWindowName, 
		DWORD dwStyle, int x, int y, int nWidth, int nHeight, HMENU hMenu = 0, LPVOID lpParam = NULL);
	int	GetCurSel(void) {return TabCtrl_GetCurSel(_hWnd);}
	BOOL	SetItemParam(int iItem, LPARAM lParam);
	LPARAM GetItemParam(int iItem);
	int	InsertItem(int iItem, UINT mask, LPTSTR pszText, int cchTextMax, int iImage, LPARAM lParam);

	protected:
        TCITEM _tie;
};

class CScintilla : public CWindow {
	public:
	HWND CreateEx(CWindow *pWindow, DWORD dwExStyle, LPCTSTR lpWindowName, 
		DWORD dwStyle, int x, int y, int nWidth, int nHeight, HMENU hMenu = 0, LPVOID lpParam = NULL);
};

#define SPLSTYLE_HORZ 	0
#define SPLSTYLE_VERT 	1
#define SPLMODE_1		0
#define SPLMODE_2		1

class CSplitter : public CWindow {
	public:
	CSplitter();
	~CSplitter() {};

	// The regular CreateEx must fail.
	virtual HWND CreateEx(CWindow *, DWORD, LPCTSTR, LPCTSTR, DWORD, int, int, 
		int, int, HMENU, LPVOID) {return 0;}

	bool CreateEx(CWindow *PaneOne, CWindow *PaneTwo, bool vertical, int splitMode, 
				int x, int y, int nWidth, int nHeight, int paneSize, bool splitFrame);
	BOOL	SetFrameMargins(int frameMarginOne, int frameMarginTwo);
	BOOL SetPosition(HWND hInsertAfter, int x, int y, int width, int height, UINT uFlags);

	void SplitFrame(bool split = true);
	bool Show(int) {return false;};
	bool OnLButtonDown(HWND hWnd, short xPos, short yPos);
	void OnLButtonUp(HWND hWnd, short xPos, short yPos);
	void OnMouseMove(HWND hWnd, short xPos, short yPos);
	bool OnSetCursor(HWND hWnd, LPARAM lParam);

	void GetFrameRect(LPRECT frameRect);
	int GetPaneSize(void) {return _paneSize;}
	bool GetSplitState(void) {return _splitFrame;}

	protected:
	void	SetBarRect(void);
	void	SizeContent(void);
	bool	MoveBar(int paneSize);
	void	DrawXorBar(HDC hdc, int x1, int y1, int width, int height);

	CWindow	*_WndOne;
	CWindow	*_WndTwo;
	bool		_wndOneVisible;
	bool		_wndTwoVisible;

	bool		_splitFrame;
	int		_minPaneSize;
	int 		_barSize;
	bool 		_isVertical;
	int 		_splitMode;
	HCURSOR	_hCursor;

	int		_frameRectLeft;
	int		_frameRectTop;
	int		_frameRectRight;
	int		_frameRectBottom;

	int		_paneSize;

	int		_frameMarginOne;
	int		_frameMarginTwo;

	RECT		_barRect;

	bool		_mouseCaptured;
	POINT 	_startDragPos;
	POINT 	_xorBarPos;

	private:   
	bool 		_initialized;
};


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
class CFileDlgBase : public CWindow {
	public:
	CFileDlgBase();
	~CFileDlgBase();

	void Reset(void);
	void SetData(char *filter, char *defExt, DWORD flags);
	void SetTitle(char *title);
	void SetFilterIndex(DWORD filterIndex);
	void SetFilter(char *filter);
	void SetDefExt(char *defExt);
	void SetFlags(DWORD flags);
	void SetInitialDir(char *lpstrInitialDir);

	const char *GetPathName(void) {return _ofn.lpstrFile;}
	bool GetDirectory(char *dirBuffer);
	WORD GetFileOffset(void);
	WORD GetFileExtension(void);
	const char *GetFileName(void);
	const char *GetNextFileName(void);
	const char *GetInitialDir(void) {return _ofn.lpstrInitialDir;}

	bool OpenFileName(CWindow *pWindow, DWORD nMaxFile);
	bool SaveFileName(CWindow *pWindow, DWORD nMaxFile);
	OPENFILENAME _ofn;

	protected:
	void FreeBuffer(void);
	char *AllocBuffer(DWORD nMaxFile);

	const char *_nextFileName;
	private:   
};

class CDlgBase : public CWindow {
	public:
	CDlgBase() {}
	virtual ~CDlgBase() {}

	int DialogBoxParam(CWindow *pWindow, WORD wResId, LPARAM lParam);
	HWND CreateParam(CWindow *pWindow, WORD wResId, LPARAM lParam);
	BOOL EndDlg(int nResult = IDCANCEL);
	virtual BOOL CDlgProc(UINT Message, WPARAM wParam, LPARAM lParam);
	HWND GetItem(int nIDDlgItem);
	BOOL SetItemText(HWND hItem, LPCTSTR lpString);
 	UINT GetItemText(HWND hItem, LPTSTR lpString, int nMaxCount);
/*	TODO:
	BOOL SetItemInt(HWND hItem, UINT uValue, BOOL bSigned);
	UINT GetItemInt(HWND hItem, BOOL *lpTranslated, BOOL bSigned);
*/
	private:   
	bool _modal;
};

class CTabbedDlg : public CDlgBase
{
	public:
	CTabbedDlg();
	virtual ~CTabbedDlg();

	virtual void OnNotify(int idCtrl, LPNMHDR notify);
	virtual void OnSelChanging(LPNMHDR notify);
	virtual void OnSelChange(LPNMHDR notify);
	virtual bool SetPagePosition(CDlgBase *pDlgBase, bool calcPos = false);

	protected:
//TODO normalize "GetTabItemParam"
	LPARAM GetTabItemParam(void);

	HWND _hWndTab;
	RECT _pgRect;
	TCITEM _tcitem;

	private:   
};

class CShellDlg {
	public:
	CShellDlg();
	~CShellDlg();
	bool BrowseForFolder(CWindow *pWindow, LPSTR pszDisplayName, LPCSTR lpszTitle, 
		UINT ulFlags, BFFCALLBACK lpfn=0, LPARAM lParam=0, int iImage=0);

	protected:
	IMalloc *_pMalloc;
	BROWSEINFO _bi;
};


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
class CWinBase : public CWindow {
	public:
	CWinBase();
	~CWinBase();

	HINSTANCE 	hPrevInst;
	LPSTR 		lpCmdLine;
	int 			nCmdShow;
	char 			msgBuf[256];

	bool	Init(HINSTANCE hInstance, HINSTANCE hPrevInstance,
			LPSTR lpCmdLine, int nCmdShow);
	bool	IsWinNT(void);
	bool	ChangeDirectory(char *dir);

	protected:
	bool	_bWinNT;
};

class CSDIBase : public CWinBase {
	public:
	CSDIBase();
	virtual ~CSDIBase() {};

	virtual int	Run(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
	virtual LRESULT CALLBACK CMainWndProc(UINT Message, WPARAM wParam, LPARAM lParam);

	protected:
	virtual bool CustomInit(void);
	virtual bool Release(void);
	virtual bool CreateUI(void);
	bool MainRegisterEx(const char * className);

	WNDCLASSEX	_wc;
	HACCEL		_hAccel;
	char			_mainClass[16];
};

// forward declaration.
class CMDIBase;

class CMDIClient : public CWindow {
	public:
	CMDIClient();
	~CMDIClient() {};

	HWND CreateEx(CWindow *pWindow, DWORD dwExStyle, LPCTSTR lpWindowName, 
		DWORD dwStyle, int x, int y, int nWidth, int nHeight, HMENU hMenu, LPCLIENTCREATESTRUCT ccs);

	
	/* FIXME: doesn't work.
	bool	MDIDestroy(CMDIChild *mdiChild){
		if (mdiChild){
			debugf("hwnd: %d", _hWnd);
			::SendMessage(_hWnd, WM_MDIDESTROY, (WPARAM) mdiChild->GetHwnd(), 0);
			return true;
		}else{
			return false;
		}
	}*/
	
	BOOL TranslateMDISysAccel(LPMSG lpMsg) {return ::TranslateMDISysAccel(_hWnd, lpMsg);}

	LRESULT DefFrameProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
		return ::DefFrameProc(hWnd, _hWnd, uMsg, wParam, lParam);
	}

	char	childClass[16];
	CList	childList;
};

class CMDIChild : public CNode, public CWindow {
	public:
	CMDIChild();
	virtual ~CMDIChild() {};

	HWND CreateEx(CMDIClient *pMdiClient, DWORD dwExStyle, LPCTSTR lpWindowName, 
		DWORD dwStyle, int x, int y, int nWidth, int nHeight, HMENU hMenu = 0, LPVOID lpParam = NULL);

	virtual LRESULT CALLBACK CChildWndProc(UINT Message, WPARAM wParam, LPARAM lParam);
	// Unused.
	CMDIBase *GetMDIFrame(void) {return _pFrame;}

	protected:
	CMDIBase *_pFrame;

	private:   
};

class CMDIBase : public CSDIBase {
	public:
	CMDIBase();
	virtual ~CMDIBase() {};

	bool SetMDIClient(CMDIClient *MdiClient);
	virtual int	Run(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
	virtual LRESULT CALLBACK CMainWndProc(UINT Message, WPARAM wParam, LPARAM lParam);

	protected:
	virtual bool CustomInit(void);
	virtual bool Release(void);
	virtual bool CreateUI(void);
	bool ChildRegisterEx(const char *className);

	CMDIClient	*_pMdiClient;
};

#endif
