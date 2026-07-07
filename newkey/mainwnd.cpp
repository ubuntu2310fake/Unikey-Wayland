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
#include "resource.h"
#include "encode.h"
#include "keydlg.h"
#include "userpref.h"
#include "font.h"
#include "tooldlg.h"
#include "vnconv.h"
#include "intrtfio.h"

#define WM_MYICON_NOTIFY	(WM_USER+1)

// Global variables
HINSTANCE AppInstance, HookDll;

HHOOK hKeyHook,hMouseHook;
TCHAR AppTitle[100];

SharedMem *pSharedMem = NULL;

BOOL initDLL(HWND hWnd);

CUserPref UserPref; // user preference
CMainWnd MainWnd(&UserPref);

BOOL CMainWnd::initInstance(HINSTANCE hInstance)
{
	HWND hWnd = Create(
		            WS_EX_TOOLWINDOW,
					MAIN_WINDOW_CLASS_NAME,
					AppTitle,
					WS_OVERLAPPEDWINDOW,
					CW_USEDEFAULT, 
					CW_USEDEFAULT,
					CW_USEDEFAULT,
					CW_USEDEFAULT,
					NULL,
					NULL,
					hInstance);
	if (!hWnd)
		return FALSE;
	ShowWindow(hWnd, SW_HIDE);
	UpdateWindow(hWnd);

	m_vietFontOK = 0;
	if (FontExists(VIET_FONT_NAME)) {
		m_vietFontOK = 1;
	}
	else {
		if (loadVietFont()) {
			m_vietFontOK = 1;
		}
	}

	if (UserPref.m_vietGUI && !m_vietFontOK)
		UserPref.m_vietGUI = 0;
	m_menu.init(hInstance, hWnd, &UserPref);
	m_menu.reload();
	UserPref.m_changes = 0;
	// Locate help file. The name of help file is "UniKey.hlp", it is supposed to be 
	// in the same location with the exe file.
	GetModuleFileName(NULL, m_helpPath, sizeof(m_helpPath)/sizeof(TCHAR));
	TCHAR *p = _tcsrchr(m_helpPath, _TEXT('\\'));
	_tcscpy(p+1, _TEXT("ukmacro.txt"));
	_tcscpy(m_macroPath, m_helpPath);

#ifndef _UNICODE
	_tcscpy(p+1, _TEXT("UniKey.hlp"));
#else
	_tcscpy(p+1, _TEXT("ukmanual.htm"));
#endif

	_tcscpy(m_currentMacPath, m_macroPath);
	m_macTab.loadFromFile(m_macroPath);
	return TRUE;
}

//-------------------------------------------------
LRESULT CMainWnd::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result;
	result = FALSE;
	static UINT taskbarCreatedMsg;

	switch (uMsg) {
	/*
	case WM_MULTI_INSTANCE:
		MessageBox(m_hWnd, "UniKey is already running", "Information", MB_ICONINFORMATION);
		result = TRUE;
		break;
	*/
	case WM_CREATE:
		taskbarCreatedMsg = RegisterWindowMessage(TEXT("TaskbarCreated"));
		result = TRUE;
		break;
	case WM_MEASUREITEM:
		m_menu.measureItem((LPMEASUREITEMSTRUCT)lParam);
		result = TRUE;
		break;
	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT lpd = (LPDRAWITEMSTRUCT)lParam;
			if (lpd->CtlType == ODT_MENU)
				m_menu.drawItem(lpd);
		}
		result = TRUE;
		break;
	case WM_MYICON_NOTIFY:
		switch (lParam) {
		case WM_LBUTTONDOWN:
			SwitchMode();
			break;
		case WM_RBUTTONUP:
			{
				POINT point;
				GetCursorPos(&point);
				SetForegroundWindow(m_hWnd);
				m_menu.trackPopupMenu(
					TPM_RIGHTALIGN | TPM_BOTTOMALIGN,
					point.x,
					point.y);
				PostMessage(m_hWnd, WM_NULL, 0, 0);
			}
			break;
		case WM_LBUTTONDBLCLK:
			{
				UINT id = GetMenuDefaultItem(m_menu.m_hMenu, FALSE, 0);
				if (id != -1) {
					SwitchMode();
					PostMessage(m_hWnd, WM_COMMAND, id, 0);
				}
			}
			break;
		}
		result = TRUE;
		break;
	case WM_DESTROY:
	case WM_QUERYENDSESSION:
		UserPref.m_vietnamese = IsVietnamese();
		UserPref.setValue();
		if (uMsg == WM_DESTROY)
			PostQuitMessage(1);
		result = TRUE;
		break;
	case WM_HOOK_TOOLKIT_SHORTCUT:
		onToolkit();
		result = TRUE;
		break;
	case WM_HOOK_PANEL_SHORTCUT:
		onPanel();
		result = TRUE;
		break;
	case WM_HOOK_FLY_CONVERT:
		{
			int convErrCode;
			ConvertClipboard(InConvCharsetList[UserPref.m_inConvCharset].id,
						     OutConvCharsetList[UserPref.m_outConvCharset].id,
						     convErrCode);
			result = TRUE;
		}
		break;
	case WM_HOOK_CHANGE_CHARSET:
		{
			UserPref.m_changes = PREF_CHANGE_CHARSET;
			UserPref.m_codeTable = wParam;
			updateChanges(UserPref);
			m_menu.update();
			UserPref.m_changes = 0;
		}
		break;
	case WM_RENDERFORMAT:
		RenderFormat(wParam);
		result = 0;
		break;
	case WM_RENDERALLFORMATS:
		RenderAllFormats();
		result = 0;
		break;
	case WM_COMMAND:
		switch (wParam) {
		case ID_OPEN_HELP:
			openHelp();
			result = TRUE;
			break;
		case ID_END_KEY:
			DeleteStatusIcon();
			DestroyWindow(m_hWnd);
			result = FALSE;
			break;
		case ID_TOOLKIT:
			onToolkit();
			result = TRUE;
			break;
		case ID_KEY_PANEL:
			onPanel();
			result = TRUE;
			break;
		case ID_FLY_CONVERT:
			{
				int convErrCode;
				ConvertClipboard(InConvCharsetList[UserPref.m_inConvCharset].id,
							     OutConvCharsetList[UserPref.m_outConvCharset].id,
							     convErrCode, 1, m_hWnd);
				result = TRUE;
			}
			break;
		case ID_CONFIRM_PANEL:
			if (UserPref.m_changes) {
				updateChanges(UserPref);
				m_menu.update();
				UserPref.m_changes = 0;
			}
			break;
		default:
			if (m_menu.processCommand(wParam)) {
				updateChanges(UserPref);
				UserPref.m_changes = 0;
			}
		}
		break;
	default:
		if (uMsg == taskbarCreatedMsg) {
			//recreate systray icon
			pSharedMem->iconShown = 0;
			ModifyStatusIcon();
		}
		result = DefWindowProc(m_hWnd, uMsg, wParam, lParam);
	}
	return result;
}

//---------------------------------------------
BOOL InitApplication(
       HINSTANCE hInstance)
{
	BOOL bOk;
	// The window class for all instances of the main window.
	WNDCLASSEX wcf;

	// Assign the appropriate values for this main frame window class.
	wcf.cbSize        = sizeof(WNDCLASSEX);
	wcf.cbClsExtra    = 0;            // No per-class extra data.
	wcf.cbWndExtra    = 0;            // No per-window extra data.
	wcf.hInstance     = hInstance;    // Application module instance.
	wcf.lpfnWndProc   = GenWindowProc;  // Global Window Procedure (defined in
									  // UTIL.H for all CVirWindows).
	wcf.hCursor       = NULL;			// Load app cursor.
	wcf.hIcon         = (HICON) LoadImage(           // Load small icon.
									hInstance,
									MAKEINTRESOURCE(IDR_MAINFRAME),
									IMAGE_ICON,
									32, 32,
									LR_SHARED);
	wcf.hIconSm       = NULL;
	wcf.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);  // Default backgnd color.
	wcf.style         = 0;				// Class style(s).
	wcf.lpszClassName = MAIN_WINDOW_CLASS_NAME; // Class name.
	wcf.lpszMenuName  = NULL;						  // Menu name.

	// Register the window class and return FALSE if unsuccesful.
	bOk = RegisterClassEx(&wcf);

	return (bOk);
}

//-------------------------------------------
int WINAPI WinMain(
		HINSTANCE hInstance,
		HINSTANCE hPrevInstance,
		LPSTR lpCmdLine,
		int nCmdShow
)
{
#ifdef _UNICODE
	OSVERSIONINFO osinfo;
	osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osinfo);
	if (osinfo.dwMajorVersion < 4 || osinfo.dwPlatformId != VER_PLATFORM_WIN32_NT) {
		MessageBoxA(NULL, "This version runs only in Windows NT.40/2000/XP only\n"
						 "You can download UniKey running on all Windows platforms at http://unikey.sourceforge.net", 
						 "Platform not supported", 
						 MB_ICONERROR);
		return -1;
	}
#endif
	InitRtfIO();

	MSG msg;
	//-- Check if a previous instance is loaded
	AppInstance = hInstance;
	LoadString(AppInstance,IDS_APP_TITLE,AppTitle,100);
	if (HWND hPrevWnd = FindWindow(MAIN_WINDOW_CLASS_NAME, NULL)) { 
		// Bring in the last instance and exit
		BringWindowToTop(hPrevWnd);
		SetForegroundWindow(hPrevWnd);
		PostMessage(hPrevWnd,WM_COMMAND,ID_KEY_PANEL,0); 
		//PostMessage(hPrevWnd, WM_MULTI_INSTANCE, 0, 0); 
		return 0;
	}
	if (!InitApplication(hInstance))
		return 0;
	UserPref.getValue();

	if (!MainWnd.initInstance(hInstance))
		return 0;
	// Init DLL
	HookDll = GetVietHookDll();
	if (!initDLL(MainWnd.m_hWnd))
		return 0;

	ModifyStatusIcon(); // Show icon on the tray
	if (UserPref.m_showDlg)
		PostMessage(MainWnd.m_hWnd, WM_COMMAND, ID_KEY_PANEL, 0);
	else 
		MessageBeep(0xFFFFFFFF);
	while (GetMessage(&msg, (HWND) NULL, 0, 0)) { 
		if ((!IsWindow(MainWnd.m_toolDlg.m_hWnd) || !IsDialogMessage(MainWnd.m_toolDlg.m_hWnd,&msg)) && 
            (!IsWindow(MainWnd.m_configDlg.m_hWnd) || !IsDialogMessage(MainWnd.m_configDlg.m_hWnd,&msg))) {

			TranslateMessage(&msg); 
			DispatchMessage(&msg);

		}
    } 

	// the program is finished
	if (hKeyHook != NULL)
		UnhookWindowsHookEx(hKeyHook);
	if (hMouseHook != NULL)
		UnhookWindowsHookEx(hMouseHook);
	MainWnd.cleanup();
	return 1;
}

//--------------------------------------------
BOOL initDLL(HWND hWnd)
{
	HOOKPROC keyProc = (HOOKPROC)GetProcAddress(HookDll, "MyKeyHook");
	HOOKPROC mouseProc = (HOOKPROC)GetProcAddress(HookDll, "MyMouseHook");
	if (keyProc == NULL || mouseProc == NULL) {
		MessageBox(NULL, _TEXT("Failed to find hook procedures"), _TEXT("Error"), MB_OK | MB_ICONERROR);
		return FALSE;
	}

	hKeyHook = SetWindowsHookEx(WH_KEYBOARD, keyProc, HookDll, 0);
	hMouseHook = SetWindowsHookEx(WH_MOUSE,mouseProc, HookDll, 0);

	if (hKeyHook == NULL || hMouseHook == NULL) {
		MessageBox(NULL, _TEXT("Failed to initialize the program"), _TEXT("Error"), MB_OK | MB_ICONERROR);
		return FALSE;
	}

	pSharedMem = GetSharedMem();

	// Init shared memory
	pSharedMem->hMainDlg = hWnd;
	pSharedMem->hEnIcon = LoadIcon(AppInstance,MAKEINTRESOURCE(IDI_EN));
	pSharedMem->hVietIcon = LoadIcon(AppInstance,MAKEINTRESOURCE(IDI_VIET));
	pSharedMem->iconShown = 0;
	pSharedMem->keyHook = hKeyHook;
	pSharedMem->mouseHook = hMouseHook;
	pSharedMem->keyMode = CharsetTable[UserPref.m_codeTable].id;
	pSharedMem->iconMsgId = WM_MYICON_NOTIFY;
	pSharedMem->switchKey = UserPref.m_switchKey;
	pSharedMem->vietKey = UserPref.m_vietnamese;
	pSharedMem->inMethod = UserPref.m_inMethod;

	SetHookOptions(UserPref);

	if (CharsetTable[UserPref.m_codeTable].encoding == UNICODE_UTF8_X) {
		pSharedMem->codeTable.encoding = UNICODE_UTF8;
		pSharedMem->codeTable.singleBackspace = 1;
	}
	else {
		pSharedMem->codeTable.encoding = CharsetTable[UserPref.m_codeTable].encoding;
		pSharedMem->codeTable.singleBackspace = 0;
	}

	//pSharedMem->codeTable.encoding = CharsetTable[UserPref.m_codeTable].encoding;

	BuildCodeTable(CharsetTable[UserPref.m_codeTable].id, UserPref.m_inMethod, &pSharedMem->codeTable);
	pSharedMem->Initialized = 1;
	MainWnd.updateMacroTable();
	return TRUE;
}


void getFontFile(TCHAR *path, int size)
{
	GetModuleFileName(NULL, path, size);
	TCHAR *p = _tcsrchr(path, _T('\\'));
	if (p != NULL) p++;
	else p = path;
	_tcscpy(p, VIET_FONT_FILE);

}

//----------------------------------------
CMainWnd::CMainWnd(CUserPref *pPref) : m_configDlg(pPref), m_toolDlg(pPref) 
{
	m_pPref = pPref;
	m_menuItems = 0;
	m_helpPath[0] = 0;
	m_fontLoaded = 0;
	m_vietFontOK = 0;
};

//----------------------------------------
int CMainWnd::loadVietFont()
{
#ifndef _UNICODE
	TCHAR path[MAX_PATH];
	getFontFile(path, sizeof(path)*sizeof(TCHAR));
	if (AddFontResource(path) > 0) {
		SendMessage(HWND_BROADCAST, WM_FONTCHANGE, 0, 0);
		m_fontLoaded = 1;
		//fontInstalled = 1;
		return 1;
	}
	return 0;
#else
	return 1;
#endif
}

//----------------------------------------
void CMainWnd::cleanup()
{
#ifndef _UNICODE
	if (m_fontLoaded) {
		TCHAR path[MAX_PATH];
		getFontFile(path, sizeof(path)*sizeof(TCHAR));
		RemoveFontResource(path);
		m_fontLoaded = 0;
	}
#endif
}

//------------------------------------------------------
void CMainWnd::updateChanges(const CUserPref &pref)
{
	if (pref.m_changes & PREF_CHANGE_CHARSET) {
		BuildCodeTable(CharsetTable[pref.m_codeTable].id, pref.m_inMethod, &pSharedMem->codeTable);

		if (CharsetTable[pref.m_codeTable].encoding == UNICODE_UTF8_X) {
			pSharedMem->codeTable.encoding = UNICODE_UTF8;
			pSharedMem->codeTable.singleBackspace = 1;
		}
		else {
			pSharedMem->codeTable.encoding = CharsetTable[pref.m_codeTable].encoding;
			pSharedMem->codeTable.singleBackspace = 0;
		}

		SetKeyMode(CharsetTable[pref.m_codeTable].id, pref.m_inMethod, &pSharedMem->codeTable);
		updateMacroTable();
	}

	if ((pref.m_changes & PREF_CHANGE_METHOD) &&
		((pref.m_changes & PREF_CHANGE_CHARSET) == 0)) {
		BuildInputMethod(pref.m_inMethod, &pSharedMem->codeTable);
		SetInputMethod(pref.m_inMethod, pSharedMem->codeTable.DT);
	}

	if (pref.m_changes & PREF_CHANGE_SWITCHKEY)
		SetSwitchKey(pref.m_switchKey);

	if (pref.m_changes & PREF_CHANGE_OPTIONS)
		SetHookOptions(pref);
}

//------------------------------------
void CMainWnd::onToolkit()
{
	if (m_toolDlg.m_hWnd == NULL) {
		m_toolDlg.init(m_hInst, m_hWnd);
		m_toolDlg.ShowModelessDialog();
	}
	else {
		ShowWindow(m_toolDlg.m_hWnd, SW_SHOW);
		BringWindowToTop(m_toolDlg.m_hWnd);
	}
}

//------------------------------------
void CMainWnd::onPanel()
{
	if (m_configDlg.m_hWnd == NULL) {
		m_configDlg.init(m_hInst, m_hWnd);
		m_configDlg.ShowModelessDialog();
	}
	else {
		ShowWindow(m_configDlg.m_hWnd, SW_SHOW);
		BringWindowToTop(m_configDlg.m_hWnd);
	}
}

//------------------------------------
void SetHookOptions(const CUserPref & pref)
{
	pSharedMem->options.freeMarking = pref.m_freeMarking;
	pSharedMem->options.toneNextToVowel = pref.m_toneNextToVowel;
	pSharedMem->options.modernStyle = pref.m_modernStyle;
	pSharedMem->options.macroEnabled = pref.m_macroEnabled;
	pSharedMem->options.useUnicodeClipboard = pref.m_useUnicodeClipboard;
	pSharedMem->options.alwaysMacro = pref.m_alwaysMacro;
	pSharedMem->options.useIME = pref.m_useIME;
}

//------------------------------------
void CMainWnd::updateMacroTable()
{
	int inLen, maxOutLen;
	int ret, i, k;
	int outCharset = CharsetTable[m_pPref->m_codeTable].vnconvId;
	if (CharsetTable[m_pPref->m_codeTable].id == UNICODE_CHARSET)
		outCharset = CONV_CHARSET_UNICODE; // it would be a waste to store macro as UTF-8, or NCR. Use pure Unicode

	char *p = pSharedMem->macroMem;
	int offset = 0;
	//int avail = MACRO_MEM_SIZE;
	int len;
	for (i=0, k=0; i<m_macTab.m_count && k < MAX_MACRO_ITEMS; i++) {
		len = strlen(m_macTab.m_table[i].key);
		//if (len+1 > avail)
		//	break;
		if (offset+len+1 > MACRO_MEM_SIZE)
			break;
		strcpy(p, m_macTab.m_table[i].key);
		pSharedMem->macroTable[k].keyOffset = offset;
		p += len+1;
		//avail -= (len+1);
		offset += (len+1);

		inLen = -1;
		maxOutLen = MACRO_MEM_SIZE - offset;
		ret = VnConvert(CONV_CHARSET_VIQR, outCharset, 
					(BYTE *)m_macTab.m_table[i].text,
					(BYTE *)p,
					inLen, maxOutLen);
		if (ret != 0) {
			p -= (len+1);
			offset -= (len+1);
			continue;
		}

		pSharedMem->macroTable[k].textOffset = offset;
		p += maxOutLen;
		//avail -= maxOutLen;
		offset += maxOutLen;
		k++;
	}
	pSharedMem->macroCount = k;
}

//------------------------------------
void CMainWnd::openHelp()
{
#ifndef _UNICODE
	WinHelp(m_hWnd, m_helpPath, HELP_FINDER, 0);
#else
	ShellExecute(NULL, _T("open"), m_helpPath, NULL, NULL, SW_SHOWNORMAL);
#endif
}
