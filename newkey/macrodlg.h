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
#ifndef __MACRO_DIALOG_H
#define __MACRO_DIALOG_H

#include "util.h"
#include "userpref.h"
#include "button.h"
#include "smedit.h"

class CMacroEdit : public CSmartEdit
{
protected:
	virtual LRESULT wndProc(
				HWND hwnd,      // handle to window
				UINT uMsg,      // message identifier
				WPARAM wParam,  // first message parameter
				LPARAM lParam   // second message parameter
				);
};

class CMacroDlg: public CMultiLangDialog
{
protected:
	CUserPref * m_pPref, *m_pTempPref;
	HFONT m_hEditFont;
	HWND m_itemListCtrl;
	HWND m_textCtrl;
	HWND m_alwaysMacroCheck;
	HWND m_pathCtrl;

	CCustomButton m_editBtn, m_deleteBtn, m_okBtn, m_cancelBtn, 
		m_selectFileBtn, m_defFileBtn;

	CMacroEdit m_keyEdit;

	int m_isEditReplace; // state of Add/Replace button
	int m_langID;
	TCHAR m_path[MAX_PATH];

public:
	CMacroDlg(CUserPref *pPref, CUserPref *pTempPref) : CMultiLangDialog()
	{
		m_pPref = pPref;
		m_pTempPref = pTempPref;
		m_hEditFont = NULL;
		m_isEditReplace = 1;
		m_langID = 0;
	}

//	void loadDlgInfo(CUserPref *pPref);
//	void storeDlgInfo(CUserPref *pPref);

	void setLangID(int langID) {
		m_langID = langID;
		if (m_langID > 1) // supports only 2 languages
			m_langID = 0;
	}

	// helper function members;
	void setEditFont();


	// overrides
	int init(HINSTANCE hInst, HWND hWndOwner = NULL);
	virtual BOOL onInitDialog();
	virtual BOOL onDestroy();
	virtual BOOL onCommand(WPARAM wParam, LPARAM lParam);
	void applyLanguage(int langIdx);

	virtual BOOL onOK();
	virtual BOOL DialogProc(
						HWND hWndDlg,
						UINT uMsg,
						WPARAM wParam,
						LPARAM lParam);

	void updateKeyHook(int ownMode);

	// event handlers
	void onItemSelChange();
	void onDeleteItem();
	void onEditItem();
	void onTextChange();
	void onKeyChange();
	BOOL onDefaultButton();

	// helper functions
	void setReplaceBtn(int replace = 1);
protected:
	int loadMacTab(CMacroTable *pTab);
	int loadMacFile(TCHAR *path);

};


#endif
