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
#ifndef __OWNER_DRAW_MENU_H
#define __OWNER_DRAW_MENU_H

#include <tchar.h>

struct ODMenuItemInfo {
	TCHAR m_text[100];
	UINT m_command;
};

class CODMenu
{
public:
	HMENU m_hMainMenu, m_hMenu; // m_hMenu is the first submenu of m_hMainMenu
	CODMenu();
	~CODMenu();

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
	BOOL initMenu(HINSTANCE hResInst, HWND hWndOwner, UINT nIDResource, TCHAR **menuStr = NULL);

	virtual void drawItem( LPDRAWITEMSTRUCT lpDrawInfo);
	virtual void measureItem( LPMEASUREITEMSTRUCT lpMeasureInfo);
	BOOL trackPopupMenu(
			UINT uFlags,         // options
			int x,               // horizontal position
			int y               // vertical position
			);
	BOOL setFont(LPCTSTR face, int size);
	BOOL setMenuItemText(const TCHAR * text, int pos, BOOL byPos);
	BOOL setAllMenuItemText(TCHAR **menuStr);

private:
	BOOL createFont(HDC hDC);
protected:
	int m_menuItems;
	HINSTANCE m_hInst;
	HWND m_hWndOwner;
	HFONT m_hStdFont, m_hBoldFont;
	COLORREF m_crSelText, m_crSelBkgnd, m_crGrayText;
	HBITMAP m_hCheckBmp;
	ODMenuItemInfo *m_itemList;
	TCHAR m_fontFace[LF_FACESIZE]; 
	int m_fontSize;
public:
	int getMenuItems()
	{
		return m_menuItems;
	}
};

#endif