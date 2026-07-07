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
#include "odmenu.h"

//--------------------------------------------------
CODMenu::CODMenu()
{
	m_hInst = NULL;
	m_hWndOwner = NULL;
	m_hMainMenu = m_hMenu = NULL;
	m_hCheckBmp = NULL;
	m_itemList = NULL;
	m_menuItems = 0;

	m_crSelText = GetSysColor(COLOR_HIGHLIGHTTEXT); 
	m_crGrayText = GetSysColor(COLOR_GRAYTEXT);
	m_crSelBkgnd = GetSysColor(COLOR_HIGHLIGHT);
//	m_crSelBkgnd = RGB(255,0,0);
//	m_crSelBkgnd = GetSysColor(COLOR_DESKTOP);
//	m_crSelBkgnd = RGB(58, 110, 165);
	m_hStdFont = m_hBoldFont = NULL;
	m_fontFace[0] = NULL;
/*
	BYTE r,g,b;
	r = GetRValue(m_crSelBkgnd);
	g = GetGValue(m_crSelBkgnd);
	b = GetBValue(m_crSelBkgnd);
*/
}

//--------------------------------------------------
CODMenu::~CODMenu()
{
	if (m_hMainMenu != NULL)
		DestroyMenu(m_hMainMenu);
	if (m_itemList != NULL)
		delete [] m_itemList;
	if (m_hStdFont != NULL)
		DeleteObject(m_hStdFont);
	if (m_hBoldFont != NULL && m_hBoldFont != m_hStdFont)
		DeleteObject(m_hBoldFont);
	if (m_hCheckBmp != NULL)
		DeleteObject(m_hCheckBmp);
}

	
//--------------------------------------------------
void CODMenu::drawItem( LPDRAWITEMSTRUCT lpdis)
{
    WORD wCheckX;               // check-mark width
    int nTextX;                 // width of menu item
    int nTextY;                 // height of menu item
    HFONT hfontOld;             // handle to old font
	BOOL textChanged = FALSE, bkgndChanged = FALSE;
	COLORREF crOldText, crOldBkgnd;

	// Get pointers to the menu item's DRAWITEMSTRUCT 
	// structure and MYITEM structure. 

	ODMenuItemInfo *pItem = (ODMenuItemInfo*)lpdis->itemData;
 
	// If the user has selected the item, use the selected 
	// text and background colors to display the item. 
	if (lpdis->itemState & ODS_SELECTED) {
		crOldBkgnd = SetBkColor(lpdis->hDC, m_crSelBkgnd); 
		bkgndChanged = TRUE;
	}
	if (lpdis->itemState & ODS_DISABLED) {
		crOldText = SetTextColor(lpdis->hDC, m_crGrayText); 
		textChanged = TRUE;
	}
	else if (lpdis->itemState & ODS_SELECTED) {
		crOldText = SetTextColor(lpdis->hDC, m_crSelText); 
		textChanged = TRUE;
	}

	// Remember to leave space in the menu item for the 
	// check-mark bitmap.
 	wCheckX = GetSystemMetrics(SM_CXMENUCHECK); 
	nTextX = wCheckX + lpdis->rcItem.left; 
	nTextY = lpdis->rcItem.top + 2; 
 
	// Select the font associated with the item into the 
	// item's device context.
	if (m_hStdFont == NULL)
		createFont(lpdis->hDC);

	if (lpdis->itemState & ODS_DEFAULT)
		hfontOld = (HFONT)SelectObject(lpdis->hDC, m_hBoldFont); 
	else
		hfontOld = (HFONT)SelectObject(lpdis->hDC, m_hStdFont); 

	ExtTextOut(lpdis->hDC, nTextX, nTextY, ETO_OPAQUE, 
			&lpdis->rcItem, pItem->m_text, 
               _tcsclen(pItem->m_text), NULL); 
/*
	if (!(lpdis->itemState & ODS_DISABLED) || lpdis->itemState & ODS_SELECTED)
		ExtTextOut(lpdis->hDC, nTextX, nTextY, ETO_OPAQUE, 
				&lpdis->rcItem, pItem->m_text, 
                _tcsclen(pItem->m_text), NULL); 
	else {
		HBRUSH br = CreateSolidBrush(GetSysColor(COLOR_MENU));
		FillRect(lpdis->hDC, &lpdis->rcItem, br);
		DrawState(lpdis->hDC, NULL, NULL, (LPARAM)pItem->m_text, 0, 
				nTextX, nTextY, // x, y
				0, 0, // cx, cy
				DST_TEXT | DSS_DISABLED); //((lpdis->itemState & ODS_GRAYED)? DSS_DISABLED : 0));
		DeleteObject(br);
	}
*/
	// Draw the check mark
	if (lpdis->itemState & ODS_CHECKED) {
		BITMAP bm;
		HDC compatibleDC = CreateCompatibleDC(lpdis->hDC);
		GetObject(m_hCheckBmp, sizeof(BITMAP), &bm);
		SelectObject(compatibleDC, m_hCheckBmp);
		BitBlt(lpdis->hDC, 
				lpdis->rcItem.left, 
				lpdis->rcItem.top + (lpdis->rcItem.bottom - lpdis->rcItem.top -bm.bmWidth)/2,
				bm.bmWidth, bm.bmHeight,
				compatibleDC,
				0, 0,
				SRCCOPY);
		DeleteDC(compatibleDC);
	}
 
	SelectObject(lpdis->hDC, hfontOld); 
 
	// Return the text and background colors to their 
	// normal state
	if (textChanged)
		SetTextColor(lpdis->hDC, crOldText);
	if (bkgndChanged)
		SetBkColor(lpdis->hDC, crOldBkgnd);
}

//--------------------------------------------------
void CODMenu::measureItem( LPMEASUREITEMSTRUCT lpMeasureInfo)
{
	// Retrieve a device context for the main window.  
    HDC hdc;                    // handle to screen DC                
    SIZE size;                  // menu-item text extents
	HFONT hFontOld;

	hdc = GetDC(m_hWndOwner); 
 
	ODMenuItemInfo *pItem = (ODMenuItemInfo *)lpMeasureInfo->itemData;
 
	// Select the font associated with the item into 
	// the main window's device context. 
	if (m_hStdFont == NULL)
		createFont(hdc);
	hFontOld = (HFONT)SelectObject(hdc, m_hStdFont);
 
	// Retrieve the width and height of the item's string, 
	// and then copy the width and height into the 
	// MEASUREITEMSTRUCT structure's itemWidth and 
	// itemHeight members. 
 
	GetTextExtentPoint32(hdc, pItem->m_text, 
			_tcsclen(pItem->m_text), &size);
	lpMeasureInfo->itemWidth = size.cx + GetSystemMetrics(SM_CXMENUCHECK);
	lpMeasureInfo->itemHeight = size.cy + 5; 
 
	// Select the old font back into the device context, 
	// and then release the device context. 
 
	SelectObject(hdc, hFontOld); 
	ReleaseDC(m_hWndOwner, hdc);
}


//-------------------------------------------------------------------
// initMenu: Initialize owner-draw menu
// Input:
//   hResInt: intance of the module containing the resource
//   hWndOwner: owner window
//   nIDResource: resource ID
//   menuStr:: the table of menu item strings. The number of strings must be the same as
//             the number of menu items (not including separators) in the resource
//             If menuStr = NULL, item strings will be extracted from the resource
//--------------------------------------------------------------------
BOOL CODMenu::initMenu(HINSTANCE hResInst, HWND hWndOwner, UINT nIDResource, TCHAR **menuStr)
{
	m_hInst = hResInst;
	m_hWndOwner = hWndOwner;
	
	// Remove previous menu if any
	if (m_hMainMenu != NULL)
		DestroyMenu(m_hMainMenu);
	if (m_itemList != NULL) {
		delete [] m_itemList;
		m_itemList = NULL;
	}
	m_menuItems = 0;

	m_hMainMenu = LoadMenu(m_hInst, MAKEINTRESOURCE(nIDResource));
	if (m_hMainMenu == NULL)
		return FALSE;
	m_hMenu = GetSubMenu(m_hMainMenu, 0);
	m_hCheckBmp = LoadBitmap(NULL, MAKEINTRESOURCE(OBM_CHECK));
	// Setup owner-draw menu
	UINT state, itemID;
	m_menuItems = GetMenuItemCount(m_hMenu);
	m_itemList = new ODMenuItemInfo[m_menuItems];

	for (int i=0, textItems=0; i<m_menuItems; i++) {
		itemID = GetMenuItemID(m_hMenu, i);
		state = GetMenuState(m_hMenu, i, MF_BYPOSITION);
		if ((state & MF_SEPARATOR) == 0) {
			m_itemList[i].m_command = GetMenuItemID(m_hMenu, i);
			if (menuStr != NULL)
				_tcscpy(m_itemList[i].m_text, menuStr[textItems++]);
			else
				GetMenuString(m_hMenu, i, m_itemList[i].m_text, sizeof(m_itemList[i].m_text)/sizeof(TCHAR), MF_BYPOSITION);
			ModifyMenu(m_hMenu, i, MF_BYPOSITION | MF_OWNERDRAW | state, itemID, (LPCTSTR)(m_itemList+i));
		}
		else {
			m_itemList[i].m_command = 0;
			m_itemList[i].m_text[0] = 0;
		}
	}
	return TRUE;
}


//--------------------------------------------------
BOOL CODMenu::trackPopupMenu(UINT uFlags, int x, int y)
{
	return ::TrackPopupMenu(m_hMenu, uFlags, x, y, 0, m_hWndOwner, NULL);
}

//--------------------------------------------------
BOOL CODMenu::createFont(HDC hDC)
{
	LOGFONT lf;
	if (m_fontFace[0] == NULL) {
		m_hStdFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		GetObject(m_hStdFont, sizeof(LOGFONT), &lf);
		lf.lfWeight = FW_BOLD;
		m_hBoldFont = CreateFontIndirect(&lf);
		if (m_hBoldFont == NULL)
			m_hBoldFont = m_hStdFont;
		return TRUE;
	}

	memset(&lf, 0, sizeof(LOGFONT));
	lf.lfCharSet = ANSI_CHARSET;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	_tcscpy(lf.lfFaceName, m_fontFace);
	lf.lfHeight = -MulDiv(m_fontSize, GetDeviceCaps(hDC, LOGPIXELSY), 72);
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	//lf.lfPitchAndFamily = 
	lf.lfQuality = DEFAULT_QUALITY;
	lf.lfWeight = FW_NORMAL;

	m_hStdFont = CreateFontIndirect(&lf);
	if (m_hStdFont == NULL) {
		m_hStdFont = m_hBoldFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		return TRUE;
	}

	lf.lfWeight = FW_BOLD;
	m_hBoldFont = CreateFontIndirect(&lf);
	if (m_hBoldFont == NULL)
		m_hBoldFont = m_hStdFont;
	return TRUE;
}

//--------------------------------------------------
BOOL CODMenu::setFont(LPCTSTR face, int size)
{
	if (m_hStdFont != NULL) {
		DeleteObject(m_hStdFont);
		if (m_hBoldFont = m_hStdFont)
			m_hBoldFont = NULL;
		m_hStdFont = NULL;
	}
	if (m_hBoldFont != NULL) {
		DeleteObject(m_hBoldFont);
		m_hBoldFont = NULL;
	}
	_tcscpy(m_fontFace, face);
	m_fontSize = size;
	return TRUE;
}


//--------------------------------------------------
BOOL CODMenu::setMenuItemText(const TCHAR * text, int pos, BOOL byPos)
{
	int i;
	if (!byPos) {
		for (i=0; i<m_menuItems; i++) {
			if (m_itemList[i].m_command == (UINT)pos)
				break;
		}
		if (i == m_menuItems)
			return FALSE; //not found
		pos = i;
	}
	else if (pos < 0 || pos >= m_menuItems)
		return FALSE;
	_tcscpy(m_itemList[pos].m_text, text);
	return TRUE;
}


//--------------------------------------------------
BOOL CODMenu::setAllMenuItemText(TCHAR **menuStr)
{
	for (int i=0, textItems=0; i<m_menuItems; i++) {
		if (m_itemList[i].m_command > 0)
			_tcscpy(m_itemList[i].m_text, menuStr[textItems++]);
	}
	return TRUE;
}
