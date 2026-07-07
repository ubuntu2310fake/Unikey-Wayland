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
//#include <windows.h>
#include "prehdr.h"
#include <windowsx.h>
#include "resource.h"
#include "keyhook.h"
#include "encode.h"
#include "about.h"
#include "keydlg.h"
#include "mainwnd.h"
#include "userpref.h"
#include "macrodlg.h"
#include "vnconv.h"

// external variable in mainwnd.cpp
extern TCHAR AppTitle[100];
extern CMainWnd MainWnd;
extern CUserPref UserPref;

char * ExpandLabel[] = {
		"Expand",
		"Më réng"};

char * ContractLabel[] = {
		"Contract",
		"Thu nhá"};

AnsiDlgItemInfo VnDlgItems[] = { //use explicit ansi string for vietnamese in TCVN3
    {IDC_FREE_MARKING,		"Cho phÐp gâ dÊu mò, mãc ë cuèi tõ"},
    {IDC_TONE_CTRL,			"Ph¶i gâ dÊu ngay sau nguyªn ©m"},
    {IDC_MODERN_STYLE,		"§Æt dÊu oµ, uý (thay v× ßa, óy)"},
	{IDC_VIET_GUI,			"Vietnamese interface"},
    {IDC_SHOW_DLG,			"BËt héi tho¹i nµy khi khëi ®éng"},
    {IDC_RESET,				"MÆc ®Þnh"},
    {IDC_EXIT,				"&KÕt thóc"},
    {IDOK,					"§ãng"},
    {IDC_OPEN_HELP,			"&H­íng dÉn"},
    {IDC_ABOUT,				"Th«ng tin"},
    {IDC_CONTROL_GROUP,		"§iÒu khiÓn"},
    {IDC_CHARSET_PROMPT,	"B¶ng m·:"},
    {IDC_SWITCHKEY_PROMPT,	"PhÝm chuyÓn:"},
    {IDC_KEYMODE_PROMPT,	"KiÓu gâ:"},
    {IDC_OPTION_GROUP,		"Tïy chän kh¸c"}, //"Lùa chän"
	{IDC_AUTO_START,		"Khëi ®éng cïng Windows"},
	{IDC_MACRO_CHECK,		"Cho phÐp gâ t¾t"},
	{IDC_MACRO_BTN,			"B¶ng gâ t¾t..."},
	{IDC_USE_CLIPBOARD,     "Sö dông clipboard cho unicode"}
	};

#define MAX_ITEM_LEN 60

#ifdef _UNICODE

wchar_t UniVnDlgItemTexts[sizeof(VnDlgItems)/sizeof(AnsiDlgItemInfo)][MAX_ITEM_LEN];
DlgItemInfo UniVnDlgItems[sizeof(VnDlgItems)/sizeof(AnsiDlgItemInfo)];

#endif

DlgItemInfo EnDlgItems[] = { // use generic string
    {IDC_FREE_MARKING,		_T("Allow typing ^, v, + ... at the end")},
    {IDC_TONE_CTRL,			_T("Tone mark next to vowel")},
    {IDC_MODERN_STYLE,		_T("Use oa`, uy' (instead of o`a, u'y)")},
	{IDC_VIET_GUI,			_T("Giao dien tieng Viet")},
    {IDC_SHOW_DLG,			_T("Show this dialog box at startup")},
    {IDC_RESET,				_T("Default")},
    {IDC_EXIT,				_T("&Exit")},
    {IDOK,					_T("&Close")},
    {IDC_OPEN_HELP,			_T("&Help")},
    {IDC_ABOUT,				_T("About")},
    {IDC_CONTROL_GROUP,		_T("Settings")},
    {IDC_CHARSET_PROMPT,	_T("Character set:")},
    {IDC_SWITCHKEY_PROMPT,	_T("Switch key:")},
    {IDC_KEYMODE_PROMPT,	_T("Input method:")},
    {IDC_OPTION_GROUP,		_T("Advanced options")},
	{IDC_AUTO_START,		_T("Auto-run UniKey at boot time")},
	{IDC_MACRO_CHECK,		_T("Enable macro")},
	{IDC_MACRO_BTN,			_T("Macro table...")},
	{IDC_USE_CLIPBOARD,     _T("Use clipboard for unicode")}
	};

TCHAR *InputMethodText[] = {
		_T("TELEX"),
		_T("VNI"),
		_T("VIQR"),
		_T("VIQR*")};

//------------------------------------------
int CKeyDlg::init(HINSTANCE hInst, HWND hWndOwner)
{
	TCHAR expandStr[MAX_ITEM_LEN], contractStr[MAX_ITEM_LEN];
#ifndef _UNICODE
	strcpy(expandStr, (UserPref.m_vietGUI)? ExpandLabel[1] : ExpandLabel[0]);
	strcpy(contractStr, (UserPref.m_vietGUI)? ContractLabel[1] : ContractLabel[0]);
#else
	int inLen, maxOutLen;
	inLen = -1;
	maxOutLen = MAX_ITEM_LEN * 2;
	VnConvert(CONV_CHARSET_TCVN3, CONV_CHARSET_UNICODE,
			(BYTE *)((UserPref.m_vietGUI)? ExpandLabel[1] : ExpandLabel[0]),
			(BYTE *)expandStr,
			inLen, maxOutLen);

	inLen = -1;
	maxOutLen = MAX_ITEM_LEN * 2;
	VnConvert(CONV_CHARSET_TCVN3, CONV_CHARSET_UNICODE,
			(BYTE *)((UserPref.m_vietGUI)? ContractLabel[1] : ContractLabel[0]),
			(BYTE *)contractStr,
			inLen, maxOutLen);
#endif

	int ret = CExpandDialog::init(
				hInst,
				MAKEINTRESOURCE(IDD_KEY_DIALOG),
				hWndOwner,
				IDC_COMPACT_FRAME,
				IDC_EXPAND,
				IDC_FOOTER_FRAME,
				expandStr,
				contractStr);

	initLanguageInfo();
	setupLanguage(0, EnDlgItems, sizeof(EnDlgItems)/sizeof(DlgItemInfo));
#ifndef _UNICODE
	setupLanguage(1, VnDlgItems, sizeof(VnDlgItems)/sizeof(AnsiDlgItemInfo), 
		          VIET_FONT_NAME, VIET_DLG_FONT_SIZE);
#else
	int items = sizeof(VnDlgItems)/sizeof(AnsiDlgItemInfo);
	for (int i=0; i<items; i++) {
		inLen = -1;
		maxOutLen = 2*MAX_ITEM_LEN;
		VnConvert(CONV_CHARSET_TCVN3, CONV_CHARSET_UNICODE,
				(BYTE *)VnDlgItems[i].text, (BYTE *)UniVnDlgItemTexts[i],
				inLen, maxOutLen);
		UniVnDlgItems[i].id = VnDlgItems[i].id;
		UniVnDlgItems[i].text = UniVnDlgItemTexts[i];
	}
	setupLanguage(1, UniVnDlgItems, items, VIET_FONT_NAME, VIET_DLG_FONT_SIZE);
#endif
	return ret;
}

//------------------------------------------
void CKeyDlg::loadDlgInfo(CUserPref *pPref)
{
	int i;
	SendMessage(m_codeList, CB_RESETCONTENT, 0, 0);
	for (i=0; i < TOTAL_CHARSET; i++)
		SendMessage(m_codeList, CB_ADDSTRING, 0, (LPARAM)CharsetTable[i].name);
	SendMessage(m_codeList, CB_SETCURSEL, pPref->m_codeTable, 0); 

	SendMessage(m_inputList, CB_RESETCONTENT, 0, 0);
	int methods = sizeof(InputMethodText)/sizeof(InputMethodText[0]);
	for (i=0; i < methods; i++)
		SendMessage(m_inputList, CB_ADDSTRING, 0, (LPARAM)InputMethodText[i]);
	SendMessage(m_inputList, CB_SETCURSEL, pPref->m_inMethod, 0); 

	SendMessage(m_showCtrl,
				BM_SETCHECK,
				(pPref->m_showDlg)? BST_CHECKED : BST_UNCHECKED,
				0);
	SendMessage(m_markCheck,
				BM_SETCHECK,
				(pPref->m_freeMarking)? BST_CHECKED : BST_UNCHECKED,
				0);
	SendMessage(m_toneCtrl,
				BM_SETCHECK,
				(pPref->m_toneNextToVowel)? BST_CHECKED : BST_UNCHECKED,
				0);

	SendMessage(m_modernStyleCheck,
				BM_SETCHECK,
				(pPref->m_modernStyle)? BST_CHECKED : BST_UNCHECKED,
				0);

	SendMessage(m_swKeyBtn1, 
				BM_SETCHECK, 
				(pPref->m_switchKey==0)? BST_CHECKED : BST_UNCHECKED, 
				0);

	SendMessage(m_swKeyBtn2, 
				BM_SETCHECK, 
				(pPref->m_switchKey==1)? BST_CHECKED : BST_UNCHECKED, 
				0);
	SendDlgItemMessage(m_hWnd, IDC_VIET_GUI, BM_SETCHECK,
				(pPref->m_vietGUI)? BST_CHECKED : BST_UNCHECKED,
				0);

	SendDlgItemMessage(m_hWnd, IDC_AUTO_START, BM_SETCHECK,
				(pPref->m_autoStart)? BST_CHECKED : BST_UNCHECKED,
				0);

	Button_SetCheck(m_macroCheck, pPref->m_macroEnabled);
	Button_SetCheck(m_useUnicodeClipboardCheck, pPref->m_useUnicodeClipboard);

	if (!MainWnd.isVietFontOK())
		EnableWindow(GetDlgItem(m_hWnd, IDC_VIET_GUI), FALSE);

	pPref->m_changes = 0;

	OSVERSIONINFO osinfo;
	osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osinfo);
	if (osinfo.dwMajorVersion < 4 || osinfo.dwPlatformId != VER_PLATFORM_WIN32_NT) {
		Button_SetCheck(m_useUnicodeClipboardCheck, 1);
		EnableWindow(m_useUnicodeClipboardCheck, FALSE);
	}

}

//------------------------------------------
void CKeyDlg::storeDlgInfo(CUserPref *pPref)
{
	int sel;
	// character set
	sel = (int)SendMessage(m_codeList,CB_GETCURSEL,0,0);
	if (sel != (int)pPref->m_codeTable) {
		pPref->m_changes |= PREF_CHANGE_CHARSET;
		pPref->m_oldTable = pPref->m_codeTable;
		pPref->m_codeTable = sel;
		if (sel != (int)pPref->m_otherCharset && sel >= POPULAR_CHARSETS) {
			pPref->m_changes |= PREF_CHANGE_OTHERCHARSET;
			pPref->m_otherCharset = sel;
		}
	}

	sel = (int)SendMessage(m_inputList,CB_GETCURSEL,0,0);
	if (sel != (int)pPref->m_inMethod) {
		pPref->m_changes |= PREF_CHANGE_METHOD;
		pPref->m_inMethod = sel;
	}
	
	sel = (SendMessage(m_swKeyBtn1, BM_GETCHECK, 0, 0) == BST_CHECKED)? 0 : 1;
	if (sel != (int)pPref->m_switchKey) {
		pPref->m_changes |= PREF_CHANGE_SWITCHKEY;
		pPref->m_switchKey = sel;
	}

	sel = (SendMessage(m_markCheck, BM_GETCHECK, 0, 0) == BST_CHECKED)? 1 : 0;
	if (sel && !pPref->m_freeMarking || !sel && pPref->m_freeMarking) {
		pPref->m_changes |= PREF_CHANGE_OPTIONS;
		pPref->m_freeMarking = sel;
	}

	sel = (SendMessage(m_toneCtrl, BM_GETCHECK, 0, 0) == BST_CHECKED)? 1 : 0;
	if (sel && !pPref->m_toneNextToVowel || !sel && pPref->m_toneNextToVowel) {
		pPref->m_changes |= PREF_CHANGE_OPTIONS;
		pPref->m_toneNextToVowel = sel;
	}

	sel = (SendMessage(m_modernStyleCheck, BM_GETCHECK, 0, 0) == BST_CHECKED)? 1 : 0;
	if (sel && !pPref->m_modernStyle || !sel && pPref->m_modernStyle) {
		pPref->m_changes |= PREF_CHANGE_OPTIONS;
		pPref->m_modernStyle = sel;
	}

	sel = Button_GetCheck(m_macroCheck);
	if (sel && !pPref->m_macroEnabled || !sel && pPref->m_macroEnabled) {
		pPref->m_changes |= PREF_CHANGE_OPTIONS;
		pPref->m_macroEnabled = sel;
	}

	sel = Button_GetCheck(m_useUnicodeClipboardCheck);
	if (sel && !pPref->m_useUnicodeClipboard || !sel && pPref->m_useUnicodeClipboard) {
		pPref->m_changes |= PREF_CHANGE_OPTIONS;
		pPref->m_useUnicodeClipboard = sel;
	}

	sel = (SendMessage(m_showCtrl,BM_GETCHECK,0,0) == BST_CHECKED) ? 1 : 0;
	if (sel && !pPref->m_showDlg || !sel && pPref->m_showDlg) {
		pPref->m_changes |= PREF_CHANGE_SHOWDLG;
		pPref->m_showDlg = sel;
	}
	

	// GUI interface
	if (SendDlgItemMessage(m_hWnd, IDC_VIET_GUI,BM_GETCHECK, 0, 0) == BST_CHECKED)
		sel = 1;
	else
		sel = 0;
	if (sel && !pPref->m_vietGUI || !sel && pPref->m_vietGUI) {
		pPref->m_changes |= PREF_CHANGE_GUI;
		pPref->m_vietGUI = sel;
	}

	if (SendDlgItemMessage(m_hWnd, IDC_AUTO_START, BM_GETCHECK, 0, 0) == BST_CHECKED)
		sel = 1;
	else
		sel = 0;
	if (sel && !pPref->m_autoStart || !sel && pPref->m_autoStart) {
		pPref->m_changes |= PREF_CHANGE_AUTOSTART;
		pPref->m_autoStart = sel;
	}

	if (m_tempPref.m_alwaysMacro != pPref->m_alwaysMacro) {
		pPref->m_alwaysMacro = m_tempPref.m_alwaysMacro;
		pPref->m_changes |= PREF_CHANGE_OPTIONS;
	}

}

//------------------------------------------
BOOL CKeyDlg::DialogProc(
					HWND hDlg,
					UINT uMsg,
					WPARAM wParam,
					LPARAM lParam)
{
	switch (uMsg) {
	case WM_SYSCOMMAND:
		{
			WORD cmd = (wParam & 0xFFF0) >> 4;
			switch (cmd) {
			case IDM_ABOUT:
				DoAbout(m_hInst, hDlg,
					(SendDlgItemMessage(m_hWnd, IDC_VIET_GUI, BM_GETCHECK, 0, 0) == BST_CHECKED)? 1 : 0);
				return TRUE;
			}
		}
		break;
	case WM_DRAWITEM:
		// Send reflection message to back the control
		return SendDlgItemMessage(hDlg, wParam, WM_DRAWITEM, wParam, lParam);
	case WM_DESTROY:
		if (m_hStdFont != NULL) {
			DeleteObject(m_hStdFont);
			m_hStdFont = NULL;
		}
		break;
	}
	return CExpandDialog::DialogProc(hDlg, uMsg, wParam, lParam);
}

//------------------------------------------
BOOL CKeyDlg::onInitDialog()
{
	if (!CExpandDialog::onInitDialog())	return FALSE;

	if (MainWnd.isVietFontOK())
		setDlgFont(1);

	m_tempPref = *m_pPref;

	applyLanguage(UserPref.m_vietGUI? 1 : 0);

	HICON mainBigIcon, mainSmallIcon;

	// Setup dialog box icons
	mainBigIcon = (HICON)LoadImage(m_hInst, 
									MAKEINTRESOURCE(IDR_MAINFRAME), 
									IMAGE_ICON,
									32, 32, 
									LR_SHARED);
	mainSmallIcon = (HICON)LoadImage(m_hInst, 
									MAKEINTRESOURCE(IDR_MAINFRAME), 
									IMAGE_ICON,
									16, 16, 
									LR_SHARED);
	SendMessage(m_hWnd, WM_SETICON, ICON_BIG, (LPARAM)mainBigIcon);
	SendMessage(m_hWnd, WM_SETICON, ICON_SMALL, (LPARAM)mainSmallIcon);

	// Setup system menu
	HMENU hSysMenu = GetSystemMenu(m_hWnd, FALSE);
	if (hSysMenu) {
		TCHAR itemStr[100];
		LoadString(m_hInst, IDS_ABOUT, itemStr, sizeof(itemStr)/sizeof(TCHAR));
		AppendMenu(hSysMenu, MF_SEPARATOR, 0, NULL);
		AppendMenu(hSysMenu, MF_STRING, ((UINT)IDM_ABOUT)<<4, itemStr);
	}
	// Setup owner-draw button
	m_aboutBtn.init(m_hInst, m_hWnd, GetDlgItem(m_hWnd, IDC_ABOUT));
	m_aboutBtn.setIcon(IDI_INFO, 16, 16);

	m_okBtn.init(m_hInst, m_hWnd, GetDlgItem(m_hWnd, IDOK));
	m_okBtn.setIcon(IDI_OK, 16, 16);
	m_okBtn.setDefaultButton(TRUE);
	SendMessage(m_hWnd, DM_SETDEFID, IDOK, 0);

	m_cancelBtn.init(m_hInst, m_hWnd, GetDlgItem(m_hWnd, IDC_EXIT));
	m_cancelBtn.setIcon(IDI_CANCEL, 16, 16);

	m_helpBtn.init(m_hInst, m_hWnd, GetDlgItem(m_hWnd, IDC_OPEN_HELP));
	m_helpBtn.setIcon(IDI_HELP, 16, 16);

	m_resetBtn.init(m_hInst, m_hWnd, GetDlgItem(m_hWnd, IDC_RESET));
	m_resetBtn.setIcon(IDI_RESET, 16, 16);

	m_expandBtn.init(m_hInst, m_hWnd, GetDlgItem(m_hWnd, IDC_EXPAND));
	m_expandBtn.setIcon(IDI_EXPAND, 16, 16);

	m_macroBtn.init(m_hInst, m_hWnd, GetDlgItem(m_hWnd, IDC_MACRO_BTN));
	//m_macroBtn.setIcon(IDI_HELP, 16, 16);

	m_codeList = GetDlgItem(m_hWnd,IDC_CODE_TABLE);
	m_showCtrl = GetDlgItem(m_hWnd,IDC_SHOW_DLG);
	m_markCheck = GetDlgItem(m_hWnd, IDC_FREE_MARKING);
	m_toneCtrl = GetDlgItem(m_hWnd, IDC_TONE_CTRL);
	m_modernStyleCheck = GetDlgItem(m_hWnd, IDC_MODERN_STYLE);
	m_macroCheck = GetDlgItem(m_hWnd, IDC_MACRO_CHECK);
	m_useUnicodeClipboardCheck = GetDlgItem(m_hWnd, IDC_USE_CLIPBOARD);

	m_swKeyBtn1 = GetDlgItem(m_hWnd, IDC_SW_KEY1);
	m_swKeyBtn2 = GetDlgItem(m_hWnd, IDC_SW_KEY2);

	m_inputList = GetDlgItem(m_hWnd, IDC_INPUT_METHOD);

	loadDlgInfo(m_pPref);
	expand(FALSE);
	return TRUE;
}

//-----------------------------------------------
BOOL CKeyDlg::onOK()
{
	HWND hFocusWnd = GetFocus();
	if (IsPushButton(hFocusWnd)) {
		int id = GetDlgCtrlID(hFocusWnd);
		if (id != IDOK) {
			SendMessage(hFocusWnd, BM_CLICK, 0, 0);
			return FALSE;
		}
	}
	storeDlgInfo(m_pPref);
	DestroyWindow(m_hWnd);
	m_hWnd = NULL;
	SendMessage(m_hWndOwner, WM_COMMAND, ID_CONFIRM_PANEL, 0);
	return TRUE;
}


//-----------------------------------------------
BOOL CKeyDlg::onCancel()
{
	ModifyStatusIcon();
	return TRUE;
}

//-----------------------------------------------
BOOL CKeyDlg::onCommand(WPARAM wParam, LPARAM lParam)
{
	WORD cmd = GET_WM_COMMAND_ID(wParam, lParam);
	switch (cmd) {
	case IDC_ABOUT:
		DoAbout(m_hInst, m_hWnd, 
			(SendDlgItemMessage(m_hWnd, IDC_VIET_GUI, BM_GETCHECK, 0, 0) == BST_CHECKED)? 1 : 0);
		return TRUE;
	case IDC_OPEN_HELP:
		MainWnd.openHelp();
		return TRUE;
	case IDC_RESET:
		{
			CUserPref current, def;
			storeDlgInfo(&current);
			def.setDefault(&current);
			loadDlgInfo(&def);
		}
		break;
	case IDC_EXIT:
		storeDlgInfo(m_pPref);
		DestroyWindow(m_hWnd);
		m_hWnd = NULL;
		SendMessage(m_hWndOwner, WM_COMMAND, ID_END_KEY, 0);
		return TRUE;
	case IDC_VIET_GUI:
		if (GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED) {
			int langID;
			langID = (SendDlgItemMessage(m_hWnd, IDC_VIET_GUI, BM_GETCHECK, 0, 0) == BST_CHECKED)? 1 : 0;
			applyLanguage(langID);
		}
		break;
	case IDC_MACRO_BTN:
		{
			CMacroDlg macroDlg(m_pPref, &m_tempPref);
			macroDlg.init(m_hInst, m_hWnd);
			macroDlg.setLangID(Button_GetCheck(GetDlgItem(m_hWnd, IDC_VIET_GUI))? 1 : 0);
			if (macroDlg.ShowDialog() == IDOK) {
				MainWnd.updateMacroTable();
			}
		}
		break;
	case IDC_INPUT_METHOD:
		if (GET_WM_COMMAND_CMD(wParam, lParam) == CBN_SELCHANGE) {
			Button_SetCheck(m_markCheck, ComboBox_GetCurSel(m_inputList) != TELEX_INPUT);
		}
		break;
	}
	return CExpandDialog::onCommand(wParam, lParam);
}

//----------------------------------------------
BOOL CKeyDlg::onDialogExpanding(BOOL bExpanded)
{
	CExpandDialog::onDialogExpanding(bExpanded);
	m_expandBtn.setIcon(bExpanded ? IDI_EXPAND : IDI_CONTRACT, 16, 16);
	return TRUE;
}

//----------------------------------------------
void CKeyDlg::applyLanguage(int langIdx)
{
	CMultiLangDialog::applyLanguage(langIdx);
	if (langIdx >= 0 && langIdx < 2) { // currently support 2 languages En,Vi
#ifndef _UNICODE
		strcpy(m_strExpand, ExpandLabel[langIdx]);
		strcpy(m_strContract, ContractLabel[langIdx]);
#else
		int inLen, maxOutLen;
		inLen = -1;
		maxOutLen = sizeof(m_strExpand);
		VnConvert(CONV_CHARSET_TCVN3, CONV_CHARSET_UNICODE,
			(BYTE *)ExpandLabel[langIdx], (BYTE *)m_strExpand,
			inLen, maxOutLen);

		inLen = -1;
		maxOutLen = sizeof(m_strContract);
		VnConvert(CONV_CHARSET_TCVN3, CONV_CHARSET_UNICODE,
			(BYTE *)ContractLabel[langIdx], (BYTE *)m_strContract,
			inLen, maxOutLen);
#endif
		SendDlgItemMessage(m_hWnd, m_nIDButton, WM_SETTEXT, 
			               0, (LPARAM)(m_bExpanded ? m_strContract : m_strExpand));
	}
}

//----------------------------------------------
BOOL CKeyDlg::onDestroy()
{
	cleanup();
	return CVirDialog::onDestroy();
}