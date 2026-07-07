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
#ifndef __MAIN_WINDOW_H
#define __MAIN_WINDOW_H

#include "util.h"
#include "keyhook.h"
#include "userPref.h"
#include "ukmenu.h"
#include "tooldlg.h"
#include "keydlg.h"
#include "mactab.h"

#define MAIN_WINDOW_CLASS_NAME _T("UniKey MainWnd")

#ifdef _UNICODE
	#define VIET_FONT_NAME _T("Arial")
	#define VIET_DLG_FONT_SIZE 8
	#define VIET_MENU_FONT_SIZE 8
#else
	#define VIET_FONT_NAME _T("ABC Sans Serif")
	#define VIET_DLG_FONT_SIZE 8
	#define VIET_MENU_FONT_SIZE 8
#endif

#define VIET_FONT_FILE _T("abcserif.fon") // used only in non-unicode version

class CMainWnd : public CVirWindow
{
protected:
//	HMENU m_menu, m_subMenu;
	CUniKeyMenu m_menu;
	int m_menuItems;
	TCHAR m_helpPath[MAX_PATH];

	int m_fontLoaded;
//	int m_vietGUI; // vietnamese user interface
	int m_vietFontOK;
	CUserPref * m_pPref;
public:
	TCHAR m_macroPath[MAX_PATH];
	TCHAR m_currentMacPath[MAX_PATH];

	CMacroTable m_macTab;

	CKeyDlg m_configDlg;
	CToolkitDlg m_toolDlg;
	void updateChanges(const CUserPref & pref);
	CMainWnd(CUserPref *pPref);
/*
	~CMainWnd()
	{
		if (m_menu)
			DestroyMenu(m_menu);
	}
*/
	BOOL initInstance(HINSTANCE hInstance);

	virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void cleanup();
	int  loadVietFont();
	void openHelp();

	int isVietFontOK()
	{
		return m_vietFontOK;
	}

	void onToolkit();
	void onPanel();
	void updateMacroTable();
};

extern SharedMem * pSharedMem;
void SetHookOptions(const CUserPref & pref);

#endif
