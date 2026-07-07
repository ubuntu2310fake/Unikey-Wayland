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
#include "button.h"

#define RECT_WIDTH(r) (r.right - r.left)
#define RECT_HEIGHT(r) (r.bottom - r.top)

int IsPushButton(HWND hWnd)
{
	if (!(SendMessage(hWnd, WM_GETDLGCODE, 0, 0) & DLGC_BUTTON))
		return 0;

	// button type is represented by only last 4 bits of window style
	// ==> not portable code, always check button styles in winuser.h for possible changes
	long btnType = GetWindowLong(hWnd, GWL_STYLE) & 0xFL;

	// Consider owner-draw buttons as push buttons
	return (btnType == BS_PUSHBUTTON || btnType == BS_DEFPUSHBUTTON || btnType == BS_OWNERDRAW);
}

LRESULT CALLBACK CustomBtnProc(
			HWND hWnd,      // handle to window
			UINT uMsg,      // message identifier
			WPARAM wParam,  // first message parameter
			LPARAM lParam   // second message parameter
			)
{
	CCustomButton *pBtn = (CCustomButton *)GetWindowLong(hWnd, GWL_USERDATA);
	if (pBtn)
		return pBtn->wndProc(hWnd, uMsg, wParam, lParam);
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
	
//-------------------------------------------------------
void CCustomButton::attachWindow(HWND hWnd)
{
	m_hWnd = hWnd;
	SetWindowLong(hWnd, GWL_USERDATA, (LONG)this);
	// Subclass the button
	m_prevWndProc = SetWindowLong(hWnd, GWL_WNDPROC, (LONG)CustomBtnProc);
}

//-------------------------------------------------------
LRESULT CCustomButton::wndProc(
				HWND hWnd,      // handle to window
				UINT uMsg,      // message identifier
				WPARAM wParam,  // first message parameter
				LPARAM lParam)  // second message parameter
{
	switch (uMsg) {
	case WM_DRAWITEM:
		drawItem((LPDRAWITEMSTRUCT)lParam);
		return TRUE;
	case WM_SETFOCUS:
		onSetFocus((HWND)wParam);
		break;
	case WM_KILLFOCUS:
		onKillFocus((HWND)wParam);
		break;
	case CBM_SETDEFAULT:
		setDefaultButton(wParam ? TRUE : FALSE);
		redraw();
		return 1;
	}
	return CallWindowProc((WNDPROC)m_prevWndProc, hWnd, uMsg, wParam, lParam);
}

void CCustomButton::redraw()
{
	InvalidateRect(m_hWnd, NULL, TRUE);
}
//-------------------------------------------------------
void CCustomButton::onSetFocus(HWND prevFocus)
{
	if (!isDefaultButton()) {
		setDefaultButton(TRUE);
		redraw();
	}
	if (!IsPushButton(prevFocus)) {
		WORD defId = LOWORD(SendMessage(m_hOwner, DM_GETDEFID, 0, 0));
		if (defId && GetDlgItem(m_hOwner, defId) != m_hWnd)
			SendDlgItemMessage(m_hOwner, defId, CBM_SETDEFAULT, 0, 0);
	}
}

//-------------------------------------------------------
void CCustomButton::onKillFocus(HWND newFocus)
{
	int newIsButton = IsPushButton(newFocus);
	WORD defId = LOWORD(SendMessage(m_hOwner, DM_GETDEFID, 0, 0));
	HWND hDefWnd = GetDlgItem(m_hOwner, defId);
	if (isDefaultButton() && (m_hWnd != hDefWnd || newIsButton)) {
		setDefaultButton(FALSE);
		redraw();
	}
	if (!newIsButton && m_hWnd != hDefWnd)
		SendDlgItemMessage(m_hOwner, defId, CBM_SETDEFAULT, 1, 0);
}

//-------------------------------------------------------
void CCustomButton::init(HINSTANCE hInst, HWND hOwner, HWND hWndMe)
{
	m_hInstance = hInst;
	m_hOwner = hOwner;
	attachWindow(hWndMe);

	m_hIcon = 0;
//	m_hBitmap = 0;
//	m_hBitmapDisabled = 0;

	m_bDefault = FALSE;
	m_nOldState = 0;
	m_nOldAction = 0;

	m_textColor = RGB(0, 0, 0);


	m_nImageOffsetFromBorder = 4;
	m_nTextOffsetFromImage = 4;
	m_bUseOffset = TRUE;
	m_pointImage.x = m_pointImage.y = 0;
	m_pointText.x = m_pointText.y = 0;
	
}

/*
//-------------------------------------------------------
CCustomButton::CCustomButton()
{
	m_hWnd = NULL;
	m_hInstance = NULL;

	m_hIcon = 0;
//	m_hBitmap = 0;
//	m_hBitmapDisabled = 0;

	m_textColor = RGB(0, 0, 0);

	m_bDefault = FALSE;
	m_nOldState = 0;
	m_nOldAction = 0;

	m_nImageOffsetFromBorder = 4;
	m_nTextOffsetFromImage = 8;
	m_bUseOffset = TRUE;
	m_pointImage.x = m_pointImage.y = 0;
	m_pointText.x = m_pointText.y = 0;
}
*/

//-------------------------------------------------------
CCustomButton::~CCustomButton()
{
	if( m_hIcon != 0 )
		DestroyIcon(m_hIcon); //DeleteObject( m_hIcon );
//	if( m_hBitmap != 0 )
//		DeleteObject( m_hBitmap );
//	if( m_hBitmapDisabled != 0 )
//		DeleteObject( m_hBitmapDisabled );
}

//-------------------------------------------------------
BOOL CCustomButton::setIcon( UINT nID, int nWidth, int nHeight )
{
	if (m_hIcon != NULL)
		DestroyIcon(m_hIcon);
	m_hIcon = (HICON)::LoadImage(m_hInstance, 
								MAKEINTRESOURCE(nID), 
								IMAGE_ICON, nWidth, nHeight, 0);

	if( m_hIcon == NULL )
		return FALSE;

	m_nImageWidth = nWidth;
	m_nImageHeight = nHeight;

//	m_hBitmap = 0;
	return TRUE;
}


//-------------------------------------------------------
BOOL CCustomButton::setDefaultButton( BOOL bState )
{
	BOOL bPrevious = m_bDefault;
	m_bDefault = bState;

	return bPrevious;
}

//	Positioning Functions
//-------------------------------------------------------
int CCustomButton::setImageOffset( int nPixels )
{
	int nPrevious = m_nImageOffsetFromBorder;

	m_bUseOffset = TRUE; 
	m_nImageOffsetFromBorder = nPixels; 
	return nPrevious;
}

//-------------------------------------------------------
int CCustomButton::setTextOffset( int nPixels ) 
{ 
	int nPrevious = m_nTextOffsetFromImage;

	m_bUseOffset = TRUE; 
	m_nTextOffsetFromImage = nPixels; 

	return nPrevious;
}

//-------------------------------------------------------
POINT CCustomButton::setImagePos(const POINT & p ) 
{ 
	POINT pointPrevious = m_pointImage;

	m_bUseOffset = FALSE; 
	m_pointImage = p; 

	return pointPrevious;
}

//-------------------------------------------------------
POINT CCustomButton::setTextPos(const POINT & p ) 
{ 
	POINT pointPrevious = m_pointText;

	m_bUseOffset = FALSE; 
	m_pointText = p; 

	return pointPrevious;
}

//-------------------------------------------------------
//	Centering a point helper function
//-------------------------------------------------------
void CCustomButton::checkPointForCentering( POINT &p, int nWidth, int nHeight)
{
	RECT	rect;
	GetClientRect(m_hWnd, &rect);

	if( p.x == CUSTOMBUTTON_CENTER )
		p.x = (RECT_WIDTH(rect) - nWidth) >> 1;
	if( p.y == CUSTOMBUTTON_CENTER )
		p.y = (RECT_HEIGHT(rect) - nHeight) >> 1;
}

//-------------------------------------------------------
void CCustomButton::drawItem(LPDRAWITEMSTRUCT lpDIS) 
{	
	TCHAR text[100];
	RECT rect = lpDIS->rcItem;

	UINT	nOffset = 0;			// For position adjustment of a pushed button
	UINT	nFrameStyle=0;
	BOOL	bDRAWFOCUSONLY = FALSE;	// Optimize a bit
	int		nStateFlag;				// Normal or Disabled
//	HBITMAP	hBitmapToDraw;			// Normal or Disabled bitmap (not used if uses icon)
	
	UINT	nNewState = lpDIS->itemState;
	UINT	nNewAction = lpDIS->itemAction;
	
	//	Find out what state the control and set some drawing flags 
	//	according to the state.
	if ( nNewState & ODS_SELECTED)
	{
		nFrameStyle = DFCS_PUSHED;
		nOffset += 1;
	}

	if( nNewState & ODS_DISABLED )
	{
		nStateFlag = DSS_DISABLED;
//		hBitmapToDraw = m_hBitmapDisabled;
	}
	else
	{
		nStateFlag = DSS_NORMAL;
//		hBitmapToDraw = m_hBitmap;
	}

	// If only the focus is changing, don't redraw the whole control
	if (nNewAction == ODA_FOCUS )
		bDRAWFOCUSONLY = TRUE;

	// If this is the default button, deflate the control so everything 
	// we do below (icon, text, focus ) is adjusted properly
	if( m_bDefault )
		InflateRect(&rect, -1, -1);

	if( !bDRAWFOCUSONLY )
	{
		//
		// Draw 'default button' rectangle
		//
		if( m_bDefault ) // Can't use ODS_DEFAULT w/owner draw!!
		{
			HGDIOBJ hOldPen = SelectObject(lpDIS->hDC, GetStockObject(BLACK_PEN));
			Rectangle(lpDIS->hDC, lpDIS->rcItem.left, lpDIS->rcItem.top, lpDIS->rcItem.right, lpDIS->rcItem.bottom);
			SelectObject(lpDIS->hDC, hOldPen);
		}

		//
		//	Draw button frame
		//
		if (GetWindowLong(m_hWnd, GWL_STYLE) & BS_FLAT)
			nFrameStyle |= DFCS_FLAT;
		DrawFrameControl(lpDIS->hDC, &rect, DFC_BUTTON, DFCS_BUTTONPUSH | nFrameStyle);

		//	Get control text
		GetWindowText(m_hWnd, text, sizeof(text)/sizeof(TCHAR));
		//
		//	Draw Image
		//
		if( hasImage() )
		{
			POINT pt;

			if( m_bUseOffset )
			{
				pt.x = (text[0] == 0) ? CUSTOMBUTTON_CENTER : rect.left + m_nImageOffsetFromBorder;
				pt.y = CUSTOMBUTTON_CENTER;
			}
			else
				pt = m_pointImage;

			checkPointForCentering( pt, m_nImageWidth, m_nImageHeight );
			
			pt.x += nOffset;
			pt.y += nOffset;

			if( m_hIcon )
				DrawState(lpDIS->hDC, NULL, NULL, (LPARAM)m_hIcon, 0, pt.x, pt.y, m_nImageWidth, m_nImageHeight, DST_ICON | nStateFlag);
			//else if( m_hBitmap )
			//	pDC->DrawState( pt, CSize(m_nImageWidth, m_nImageHeight), (HBITMAP)hBitmapToDraw, DST_BITMAP | nStateFlag );
		}
		//	
		//	Draw Text
		//
		if ( text[0] != 0)
		{
			POINT pt;
			SIZE size;
			GetTextExtentPoint32(lpDIS->hDC, text, _tcslen(text), &size);
			if( m_bUseOffset )
			{
				RECT rcClient;
				GetClientRect(m_hWnd, &rcClient);
				if (hasImage()) {
					if (RECT_WIDTH(rcClient) 
						- (m_nImageWidth + m_nTextOffsetFromImage + m_nImageOffsetFromBorder)
					    - size.cx > 0)
						pt.x = m_nImageWidth + m_nTextOffsetFromImage + m_nImageOffsetFromBorder;
					else
						pt.x = m_nImageWidth +  m_nImageOffsetFromBorder + 2;//m_nTextOffsetFromImage;
				}
				else pt.x = CUSTOMBUTTON_CENTER;
				pt.y = CUSTOMBUTTON_CENTER; 
			}
			else
				pt = m_pointText;

			//	If we are centering the text vertically, it looks best of we
			//	center based on the height of the text, then move it up 1 more pixel
			int nOffsetFixY = pt.y == CUSTOMBUTTON_CENTER ? -1 : 0;

			checkPointForCentering( pt, size.cx, size.cy);

			pt.x += nOffset;
			pt.y += nOffset + nOffsetFixY;
			COLORREF oldColor = SetTextColor(lpDIS->hDC, m_textColor);
			DrawState(lpDIS->hDC, NULL, NULL, (LPARAM)text, 0, pt.x, pt.y, 0, 0, DST_PREFIXTEXT|nStateFlag);
			SetTextColor(lpDIS->hDC, oldColor);
		}

	} // End !focus only

	//
	//	Draw focus rectange
	//
	if( !( nNewState & ODS_DISABLED ) )	// Make sure it's not disabled
	{
		// Redraw the focus if:
		//		1. There is a change in focus state 
		// OR	2. The entire control was just redrawn and Focus is set
		if( ( nNewState & ODS_FOCUS ) ^ ( m_nOldState & ODS_FOCUS ) ||
			( !bDRAWFOCUSONLY && ( nNewState & ODS_FOCUS ) ) )
		{

			#define FOCUSOFFSET 3
			RECT tempRect = rect;
			
			// As control gets smaller, decrease focus size
			int nDeflate = min( FOCUSOFFSET,
								min( RECT_WIDTH(tempRect), RECT_HEIGHT(tempRect)) >> 2 );
			InflateRect(&tempRect, -nDeflate, -nDeflate);
			DrawFocusRect(lpDIS->hDC, &tempRect);
		}
	}

	m_nOldAction = nNewAction;
	m_nOldState = nNewState;
}
