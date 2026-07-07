#include "prehdr.h"
#include <windowsx.h>
#include <stdio.h>
#include "resource.h"
#include "tooldlg.h"
#include "mainwnd.h"
#include "vnconv.h"
#include "button.h"

ConvCharset ConvCharsetList[] = {
	{_TEXT("TCVN3 (ABC)"), CONV_CHARSET_TCVN3},
	{_TEXT("Unicode UCS-2"), CONV_CHARSET_UNICODE},
	{_TEXT("Unicode UTF-8"), CONV_CHARSET_UNIUTF8},
	{_TEXT("Unicode NCR Decimal"), CONV_CHARSET_UNIREF},
	{_TEXT("VIQR"), CONV_CHARSET_VIQR},
	{_TEXT("VNI Windows"), CONV_CHARSET_VNIWIN},
	{_TEXT("VISCII"), CONV_CHARSET_VISCII},
	{_TEXT("VPS"), CONV_CHARSET_VPS},
	{_TEXT("BK HCM 2"), CONV_CHARSET_BKHCM2},
	{_TEXT("BK HCM 1"),	CONV_CHARSET_BKHCM1},
	{_TEXT("Vietware X"), CONV_CHARSET_VIETWAREX},
	{_TEXT("Vietware F"), CONV_CHARSET_VIETWAREF}
};

int inputFileDlg(HWND hWndOwner, TCHAR *fileName, int size);
int outFileDlg(HWND hWndOwner, TCHAR *fileName, int size);
int guessOutSize(int inCharset, int outCharset, int inSize);

extern CMainWnd MainWnd;

/*
//----------------------------
void uniToAnsi(VOID *buf, int size)
{
	WideCharToMultiByte(
	BYTE *pByte = (BYTE *)buf;
	WORD *pWord = (WORD *)buf;
	int count = 0;
	while (count < size) {
		*pByte++ = (BYTE)*pWord++;
		count += 2;
	}
}
*/
//------------------------------------------
int CToolkitDlg::init(HINSTANCE hInst, HWND hWndOwner)
{
	return CVirDialog::init(hInst, MAKEINTRESOURCE(IDD_TOOLKIT), hWndOwner);
}

//------------------------------------------
void CToolkitDlg::loadDlgInfo(CUserPref *pPref)
{
	int i;
	int count = sizeof(ConvCharsetList)/sizeof(ConvCharset);
	for (i=0; i < count; i++) {
		SendMessage(m_inCharsetCtrl, CB_ADDSTRING, 0, (LPARAM)ConvCharsetList[i].name);
		SendMessage(m_outCharsetCtrl, CB_ADDSTRING, 0, (LPARAM)ConvCharsetList[i].name);
	}
	int inCharset = pPref->m_inConvCharset;
	int outCharset = pPref->m_outConvCharset;
	if (inCharset < 0 || inCharset >= count)
		inCharset = 0;
	if (outCharset < 0 || outCharset >= count)
		outCharset = 0;
		
	SendMessage(m_inCharsetCtrl, CB_SETCURSEL, inCharset, 0);
	SendMessage(m_outCharsetCtrl, CB_SETCURSEL, outCharset, 0);
	SendMessage(m_clipboardCtrl, BM_SETCHECK, 
		        pPref->m_clipboardConvert ? BST_CHECKED : BST_UNCHECKED, 0);
	EnableWindow(m_inFileCtrl, !pPref->m_clipboardConvert);
	EnableWindow(m_inBrowseBtn, !pPref->m_clipboardConvert);
	EnableWindow(m_outFileCtrl, !pPref->m_clipboardConvert);
	EnableWindow(m_outBrowseBtn, !pPref->m_clipboardConvert);
}


//------------------------------------------
void CToolkitDlg::storeDlgInfo(CUserPref *pPref)
{
	pPref->m_inConvCharset = SendMessage(m_inCharsetCtrl, CB_GETCURSEL, 0, 0);
	pPref->m_outConvCharset = SendMessage(m_outCharsetCtrl, CB_GETCURSEL, 0, 0);
	pPref->m_clipboardConvert = (SendMessage(m_clipboardCtrl, BM_GETCHECK, 0, 0) == BST_CHECKED)? 1 : 0;
}

//------------------------------------------
BOOL CToolkitDlg::onInitDialog()
{
	if (!CMultiLangDialog::onInitDialog())	
		return FALSE;

	m_inCharsetCtrl = GetDlgItem(m_hWnd, IDC_IN_CHARSET);
	m_outCharsetCtrl = GetDlgItem(m_hWnd, IDC_OUT_CHARSET);
	m_inFileCtrl = GetDlgItem(m_hWnd, IDC_IN_FILE);
	m_outFileCtrl = GetDlgItem(m_hWnd, IDC_OUT_FILE);
	m_inBrowseBtn = GetDlgItem(m_hWnd, IDC_IN_BROWSE);
	m_outBrowseBtn = GetDlgItem(m_hWnd, IDC_OUT_BROWSE);
	m_clipboardCtrl = GetDlgItem(m_hWnd, IDC_CLIPBOARD);

	loadDlgInfo(m_pPref);
	return TRUE;
}

//------------------------------------------
BOOL CToolkitDlg::onCommand(WPARAM wParam, LPARAM lParam)
{
	TCHAR fname[MAX_PATH];
	WORD cmd = GET_WM_COMMAND_ID(wParam, lParam);
	switch (cmd) {
	case IDC_IN_BROWSE:
		if (inputFileDlg(m_hWnd, fname, sizeof(fname))) {
			SendMessage(m_inFileCtrl, WM_SETTEXT, 0, (LPARAM)fname);

		}
		return TRUE;
	case IDC_OUT_BROWSE:
		if (outFileDlg(m_hWnd, fname, sizeof(fname))) {
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
			EnableWindow(m_inBrowseBtn, !useClipboard);
			EnableWindow(m_outFileCtrl, !useClipboard);
			EnableWindow(m_outBrowseBtn, !useClipboard);
		}
		return TRUE;
	}
	return CMultiLangDialog::onCommand(wParam, lParam);
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
	of.lpstrFilter = _TEXT("Text files\0*.TXT\0All files\0*.*\0");
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
	of.lpstrFilter = _TEXT("Text files\0*.TXT\0All files\0*.*\0");
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
	inCharset = ConvCharsetList[inCharset].id;
	outCharset = ConvCharsetList[outCharset].id;

	// Check if clipboard is selected
	int useClipboard = (SendDlgItemMessage(m_hWnd, IDC_CLIPBOARD, BM_GETCHECK, 0, 0) == BST_CHECKED)? 1 : 0;

	if (useClipboard) {
		int errCode;
		ConvertClipboard(inCharset, outCharset, errCode, 1, m_hWnd);
		return;
	}


	// File convert
	TCHAR inFile[MAX_PATH], outFile[MAX_PATH];
	SendMessage(m_inFileCtrl, WM_GETTEXT, sizeof(inFile), (LPARAM)inFile);
	if (inFile[0] == NULL) {
		MessageBox(m_hWnd, 
			       _TEXT("You must provide an input file"), 
			       _TEXT("Invalid operation"), 
				   MB_ICONEXCLAMATION);
		SetFocus(m_inFileCtrl);
		return;
	}
	SendMessage(m_outFileCtrl, WM_GETTEXT, sizeof(outFile), (LPARAM)outFile);
	if (outFile[0] == NULL) {
		MessageBox(m_hWnd, 
			       _TEXT("You must provide an output file"), 
				   _TEXT("Invalid operation"), 
				   MB_ICONEXCLAMATION);
		SetFocus(m_outFileCtrl);
		return;
	}

#ifndef _UNICODE
	int ret = VnFileConvert(inCharset, outCharset, inFile, outFile);
#else
	char inFileAnsi[MAX_PATH], outFileAnsi[MAX_PATH];
	WideCharToMultiByte(CP_ACP, 0, inFile, -1, inFileAnsi, MAX_PATH, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, outFile, -1, outFileAnsi, MAX_PATH, NULL, NULL);
	int ret = VnFileConvert(inCharset, outCharset, inFileAnsi, outFileAnsi);
#endif

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
	storeDlgInfo(m_pPref);
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
	case WM_DESTROY:
		if (m_hStdFont != NULL) {
			DeleteObject(m_hStdFont);
			m_hStdFont = NULL;
		}
		break;
	}
	return CMultiLangDialog::DialogProc(hDlg, uMsg, wParam, lParam);
}

struct ConvSignature
{
	WORD terminator;
	int charSet;
	WORD signature;
};

#define VNCONV_SIGNATURE 0xF1E3

//--------------------------------------
int ConvertClipboard(int inCharset, int outCharset, int & convErrCode, int promptInfo, HWND hOwnerWnd)
{
	static lastFormat = CF_TEXT; // format of last converted clipboard (CF_TEXT or CF_UNICODETEXT)

	HANDLE hInData = NULL;
	HANDLE hOutData = NULL;
	HANDLE hTemp; //, hAnsiClip, hUniClip;


	BYTE *pInData = NULL;
	BYTE *pOutData = NULL;
	BYTE *pActualData = NULL;

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

	//inFormat = (inCharset == CONV_CHARSET_UNICODE)? CF_UNICODETEXT : CF_TEXT;
	outFormat = (outCharset == CONV_CHARSET_UNICODE)? CF_UNICODETEXT : CF_TEXT;

	if (!lastConverted) {
		if (IsClipboardFormatAvailable(CF_UNICODETEXT))
			inFormat = CF_UNICODETEXT;
		else
			inFormat = CF_TEXT;
		//inFormat = (inCharset == CONV_CHARSET_UNICODE)? CF_UNICODETEXT : CF_TEXT;
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

		if (pInData && inCharset != CONV_CHARSET_UNICODE && inFormat == CF_UNICODETEXT) {
			inSize = inSize / 2;
			pActualData = new char[inSize+1];
			WideCharToMultiByte(CP_ACP, 0, pInData, -1, pActualData, inSize+1, 0, 0);
			// convert input data to ansi
			//uniToAnsi(pInData, inSize);
		}
		else pActualData = pInData;
	}
	else pActualData = pInData;


//	if (pInData != NULL && inSize > 0) {
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
		ret = VnConvert(inCharset, outCharset, pActualData, pOutData, inLen, outSize);

		if (ret != 0) {
			if (promptInfo) {
#ifndef _UNICODE
				_stprintf(msg, _TEXT("Error[%d]: %s"), ret, VnConvErrMsg(ret));
#else
				_stprintf(msg, _TEXT("Error[%d]"), ret);
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
		
		// Set clipboard data
		GlobalUnlock(hInData);
		pInData = NULL;
		GlobalUnlock(hOutData);
		pOutData = NULL;

		hTemp = GlobalReAlloc(hOutData, outSize, GMEM_MOVEABLE);
		if (hTemp != NULL)
			hOutData = hTemp;

		EmptyClipboard();

		SetClipboardData(outFormat, hOutData);
		hOutData = NULL;  // don't want to free output data, it is owned by system
		if (promptInfo)
			MessageBox(hOwnerWnd, _TEXT("Successfully converted"), _TEXT("Result"), MB_ICONINFORMATION);
		lastFormat = outFormat;
		ret = 1;
	}

end:
	if (pInData) GlobalUnlock(hInData);
	if (pOutData) GlobalUnlock(hOutData);
	if (hOutData) GlobalFree(hOutData);
	if (clipboardOpen)
		CloseClipboard();
	return ret;
}

//----------------------------
int guessOutSize(int inCharset, int outCharset, int inSize)
{
	int charCount;
	int outSize;

	if (inCharset == CONV_CHARSET_UNICODE)
		charCount = (inSize / 2)+1;
	else
		charCount = inSize;

	if (inCharset == CONV_CHARSET_UNIREF)
		outSize = charCount * 8;
	if (IS_DOUBLE_BYTE_CHARSET(outCharset) || outCharset == CONV_CHARSET_UNICODE)
		outSize = charCount * 2;
	else if (IS_SINGLE_BYTE_CHARSET(outCharset))
		outSize = charCount;
	else
		outSize = charCount * 3;
	return outSize;
}

