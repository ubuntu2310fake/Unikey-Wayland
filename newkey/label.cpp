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
#include "label.h"

LRESULT CALLBACK GenLabelProc(
			HWND hWnd,      // handle to window
			UINT uMsg,      // message identifier
			WPARAM wParam,  // first message parameter
			LPARAM lParam   // second message parameter
			)
{
	CSmLabel *pLabel = (CSmLabel *)GetWindowLong(hWnd, GWL_USERDATA);
	if (pLabel)
		return pLabel->wndProc(hWnd, uMsg, wParam, lParam);
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

//-----------------------------------------------
// Attach the static label to this CSmLabel object
//-----------------------------------------------
void CSmLabel::attachWindow(HWND hWnd)
{
	m_hWnd = hWnd;
	SetWindowLong(hWnd, GWL_USERDATA, (LONG)this);
	SetWindowLong(hWnd, GWL_STYLE, SS_NOTIFY | GetWindowLong(hWnd, GWL_STYLE));
	// Subclass the button
	m_prevWndProc = SetWindowLong(hWnd, GWL_WNDPROC, (LONG)GenLabelProc);
}

//-----------------------------------------------
void CSmLabel::init(HINSTANCE hInst, HWND hOwner, HWND hWnd)
{
	m_hInstance = hInst;
	m_hOwner = hOwner;
	attachWindow(hWnd);
	// setup GDI
	m_textColor = GetSysColor(COLOR_WINDOWTEXT);
	m_hBrush = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
	HFONT hFont = (HFONT)SendMessage(hWnd, WM_GETFONT, 0, 0);
	if (hFont != NULL)
		GetObject(hFont,sizeof(m_lf),&m_lf);
	else
		GetObject((HFONT)GetStockObject(DEFAULT_GUI_FONT),sizeof(m_lf),&m_lf);
	m_hFont = CreateFontIndirect(&m_lf);
	m_hCursor = NULL;
}

//-----------------------------------------------
CSmLabel::~CSmLabel()
{
	DeleteObject(m_hFont);
	DeleteObject(m_hBrush);
}

//-----------------------------------------------
void CSmLabel::setBkColor(COLORREF color)
{
	if (m_hBrush)
		::DeleteObject(m_hBrush);
	m_hBrush = ::CreateSolidBrush(color);
}

//-----------------------------------------------
void CSmLabel::reconstructFont()
{
	DeleteObject(m_hFont);
	m_hFont = CreateFontIndirect(&m_lf);

}

//-----------------------------------------------
HBRUSH CSmLabel::onCtlColor(HDC hDC, HWND hCtrl)
{
	SetTextColor(hDC, m_textColor);
	SelectObject(hDC, m_hFont);
	SetBkMode(hDC, TRANSPARENT);
	return m_hBrush;
}

//-------------------------------------------------------
LRESULT CSmLabel::wndProc(
				HWND hWnd,      // handle to window
				UINT uMsg,      // message identifier
				WPARAM wParam,  // first message parameter
				LPARAM lParam)  // second message parameter
{
	switch (uMsg) {
	case WM_CTLCOLORSTATIC:
		// This is actually a reflection message from the parent window
		// The parent window must send this message each time it receives WM_CTLCOLORSTATIC
		return (LRESULT)onCtlColor((HDC)wParam, (HWND)lParam);
	case WM_SETCURSOR:
		if (m_hCursor)
			SetCursor(m_hCursor);
		return TRUE;
	}
	return CallWindowProc((WNDPROC)m_prevWndProc, hWnd, uMsg, wParam, lParam);
}


HCURSOR CLinkLabel::m_shLinkCursor = NULL;

//-------------------------------------------------------
void CLinkLabel::init(HINSTANCE hInst, HWND hOwner, HWND hWnd, const TCHAR *link)
{
	CSmLabel::init(hInst, hOwner, hWnd);
	setCursor(m_shLinkCursor);
	setHyperLink(link);
	setTextColor(RGB(0, 0, 255));
	setFontUnderline();
	//setFontBold();
	reconstructFont();
}


//-------------------------------------------------------
LRESULT CLinkLabel::wndProc(
				HWND hWnd,      // handle to window
				UINT uMsg,      // message identifier
				WPARAM wParam,  // first message parameter
				LPARAM lParam)  // second message parameter
{
	switch (uMsg) {
	case WM_LBUTTONDOWN:
		ShellExecute(NULL, _TEXT("open"), m_link, NULL, NULL, SW_SHOWNORMAL);
		break;
	}
	return CSmLabel::wndProc(hWnd, uMsg, wParam, lParam);
}

