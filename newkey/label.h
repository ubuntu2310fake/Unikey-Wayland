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
#ifndef __SMART_LABEL_H
#define __SMART_LABEL_H

LRESULT CALLBACK GenLabelProc(
			HWND hWnd,      // handle to window
			UINT uMsg,      // message identifier
			WPARAM wParam,  // first message parameter
			LPARAM lParam   // second message parameter
			);

class CSmLabel
{
protected:
	HWND m_hOwner;
	HINSTANCE m_hInstance;
	LONG m_prevWndProc; // previous window procedure

	// color
	COLORREF	m_textColor;

	// GDI objects
	LOGFONT		m_lf;
	HFONT		m_hFont;
	HBRUSH		m_hBrush;
	HCURSOR		m_hCursor;

public:
	HWND m_hWnd;
	//CSmLabel();
	~CSmLabel();
	void init(HINSTANCE hInst, HWND hOwner, HWND hWnd);

	// color
	void setTextColor(COLORREF color);
	void setBkColor(COLORREF color);

	void setText(const TCHAR *text)
	{
		SetWindowText(m_hWnd, text);
	}

	// font settings
	void setFontBold(BOOL bBold = TRUE);
	void setFontUnderline(BOOL bSet = TRUE);
	void setFontItalic(BOOL bSet = TRUE);
	void setFontName(const TCHAR *fntName);
	void setFontSize(int nSize);
	void reconstructFont();

	void setCursor(HCURSOR hCursor);

	void redraw();

	friend LRESULT CALLBACK ::GenLabelProc(
			HWND hwnd,      // handle to window
			UINT uMsg,      // message identifier
			WPARAM wParam,  // first message parameter
			LPARAM lParam   // second message parameter
			);

protected:
	virtual LRESULT wndProc(
				HWND hwnd,      // handle to window
				UINT uMsg,      // message identifier
				WPARAM wParam,  // first message parameter
				LPARAM lParam   // second message parameter
				);
	virtual HBRUSH onCtlColor(HDC hDC, HWND hCtrl);
private:
	void attachWindow(HWND hWnd);
};

class CLinkLabel: public CSmLabel
{
public:
	void init(HINSTANCE hInst, HWND hOwner, HWND hWnd, const TCHAR *link);
	void setHyperLink(const TCHAR *link)
	{
		_tcscpy(m_link, link);
	}

	virtual LRESULT wndProc(
				HWND hwnd,      // handle to window
				UINT uMsg,      // message identifier
				WPARAM wParam,  // first message parameter
				LPARAM lParam   // second message parameter
				);
	static setLinkCursor(HCURSOR hCursor)
	{
		m_shLinkCursor = hCursor;
	}

protected:
	TCHAR m_link[100];
	static HCURSOR m_shLinkCursor;
};

//-----------------------------------------------
inline void CSmLabel::setFontBold(BOOL bBold)
{	
	m_lf.lfWeight = bBold ? FW_BOLD : FW_NORMAL;
}

//-----------------------------------------------
inline void CSmLabel::setFontUnderline(BOOL bSet)
{	
	m_lf.lfUnderline = bSet;
}

//-----------------------------------------------
inline void CSmLabel::setFontItalic(BOOL bSet)
{
	m_lf.lfItalic = bSet;
}

//-----------------------------------------------
inline void CSmLabel::setFontName(const TCHAR *fntName)
{	
	_tcscpy(m_lf.lfFaceName, fntName);
}

//-----------------------------------------------
inline void CSmLabel::redraw()
{
	RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
}

//-----------------------------------------------
inline void CSmLabel::setCursor(HCURSOR hCursor)
{
	m_hCursor = hCursor;
}

//-----------------------------------------------
inline void CSmLabel::setTextColor(COLORREF color)
{
	m_textColor = color;
}

#endif

