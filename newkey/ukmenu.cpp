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
#include "mainwnd.h"
#include "ukmenu.h"
#include "resource.h"
#include "encode.h"
#include "vnconv.h"

char *VnMenuText[] = {
		"KÕt thóc",
		"H­íng dÉn",
		"B¶ng ®iÒu khiÓn...[CS+F5]",
		"C«ng cô...[CS+F6]",
		"ChuyÓn m· nhanh - [CS+F9]",
		"Gâ kiÓu TELEX",
		"Gâ kiÓu VNI",
		"Gâ kiÓu VIQR",
		"Gâ kiÓu VIQR*",
		"Unicode",
		"TCVN3 - ABC",
		"VNI Windows",
		"VIQR",
		"Vietnamese locale CP 1258",
		"UTF-8 Literal",
		"Other"};

#define inputMethodToCmd(method) \
((method == TELEX_INPUT)? ID_TELEX_TYPING : \
((method == VNI_INPUT)? ID_VNI_TYPING : \
((method == VIQR_INPUT)? ID_VIQR_TYPING : ID_VIQR_STAR_TYPING)))

#define cmdToInputMethod(cmd) \
((cmd == ID_TELEX_TYPING)? TELEX_INPUT : \
((cmd == ID_VNI_TYPING)? VNI_INPUT : \
((cmd == ID_VIQR_TYPING)? VIQR_INPUT : VIQR_STAR_INPUT)))

CUniKeyMenu::CUniKeyMenu()
	: CODMenu()
{
	m_pPref = NULL;
	m_charsetItemID = 0;
	m_inputMethodItemID = 0;
}

void CUniKeyMenu::reload()
{
	if (m_pPref->m_vietGUI) {
#ifndef _UNICODE
		initMenu(m_hInst, m_hParentWnd, IDR_KEY_MENU, VnMenuText);
#else
		wchar_t * WVnMenuText[sizeof(VnMenuText)/sizeof(char *)];
		int items = sizeof(VnMenuText)/sizeof(char *);
		int i, maxOutLen, inLen;
		for (i=0; i<items; i++) {
			inLen = -1;
			WVnMenuText[i] = new wchar_t[40];
			maxOutLen = 80;
			VnConvert(CONV_CHARSET_TCVN3, CONV_CHARSET_UNICODE, 
					 (BYTE *)VnMenuText[i], (BYTE *)WVnMenuText[i],
					 inLen, maxOutLen);
		}
		initMenu(m_hInst, m_hParentWnd, IDR_KEY_MENU, WVnMenuText);
		for (i=0; i<items; i++)
			delete [] WVnMenuText[i];
#endif
		setFont(VIET_FONT_NAME, VIET_MENU_FONT_SIZE);
	}
	else {
		initMenu(m_hInst, m_hParentWnd, IDR_KEY_MENU); // Use menu text from the resource
		setFont(_T(""), 0); // use Windows standard font
	}
	SetMenuDefaultItem(m_hMenu, ID_KEY_PANEL, FALSE);
	setMenuItemText(CharsetTable[m_pPref->m_otherCharset].name, ID_KEY_OTHER, FALSE);
	m_charsetItemID = 0;
	selectCharsetMenu(m_pPref->m_codeTable);
	m_inputMethodItemID = inputMethodToCmd(m_pPref->m_inMethod);
	selectInputMethod(inputMethodToCmd(m_pPref->m_inMethod));
}

//-------------------------------------------------------------------
// process a menu command, returns non-zero if the command was processed
//-------------------------------------------------------------------
int CUniKeyMenu::processCommand(WORD cmd)
{
	int sel;
	switch (cmd) {
	case ID_TELEX_TYPING:
	case ID_VNI_TYPING:
	case ID_VIQR_TYPING:
	case ID_VIQR_STAR_TYPING:
		selectInputMethod(cmd);
		return 1;
	default:
		// determine if cmd is a command which selects a charset
		for (sel=FIRST_KEY_MENU_INDEX; sel <= FIRST_KEY_MENU_INDEX + POPULAR_CHARSETS; sel++) {
			if (GetMenuItemID(m_hMenu, sel) == cmd) {
				int charSet = sel - FIRST_KEY_MENU_INDEX;
				if (charSet == POPULAR_CHARSETS)
					charSet = m_pPref->m_otherCharset;
				selectCharsetMenu(charSet);
				return 1;
			}
		}
	}
	return 0;
}

//----------------------------------------------------------------------
void CUniKeyMenu::selectCharsetMenu(int charSet)
{
	UINT sel;
	if (charSet > POPULAR_CHARSETS)
		sel = FIRST_KEY_MENU_INDEX + POPULAR_CHARSETS;
	else
		sel = FIRST_KEY_MENU_INDEX + charSet;

	if (sel != m_charsetItemID) {
		CheckMenuItem(m_hMenu, m_charsetItemID, MF_BYPOSITION | MF_UNCHECKED);
		CheckMenuItem(m_hMenu, sel, MF_BYPOSITION | MF_CHECKED);
		m_charsetItemID = sel;
		m_pPref->m_codeTable = charSet;
		m_pPref->m_changes |= PREF_CHANGE_CHARSET;
	}
}

//----------------------------------------------------------------------
void CUniKeyMenu::selectInputMethod(WORD cmd)
{
	CheckMenuItem(m_hMenu, cmd, MF_BYCOMMAND | MF_CHECKED);
	if (cmd != m_inputMethodItemID) {
		CheckMenuItem(m_hMenu, m_inputMethodItemID, MF_BYCOMMAND | MF_UNCHECKED);
		m_inputMethodItemID = cmd;
		m_pPref->m_inMethod = cmdToInputMethod(cmd);
		m_pPref->m_changes |= PREF_CHANGE_METHOD;
		WORD markOption = (m_pPref->m_inMethod != TELEX_INPUT);
		if (markOption != m_pPref->m_freeMarking) {
			m_pPref->m_freeMarking = markOption;
			m_pPref->m_changes |= PREF_CHANGE_OPTIONS;
		}
	}
}

//----------------------------------------------------------------------
void CUniKeyMenu::update()
{
	if (m_pPref->m_changes & PREF_CHANGE_OTHERCHARSET)
		setMenuItemText(CharsetTable[m_pPref->m_otherCharset].name,	ID_KEY_OTHER, FALSE);

	if (m_pPref->m_changes & PREF_CHANGE_CHARSET)
		selectCharsetMenu(m_pPref->m_codeTable);

	if (m_pPref->m_changes & PREF_CHANGE_METHOD)
		selectInputMethod(inputMethodToCmd(m_pPref->m_inMethod));

	if (m_pPref->m_changes & PREF_CHANGE_GUI)
		reload();
}
