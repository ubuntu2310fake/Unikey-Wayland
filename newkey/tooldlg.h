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

#ifndef __TOOLKIT_DIALOG_H
#define __TOOLKIT_DIALOG_H

#include "util.h"
#include "userpref.h"
#include "button.h"

class CToolkitDlg: public CMultiLangDialog
{
protected:
	HWND m_inCharsetCtrl, m_outCharsetCtrl, 
		 m_inFileCtrl, m_outFileCtrl,
		 m_clipboardCtrl,
		 m_toLowerCheck, m_toUpperCheck,
		 m_removeToneCheck, m_noRichTextCheck, m_minimalFontCheck;
//		 m_inBrowseBtn, m_outBrowseBtn,
	CCustomButton m_inBrowseBtn, m_outBrowseBtn, m_closeBtn, m_convertBtn, m_swapBtn;
	CUserPref * m_pPref;

public:
	CToolkitDlg(CUserPref *pPref) : CMultiLangDialog()
	{
		m_pPref = pPref;
	}
	int init(HINSTANCE hInst, HWND hWndOwner = NULL);
	void loadDlgInfo();
	void storeDlgInfo();

	// overrides
	virtual BOOL onInitDialog();
	virtual BOOL onCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL onOK();
	virtual void onConvert();
	virtual BOOL DialogProc(
						HWND hWndDlg,
						UINT uMsg,
						WPARAM wParam,
						LPARAM lParam);
	virtual BOOL onDestroy();

	//void clipboardConvert();
	void onSwapCharset();
};

struct ConvCharset
{
	TCHAR name[40];
	int id; // id of the charset (defined by converter module)
};

extern ConvCharset InConvCharsetList[];
extern ConvCharset OutConvCharsetList[];


int ConvertClipboard(int inCharset, int outCharset, int & convErrCode, 
					 int promptInfo = 0, HWND hOwnerWnd = NULL);

int RenderFormat(UINT format);
int RenderAllFormats();

#endif
