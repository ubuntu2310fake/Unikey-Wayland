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
#ifndef __KEY_DLG_H
#define __KEY_DLG_H

#include "util.h"
#include "ExpDlg.h"
#include "button.h"
#include "userPref.h"

class CKeyDlg : public CExpandDialog, public CMultiLangDialog
{
public:
	int init(HINSTANCE hInst, HWND hWndOwner = NULL);
	// overrides
	virtual BOOL onCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL onOK();
	virtual BOOL onCancel();
	virtual BOOL onInitDialog();

	// update text labels according to the chosen GUI language
	// langID = 0 -> English (default)
	//void setItemsText(int langID); //TEST
	//void setDlgFont(int langID); //TEST
protected:
	//HFONT m_hStdFont; //TEST
	CCustomButton m_aboutBtn, m_okBtn, m_helpBtn, m_cancelBtn, m_resetBtn, m_expandBtn, m_macroBtn;
	HWND m_codeList, m_inputList, m_showCtrl, m_swKeyBtn1, m_swKeyBtn2, 
		 m_markCheck, m_toneCtrl, m_modernStyleCheck, m_macroCheck, m_useUnicodeClipboardCheck;
	CUserPref *m_pPref, m_tempPref;
	void loadDlgInfo(CUserPref *pPref);
	void storeDlgInfo(CUserPref *pPref);

public:
	CKeyDlg(CUserPref *pPref) : CExpandDialog()
	{
		m_pPref = pPref;
		m_hStdFont = NULL;
	}

	virtual BOOL DialogProc(
						HWND hWndDlg,
						UINT uMsg,
						WPARAM wParam,
						LPARAM lParam);
	//overrides
	virtual BOOL onDialogExpanding(BOOL bExpanded);
	virtual BOOL onDestroy();
	void applyLanguage(int langIdx);
};


#endif
