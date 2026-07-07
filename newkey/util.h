/*------------------------------------------------------------------------------
UniKey - Vietnamese Keyboard for Windows
Copyright (C) 1998-2002 Pham Kim Long
Contact: longp@cslab.felk.cvut.cz

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
--------------------------------------------------------------------------------*/
#ifndef _UTIL_H
#define _UTIL_H
// Define window wrapper classes (like MFC window classes)

LRESULT CALLBACK GenWindowProc(
							HWND hWnd,
							UINT uMsg,
							WPARAM wParam,
							LPARAM lParam);

/*--------------------------------------------------------------------------
  Class:    CVirWindow

  Summary:  Abstract base class for wrapping a window.
----------------------------------------------------------------------------*/
class CVirWindow
{
public:
  HWND m_hWnd;
  // Constructor
  CVirWindow()
  {
    m_hInst = NULL;
    m_hWnd = NULL;
  };

  // Destructor
  virtual ~CVirWindow(){};

  // Envelopes the Windows' CreateWindow function call.
  HWND Create(
	     DWORD dwExStyle,
         LPTSTR lpszClassName,   // Address of registered class name
         LPTSTR lpszWindowName,  // Address of window name
         DWORD dwStyle,          // Window style
         int x,                  // Horizontal position of window
         int y,                  // Vertical position of window
         int nWidth,             // Window width
         int nHeight,            // Window height
         HWND hWndParent,        // Handle of parent or owner window
         HMENU hmenu,            // Handle of menu, or child window identifier
         HINSTANCE hinst);       // Handle of application instance

  // Get the protected handle of this window.
  HWND GetHwnd(void)
  {
    return(m_hWnd);
  }

  virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;

protected:
  // Application instance handle.
  HINSTANCE m_hInst;

  friend LRESULT CALLBACK ::GenWindowProc(
								HWND hWnd,
								UINT uMsg,
								WPARAM wParam,
								LPARAM lParam);
};


BOOL CALLBACK GenDialogProc(
						HWND hWndDlg,
						UINT uMsg,
						WPARAM wParam,
						LPARAM lParam);

/*--------------------------------------------------------------------------
  Class:    CVirDialog

  Summary:  Abstract base class for wrapping a Windows dialog box window.
----------------------------------------------------------------------------*/
class CVirDialog
{
public:
	HWND m_hWnd;
	// Destructor
	//virtual ~CVirDialog(){};
	//CVirDialog(HINSTANCE hInst, LPCSTR lpszTemplate, HWND hWndOwner = NULL);
	CVirDialog();

	int init(HINSTANCE hInst, LPCTSTR lpszTemplate, HWND hWndOwner = NULL);
	virtual int ShowDialog();
	virtual HWND ShowModelessDialog();

	virtual BOOL DialogProc(
						HWND hWndDlg,
						UINT uMsg,
						WPARAM wParam,
						LPARAM lParam);
	virtual BOOL onInitDialog();
	virtual BOOL onCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL onOK();
	virtual BOOL onCancel();
	virtual BOOL onDestroy();

protected:
  HINSTANCE m_hInst;
  LPCTSTR m_lpszTemplate;
  HWND m_hWndOwner;
  int m_isModeless;

  friend BOOL CALLBACK ::GenDialogProc(
								HWND hWndDlg,
								UINT uMsg,
								WPARAM wParam,
								LPARAM lParam);
};

/*
struct DlgItemInfo {
	UINT id;
	TCHAR *text;
};
*/

struct AnsiDlgItemInfo {
	UINT id;
	char *text;
};

struct UniDlgItemInfo {
	UINT id;
	wchar_t *text;
};

#ifdef _UNICODE
typedef UniDlgItemInfo DlgItemInfo;
#else
typedef AnsiDlgItemInfo DlgItemInfo;
#endif

struct DlgText {
	DlgItemInfo *list;
	int count;
	TCHAR fontName[100];
	int fontSize;
};

#define DLG_LANG_TOTAL 2 // currently supports 2 languages

class CMultiLangDialog : virtual public CVirDialog
{
protected:
	HFONT m_hStdFont;
	DlgText m_dlgText[DLG_LANG_TOTAL]; 
public:
	//int init(HINSTANCE hInst, LPCTSTR lpszTemplate, HWND hWndOwner = NULL);
	// helper functions, must be called explicitly by subclasses
	void initLanguageInfo();
	void setupLanguage(int langIdx, DlgItemInfo *texts, int count, const TCHAR *fontName = NULL, int fontSize = 8);
	void applyLanguage(int langIdx);
	void setDlgFont(int langIdx);
	void cleanup(); // derive classes must explicitly call this function for cleaning up.
};

HFONT CustomCreateFont(HDC hDC, LPCTSTR face, int size);


#endif
