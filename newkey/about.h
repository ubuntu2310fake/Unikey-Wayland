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
#ifndef _ABOUT_DLG_H
#define _ABOUT_DLG_H
#include "util.h"
#include "button.h"
#include "label.h"

void DoAbout(HINSTANCE hInst, HWND hParent, int langID);

class CAboutDlg : public CVirDialog
{
protected:
	CCustomButton m_okBtn;
	CLinkLabel m_webLink, m_supportLink;
	//CLinkLabel m_backupWebLink;
	//CLinkLabel m_mailLink;
	CSmLabel m_author;
	HICON m_hLinkCursor;
	HFONT m_hStdFont;
	int m_langID; // 0: English, 1: Vietnamese
public:
	CAboutDlg(int langID = 0) : CVirDialog()
	{
		m_hStdFont = NULL;
		m_langID = langID;
	}
	// Overrides
	virtual BOOL DialogProc(
						HWND hWndDlg,
						UINT uMsg,
						WPARAM wParam,
						LPARAM lParam);
	virtual BOOL onInitDialog();
	void setItemsText(int langID);
	void setDlgFont(int langID);
	void setLangID(int langID)
	{
		m_langID = langID;
	}
	friend void DoAbout(HINSTANCE hInst, HWND hParent, int langID);
};

#endif
