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
#ifndef __USER_PREFERENCE_H
#define __USER_PREFERENCE_H

#include "keycons.h"

#define POPULAR_CHARSETS 6 // pouplar code tables which are always visible in the menu

// Change flags
#define PREF_CHANGE_CHARSET			0x0001
#define PREF_CHANGE_METHOD			0x0002
#define PREF_CHANGE_SWITCHKEY		0x0004
#define PREF_CHANGE_OPTIONS			0x0008
#define PREF_CHANGE_SHOWDLG			0x0010
#define PREF_CHANGE_OTHERCHARSET	0x0020
#define PREF_CHANGE_GUI				0x0040
#define PREF_CHANGE_AUTOSTART		0x0080

class CUserPref
{
public:
	int setValue();
	int getValue();
	void setDefault(const CUserPref *pCopy = NULL);
	CUserPref() {
		setDefault();
	}
	DWORD m_showDlg;
	DWORD m_vietnamese;
	DWORD m_codeTable;
	DWORD m_switchKey;
	DWORD m_otherCharset;
	DWORD m_inMethod;
	DWORD m_freeMarking;
	DWORD m_toneNextToVowel;
	DWORD m_modernStyle;
	DWORD m_vietGUI;
	DWORD m_inConvCharset;
	DWORD m_outConvCharset;
	DWORD m_clipboardConvert;
	DWORD m_macroEnabled;
	DWORD m_useUnicodeClipboard;
	DWORD m_alwaysMacro; // enable macro even Vietnamese keyboard is off
	DWORD m_useIME;

	//values not stored in registry, for processing only
	DWORD m_oldTable; 
	WORD  m_changes;
	int m_autoStart;

	// temporary options
	int m_noRichText;
	int m_toLower;
	int m_toUpper;
	int m_removeTone;
	int m_minimalFontset;

};

#endif
