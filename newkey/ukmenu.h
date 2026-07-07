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

#ifndef __UNIKEY_MENU_H
#define __UNIKEY_MENU_H

#include "odmenu.h"
#include "userpref.h"

#define FIRST_KEY_MENU_INDEX 12 // This must be changed when menu resource is changed

class CUniKeyMenu : public CODMenu {
protected:
	CUserPref *m_pPref;
	HINSTANCE m_hInst;
	HWND m_hParentWnd;
	UINT m_charsetItemID;      // selected charset item ID, initialized to 0
	UINT m_inputMethodItemID;  // selected input method item ID, initialized to 0
public:
	CUniKeyMenu();

	void init(HINSTANCE hInst, HWND parentWnd, CUserPref *pPref)
	{
		m_pPref = pPref;
		m_hInst = hInst;
		m_hParentWnd = parentWnd;
	}

	void reload();
	// process a menu command, returns non-zero if the command was processed
	int processCommand(WORD cmd);
	void update();
protected:
	void selectCharsetMenu(int charSet);
	void selectInputMethod(WORD cmd);
};


#endif
