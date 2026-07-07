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
#include "resource.h"
#include <windowsx.h>
#include <stdio.h>

#include "keycons.h"
#include "mainwnd.h"
#include "macrodlg.h"
#include "encode.h"
#include "util.h"
#include "vnconv.h"

//#define MAX_MACRO_LEN 20
//#define MAX_REPL_TEXT_LEN 128

#define MACRO_EDIT_KEYDOWN (WM_USER + 105)

char * ReplaceLabel[] = {
		"Replace",
		"&Söa"};

char * AddLabel[] = {
		"&Add",
		"&Thªm"};


AnsiDlgItemInfo VnMacroDlgItems[] = {
	{IDC_STATIC_REPLACE,	"Thay thÕ:"},
	{IDC_STATIC_WITH,		"Bëi:"},
	{IDOK,					"&L­u"},
	{IDCANCEL,				"&Ngõng"},
	{IDC_DELETE,			"&Xo¸"},
	{IDC_NOTE,				"L­u ý: th«ng tin ë ®©y ®­îc thÓ hiÖn d­íi d¹ng VIQR"},
	{IDC_ALWAYS_MACRO,      "Cho phÐp gâ t¾t c¶ khi t¾t tiÕng ViÖt"},
	{IDC_SELECT_FILE,       "Chän File..."},
	{IDC_SELECT_DEFAULT,    "File mÆc dÞnh"},
	{IDC_PATH_LABEL,        "File gâ t¾t:"}
	};

DlgItemInfo EnMacroDlgItems[] = {
	{IDC_STATIC_REPLACE,	_T("Replace:")},
	{IDC_STATIC_WITH,		_T("With:")},
	{IDOK,					_T("&OK")},
	{IDCANCEL,				_T("&Cancel")},
	{IDC_DELETE,			_T("&Delete")},
	{IDC_NOTE,				_T("Note: Replacement texts are displayed here in VIQR format")},
	{IDC_ALWAYS_MACRO,      _T("Allow macros even if Vietnamese is off")},
	{IDC_SELECT_FILE,       _T("Select file...")},
	{IDC_SELECT_DEFAULT,    _T("Select default")},
	{IDC_PATH_LABEL,        _T("Macro file:")}
	};

#define MAX_ITEM_LEN 60

#ifdef _UNICODE

wchar_t UniVnMacroDlgTexts[sizeof(VnMacroDlgItems)/sizeof(AnsiDlgItemInfo)][MAX_ITEM_LEN];
DlgItemInfo UniVnMacroDlgItems[sizeof(VnMacroDlgItems)/sizeof(AnsiDlgItemInfo)];

#endif

extern CMainWnd MainWnd;

//local helpers
const TCHAR *getKey(const TCHAR *text);
const TCHAR *getReplText(const TCHAR *text);
int getExactItem(HWND listCtrl, TCHAR *key, int & nearest);


//------------------------------------------
int CMacroDlg::init(HINSTANCE hInst, HWND hWndOwner)
{
	int ret = CMultiLangDialog::init(hInst, MAKEINTRESOURCE(IDD_MACRO), hWndOwner);
	initLanguageInfo();
	setupLanguage(0, EnMacroDlgItems, sizeof(EnMacroDlgItems)/sizeof(DlgItemInfo));

#ifndef _UNICODE
	setupLanguage(1, VnMacroDlgItems, sizeof(VnMacroDlgItems)/sizeof(AnsiDlgItemInfo), 
		          VIET_FONT_NAME, VIET_DLG_FONT_SIZE);
#else
	int items = sizeof(VnMacroDlgItems)/sizeof(AnsiDlgItemInfo);
	int inLen, maxOutLen;
	for (int i=0; i<items; i++) {
		inLen = -1;
		maxOutLen = 2*MAX_ITEM_LEN;
		VnConvert(CONV_CHARSET_TCVN3, CONV_CHARSET_UNICODE,
				(BYTE *)VnMacroDlgItems[i].text, (BYTE *)UniVnMacroDlgTexts[i],
				inLen, maxOutLen);
		UniVnMacroDlgItems[i].id = VnMacroDlgItems[i].id;
		UniVnMacroDlgItems[i].text = UniVnMacroDlgTexts[i];
	}
	setupLanguage(1, UniVnMacroDlgItems, items, VIET_FONT_NAME, VIET_DLG_FONT_SIZE);
#endif

	return ret;
}

//--------------------------------------------
int CMacroDlg::loadMacTab(CMacroTable *pTab)
{
	// init list box items
	TCHAR str[MAX_MACRO_LINE];
	ListBox_ResetContent(m_itemListCtrl);
	for (int i=0; i < pTab->m_count; i++) {
#ifndef _UNICODE
		sprintf(str, "%s\t%s", 
					pTab->m_table[i].key, 
					pTab->m_table[i].text);
#else
		wchar_t wKeyStr[MAX_MACRO_KEY_LEN];
		wchar_t wTextStr[MAX_MACRO_TEXT_LEN];
		int inLen, maxOutLen;
		inLen = -1;
		maxOutLen = sizeof(wKeyStr);
		VnConvert(CONV_CHARSET_VIQR, CONV_CHARSET_UNICODE,
				(BYTE *)pTab->m_table[i].key,
				(BYTE *)wKeyStr,
				inLen, maxOutLen);

		inLen = -1;
		maxOutLen = sizeof(wTextStr);
		VnConvert(CONV_CHARSET_VIQR, CONV_CHARSET_UNICODE,
				(BYTE *)pTab->m_table[i].text,
				(BYTE *)wTextStr,
				inLen, maxOutLen);

		//MultiByteToWideChar(CP_US_ANSI, 0, pTab->m_table[i]->key, -1, wKeyStr, 256);
		//MultiByteToWideChar(CP_US_ANSI, 0, pTab->m_table[i]->text, -1, wTextStr, 256);
		swprintf(str, _TEXT("%s\t%s"), wKeyStr, wTextStr); 
#endif
		ListBox_AddString(m_itemListCtrl, str);
	}
	return 1;
}
//------------------------------------------
BOOL CMacroDlg::onInitDialog()
{
	if (!CMultiLangDialog::onInitDialog())	
		return FALSE;

	if (MainWnd.isVietFontOK())
		setDlgFont(1);

	applyLanguage(m_langID? 1 : 0);

	m_selectFileBtn.init(m_hInst, m_hWnd, GetDlgItem(m_hWnd, IDC_SELECT_FILE));
	m_selectFileBtn.setIcon(IDI_OPEN_FOLDER, 16, 16);

	m_defFileBtn.init(m_hInst, m_hWnd, GetDlgItem(m_hWnd, IDC_SELECT_DEFAULT));
	m_defFileBtn.setIcon(IDI_RESET, 16, 16);

	m_okBtn.init(m_hInst, m_hWnd, GetDlgItem(m_hWnd, IDOK));
	m_okBtn.setIcon(IDI_OK, 16, 16);

	m_cancelBtn.init(m_hInst, m_hWnd, GetDlgItem(m_hWnd, IDCANCEL));
	m_cancelBtn.setIcon(IDI_CANCEL, 16, 16);

	m_editBtn.init(m_hInst, m_hWnd, GetDlgItem(m_hWnd, IDC_EDIT));
	m_editBtn.setIcon(IDI_ADD, 16, 16);
	m_editBtn.setDefaultButton(TRUE);
	SendMessage(m_hWnd, DM_SETDEFID, IDC_EDIT, 0);

	m_deleteBtn.init(m_hInst, m_hWnd, GetDlgItem(m_hWnd, IDC_DELETE));
	m_deleteBtn.setIcon(IDI_DELETE, 16, 16);

	m_itemListCtrl = GetDlgItem(m_hWnd, IDC_ITEMLIST);

	//m_textEdit.init(m_hInst, m_hWnd, GetDlgItem(m_hWnd, IDC_TEXT));
	m_textCtrl = GetDlgItem(m_hWnd, IDC_TEXT);
	m_keyEdit.init(m_hInst, m_hWnd, GetDlgItem(m_hWnd, IDC_KEY));

	m_alwaysMacroCheck = GetDlgItem(m_hWnd, IDC_ALWAYS_MACRO);
	Button_SetCheck(m_alwaysMacroCheck, m_pTempPref->m_alwaysMacro);

	m_pathCtrl = GetDlgItem(m_hWnd, IDC_FILE_NAME);
	_tcscpy(m_path, MainWnd.m_currentMacPath);
	Static_SetText(m_pathCtrl, m_path);

	int tab = 56;
	ListBox_SetTabStops(m_itemListCtrl, 1, &tab);
	setReplaceBtn(0);
	EnableWindow(m_editBtn.m_hWnd, FALSE);
	EnableWindow(m_deleteBtn.m_hWnd, FALSE);

//	setEditFont();
	loadMacTab(&MainWnd.m_macTab);

	return TRUE;
}

//-----------------------------------------------
BOOL CMacroDlg::onOK()
{
	//DestroyWindow(m_hWnd);
	//m_hWnd = NULL;
	_tcscpy(MainWnd.m_currentMacPath, m_path);
	MainWnd.m_macTab.resetContent();
	int count = ListBox_GetCount(m_itemListCtrl);
	TCHAR str[MAX_MACRO_LINE];
	for (int i=0; i < count; i++) {
		ListBox_GetText(m_itemListCtrl, i, str);

#ifndef _UNICODE
		MainWnd.m_macTab.addItem(getKey(str), getReplText(str));
#else
		char keyStr[MAX_MACRO_KEY_LEN];
		char textStr[MAX_MACRO_TEXT_LEN];
//		WideCharToMultiByte(CP_US_ANSI, 0, getKey(str), -1, keyStr, sizeof(keyStr), NULL, NULL);
//		WideCharToMultiByte(CP_US_ANSI, 0, getReplText(str), -1, textStr, sizeof(textStr), NULL, NULL);
		int inLen, maxOutLen;

		inLen = -1;
		maxOutLen = sizeof(keyStr);
		if (VnConvert(CONV_CHARSET_UNICODE, CONV_CHARSET_VIQR,
				(BYTE *)getKey(str),
				(BYTE *)keyStr,
				inLen, maxOutLen) != 0)
			continue;

		inLen = -1;
		maxOutLen = sizeof(textStr);
		if (VnConvert(CONV_CHARSET_UNICODE, CONV_CHARSET_VIQR,
				(BYTE *)getReplText(str),
				(BYTE *)textStr,
				inLen, maxOutLen) != 0)
			continue;

		MainWnd.m_macTab.addItem(keyStr, textStr);
#endif

	}
	MainWnd.m_macTab.writeToFile(m_path);
	m_pTempPref->m_alwaysMacro = Button_GetCheck(m_alwaysMacroCheck);

	return TRUE;
}

//-----------------------------------------------
BOOL CMacroDlg::onDefaultButton()
{
	HWND hFocusWnd = GetFocus();
	if (IsPushButton(hFocusWnd)) {
		WORD defId = LOWORD(SendMessage(m_hWnd, DM_GETDEFID, 0, 0));
		int id = GetDlgCtrlID(hFocusWnd);
		if (id != defId) {
			SendMessage(hFocusWnd, BM_CLICK, 0, 0);
			return FALSE;
		}
	}
	return TRUE;
}

//------------------------------------------
void CMacroDlg::applyLanguage(int langIdx)
{
	CMultiLangDialog::applyLanguage(langIdx);
	if (langIdx >= 0 && langIdx < 2) { // currently support 2 languages En,Vi
		char *text;
		if (m_isEditReplace)
			text = ReplaceLabel[langIdx];
		else
			text = AddLabel[langIdx];
#ifndef _UNICODE
		SendDlgItemMessage(m_hWnd, IDC_EDIT, WM_SETTEXT, 0, (LPARAM)text);
#else
		wchar_t str[MAX_ITEM_LEN];
		int inLen, maxOutLen;
		inLen = -1;
		maxOutLen = sizeof(str);
		VnConvert(CONV_CHARSET_TCVN3, CONV_CHARSET_UNICODE,
			(BYTE *)text, (BYTE *)str,
			inLen, maxOutLen);
		SendDlgItemMessage(m_hWnd, IDC_EDIT, WM_SETTEXT, 0, (LPARAM)str);
#endif

	}
}

//------------------------------------------
BOOL CMacroDlg::onDestroy()
{
	if (m_hEditFont != NULL) {
		DeleteObject(m_hEditFont);
		m_hEditFont = NULL;
	}
	return CMultiLangDialog::onDestroy();
}


//------------------------------------------
void CMacroDlg::setEditFont()
{
	if (m_hEditFont != NULL) {
		DeleteObject(m_hEditFont);
		m_hEditFont = NULL;
	}
	HDC hDC = GetDC(m_hWnd);
	m_hEditFont = CustomCreateFont(hDC, _TEXT("Arial"), 10);
//	if (m_hEditFont)
//		SendMessage(m_contentCtrl, WM_SETFONT, (WPARAM)m_hEditFont, TRUE);
}


//--------------------------------------------------
BOOL CMacroDlg::DialogProc(
						HWND hDlg,
						UINT uMsg,
						WPARAM wParam,
						LPARAM lParam)
{
	switch (uMsg) {
	case WM_DRAWITEM:
		// Send reflection message to back the control
		return SendDlgItemMessage(hDlg, wParam, WM_DRAWITEM, wParam, lParam);
	case WM_ACTIVATE:
		updateKeyHook(wParam != WA_INACTIVE);
		break;
	
	case WM_KEYDOWN:
		{
			HWND hWndFocus = GetFocus();
			if (hWndFocus == m_keyEdit.m_hWnd) {
				if (wParam == VK_DOWN || wParam == VK_UP || 
					wParam == VK_PRIOR || wParam == VK_NEXT)
					//SetFocus(m_itemListCtrl);
					SendMessage(m_itemListCtrl, WM_KEYDOWN, wParam, lParam);
				return 0;
			}
		}
		break;
	}
	return CMultiLangDialog::DialogProc(hDlg, uMsg, wParam, lParam);
}

//------------------------------------------------------------------
// ownMode: 0 : Reset to system mode
//          1 : Set appropriate mode in order to input macro text (VIQR/Unicode)
//------------------------------------------------------------------
void CMacroDlg::updateKeyHook(int ownMode)
{
	if (ownMode) {
#ifndef _UNICODE
		BuildCodeTable(VIQR_CHARSET, m_pPref->m_inMethod, &pSharedMem->codeTable);
		SetKeyMode(VIQR_CHARSET, m_pPref->m_inMethod, &pSharedMem->codeTable);
#else
		BuildCodeTable(UNICODE_CHARSET, m_pPref->m_inMethod, &pSharedMem->codeTable);
		SetKeyMode(UNICODE_CHARSET, m_pPref->m_inMethod, &pSharedMem->codeTable);
/*
		BuildCodeTable(WINCP1258_CHARSET, m_pPref->m_inMethod, &pSharedMem->codeTable);
		SetKeyMode(WINCP1258_CHARSET, m_pPref->m_inMethod, &pSharedMem->codeTable);
*/
#endif
		pSharedMem->codeTable.encoding = 0;
		DWORD temp = m_pPref->m_macroEnabled;
		m_pPref->m_macroEnabled = 0;
		SetHookOptions(*m_pPref);
		m_pPref->m_macroEnabled = temp;
	}
	else {
		//MessageBeep(MB_ICONASTERISK);
		BuildCodeTable(CharsetTable[m_pPref->m_codeTable].id, m_pPref->m_inMethod, &pSharedMem->codeTable);
		pSharedMem->codeTable.encoding = CharsetTable[m_pPref->m_codeTable].encoding;
		SetKeyMode(CharsetTable[m_pPref->m_codeTable].id, m_pPref->m_inMethod, &pSharedMem->codeTable);
		SetHookOptions(*m_pPref);
	}
}


//------------------------------------------
int selectFileDlg(HWND hWndOwner, TCHAR *fileName, int size)
{
	OPENFILENAME of;
	ZeroMemory(&of, sizeof(OPENFILENAME));

	*fileName = 0;
	of.lStructSize = sizeof(OPENFILENAME);
	of.hwndOwner = hWndOwner;
	of.lpstrFile = fileName;
	of.nMaxFile = size;
	of.lpstrFilter = _TEXT("Plain text (*.txt)\0*.txt\0All files (*.*)\0*.*\0");
	of.nFilterIndex = 1;
	of.lpstrFileTitle = NULL;
	of.nMaxFileTitle = 0;
	of.lpstrInitialDir = NULL;
	of.lpstrTitle = _TEXT("Select macro file");
	of.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

	return GetOpenFileName(&of);
}

//---------------------------------------------------
int CMacroDlg::loadMacFile(TCHAR *path)
{
	CMacroTable macTab;
	if (!macTab.loadFromFile(path))
		return 0;
	loadMacTab(&macTab);
	return 1;
}

//-----------------------------------------------
BOOL CMacroDlg::onCommand(WPARAM wParam, LPARAM lParam)
{
	WORD cmdID = GET_WM_COMMAND_ID(wParam, lParam);
	WORD cmd = GET_WM_COMMAND_CMD(wParam, lParam);

	switch (cmdID) {
	case IDC_ITEMLIST:
		if (cmd == LBN_SELCHANGE) {
			onItemSelChange();
		}
		break;
	case IDC_DELETE:
		if (cmd == BN_CLICKED) {
			onDeleteItem();
		}
		break;
	case IDC_EDIT:
		if (!onDefaultButton())
			break;
		if (cmd == BN_CLICKED) {
			onEditItem();
		}
		break;
	case IDC_TEXT:
		if (cmd == EN_CHANGE) {
			onTextChange();
		}
		break;
	case IDC_KEY:
		if (cmd == EN_CHANGE) {
			onKeyChange();
		}
		break;
	case IDC_SELECT_FILE:
		{
			TCHAR path[MAX_PATH];
			_tcscpy(path, m_path);
			if (selectFileDlg(m_hWnd, path, sizeof(path)/sizeof(TCHAR))) {
				if (loadMacFile(path)) {
					_tcscpy(m_path, path);
					Static_SetText(m_pathCtrl, m_path);
				}
				else MessageBox(m_hWnd, _T("Failed to load the specified macro file"), 
								_T("Error"), MB_ICONERROR);
			}
		}
		break;
	case IDC_SELECT_DEFAULT:
		{
			if (loadMacFile(MainWnd.m_macroPath)) {
				_tcscpy(m_path, MainWnd.m_macroPath);
				Static_SetText(m_pathCtrl, m_path);
			}
			else MessageBox(m_hWnd, _T("Failed to load the specified macro file"), 
							_T("Error"), MB_ICONERROR);
		}
		break;
	}

	return CMultiLangDialog::onCommand(wParam, lParam);
}


//-----------------------------------------------
void CMacroDlg::onItemSelChange()
{
	int sel = ListBox_GetCurSel(m_itemListCtrl);
	if (sel != LB_ERR) {
		TCHAR str[MAX_MACRO_LINE];
		ListBox_GetText(m_itemListCtrl, sel, str);
		SetWindowText(m_keyEdit.m_hWnd, getKey(str));
		SetWindowText(m_textCtrl, getReplText(str));
		SetFocus(m_keyEdit.m_hWnd);
		Edit_SetSel(m_keyEdit.m_hWnd, 0, -1);
		setReplaceBtn(1);
		EnableWindow(m_editBtn.m_hWnd, FALSE);
		EnableWindow(m_deleteBtn.m_hWnd, TRUE);
	}
}

//-----------------------------------------------
void CMacroDlg::setReplaceBtn(int replace)
{
	m_isEditReplace = replace;
	char *text = (replace)? ReplaceLabel[m_langID] : AddLabel[m_langID];

#ifndef _UNICODE
	Button_SetText(m_editBtn.m_hWnd, text);
#else
	wchar_t str[MAX_ITEM_LEN];
	int inLen, maxOutLen;
	inLen = -1;
	maxOutLen = sizeof(str);
	VnConvert(CONV_CHARSET_TCVN3, CONV_CHARSET_UNICODE,
		(BYTE *)text, (BYTE *)str,
		inLen, maxOutLen);
	Button_SetText(m_editBtn.m_hWnd, str);
#endif

}


//-----------------------------------------------
void CMacroDlg::onDeleteItem()
{
	TCHAR key[MAX_MACRO_KEY_LEN+1];
	TCHAR text[MAX_MACRO_TEXT_LEN];

	Edit_GetText(m_textCtrl, text, MAX_MACRO_TEXT_LEN);
	Edit_GetText(m_keyEdit.m_hWnd, key, MAX_MACRO_KEY_LEN);
//	_tcscat(key, _TEXT("\t"));

	int exactSel, nearest;
	exactSel = getExactItem(m_itemListCtrl, key, nearest);
	if (exactSel != LB_ERR) {
		ListBox_DeleteString(m_itemListCtrl, exactSel);
		EnableWindow(m_deleteBtn.m_hWnd, FALSE);
		setReplaceBtn(0); // change to add mode
		EnableWindow(m_editBtn.m_hWnd, *text != NULL);
		SetFocus(m_keyEdit.m_hWnd);
		Edit_SetSel(m_keyEdit.m_hWnd, 0, -1);
	}
}

//-----------------------------------------------
void CMacroDlg::onEditItem()
{
	TCHAR key[MAX_MACRO_KEY_LEN+1], text[MAX_MACRO_TEXT_LEN];
	TCHAR str[MAX_MACRO_LINE+1];


	Edit_GetText(m_keyEdit.m_hWnd, key, MAX_MACRO_KEY_LEN);
	Edit_GetText(m_textCtrl, text, MAX_MACRO_TEXT_LEN);
	_stprintf(str, _TEXT("%s\t%s"), key, text);

	int sel, exactSel;
	exactSel = getExactItem(m_itemListCtrl, key, sel);
	if (exactSel != LB_ERR)
		ListBox_DeleteString(m_itemListCtrl, exactSel);

	ListBox_SetCurSel(m_itemListCtrl, ListBox_AddString(m_itemListCtrl, str));
	// update control states
	setReplaceBtn(1);
	EnableWindow(m_editBtn.m_hWnd, FALSE);
	EnableWindow(m_deleteBtn.m_hWnd, TRUE);
	SetFocus(m_keyEdit.m_hWnd);
	Edit_SetSel(m_keyEdit.m_hWnd, 0, -1);
}

//-----------------------------------------------
void CMacroDlg::onTextChange()
{
	TCHAR key[MAX_MACRO_KEY_LEN];
	TCHAR text[MAX_MACRO_TEXT_LEN];

	Edit_GetText(m_textCtrl, text, MAX_MACRO_TEXT_LEN);
	Edit_GetText(m_keyEdit.m_hWnd, key, MAX_MACRO_KEY_LEN);

	EnableWindow(m_editBtn.m_hWnd, *text != NULL && *key != NULL);
}

//-----------------------------------------------
void CMacroDlg::onKeyChange()
{
	TCHAR key[MAX_MACRO_KEY_LEN+1];
	TCHAR text[MAX_MACRO_TEXT_LEN];

	Edit_GetText(m_textCtrl, text, MAX_MACRO_TEXT_LEN);
	Edit_GetText(m_keyEdit.m_hWnd, key, MAX_MACRO_KEY_LEN);

	int sel;
	int exactSel = LB_ERR;

	if (*key != NULL) {
		exactSel = getExactItem(m_itemListCtrl, key, sel);
		ListBox_SetCurSel(m_itemListCtrl, 
			(exactSel != LB_ERR)? exactSel : ((sel != LB_ERR)? sel : -1));
	}
	else ListBox_SetCurSel(m_itemListCtrl, -1);

	if (exactSel != LB_ERR) {
		setReplaceBtn(1);
		EnableWindow(m_editBtn.m_hWnd, *text != NULL);
		EnableWindow(m_deleteBtn.m_hWnd, TRUE);
	}
	else {
		setReplaceBtn(0);
		EnableWindow(m_editBtn.m_hWnd, *key!=NULL && *text!=NULL);
		EnableWindow(m_deleteBtn.m_hWnd, FALSE);
	}
}

//-----------------------------------------------
const TCHAR *getKey(const TCHAR *text)
{
	static TCHAR str[MAX_MACRO_LINE];
	_tcscpy(str, text);
	TCHAR *p = _tcschr(str, _TEXT('\t'));
	if (p != NULL)
		*p = NULL;
	return str;
}

//-----------------------------------------------
const TCHAR *getReplText(const TCHAR *text)
{
	static TCHAR str[MAX_MACRO_LINE];
	_tcscpy(str, text);
	TCHAR *p = _tcschr(str, _TEXT('\t'));
	if (p == NULL)
		return NULL;
	return (p+1);
}



//-------------------------------------------------------
LRESULT CMacroEdit::wndProc(
				HWND hWnd,      // handle to window
				UINT uMsg,      // message identifier
				WPARAM wParam,  // first message parameter
				LPARAM lParam)  // second message parameter
{
	switch (uMsg) {
	case WM_KEYDOWN:
		if (wParam == VK_DOWN || wParam == VK_UP || wParam == VK_PRIOR || wParam == VK_NEXT) {
			SendMessage(m_hOwner, WM_KEYDOWN, wParam, lParam);
			return 0;
		}
	/*
	case WM_CHAR:
		{
			if (wParam > 255) {
				TCHAR msg[100];
				_stprintf(msg, _T("wParam: %d"), wParam);
				MessageBox(NULL, msg, _T("Info"), MB_OK);
			}
			else if (wParam == 0xFD)
				MessageBox(NULL, _T("Something is wrong"), _T("Info"), MB_OK);
		}
		break;
	*/

	}
	return CSmartEdit::wndProc(hWnd, uMsg, wParam, lParam);
}

//-------------------------------------------------------
int getExactItem(HWND listCtrl, TCHAR *key, int & nearest)
{
	nearest	= ListBox_FindString(listCtrl, -1, key);
	if (nearest == LB_ERR)
		return LB_ERR;

	TCHAR itemStr[MAX_MACRO_LINE];
	const TCHAR *pItemKey;
	int exactFound = 0;
	int i = nearest;
	int count = ListBox_GetCount(listCtrl);
	while (i < count) {
		ListBox_GetText(listCtrl, i, itemStr);
		pItemKey = getKey(itemStr);
		if (_tcsicmp(key, pItemKey) != 0)
			break;
		if (_tcscmp(key, pItemKey) == 0) {// exact found
			exactFound = 1;
			break;
		}
		i++;
	}
	return exactFound? i : LB_ERR;
}
