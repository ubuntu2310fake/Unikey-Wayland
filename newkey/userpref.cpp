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
#include "userpref.h"

#define VNKEY_REG _TEXT("Software\\PkLong\\UniKey")
#define STARTUP_REG_VALUENAME _T("UniKey")

int enableAutoStart(int enabled);
int isAutoStartEnabled();

void CUserPref::setDefault(const CUserPref *pCopy)
{
	m_showDlg = 1;
	m_codeTable = 0;
	m_otherCharset = POPULAR_CHARSETS;
	m_switchKey = 0;
	m_inMethod = TELEX_INPUT;
	m_freeMarking = 0;
	m_toneNextToVowel = 0;
	m_modernStyle = 0;
	m_changes = 0;
	m_oldTable = 0;
	m_autoStart = isAutoStartEnabled();
	if (pCopy == NULL) {
		m_vietGUI = 1;
		m_vietnamese = 0;
	}
	else {
		m_vietGUI = pCopy->m_vietGUI;
		m_vietnamese = pCopy->m_vietnamese;
	}
	m_inConvCharset = 0;
	m_outConvCharset = 1;
	m_clipboardConvert = 1;
	m_macroEnabled = 0;
	m_alwaysMacro = 0;
	m_useIME = 0;
	m_useUnicodeClipboard = 0;

	m_toLower = 0;
	m_toUpper = 0;
	m_removeTone = 0;
	m_noRichText = 0;
	m_minimalFontset = 0;
}

int CUserPref::setValue()
{
	long res;
	HKEY key;
	DWORD state;
	res = RegCreateKeyEx(
				HKEY_CURRENT_USER,
				VNKEY_REG,
				0,
				NULL,
				REG_OPTION_NON_VOLATILE,    // special options flag
				KEY_QUERY_VALUE | KEY_SET_VALUE,        // desired security access
				NULL,
				&key,          // receives opened handle
				&state);
	if (res!=ERROR_SUCCESS)
		return 0;

	RegSetValueEx(key, _TEXT("ShowDlg"),0,REG_DWORD,(LPBYTE)&m_showDlg,sizeof(DWORD));
	RegSetValueEx(key, _TEXT("Vietnamese"),0,REG_DWORD,(LPBYTE)&m_vietnamese,sizeof(DWORD));
	RegSetValueEx(key, _TEXT("CodeTable"),0,REG_DWORD,(LPBYTE)&m_codeTable,sizeof(DWORD));
	RegSetValueEx(key, _TEXT("SwitchKey"),0,REG_DWORD,(LPBYTE)&m_switchKey, sizeof(DWORD));
	RegSetValueEx(key, _TEXT("OtherCharset"), 0, REG_DWORD, (LPBYTE)&m_otherCharset, sizeof(DWORD));
	RegSetValueEx(key, _TEXT("InputMethod"), 0, REG_DWORD, (LPBYTE)&m_inMethod, sizeof(DWORD));
	RegSetValueEx(key, _TEXT("FreeMarking"), 0, REG_DWORD, (LPBYTE)&m_freeMarking, sizeof(DWORD));
	RegSetValueEx(key, _TEXT("ToneNextToVowel"), 0, REG_DWORD, (LPBYTE)&m_toneNextToVowel, sizeof(DWORD));
	RegSetValueEx(key, _TEXT("ModernStyle"), 0, REG_DWORD, (LPBYTE)&m_modernStyle, sizeof(DWORD));
	RegSetValueEx(key, _TEXT("VietGUI"), 0, REG_DWORD, (LPBYTE)&m_vietGUI, sizeof(DWORD));
	RegSetValueEx(key, _TEXT("InConvCharset"), 0, REG_DWORD, (LPBYTE)&m_inConvCharset, sizeof(DWORD));
	RegSetValueEx(key, _TEXT("OutConvCharset"), 0, REG_DWORD, (LPBYTE)&m_outConvCharset, sizeof(DWORD));
	RegSetValueEx(key, _TEXT("ClipboardConvert"), 0, REG_DWORD, (LPBYTE)&m_clipboardConvert, sizeof(DWORD));
	RegSetValueEx(key, _TEXT("MacroEnabled"), 0, REG_DWORD, (LPBYTE)&m_macroEnabled, sizeof(DWORD));
	RegSetValueEx(key, _TEXT("UseUnicodeClipboard"), 0, REG_DWORD, (LPBYTE)&m_useUnicodeClipboard, sizeof(DWORD));
	RegSetValueEx(key, _TEXT("AlwaysMacro"), 0, REG_DWORD, (LPBYTE)&m_alwaysMacro, sizeof(DWORD));
	RegSetValueEx(key, _TEXT("UseIME"), 0, REG_DWORD, (LPBYTE)&m_useIME, sizeof(DWORD));

	RegCloseKey(key);
	enableAutoStart(m_autoStart);
	return 1;
}

int CUserPref::getValue()
{
	long res;
	HKEY key;
	DWORD nbytes, valType;
	int err = 0;
	res = RegOpenKeyEx(
				HKEY_CURRENT_USER,
				VNKEY_REG,
				0,
				KEY_QUERY_VALUE,
				&key);
	if (res!=ERROR_SUCCESS) {
		setDefault();
		return 0;
	}
	nbytes = sizeof(DWORD);
	err = err || (ERROR_SUCCESS != 
					RegQueryValueEx(key, _TEXT("ShowDlg"),0,&valType,(LPBYTE)&m_showDlg,&nbytes));
	nbytes = sizeof(DWORD);
	err = err || (ERROR_SUCCESS != 
					RegQueryValueEx(key, _TEXT("Vietnamese"),0,&valType,(LPBYTE)&m_vietnamese,&nbytes));
	nbytes = sizeof(DWORD);
	err = err || (ERROR_SUCCESS != 
					RegQueryValueEx(key, _TEXT("CodeTable"),0,&valType,(LPBYTE)&m_codeTable, &nbytes));
	err = err || (ERROR_SUCCESS != 
					RegQueryValueEx(key, _TEXT("SwitchKey"),0,&valType,(LPBYTE)&m_switchKey, &nbytes));
	err = err || (ERROR_SUCCESS != 
					RegQueryValueEx(key, _TEXT("OtherCharset"),0,&valType,(LPBYTE)&m_otherCharset, &nbytes));
	err = err || (ERROR_SUCCESS != 
					RegQueryValueEx(key, _TEXT("InputMethod"),0,&valType,(LPBYTE)&m_inMethod, &nbytes));

	err = err || (ERROR_SUCCESS != 
					RegQueryValueEx(key, _TEXT("FreeMarking"),0,&valType,(LPBYTE)&m_freeMarking, &nbytes));

	err = err || (ERROR_SUCCESS != 
					RegQueryValueEx(key, _TEXT("ToneNextToVowel"),0,&valType,(LPBYTE)&m_toneNextToVowel, &nbytes));

	err = err || (ERROR_SUCCESS != 
					RegQueryValueEx(key, _TEXT("ModernStyle"),0,&valType,(LPBYTE)&m_modernStyle, &nbytes));

	err = err || (ERROR_SUCCESS != 
					RegQueryValueEx(key, _TEXT("VietGUI"),0,&valType,(LPBYTE)&m_vietGUI, &nbytes));

	err = err || (ERROR_SUCCESS != 
					RegQueryValueEx(key, _TEXT("InConvCharset"),0,&valType,(LPBYTE)&m_inConvCharset, &nbytes));

	err = err || (ERROR_SUCCESS != 
					RegQueryValueEx(key, _TEXT("OutConvCharset"),0,&valType,(LPBYTE)&m_outConvCharset, &nbytes));

	err = err || (ERROR_SUCCESS != 
					RegQueryValueEx(key, _TEXT("ClipboardConvert"),0,&valType,(LPBYTE)&m_clipboardConvert, &nbytes));

	err = err || (ERROR_SUCCESS != 
					RegQueryValueEx(key, _TEXT("MacroEnabled"),0,&valType,(LPBYTE)&m_macroEnabled, &nbytes));

	err = err || (ERROR_SUCCESS != 
					RegQueryValueEx(key, _TEXT("UseUnicodeClipboard"),0,&valType,(LPBYTE)&m_useUnicodeClipboard, &nbytes));

	err = err || (ERROR_SUCCESS != 
					RegQueryValueEx(key, _TEXT("AlwaysMacro"),0,&valType,(LPBYTE)&m_alwaysMacro, &nbytes));

	err = err || (ERROR_SUCCESS != 
					RegQueryValueEx(key, _TEXT("UseIME"),0,&valType,(LPBYTE)&m_useIME, &nbytes));

	if (err)
		setDefault();
	RegCloseKey(key);
	m_autoStart = isAutoStartEnabled();

	// check for valid values from registry
	if (m_codeTable >= TOTAL_CHARSET || m_codeTable < 0)
		m_codeTable = 0;
	if (m_switchKey >= 2 || m_switchKey < 0)
		m_switchKey = 0;
	if (m_otherCharset < POPULAR_CHARSETS || m_otherCharset >= TOTAL_CHARSET)
		m_otherCharset = POPULAR_CHARSETS;
	if (m_codeTable >= POPULAR_CHARSETS && m_codeTable != m_otherCharset)
		m_codeTable = m_otherCharset;
	if (m_inMethod >= INPUT_METHODS)
		m_inMethod = TELEX_INPUT;
	if (m_toneNextToVowel != 0 && m_toneNextToVowel != 1)
		m_toneNextToVowel = 0;
	if (m_freeMarking != 0 && m_freeMarking != 1)
		m_freeMarking = 0;
	if (m_vietGUI != 0 && m_vietGUI != 1)
		m_vietGUI = 0;
	return 1;
}


//---------------------------------------------
int enableAutoStart(int enabled)
{
	long res;
	HKEY key, hRoot;
	DWORD state;
	TCHAR path[_MAX_PATH];
	int ok;
	OSVERSIONINFO version;
	version.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&version);
	if (version.dwPlatformId == VER_PLATFORM_WIN32_NT)
		hRoot = HKEY_CURRENT_USER;
	else
		hRoot = HKEY_LOCAL_MACHINE;

/*
	res = RegOpenKeyEx(
				hRoot, 
				_T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
				0, // option, not used
				KEY_SET_VALUE | KEY_QUERY_VALUE, // allow setting values
				&key);
*/
	res = RegCreateKeyEx(
				hRoot,
				_T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
				0,
				NULL,
				REG_OPTION_NON_VOLATILE,    // special options flag
				KEY_QUERY_VALUE | KEY_SET_VALUE,        // desired security access
				NULL,
				&key,          // receives opened handle
				&state);

	if (res!=ERROR_SUCCESS)
		return 0;

	if (enabled) {
		GetModuleFileName(NULL, path, sizeof(path)/sizeof(TCHAR));
		res = RegSetValueEx(key, STARTUP_REG_VALUENAME, 0, REG_SZ, (LPBYTE)path, (_tcsclen(path)+1)*sizeof(TCHAR));
		ok = (res == ERROR_SUCCESS);
	}
	else {
		RegDeleteValue(key, STARTUP_REG_VALUENAME);
		res = RegQueryValueEx(key, STARTUP_REG_VALUENAME, 0, NULL, NULL, NULL);
		ok = (res != ERROR_SUCCESS);
	}
	RegCloseKey(key);
	return ok;
}


//---------------------------------------------
int isAutoStartEnabled()
{
	long res;
	HKEY key, hRoot;
	OSVERSIONINFO version;
	version.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&version);
	if (version.dwPlatformId == VER_PLATFORM_WIN32_NT)
		hRoot = HKEY_CURRENT_USER;
	else
		hRoot = HKEY_LOCAL_MACHINE;
	res = RegOpenKeyEx(
				hRoot, 
				_T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
				0, // option, not used
				KEY_SET_VALUE | KEY_QUERY_VALUE, // allow setting values
				&key);
	if (res!=ERROR_SUCCESS)
		return 0;
	res = RegQueryValueEx(key, STARTUP_REG_VALUENAME, 0, NULL, NULL, NULL);
	RegCloseKey(key);
	return (res == ERROR_SUCCESS);
}
