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
#include "prehdr.h"
#include "util.h"

/*--------------------------------------------------------------------------
  Function: GenWindowProc

  Summary:  Standard WindowProc callback function that forwards Windows
            messages on to the CVirWindow::WindowProc method.  This
            Window procedure expects that it will receive a "this"
            pointer as the lpCreateParams member passed as part of the
            WM_NCCREATE message.  It saves the "this" pointer in the
            GWL_USERDATA field of the window structure.
----------------------------------------------------------------------------*/
LRESULT CALLBACK GenWindowProc(
						HWND hWnd,
						UINT uMsg,
						WPARAM wParam,
						LPARAM lParam)
{
  // Get a pointer to the window class object.
  CVirWindow* pWin = (CVirWindow*) GetWindowLong(hWnd, GWL_USERDATA);

  switch (uMsg)
  {
    case WM_NCCREATE:
      // Get the initial creation pointer to the window object
      pWin = (CVirWindow *) ((CREATESTRUCT *)lParam)->lpCreateParams;

      pWin->m_hWnd = hWnd;

      // Set its USERDATA DWORD to point to the window object
      SetWindowLong(hWnd, GWL_USERDATA, (long) pWin);
      break;
/*
    case WM_DESTROY:
      // This is our signal to destroy the window object.
      SetWindowLong(hWnd, GWL_USERDATA, 0);
	  if (pWin) {
		  delete pWin;
		  pWin = NULL;
	  }
      break;
*/
    default:
      break;
  }

  // Call its message proc method.
  if (NULL != pWin)
    return (pWin->WindowProc(uMsg, wParam, lParam));
  else
    return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}


/*--------------------------------------------------------------------------
  Method:   CVirWindow::Create

  Summary:  Envelopes the Windows' CreateWindow call.  Uses its
            window-creation data pointer to pass the 'this' pointer.
----------------------------------------------------------------------------*/
HWND CVirWindow::Create(
	   DWORD dwExStyle,
       LPTSTR lpszClassName,
       LPTSTR lpszWindowName,
       DWORD dwStyle,
       int x,
       int y,
       int nWidth,
       int nHeight,
       HWND hWndParent,
       HMENU hMenu,
       HINSTANCE hInst)
{
  // Remember the passed instance handle in a member variable of the
  // C++ Window object.
  m_hInst = hInst;

  m_hWnd = CreateWindowEx(dwExStyle,
				lpszClassName,
				lpszWindowName,
				dwStyle,
				x,
				y,
				nWidth,
				nHeight,
				hWndParent,
				hMenu,
				hInst,
				this);

  return (m_hWnd);
}

/*-------------------------------------------------------------------------
  Function: GenDialogProc

  Summary:  The general dialog procedure callback function.  Used by all
            CVirDialog class objects.  This procedure is the DialogProc.
            registered for all dialogs created with the CVirDialog class.
            It uses the parameter passed with the WM_INITDIALOG message
            to identify the dialog classes' "this" pointer which it then
            stores in the window structure's GWL_USERDATA field.
            All subsequent messages can then be forwarded on to the
            correct dialog class's DialogProc method by using the pointer
            stored in the GWL_USERDATA field.
-------------------------------------------------------------------------*/
BOOL CALLBACK GenDialogProc(
					HWND hWndDlg,
					UINT uMsg,
					WPARAM wParam,
					LPARAM lParam)
{
	// Get a pointer to the window class object.
	CVirDialog* pdlg = (CVirDialog*) GetWindowLong(hWndDlg, GWL_USERDATA);

	switch (uMsg) {
	case WM_INITDIALOG:
		// Get a pointer to the window class object.
		pdlg = (CVirDialog*) lParam;

		// Assign the m_hWnd member variable.
		pdlg->m_hWnd = hWndDlg;

		// Set the USERDATA word to point to the class object.
		SetWindowLong(hWndDlg, GWL_USERDATA, (long) pdlg);
		break;
    case WM_DESTROY:
		pdlg->m_hWnd = NULL;
		break;
	default:
		break;
	}

	// Call its message proc method.
	BOOL ret = FALSE;
	if (pdlg != (CVirDialog *) 0)
		ret = pdlg->DialogProc(hWndDlg, uMsg, wParam, lParam);
	if (uMsg == WM_DESTROY && pdlg)
		pdlg->m_hWnd = 0;
	return ret;
}

/*-------------------------------------------------------------------------
  CVirDialog:constructor
-------------------------------------------------------------------------*/
CVirDialog::CVirDialog()
{
	m_hWnd = m_hWndOwner = NULL;
	m_hInst = NULL;
	m_lpszTemplate = NULL;
	m_isModeless = 0;
}


/*-------------------------------------------------------------------------
 Initialize the dialog
-------------------------------------------------------------------------*/
int CVirDialog::init(HINSTANCE hInst, LPCTSTR lpszTemplate, HWND hWndOwner)
{
	m_hInst = hInst;
	m_lpszTemplate = lpszTemplate;
	m_hWndOwner = hWndOwner;
	return 1;
}

/*-------------------------------------------------------------------------
  Method:   CVirDialog::ShowDialog

  Summary:  like DialogBox Windows API function.
-------------------------------------------------------------------------*/
int CVirDialog::ShowDialog()
{
  int iResult;

  m_isModeless = 0;
  // Create and show the dialog on screen. Pass the 'this' pointer to the
  // dialog object so that it can be assigned inside the dialog object
  // during WM_INITDIALOG and later available to the dailog procedure
  // via the GWL_USERDATA associated with the dialog window.
  iResult = ::DialogBoxParam(
                m_hInst,
                m_lpszTemplate,
                m_hWndOwner,
                (DLGPROC)::GenDialogProc,
                (long)this);

  return (iResult);
}

//------------------------------------------
HWND CVirDialog::ShowModelessDialog()
{
	m_isModeless = 1;
	HWND hWnd = CreateDialogParam(m_hInst, m_lpszTemplate, m_hWndOwner, (DLGPROC)::GenDialogProc, (LPARAM)this);
	if (hWnd != NULL)
		ShowWindow(hWnd, SW_SHOW);
	return hWnd;
}

//------------------------------------------
BOOL CVirDialog::DialogProc(
						HWND hWndDlg,
						UINT uMsg,
						WPARAM wParam,
						LPARAM lParam)
{
	switch (uMsg) {
	case WM_COMMAND:
		return onCommand(wParam, lParam);
	case WM_INITDIALOG:
		return onInitDialog();
	case WM_DESTROY:
		return onDestroy();
	}
	return FALSE;
}

//------------------------------------------
BOOL CVirDialog::onCommand(WPARAM wParam, LPARAM lParam)
{
	int id = GET_WM_COMMAND_ID(wParam, lParam);

	switch (id) {
	case IDOK:
		if (onOK()) {
			if (m_isModeless) {
				DestroyWindow(m_hWnd);
				m_hWnd = NULL;
			}
			else
				EndDialog(m_hWnd, IDOK);
		}
		return TRUE;
	case IDCANCEL:
		if (onCancel()) {
			if (m_isModeless) {
				DestroyWindow(m_hWnd);
				m_hWnd = NULL;
			}
			else
				EndDialog(m_hWnd, IDCANCEL);
		}
		return TRUE;
	}
	return FALSE;
}

//------------------------------------------
BOOL CVirDialog::onOK()
{
	return TRUE;
}

//------------------------------------------
BOOL CVirDialog::onDestroy()
{
	return TRUE;
}

//------------------------------------------
BOOL CVirDialog::onCancel()
{
	return TRUE;
}


//------------------------------------------
BOOL CVirDialog::onInitDialog()
{
	return TRUE;
}

////////////////////////////
// Multi-language dialogs //
////////////////////////////
void CMultiLangDialog::initLanguageInfo()
{
	for (int i=0; i < DLG_LANG_TOTAL; i++) {
		m_dlgText[i].list = NULL;
		m_dlgText[i].fontName[0] = NULL;
	}
}

//----------------------------------------------
void CMultiLangDialog::setDlgFont(int langIdx)
{
	if (langIdx < 0 || langIdx >= DLG_LANG_TOTAL)
		return;
	if (m_hStdFont != NULL)
		DeleteObject(m_hStdFont);
	HDC hDC = GetDC(m_hWnd);

	// create font object
	if (m_dlgText[langIdx].fontName[0] == NULL)
		m_hStdFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	else
		m_hStdFont = CustomCreateFont(hDC, m_dlgText[langIdx].fontName, m_dlgText[langIdx].fontSize);

	// update dialog items
	SendMessage(m_hWnd, WM_SETFONT, (WPARAM)m_hStdFont, TRUE);
	HWND wndChild = GetTopWindow(m_hWnd);
	for ( ; wndChild != NULL; wndChild = GetWindow(wndChild, GW_HWNDNEXT))
		SendMessage(wndChild, WM_SETFONT, (WPARAM)m_hStdFont, TRUE);
	ReleaseDC(m_hWnd, hDC);
}

//----------------------------------------------
void CMultiLangDialog::setupLanguage(int langIdx, DlgItemInfo *texts, int count, const TCHAR *fontName, int fontSize)
{
	if (langIdx >= 0 && langIdx < DLG_LANG_TOTAL) {
		m_dlgText[langIdx].list = texts;
		m_dlgText[langIdx].count = count;
		m_dlgText[langIdx].fontSize = fontSize;
		if (fontName == NULL)
			m_dlgText[langIdx].fontName[0] = 0;
		else
			_tcscpy(m_dlgText[langIdx].fontName, fontName);
	}
}

//----------------------------------------------
// set text labels according to the chosen GUI language
// langID = 0 -> English (default)
//----------------------------------------------
void CMultiLangDialog::applyLanguage(int langIdx)
{
	//int items, i;
	if (langIdx < 0 || langIdx > DLG_LANG_TOTAL || m_dlgText[langIdx].list == NULL)
		return;
	DlgItemInfo *texts = m_dlgText[langIdx].list;
	int count = m_dlgText[langIdx].count;
	for (int i = 0; i < count; i++)
		SendDlgItemMessage(m_hWnd, texts[i].id, WM_SETTEXT, 0, (LPARAM)texts[i].text);
}

//-----------------------------------------
void CMultiLangDialog::cleanup()
{
	if (m_hStdFont != NULL) {
		DeleteObject(m_hStdFont);
		m_hStdFont = NULL;
	}
}

//-----------------------------------------
HFONT CustomCreateFont(HDC hDC, LPCTSTR face, int size)
{
	LOGFONT lf;
	if (face == NULL || face[0] == NULL)
		return (HFONT)GetStockObject(DEFAULT_GUI_FONT);

	memset(&lf, 0, sizeof(LOGFONT));
	lf.lfCharSet = ANSI_CHARSET;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	_tcscpy(lf.lfFaceName, face);
	lf.lfHeight = -MulDiv(size, GetDeviceCaps(hDC, LOGPIXELSY), 72);
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	//lf.lfPitchAndFamily = 
	lf.lfQuality = DEFAULT_QUALITY;
	lf.lfWeight = FW_NORMAL;
	return CreateFontIndirect(&lf);
}

