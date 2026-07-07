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
#include <windowsx.h>
#include <stdio.h>
#include "resource.h"
#include "tooldlg.h"
#include "mainwnd.h"
#include "vnconv.h"
#include "button.h"
#include "util.h"
#include "intrtfio.h"

extern CUserPref UserPref;

ConvCharset InConvCharsetList[] = {
	{_TEXT("TCVN3 (ABC)"), CONV_CHARSET_TCVN3},
	{_TEXT("Unicode"), CONV_CHARSET_UNIDECOMPOSED},
	{_TEXT("VNI Windows"), CONV_CHARSET_VNIWIN},
	{_TEXT("Windows CP 1258"), CONV_CHARSET_WINCP1258},
	{_TEXT("VIQR"), CONV_CHARSET_VIQR},
	{_TEXT("VISCII"), CONV_CHARSET_VISCII},
	{_TEXT("VPS"), CONV_CHARSET_VPS},
	{_TEXT("BK HCM 2"), CONV_CHARSET_BKHCM2},
	{_TEXT("BK HCM 1"),	CONV_CHARSET_BKHCM1},
	{_TEXT("Vietware X"), CONV_CHARSET_VIETWAREX},
	{_TEXT("Vietware F"), CONV_CHARSET_VIETWAREF},
	{_TEXT("UTF-8"), CONV_CHARSET_UNIUTF8},
	{_TEXT("NCR Decimal"), CONV_CHARSET_UNIREF},
	{_TEXT("NCR Hex"), CONV_CHARSET_UNIREF_HEX},
	{_TEXT("Unicode C string"), CONV_CHARSET_UNI_CSTRING}
};

const unsigned int InCharsetCount = sizeof(InConvCharsetList)/sizeof(ConvCharset);

ConvCharset OutConvCharsetList[] = {
	{_TEXT("TCVN3 (ABC)"), CONV_CHARSET_TCVN3},
	{_TEXT("Unicode"), CONV_CHARSET_UNICODE},
	{_TEXT("VNI Windows"), CONV_CHARSET_VNIWIN},
	{_TEXT("Unicode (Composite - To hop)"), CONV_CHARSET_UNIDECOMPOSED},
	{_TEXT("Windows CP 1258"), CONV_CHARSET_WINCP1258},
	{_TEXT("VIQR"), CONV_CHARSET_VIQR},
	{_TEXT("VISCII"), CONV_CHARSET_VISCII},
	{_TEXT("VPS"), CONV_CHARSET_VPS},
	{_TEXT("BK HCM 2"), CONV_CHARSET_BKHCM2},
	{_TEXT("BK HCM 1"),	CONV_CHARSET_BKHCM1},
	{_TEXT("Vietware X"), CONV_CHARSET_VIETWAREX},
	{_TEXT("Vietware F"), CONV_CHARSET_VIETWAREF},
	{_TEXT("UTF-8"), CONV_CHARSET_UNIUTF8},
	{_TEXT("NCR Decimal"), CONV_CHARSET_UNIREF},
	{_TEXT("NCR Hex"), CONV_CHARSET_UNIREF_HEX},
	{_TEXT("Unicode C string"), CONV_CHARSET_UNI_CSTRING}
};

const unsigned int OutCharsetCount = sizeof(OutConvCharsetList)/sizeof(ConvCharset);

AnsiDlgItemInfo VnToolDlgItems[] = {
	{IDC_STATIC_CHARSET,		"B¶ng m·"}, //"Nguån"},
	{IDC_STATIC_OPTION,         "Lùa chän"},
	{IDC_STATIC_FILE,			"File"}, //"§Ých"},
	{IDC_STATIC_IN_CHARSET,		"Nguån"},
	{IDC_STATIC_OUT_CHARSET,	"§Ých"},
	{IDC_STATIC_IN_FILE,		"File nguån"},
	{IDC_STATIC_OUT_FILE,		"File ®Ých"},
	{IDC_CLIPBOARD,				"ChuyÓn m· clipboard"},
	{IDC_CONVERT,				"ChuyÓn m·"},
	{IDC_SWAP_CHARSET,          "§¶o b¶ng m·"},
	{IDC_TO_LOWER,				"Sang ch÷ th­êng"},
	{IDC_TO_UPPER,				"Sang ch÷ hoa"},
	{IDC_REMOVE_TONE,			"Lo¹i bá dÊu"},
	{IDC_NO_RICH_TEXT,          "Kh«ng dïng rich text"},
	{IDC_MINIMAL_FONTSET,       "Dïng bé font tèi thiÓu"},
	{IDC_FILE_NOTE,             "ChØ hç trî file text vµ file Rich Text Format (RTF)"},
	{IDOK,						"§ãng"}
	};

DlgItemInfo EnToolDlgItems[] = {
	{IDC_STATIC_CHARSET,		_T("Character set")},
	{IDC_STATIC_OPTION,         _T("Options")},
	{IDC_STATIC_FILE,			_T("File")},
	{IDC_STATIC_IN_CHARSET,		_T("Source")},
	{IDC_STATIC_OUT_CHARSET,	_T("Destination")},
	{IDC_STATIC_IN_FILE,		_T("Input")},
	{IDC_STATIC_OUT_FILE,		_T("Output")},
	{IDC_CONVERT,				_T("Convert")},
	{IDC_SWAP_CHARSET,          _T("Swap charset")},
	{IDC_TO_LOWER,				_T("To lower case")},
	{IDC_TO_UPPER,				_T("To upper case")},
	{IDC_REMOVE_TONE,			_T("Remove tone marks")},
	{IDC_NO_RICH_TEXT,          _T("Do not use rich text")},
	{IDC_MINIMAL_FONTSET,       _T("Use minimal font set")},
	{IDC_FILE_NOTE,             _T("Only plain text and Rich Text Format are supported")},
	{IDOK,						_T("Close")}
	};

#define MAX_ITEM_LEN 60

#ifdef _UNICODE

wchar_t UniVnToolDlgTexts[sizeof(VnToolDlgItems)/sizeof(AnsiDlgItemInfo)][MAX_ITEM_LEN];
DlgItemInfo UniVnToolDlgItems[sizeof(VnToolDlgItems)/sizeof(AnsiDlgItemInfo)];

#endif


int inputFileDlg(HWND hWndOwner, TCHAR *fileName, int size);
int outFileDlg(HWND hWndOwner, TCHAR *fileName, int size);
int guessOutSize(int inCharset, int outCharset, int inSize);

extern CMainWnd MainWnd;

HANDLE hRtfClipboard = NULL;

//------------------------------------------
int CToolkitDlg::init(HINSTANCE hInst, HWND hWndOwner)
{
	int ret = CMultiLangDialog::init(hInst, MAKEINTRESOURCE(IDD_TOOLKIT), hWndOwner);
	initLanguageInfo();

	setupLanguage(0, EnToolDlgItems, sizeof(EnToolDlgItems)/sizeof(DlgItemInfo));

#ifndef _UNICODE
	setupLanguage(1, VnToolDlgItems, sizeof(VnToolDlgItems)/sizeof(AnsiDlgItemInfo),
		          VIET_FONT_NAME, VIET_DLG_FONT_SIZE);
#else
	int items = sizeof(VnToolDlgItems)/sizeof(AnsiDlgItemInfo);
	int inLen, maxOutLen;
	for (int i=0; i<items; i++) {
		inLen = -1;
		maxOutLen = 2*MAX_ITEM_LEN;
		VnConvert(CONV_CHARSET_TCVN3, CONV_CHARSET_UNICODE,
				(BYTE *)VnToolDlgItems[i].text, (BYTE *)UniVnToolDlgTexts[i],
				inLen, maxOutLen);
		UniVnToolDlgItems[i].id = VnToolDlgItems[i].id;
		UniVnToolDlgItems[i].text = UniVnToolDlgTexts[i];
	}
	setupLanguage(1, UniVnToolDlgItems, items, VIET_FONT_NAME, VIET_DLG_FONT_SIZE);
#endif
	return ret;
}

//------------------------------------------
void CToolkitDlg::loadDlgInfo()
{
	unsigned int i;
	for (i=0; i < InCharsetCount; i++) {
		SendMessage(m_inCharsetCtrl, CB_ADDSTRING, 0, (LPARAM)InConvCharsetList[i].name);
	}

	SendMessage(m_inCharsetCtrl, CB_SETCURSEL, 
		(m_pPref->m_inConvCharset < 0 || 
		 m_pPref->m_inConvCharset >= InCharsetCount) ? 0 : m_pPref->m_inConvCharset,
		0);


	for (i=0; i < OutCharsetCount; i++) {
		SendMessage(m_outCharsetCtrl, CB_ADDSTRING, 0, (LPARAM)OutConvCharsetList[i].name);
	}

	SendMessage(m_outCharsetCtrl, CB_SETCURSEL, 
		(m_pPref->m_outConvCharset < 0 || 
		 m_pPref->m_outConvCharset >= OutCharsetCount) ? 0 : m_pPref->m_outConvCharset,
		0);

	SendMessage(m_clipboardCtrl, BM_SETCHECK, 
		        m_pPref->m_clipboardConvert ? BST_CHECKED : BST_UNCHECKED, 0);
	EnableWindow(m_inFileCtrl, !m_pPref->m_clipboardConvert);
	EnableWindow(m_inBrowseBtn.m_hWnd, !m_pPref->m_clipboardConvert);
	EnableWindow(m_outFileCtrl, !m_pPref->m_clipboardConvert);
	EnableWindow(m_outBrowseBtn.m_hWnd, !m_pPref->m_clipboardConvert);
	EnableWindow(m_minimalFontCheck, !m_pPref->m_noRichText);

	Button_SetCheck(m_toLowerCheck, m_pPref->m_toLower);
	Button_SetCheck(m_toUpperCheck, m_pPref->m_toUpper && !m_pPref->m_toLower);
	Button_SetCheck(m_removeToneCheck, m_pPref->m_removeTone);
	Button_SetCheck(m_noRichTextCheck, m_pPref->m_noRichText);
	Button_SetCheck(m_minimalFontCheck, m_pPref->m_minimalFontset);
}


//------------------------------------------
void CToolkitDlg::storeDlgInfo()
{
	m_pPref->m_inConvCharset = SendMessage(m_inCharsetCtrl, CB_GETCURSEL, 0, 0);
	m_pPref->m_outConvCharset = SendMessage(m_outCharsetCtrl, CB_GETCURSEL, 0, 0);
	m_pPref->m_clipboardConvert = (SendMessage(m_clipboardCtrl, BM_GETCHECK, 0, 0) == BST_CHECKED)? 1 : 0;

	m_pPref->m_toLower = Button_GetCheck(m_toLowerCheck);
	m_pPref->m_toUpper = Button_GetCheck(m_toUpperCheck);
	m_pPref->m_removeTone = Button_GetCheck(m_removeToneCheck);
	m_pPref->m_noRichText = Button_GetCheck(m_noRichTextCheck);
	m_pPref->m_minimalFontset = Button_GetCheck(m_minimalFontCheck);
}

//------------------------------------------
BOOL CToolkitDlg::onInitDialog()
{
	if (!CMultiLangDialog::onInitDialog())	
		return FALSE;

	if (MainWnd.isVietFontOK())
		setDlgFont(1);
	applyLanguage(m_pPref->m_vietGUI? 1 : 0);

	m_inBrowseBtn.init(m_hInst, m_hWnd, GetDlgItem(m_hWnd, IDC_IN_BROWSE));
	m_inBrowseBtn.setIcon(IDI_OPEN_FOLDER, 16, 16);

	m_outBrowseBtn.init(m_hInst, m_hWnd, GetDlgItem(m_hWnd, IDC_OUT_BROWSE));
	m_outBrowseBtn.setIcon(IDI_OPEN_FOLDER, 16, 16);

	m_swapBtn.init(m_hInst, m_hWnd, GetDlgItem(m_hWnd, IDC_SWAP_CHARSET));

	m_convertBtn.init(m_hInst, m_hWnd, GetDlgItem(m_hWnd, IDC_CONVERT));
	m_convertBtn.setIcon(IDI_CONVERT, 16, 16);

	m_closeBtn.init(m_hInst, m_hWnd, GetDlgItem(m_hWnd, IDOK));
	m_closeBtn.setIcon(IDI_OK, 16, 16);
	SendMessage(m_hWnd, DM_SETDEFID, IDOK, 0);
	m_closeBtn.setDefaultButton(TRUE);

	m_inCharsetCtrl = GetDlgItem(m_hWnd, IDC_IN_CHARSET);
	m_outCharsetCtrl = GetDlgItem(m_hWnd, IDC_OUT_CHARSET);
	m_inFileCtrl = GetDlgItem(m_hWnd, IDC_IN_FILE);
	m_outFileCtrl = GetDlgItem(m_hWnd, IDC_OUT_FILE);
	m_clipboardCtrl = GetDlgItem(m_hWnd, IDC_CLIPBOARD);
	m_toLowerCheck = GetDlgItem(m_hWnd, IDC_TO_LOWER);
	m_toUpperCheck = GetDlgItem(m_hWnd, IDC_TO_UPPER);
	m_removeToneCheck = GetDlgItem(m_hWnd, IDC_REMOVE_TONE);
	m_noRichTextCheck = GetDlgItem(m_hWnd, IDC_NO_RICH_TEXT);
	m_minimalFontCheck = GetDlgItem(m_hWnd, IDC_MINIMAL_FONTSET);

	loadDlgInfo();
	return TRUE;
}

//------------------------------------------
BOOL CToolkitDlg::onCommand(WPARAM wParam, LPARAM lParam)
{
	TCHAR fname[MAX_PATH];
	WORD cmd = GET_WM_COMMAND_ID(wParam, lParam);
	switch (cmd) {
	case IDC_IN_BROWSE:
		if (inputFileDlg(m_hWnd, fname, sizeof(fname)/sizeof(TCHAR))) {
			SendMessage(m_inFileCtrl, WM_SETTEXT, 0, (LPARAM)fname);

		}
		return TRUE;
	case IDC_OUT_BROWSE:
		if (outFileDlg(m_hWnd, fname, sizeof(fname)/sizeof(TCHAR))) {
			SendMessage(m_outFileCtrl, WM_SETTEXT, 0, (LPARAM)fname);
		}
		return TRUE;
	case IDC_CONVERT:
		onConvert();
		return TRUE;
	case IDC_CLIPBOARD:
		if (GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED) {
			int useClipboard = (SendMessage(m_clipboardCtrl, BM_GETCHECK, 0, 0) == BST_CHECKED)? 1 : 0;
			EnableWindow(m_inFileCtrl, !useClipboard);
			EnableWindow(m_inBrowseBtn.m_hWnd, !useClipboard);
			EnableWindow(m_outFileCtrl, !useClipboard);
			EnableWindow(m_outBrowseBtn.m_hWnd, !useClipboard);
		}
		return TRUE;
	case IDC_TO_LOWER:
		if (Button_GetCheck(m_toLowerCheck))
			Button_SetCheck(m_toUpperCheck, 0);
		break;
	case IDC_TO_UPPER:
		if (Button_GetCheck(m_toUpperCheck))
			Button_SetCheck(m_toLowerCheck, 0);
		break;
	case IDC_NO_RICH_TEXT:
		EnableWindow(m_minimalFontCheck, !Button_GetCheck(m_noRichTextCheck));
		break;
	case IDC_SWAP_CHARSET:
		onSwapCharset();
		break;
	}
	return CMultiLangDialog::onCommand(wParam, lParam);
}

//------------------------------------------
void CToolkitDlg::onSwapCharset()
{
	int inSel = ComboBox_GetCurSel(m_inCharsetCtrl);
	int outSel = ComboBox_GetCurSel(m_outCharsetCtrl);

	if (inSel != CB_ERR && outSel != CB_ERR) {
		int newIn, newOut, i, outId, inId;

		outId = OutConvCharsetList[outSel].id;
		if (outId == CONV_CHARSET_UNICODE)
			outId = CONV_CHARSET_UNIDECOMPOSED;

		newIn = 0;
		for (i=0; i < InCharsetCount; i++) {
			if (InConvCharsetList[i].id == outId) {
				newIn = i;
				break;
			}
		}

		inId = InConvCharsetList[inSel].id;
		if (inId == CONV_CHARSET_UNIDECOMPOSED)
			inId = CONV_CHARSET_UNICODE;

		newOut = 0;
		for (i=0; i < OutCharsetCount; i++) {
			if (OutConvCharsetList[i].id == inId) {
				newOut = i;
				break;
			}
		}
			
		ComboBox_SetCurSel(m_inCharsetCtrl, newIn);
		ComboBox_SetCurSel(m_outCharsetCtrl, newOut);
	}
}

//------------------------------------------
int inputFileDlg(HWND hWndOwner, TCHAR *fileName, int size)
{
	OPENFILENAME of;
	ZeroMemory(&of, sizeof(OPENFILENAME));

	*fileName = 0;
	of.lStructSize = sizeof(OPENFILENAME);
	of.hwndOwner = hWndOwner;
	of.lpstrFile = fileName;
	of.nMaxFile = size;
	of.lpstrFilter = _TEXT("Plain text (*.txt)\0*.txt\0Rich Text Format (*.rtf)\0*.rtf\0All files (*.*)\0*.*\0");
	of.nFilterIndex = 1;
	of.lpstrFileTitle = NULL;
	of.nMaxFileTitle = 0;
	of.lpstrInitialDir = NULL;
	of.lpstrTitle = _TEXT("Select input file");
	of.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

	return GetOpenFileName(&of);
}


//------------------------------------------
int outFileDlg(HWND hWndOwner, TCHAR *fileName, int size)
{
	OPENFILENAME of;
	ZeroMemory(&of, sizeof(OPENFILENAME));

	*fileName = 0;
	of.lStructSize = sizeof(OPENFILENAME);
	of.hwndOwner = hWndOwner;
	of.lpstrFile = fileName;
	of.nMaxFile = size;
	of.lpstrFilter = _TEXT("Plain text (*.txt)\0*.txt\0Rich Text Format (*.rtf)\0*.rtf\0All files (*.*)\0*.*\0");
	of.nFilterIndex = 1;
	of.lpstrFileTitle = NULL;
	of.nMaxFileTitle = 0;
	of.lpstrInitialDir = NULL;
	of.lpstrTitle = _TEXT("Select output file");
	of.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

	return GetSaveFileName(&of);
}

//------------------------------------------
void CToolkitDlg::onConvert()
{
	storeDlgInfo();

	// Get charsets
	int inCharset = SendMessage(m_inCharsetCtrl, CB_GETCURSEL, 0, 0);
	int outCharset = SendMessage(m_outCharsetCtrl, CB_GETCURSEL, 0, 0);

	if (inCharset == CB_ERR || outCharset == CB_ERR) {
		MessageBox(m_hWnd, 
			       _TEXT("You must specify input and output charsets"), 
				   _TEXT("Invalid operation"), 
				   MB_ICONEXCLAMATION);
		return;
	}

	inCharset = InConvCharsetList[inCharset].id;
	outCharset = OutConvCharsetList[outCharset].id;

	// Check if clipboard is selected
	int useClipboard = (SendDlgItemMessage(m_hWnd, IDC_CLIPBOARD, BM_GETCHECK, 0, 0) == BST_CHECKED)? 1 : 0;

	if (useClipboard) {
		int errCode;
		ConvertClipboard(inCharset, outCharset, errCode, 1, m_hWnd);
		return;
	}

	// File convert
	TCHAR inFile[MAX_PATH], outFile[MAX_PATH];
	SendMessage(m_inFileCtrl, WM_GETTEXT, sizeof(inFile)/sizeof(TCHAR), (LPARAM)inFile);
	if (inFile[0] == NULL) {
		MessageBox(m_hWnd, 
			       _TEXT("You must provide an input file"), 
			       _TEXT("Invalid operation"), 
				   MB_ICONEXCLAMATION);
		SetFocus(m_inFileCtrl);
		return;
	}
	SendMessage(m_outFileCtrl, WM_GETTEXT, sizeof(outFile)/sizeof(TCHAR), (LPARAM)outFile);
	if (outFile[0] == NULL) {
		MessageBox(m_hWnd, 
			       _TEXT("You must provide an output file"), 
				   _TEXT("Invalid operation"), 
				   MB_ICONEXCLAMATION);
		SetFocus(m_outFileCtrl);
		return;
	}

	//set conversion options
	VnConvOptions opt, oldOpt;
	VnConvGetOptions(&oldOpt);
	VnConvResetOptions(&opt);
	opt.toLower = m_pPref->m_toLower;
	opt.toUpper = m_pPref->m_toUpper;
	opt.removeTone = m_pPref->m_removeTone;
	VnConvSetOptions(&opt);

	TCHAR ext[_MAX_EXT];
	_tsplitpath(inFile, NULL, NULL, NULL, ext);

	int ret;
#ifndef _UNICODE
	if (_tcsicmp(ext, _T(".RTF")) == 0)
		ret = RtfFileConvert(inCharset, outCharset, inFile, outFile, m_pPref->m_minimalFontset);
	else 
		ret = VnFileConvert(inCharset, outCharset, inFile, outFile);
#else
	char inFileAnsi[MAX_PATH], outFileAnsi[MAX_PATH];
	WideCharToMultiByte(CP_US_ANSI, 0, inFile, -1, inFileAnsi, MAX_PATH, NULL, NULL);
	WideCharToMultiByte(CP_US_ANSI, 0, outFile, -1, outFileAnsi, MAX_PATH, NULL, NULL);
	if (_tcsicmp(ext, _T(".RTF")) == 0)
		ret = RtfFileConvert(inCharset, outCharset, inFileAnsi, outFileAnsi, m_pPref->m_minimalFontset);
	else
		ret = VnFileConvert(inCharset, outCharset, inFileAnsi, outFileAnsi);
#endif
	VnConvSetOptions(&oldOpt);

	if (ret == 0) {
		MessageBox(m_hWnd, 
		           _TEXT("Successfully converted"), 
				   _TEXT("Result"), 
				   MB_ICONINFORMATION);
//		SendMessage(m_outFileCtrl, WM_SETTEXT, 0, (LPARAM)"");
	}
	else {
		TCHAR msg[200];
#ifndef _UNICODE
		_stprintf(msg, _TEXT("Error[%d]: %s"), ret, VnConvErrMsg(ret));
#else
		_stprintf(msg, _TEXT("Error[%d]"), ret);
#endif
		MessageBox(m_hWnd, msg, _TEXT("Result"), MB_ICONEXCLAMATION);
	}
}


//-----------------------------------------------
BOOL CToolkitDlg::onOK()
{
	HWND hFocusWnd = GetFocus();
	if (IsPushButton(hFocusWnd)) {
		int id = GetDlgCtrlID(hFocusWnd);
		if (id != IDOK) {
			SendMessage(hFocusWnd, BM_CLICK, 0, 0);
			return FALSE;
		}
	}
	storeDlgInfo();
	return TRUE;
}

//--------------------------------------------------
BOOL CToolkitDlg::DialogProc(
						HWND hDlg,
						UINT uMsg,
						WPARAM wParam,
						LPARAM lParam)
{
	switch (uMsg) {
	case WM_DRAWITEM:
		// Send reflection message to back the control
		return SendDlgItemMessage(hDlg, wParam, WM_DRAWITEM, wParam, lParam);
	}
	return CMultiLangDialog::DialogProc(hDlg, uMsg, wParam, lParam);
}

//----------------------------------------------
BOOL CToolkitDlg::onDestroy()
{
	cleanup();
	return CVirDialog::onDestroy();
}


struct ConvSignature
{
	WORD terminator;
	int charSet;
	WORD signature;
};

#define VNCONV_SIGNATURE 0xF1E3
//#define DEFAULT_RTF_BUF_SIZE (256*1024) //256 KB

//--------------------------------------------------------------
// Returns 1 if RTF clipboard is available.
// The clipboard will be converted if available
//--------------------------------------------------------------
int convertRTF(int inCharset, int outCharset, int & errCode, int promptInfo, HWND hOwnerWnd)
{
	HANDLE hTmp;
	HANDLE hRtfIn = NULL;
	BYTE * pRtfIn = NULL;

	HANDLE hRtfOut = NULL;
	BYTE * pRtfOut = NULL;

	int inSize, outSize, inConvLen, outConvLen;
	int ret = 0;

	errCode = 0;

	UINT cfRtf = RegisterClipboardFormat(_T("Rich Text Format"));


	if (cfRtf==0 || !IsClipboardFormatAvailable(cfRtf))
		return 0;

	hRtfIn = GetClipboardData(cfRtf);
	inSize = GlobalSize(hRtfIn);

	pRtfIn = (BYTE *)GlobalLock(hRtfIn);
	if (pRtfIn == NULL)	goto end;

//	outSize = DEFAULT_RTF_BUF_SIZE;
	outSize = 2*inSize; // just guess that we may need at most 2*inSize

	hRtfOut = GlobalAlloc(GMEM_MOVEABLE, outSize);
	if (hRtfOut == NULL) {
		if (promptInfo)
			MessageBox(hOwnerWnd, _T("Not enough memory"), _T("Error"), MB_ICONERROR);
		goto end;
	}

	pRtfOut = (BYTE *)GlobalLock(hRtfOut);
	if (pRtfOut == NULL) goto end;

	inConvLen = inSize;
	outConvLen = outSize;
	errCode = RtfMemConvert(inCharset, outCharset, pRtfIn, pRtfOut, 
		                    inConvLen, outConvLen,
							UserPref.m_minimalFontset);

	if (errCode == VNCONV_ERR_WRITING) {
		//not enough memory. Allocate more
		outSize = outConvLen;
		GlobalUnlock(hRtfOut);
		pRtfOut = NULL;
		hTmp = GlobalReAlloc(hRtfOut, outSize, GMEM_MOVEABLE);
		if (hTmp == NULL) {
			if (promptInfo)
				MessageBox(hOwnerWnd, _T("Not enough memory"), _T("Error"), MB_ICONERROR);
			goto end;
		}
		hRtfOut = hTmp;

		pRtfOut = (BYTE *)GlobalLock(hRtfOut);
		if (pRtfOut == NULL)
			goto end;

		inConvLen = inSize;
		errCode = RtfMemConvert(inCharset, outCharset, pRtfIn, pRtfOut, 
			                    inConvLen, outConvLen,
								UserPref.m_minimalFontset);
	}

	if (errCode != 0) {
		if (promptInfo)
			MessageBox(hOwnerWnd, 
			           _T("Could not convert RTF clipboard. Possibly not enough memory"), 
			           _T("Information"), MB_ICONERROR);
		goto end;
	}

	GlobalUnlock(hRtfIn);
	GlobalUnlock(hRtfOut);
	pRtfIn = NULL;
	pRtfOut = NULL;

	if (outSize > outConvLen) {
		//reallocate
		outSize = outConvLen;
		hTmp = GlobalReAlloc(hRtfOut, outSize, GMEM_MOVEABLE);
		if (hTmp == NULL) goto end;
		hRtfOut = hTmp;
	}

	//Set clipboard
	EmptyClipboard();
	SetClipboardData(cfRtf, hRtfOut);
	SetClipboardData(CF_TEXT, NULL); //delay rendering ansi text
	SetClipboardData(CF_UNICODETEXT, NULL); //delay rendering unicode text
	hRtfClipboard = hRtfOut; // for later conversion from RTF to text as needed

	hRtfOut = NULL;
	if (promptInfo)
		MessageBox(hOwnerWnd, _T("Successfully converted RTF clipboard"), _T("Result"), MB_ICONINFORMATION);
	ret = 1;

end:
	if (pRtfIn)
		GlobalUnlock(hRtfIn);
	if (pRtfOut)
		GlobalUnlock(hRtfOut);
	if (hRtfOut)
		GlobalFree(hRtfOut);
	return ret;
}

int getRTFText()
{
	UINT cfRTF = RegisterClipboardFormat(_T("Rich Text Format"));
	if (cfRTF && IsClipboardFormatAvailable(cfRTF)) {
		HANDLE hRTFData = GetClipboardData(cfRTF);
		int size = GlobalSize(hRTFData);
		LPVOID pRTFData = GlobalLock(hRTFData);
		HANDLE hTextData = GlobalAlloc(GMEM_MOVEABLE, size);
		LPVOID pTextData = GlobalLock(hTextData);
		CopyMemory(pTextData, pRTFData, size);
		GlobalUnlock(hRTFData);
		GlobalUnlock(hTextData);
		EmptyClipboard();
		SetClipboardData(CF_TEXT, hTextData);
		return 1;
	}
	return 0;

}


//--------------------------------------
int ConvertClipboard(int inCharset, int outCharset, int & convErrCode, int promptInfo, HWND hOwnerWnd)
{
	static lastFormat = CF_TEXT; // format of last converted clipboard (CF_TEXT or CF_UNICODETEXT)

	HANDLE hInData = NULL;
	HANDLE hOutData = NULL;
	HANDLE hAnsiData = NULL;
	HANDLE hTemp; //, hAnsiClip, hUniClip;

	BYTE *pInData = NULL;
	BYTE *pOutData = NULL;
	BYTE *pActualData = NULL;
	BYTE *pAnsiData = NULL;

	VnConvOptions opt, oldOpt;

	int inSize, outSize, ret, inLen;
	int lastConverted;

	ConvSignature sig;

	UINT inFormat, outFormat;
	int clipboardOpen = 0;
	TCHAR msg[200];
	
	ret = 0;
	convErrCode = 0;

	HWND hOldOwner = GetClipboardOwner();

	if (!OpenClipboard(MainWnd.m_hWnd)) {// use Main window as owner of the clipboard
		if (promptInfo)
			MessageBox(hOwnerWnd, 
			           _TEXT("Cannot open clipboard"), 
				       _TEXT("Error"), 
				       MB_ICONERROR);
		goto end;
	}
	clipboardOpen = 1;

	// Set conversions options
	VnConvGetOptions(&oldOpt);
	VnConvResetOptions(&opt);
	opt.toLower = UserPref.m_toLower;
	opt.toUpper = UserPref.m_toUpper;
	opt.removeTone = UserPref.m_removeTone;
	VnConvSetOptions(&opt);

	// Try to convert RTF first
	if (!UserPref.m_noRichText	&& convertRTF(inCharset, outCharset, convErrCode, promptInfo, hOwnerWnd)) {
		CloseClipboard();
		VnConvSetOptions(&oldOpt);
		return 1;
	}

/*
	if (getRTFText()) {
		CloseClipboard();
		MessageBox(NULL, _T("OK"), _T("Info"), MB_OK);
		return 1;
	}
*/
	///////
	// test if data was converted last time
	lastConverted = 0;
	if (hOldOwner == MainWnd.m_hWnd) {
		hInData = GetClipboardData(lastFormat);
		if (hInData != NULL) {
			inSize = GlobalSize(hInData);
			pInData = (BYTE *)GlobalLock(hInData);
			if (pInData && inSize >= sizeof(ConvSignature)) {
				sig = *(ConvSignature *)(pInData + inSize - sizeof(ConvSignature));
				if (sig.signature == VNCONV_SIGNATURE) {
					lastConverted = 1;
					inCharset = sig.charSet; // force the use of converted charset
				}
			}
			if (pInData && !lastConverted) {
				GlobalUnlock(hInData);
				pInData = NULL;
				hInData = NULL;
			}
		}
	}

	outFormat = (outCharset == CONV_CHARSET_UNICODE || outCharset == CONV_CHARSET_UNIDECOMPOSED)? 
			    CF_UNICODETEXT : CF_TEXT;

	if (!lastConverted) {
		if (IsClipboardFormatAvailable(CF_UNICODETEXT) &&
			(inCharset == CONV_CHARSET_UNICODE || 
			 inCharset == CONV_CHARSET_UNIDECOMPOSED|| 
			 inCharset == CONV_CHARSET_TCVN3 ||
			 inCharset == CONV_CHARSET_VISCII))
			inFormat = CF_UNICODETEXT;
		else
			inFormat = CF_TEXT;
		hInData = GetClipboardData(inFormat);
		if (hInData == NULL) {
			if (promptInfo)
				MessageBox(hOwnerWnd, 
					       _TEXT("Cannot read clipboard"), 
						   _TEXT("Error"), 
						   MB_ICONERROR);
			goto end;
		}
		inSize = GlobalSize(hInData);
		pInData = (BYTE *)GlobalLock(hInData);

		if (pInData && inCharset != CONV_CHARSET_UNICODE && inCharset != CONV_CHARSET_UNIDECOMPOSED &&
			inFormat == CF_UNICODETEXT) {
			inSize = inSize / 2;
			pActualData = new BYTE[inSize+1];
			WideCharToMultiByte(CP_US_ANSI, 0, (LPCWSTR)pInData, -1, (LPSTR)pActualData, inSize+1, 0, 0);
		}
		else pActualData = pInData;
	}
	else pActualData = pInData;


	if (pActualData != NULL && inSize > 0) {
		outSize = guessOutSize(inCharset, outCharset, inSize) + sizeof(ConvSignature);

		hOutData = GlobalAlloc(GMEM_MOVEABLE, outSize);
		if (hOutData == NULL) {
			if (promptInfo)
				MessageBox(hOwnerWnd, 
					       _TEXT("Could not allocate enough memory to do the conversion"), 
						   _TEXT("Error"), 
						   MB_ICONERROR);
			goto end;
		}

		pOutData = (BYTE *)GlobalLock(hOutData);
		if (pOutData == NULL) goto end;

		inLen = -1; // use null-terminated format
		convErrCode = VnConvert(inCharset, outCharset, pActualData, pOutData, inLen, outSize);

		if (convErrCode != 0) {
			if (promptInfo) {
#ifndef _UNICODE
				_stprintf(msg, _TEXT("Error[%d]: %s"), convErrCode, VnConvErrMsg(convErrCode));
#else
				_stprintf(msg, _TEXT("Error[%d]"), convErrCode);
#endif
				MessageBox(hOwnerWnd, msg, _TEXT("Result"), MB_ICONEXCLAMATION);
			}
			goto end;
		}

		// Add signature
		sig.signature = VNCONV_SIGNATURE;
		sig.terminator = 0;
		sig.charSet = outCharset;
		*(ConvSignature *)(pOutData+outSize) = sig;
		outSize += sizeof(ConvSignature);
		
		if (pActualData != pInData) {
			delete pActualData;
			pActualData = NULL;
		}

		///////////////////////////////////
		// Set clipboard data
		///////////////////////////////////
		GlobalUnlock(hInData);
		pInData = NULL;
		GlobalUnlock(hOutData);
		pOutData = NULL;

		hTemp = GlobalReAlloc(hOutData, outSize, GMEM_MOVEABLE);
		if (hTemp != NULL)
			hOutData = hTemp;

/*
		// convert CF_HTML clipboard if available
		int htmlErr;
		HANDLE hConvertedHtml = ConvertHtmlClipboard(inCharset, outCharset, htmlErr, hOwnerWnd);
*/

		// conversion finished, empty clipboard
		EmptyClipboard();

		// set new clipboards
		SetClipboardData(outFormat, hOutData); // text clipboard
		hOutData = NULL;  // don't want to free output data, it is owned by system
		hRtfClipboard = NULL;
/*
		if (hConvertedHtml) { //also HTML clipboard
			UINT cfHtml = RegisterClipboardFormat(_T("HTML Format"));
			if (!SetClipboardData(cfHtml, hConvertedHtml))
				GlobalFree(hConvertedHtml);
		}
*/
		
		// set CF_TEXT clipboard together with CF_UNICODETEXT
		if (outFormat == CF_UNICODETEXT) {
			char *dummy = "###";
			hAnsiData = GlobalAlloc(GMEM_MOVEABLE, strlen(dummy)+1);
			pAnsiData = (BYTE *)GlobalLock(hAnsiData);
			strcpy((char *)pAnsiData, dummy);
			GlobalUnlock(hAnsiData);
			pAnsiData = NULL;
			SetClipboardData(CF_TEXT, hAnsiData);
			hAnsiData = NULL; // avoid freeing this data, it is now owned by system
		}
		else if (outFormat == CF_TEXT && outCharset == CONV_CHARSET_WINCP1258) {
			HANDLE hTemp = GlobalAlloc(GMEM_MOVEABLE, sizeof(DWORD));
			DWORD *pTemp = (DWORD *)GlobalLock(hTemp);
			if (pTemp) {
				*pTemp = MAKELCID(0x042a, SORT_DEFAULT);
				GlobalUnlock(hTemp);
			}
			SetClipboardData(CF_LOCALE, hTemp); //(HANDLE)MAKELCID(0x042a, SORT_DEFAULT));
//			MessageBox(NULL, "Good", "Info", MB_OK);
		}

		if (promptInfo)
			MessageBox(hOwnerWnd, _TEXT("Successfully converted"), _TEXT("Result"), MB_ICONINFORMATION);
		lastFormat = outFormat;
		ret = 1;
	}

end:
	VnConvSetOptions(&oldOpt);
	if (pInData) {
		if (pActualData != NULL && pActualData != pInData)
			delete pActualData;
		GlobalUnlock(hInData);
	}
	if (pOutData) GlobalUnlock(hOutData);
	if (hOutData) GlobalFree(hOutData);
	if (hAnsiData) GlobalFree(hAnsiData);
	if (clipboardOpen)
		CloseClipboard();
	return ret;
}

//----------------------------
int guessOutSize(int inCharset, int outCharset, int inSize)
{
	int charCount;
	int outSize;

	if (inCharset == CONV_CHARSET_UNICODE || inCharset == CONV_CHARSET_UNIDECOMPOSED)
		charCount = (inSize / 2)+1;
	else
		charCount = inSize;

	if (inCharset == CONV_CHARSET_UNIREF)
		outSize = charCount * 8;
	if (IS_DOUBLE_BYTE_CHARSET(outCharset) || outCharset == CONV_CHARSET_UNICODE)
		outSize = charCount * 2;
	else if (outCharset == CONV_CHARSET_UNIDECOMPOSED)
		outSize = charCount * 4;
	else if (IS_SINGLE_BYTE_CHARSET(outCharset))
		outSize = charCount;
	else
		outSize = charCount * 3;
	return outSize;
}


//---------------------------------
int renderText(int unicode)
{
	HANDLE hRtfIn = NULL;
	BYTE * pRtfIn = NULL;
	HANDLE hOut = NULL;
	BYTE * pOut = NULL;

	int inSize, outSize, inMemLen, outMemLen;
	int ret = 0;

	int errCode = 0;
	if (hRtfClipboard == NULL)
		return 0;
	hRtfIn = hRtfClipboard;
	inSize = GlobalSize(hRtfIn);

	pRtfIn = (BYTE *)GlobalLock(hRtfIn);
	if (pRtfIn == NULL)	goto end;

	inMemLen = inSize;
	outMemLen = 0;
	errCode = RtfMemExtractText(pRtfIn, NULL, inMemLen, outMemLen, unicode);

	if (errCode == VNCONV_ERR_WRITING) {
		//not enough memory. Allocate more
		outSize = outMemLen;
		hOut = GlobalAlloc(GMEM_MOVEABLE, outSize);
		if (hOut == NULL) goto end;
		pOut = (BYTE *)GlobalLock(hOut);
		if (pOut == NULL) goto end;

		inMemLen = inSize;
		errCode = RtfMemExtractText(pRtfIn, pOut, inMemLen, outMemLen, unicode);
	}
	if (errCode != 0) 
		goto end;

	//put to text clipboard
	GlobalUnlock(hRtfIn);
	GlobalUnlock(hOut);
	pRtfIn = NULL;
	pOut = NULL;

	//Set clipboard
	SetClipboardData(unicode ? CF_UNICODETEXT : CF_TEXT, hOut);
	hOut = NULL;
	ret = 1;

end:
	if (pRtfIn)
		GlobalUnlock(hRtfIn);
	if (pOut)
		GlobalUnlock(hOut);
	if (hOut)
		GlobalFree(hOut);

	return ret;
}

//---------------------------------
int RenderFormat(UINT format)
{
	if (format == CF_TEXT)
		return renderText(0);
	if (format == CF_UNICODETEXT)
		return renderText(1);
	return 1;
}

//---------------------------------
int RenderAllFormats()
{
	renderText(0);
	renderText(1);
	return 1;
}

///////////////
/*
int testClipboard()
{
	UINT cfHtml = RegisterClipboardFormat(_T("HTML Format"));
	if (cfHtml && IsClipboardFormatAvailable(cfHtml)) {
		HANDLE hHtmlData = GetClipboardData(cfHtml);
		int size = GlobalSize(hHtmlData);
		LPVOID pHtmlData = GlobalLock(hHtmlData);
		HANDLE hTextData = GlobalAlloc(GMEM_MOVEABLE, size);
		LPVOID pTextData = GlobalLock(hTextData);
		CopyMemory(pTextData, pHtmlData, size);
		GlobalUnlock(hHtmlData);
		GlobalUnlock(hTextData);
		EmptyClipboard();
		SetClipboardData(CF_TEXT, hTextData);
		return 1;
	}
	return 0;
		
}

//------------------------------------------------------------------
// Convert from UTF-8 to ANSI
// Input:
//   utf8Buf: input utf-8 buffer
//   inSize: size of input in bytes, -1 if data is null-terminated
// Output:
//   hOutput: handle of the global output data
//   pOutput: pointer to global output data (hOutput is locked)
// Returns: size of output in bytes if success, 0 otherwise
//------------------------------------------------------------------
int fromUtf8ToAnsi(const char * utf8Buf, int inSize, HANDLE & hOutput, LPVOID & pOutput)
{
	HANDLE hWCharData = NULL;
	HANDLE hAnsiData = NULL;

	LPVOID pWCharData = NULL;
	LPVOID pAnsiData = NULL;

	int reqWChars, reqBytes;
	int rval = 0;
	
	// convert from UTF-8 to  UCS-2
	reqWChars = MultiByteToWideChar(CP_UTF8, 0, utf8Buf, inSize, NULL, 0);
	hWCharData =GlobalAlloc(GMEM_MOVEABLE, reqWChars * sizeof(wchar_t)); 
	pWCharData = GlobalLock(hWCharData);
	if (pWCharData == NULL) 
		goto end;

	if (!MultiByteToWideChar(CP_UTF8, 0, utf8Buf, inSize, (LPWSTR)pWCharData, reqWChars))
		goto end;

	// convert from UCS-2 to ANSI
	reqBytes = reqWChars * sizeof(wchar_t);
	hAnsiData = GlobalAlloc(GMEM_MOVEABLE, reqBytes);
	pAnsiData = GlobalLock(hAnsiData);
	if (pAnsiData == NULL) 
		goto end;
		
	if (!WideCharToMultiByte(CP_US_ANSI, 0, (LPCWSTR)pWCharData, reqWChars, (LPSTR)pAnsiData, reqBytes, NULL, NULL))
		goto end;
	pOutput = pAnsiData;
	hOutput = hAnsiData;
	hAnsiData = NULL;
	pAnsiData = NULL;
	rval = reqBytes;
end:
	if (pWCharData)
		GlobalUnlock(hWCharData);
	if (hWCharData)
		GlobalFree(hWCharData);
	if (pAnsiData)
		GlobalUnlock(hAnsiData);
	if (hAnsiData)
		GlobalFree(hAnsiData);

	return rval;
}

//------------------------------------------------------------------
// Convert from ANSI to UTF-8
// Input:
//   ansiBuf: input utf-8 buffer
//   inSize: size of input in bytes, -1 if data is null-terminated
// Output:
//   hOutput: handle of the global output data
//   pOutput: pointer to global output data (hOutput is locked)
// Returns: size of output in bytes if success, 0 otherwise
//------------------------------------------------------------------
int fromAnsiToUtf8(const char * ansiBuf, int inSize, HANDLE & hOutput, LPVOID & pOutput)
{
	HANDLE hWCharData = NULL;
	HANDLE hUtf8Data = NULL;

	LPVOID pWCharData = NULL;
	LPVOID pUtf8Data = NULL;

	int reqBytes, reqWChars;
	int rval = 0;
	
	// convert from ANSI to  UCS-2
	reqWChars = MultiByteToWideChar(CP_US_ANSI, 0, ansiBuf, inSize, NULL, 0);
	hWCharData =GlobalAlloc(GMEM_MOVEABLE, reqWChars * sizeof(wchar_t)); 
	pWCharData = GlobalLock(hWCharData);
	if (pWCharData == NULL) 
		goto end;
	if (!MultiByteToWideChar(CP_US_ANSI, 0, ansiBuf, inSize, (LPWSTR)pWCharData, reqWChars))
		goto end;

	// convert from UCS-2 to UTF-8
	reqBytes = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)pWCharData, reqWChars, NULL, 0, NULL, NULL);
	if (reqBytes == 0)
		goto end;
	hUtf8Data = GlobalAlloc(GMEM_MOVEABLE, reqBytes);
	pUtf8Data = GlobalLock(hUtf8Data);
	if (pUtf8Data == NULL) 
		goto end;
		
	if (!WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)pWCharData, reqWChars, (LPSTR)pUtf8Data, reqBytes, NULL, NULL))
		goto end;
	pOutput = pUtf8Data;
	hOutput = hUtf8Data;
	hUtf8Data = NULL;
	pUtf8Data = NULL;
	rval = reqBytes;
end:
	if (pWCharData)
		GlobalUnlock(hWCharData);
	if (hWCharData)
		GlobalFree(hWCharData);
	if (pUtf8Data)
		GlobalUnlock(hUtf8Data);
	if (hUtf8Data)
		GlobalFree(hUtf8Data);

	return rval;
}

//---------------------------------------
// Suppose: clipboard is already open
// Returns: handle to the converted HTML data (in CF_HTML format
//---------------------------------------
HANDLE ConvertHtmlClipboard(int inCharset, int outCharset, int & convErrCode, HWND hOwnerWnd)
{
	UINT cfHtml = RegisterClipboardFormat(_T("HTML Format"));

	if (!cfHtml && !IsClipboardFormatAvailable(cfHtml))
		return 0;

	HANDLE retHandle = NULL;

	HANDLE hInData = NULL;
	LPVOID pInData = NULL;

	HANDLE hOutData = NULL;
	LPVOID pOutData = NULL;

	int convertCharset;
	int inSize, outSize, inBufLen;
	int freeInput = 0;

	hInData = GetClipboardData(cfHtml);
	inSize = GlobalSize(hInData);
	pInData = GlobalLock(hInData);

	if (pInData == NULL)
		goto end;

	////////////////////////////////////////////////////////////////////
	//clipboard is in UTF-8, convert to ANSI if necessary
	////////////////////////////////////////////////////////////////////
	if (inCharset != CONV_CHARSET_UNICODE) {
		HANDLE hAnsiData = NULL;
		LPVOID pAnsiData = NULL;
		int outBytes = fromUtf8ToAnsi((const char *)pInData, inSize, hAnsiData, pAnsiData);
		if (outBytes == 0)
			goto end;
		// switch input data
		GlobalUnlock(hInData);
		hInData = hAnsiData;
		pInData = pAnsiData;
		inSize = outBytes;
		freeInput = 1;
	}
	else {
		inCharset = CONV_CHARSET_UNIUTF8;
	}

	///////////////////////////////////////
	// convert between Vietnamese charsets
	///////////////////////////////////////
	convertCharset = outCharset;
	if (outCharset == CONV_CHARSET_UNICODE)
		convertCharset = CONV_CHARSET_UNIUTF8;
	outSize = guessOutSize(inCharset, convertCharset, inSize);
	hOutData = GlobalAlloc(GMEM_MOVEABLE, outSize);
	pOutData = GlobalLock(hOutData);
	if (pOutData == NULL)
		goto end;

	inBufLen = inSize;
	convErrCode = VnConvert(inCharset, convertCharset, (BYTE *)pInData, (BYTE *)pOutData, inBufLen, outSize);
	if (convErrCode != 0)
		goto end;

	/////////////////////////////////////////////////
	// Encode output as UTF-8 if neccessary
	/////////////////////////////////////////////////
	if (outCharset != CONV_CHARSET_UNICODE && outCharset != CONV_CHARSET_VIQR) {
		HANDLE hUtf8Data = NULL;
		LPVOID pUtf8Data = NULL;
		int outBytes = fromAnsiToUtf8((const char *)pOutData, outSize, hUtf8Data, pUtf8Data);
		if (outBytes == 0)
			goto end;
		// switch output data
		GlobalUnlock(hOutData);
		GlobalFree(hOutData);
		hOutData = hUtf8Data;
		pOutData = pUtf8Data;
		outSize = outBytes;;
	}

	GlobalUnlock(hOutData);
	pOutData = NULL;
	retHandle = hOutData;
	hOutData = NULL; // don't want to free output data

end:
	if (pInData)
		GlobalUnlock(hInData);
	if (hInData && freeInput)
		GlobalFree(hInData);
	if (pOutData)
		GlobalUnlock(hOutData);
	if (hOutData)
		GlobalFree(hOutData);

	return retHandle;
}
*/

/*
//----------------------------------------------
// Detect if a memory block is the result of
// previous conversion. If so, returns the charset.
// Otherwise, returns -1
//----------------------------------------------
int getPrevCharset(BYTE *pMem, int size)
{
	if (pMem && size >= sizeof(ConvSignature)) {
		ConvSignature *pSig = (ConvSignature *)(pMem + size - sizeof(ConvSignature));
		if (pSig->signature == VNCONV_SIGNATURE)
			return pSig->charSet;
	}
	return -1;
}

void setSignature(BYTE *pAddr, int charset)
{
	ConvSignature *pSig = (ConvSignature *)pAddr;
	pSig->charSet = charset;
	pSig->signature = VNCONV_SIGNATURE;
	pSig->terminator = 0;
}
*/

