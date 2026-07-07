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
#ifndef __CUSTOM_BUTTON_H
#define __CUSTOM_BUTTON_H

#define	CUSTOMBUTTON_CENTER	-1

//-----------------------------------------------
// Message: Set default button
//          wParam = 1: Set, wParam = 0 clear
//-----------------------------------------------
#define CBM_SETDEFAULT (WM_USER + 1)

//----------------------------------------------
// Check if a control is a push button (owner-draw button included)
//----------------------------------------------
int IsPushButton(HWND hWnd);

LRESULT CALLBACK CustomBtnProc(
			HWND hwnd,      // handle to window
			UINT uMsg,      // message identifier
			WPARAM wParam,  // first message parameter
			LPARAM lParam   // second message parameter
			);

class CCustomButton {
public:
	HWND m_hWnd;
	HINSTANCE m_hInstance;
	//CCustomButton();
	~CCustomButton();
	void init(HINSTANCE hInst, HWND hOwner, HWND hWndMe);
	void drawItem(LPDRAWITEMSTRUCT lpDIS);
	BOOL setIcon( UINT nID, int nWidth, int nHeight);
	BOOL setDefaultButton( BOOL bState ); // returns previous state
	BOOL isDefaultButton()
	{
		return m_bDefault;
	}
	int setImageOffset( int nPixels ); // returns previous state
	int setTextOffset( int nPixels );
	POINT setImagePos(const POINT & p);
	POINT setTextPos(const POINT & p );
	void checkPointForCentering( POINT & p, int nWidth, int nHeight );
	void setTextColor(COLORREF color)
	{
		m_textColor = color;
	}

	BOOL hasImage() 
	{ 
		return (BOOL)( m_hIcon != 0);
	}
	void redraw();
protected:
	virtual LRESULT wndProc(
					HWND hwnd,      // handle to window
					UINT uMsg,      // message identifier
					WPARAM wParam,  // first message parameter
					LPARAM lParam   // second message parameter
					);
	virtual void onSetFocus(HWND prevFocus);
	virtual void onKillFocus(HWND newFocus);

	HWND m_hOwner;
	LONG m_prevWndProc;
	//	Positioning
	BOOL		m_bUseOffset;
	POINT 		m_pointImage;
	POINT		m_pointText;
	int			m_nImageOffsetFromBorder;
	int			m_nTextOffsetFromImage;
	// color
	COLORREF	m_textColor;
	//	Image
	HICON		m_hIcon;
//	HBITMAP		m_hBitmap;
//	HBITMAP		m_hBitmapDisabled;
	int			m_nImageWidth, m_nImageHeight;

	//	State
	BOOL		m_bDefault;
	UINT		m_nOldAction;
	UINT		m_nOldState;

	friend LRESULT CALLBACK ::CustomBtnProc(
			HWND hwnd,      // handle to window
			UINT uMsg,      // message identifier
			WPARAM wParam,  // first message parameter
			LPARAM lParam   // second message parameter
			);
private:
	void attachWindow(HWND hWnd);
};

#endif