/*------------------------------------------------------------------------------
UniKey - Vietnamese Keyboard for Windows
Copyright (C) 1998-2001 Pham Kim Long
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
#include <stdio.h>
#include <memory.h>
#include "keycons.h"
#include "keyhook.h"
#include "vietkey.h"

const long IsExtended = 0x1000000,
		   FlagScancode = 0xFF0000,
           IsALT = 0x20000000,
		   IsPrevDown = 0x40000000,
		   IsReleased = 0x80000000,
		   KeyUp = 0xC0000000;

VietKey VnKbd;

SharedMem * pShMem = NULL ;
HINSTANCE MyInstance = NULL;

unsigned char  KeyState[256];
unsigned short CharBuf;
int BackTracks;
int ClipboardIsEmpty = 1;
int PendingSwitch = 0;
int PuttingBacks = 0;

//DWORD TlsIdx;

void InitProcess();
void ClearClipboard();
void ResetBuffer();
//HMODULE LoadUnicowsProc(void);
//void UnloadUnicowsProc();

#define VK_V 0x56

//////////////////////////////////
/*
struct ThreadData {
	HHOOK GetMsgHook;
	int receivingClipboard;
};
*/
//void InitThread();


int CheckSwitchKey(WPARAM wParam, LPARAM lParam);
int CheckShortcuts(WPARAM wParam, LPARAM lParam);

int CheckBack(WPARAM wParam);
void PushBacks();
void PushBuffer(HWND wndFocused);


#define ID_VIETKEY_ICON 1000
#define VIETNAMESE_INPUT_ID 0x042a //language identifier for Vietnamese

//UINT VK_SHIFT_SCAN;
UINT VK_BACK_SCAN;
//UINT VK_INSERT_SCAN;
UINT VK_PAUSE_SCAN;

// local helper functions
void initDLL();

// temporarily release shift keys
// release = 1: release
// release = 0: change shifts to original state
void ReleaseShift(int release);

//-------------------------------------------------
BOOL APIENTRY DllMain(HINSTANCE hModule, DWORD reason, LPVOID reserved)
{
    HANDLE hMapObject = NULL;   /* handle to file mapping */ 
    BOOL fInit, fIgnore; 
//	ThreadData *pThread;
    switch (reason) { 
 		case DLL_PROCESS_ATTACH: 
 
            /* Create a named file mapping object */ 
			MyInstance =  hModule;
            hMapObject = CreateFileMapping( 
                (HANDLE) 0xFFFFFFFF, /* use paging file    */ 
                NULL,                /* no security attr.  */ 
                PAGE_READWRITE,      /* read/write access  */ 
                0,                   /* size: high 32-bits */ 
                sizeof(SharedMem),   /* size: low 32-bits  */ 
                _TEXT("UniKeyHookShareMem3.5.Final-testing"));/* name of map object */ 
            if (hMapObject == NULL) 
                return FALSE; 
			
			fInit = (GetLastError() != ERROR_ALREADY_EXISTS);
            /* Get a pointer to the file-mapped shared memory. */ 
 
            pShMem = (SharedMem *) MapViewOfFile( 
				hMapObject,     /* object to map view of    */ 
                FILE_MAP_WRITE, /* read/write access        */ 
                0,              /* high offset:   map from  */ 
                0,              /* low offset:    beginning */ 
                0);             /* default: map entire file */ 
            if (pShMem == NULL) 
                return FALSE; 
			InitProcess();
			if (fInit)
				initDLL();
//			TlsIdx = TlsAlloc();
//			InitThread();
         break; 
 
        /* 
         * The DLL is detaching from a process due to 
         * process termination or a call to FreeLibrary. 
         */ 
/*
		  case DLL_THREAD_ATTACH:
			  InitThread();
			  break;
		  case DLL_THREAD_DETACH:
			  pThread = (ThreadData *)TlsGetValue(TlsIdx);
			  if (pThread != NULL) 
				delete pThread;
			  break;
*/
		case DLL_PROCESS_DETACH: 
 
            /* Unmap shared memory from the process's address space. */ 
 
            fIgnore = UnmapViewOfFile(pShMem); 
 
            /* Close the process's handle to the file-mapping object. */ 
 
            fIgnore = CloseHandle(hMapObject); 
//			TlsFree(TlsIdx);
            break; 
 
        default: 
          break; 
 
  } 
 
  return TRUE; 
  UNREFERENCED_PARAMETER(hModule); 
  UNREFERENCED_PARAMETER(reserved); 
 
}

/*
void InitThread()
{
	ThreadData *pThread;
	pThread = new ThreadData();
	memset(pThread, 0, sizeof(ThreadData));
	TlsSetValue(TlsIdx, pThread);
}
*/

//-------------------------------------------------
void InitProcess()
{
	BackTracks = 0;
	VnKbd.setCodeTable(&pShMem->codeTable);
	VK_BACK_SCAN = MapVirtualKey(VK_BACK, 0);
	VK_PAUSE_SCAN = MapVirtualKey(VK_PAUSE, 0);

	/*
	TCHAR fileName[100];
	GetModuleFileName(NULL, fileName, 100);
	_tcsupr(fileName);
	FILE *f;
	f = _tfopen(_T("d:\\uklog.txt"), _T("at"));
	_ftprintf(f, _T("%s\n"), fileName);
	fclose(f);
	IsConsoleApp = (_tcsstr(fileName, _T("UNIKEY")) != NULL);
	*/
}

//-------------------------------------------------
LRESULT CALLBACK MyKeyHook(int code, WPARAM wParam, LPARAM lParam)
{
	int keyType;
	if (!pShMem->Initialized) return 0;
	if (code < 0) 
		return CallNextHookEx(pShMem->keyHook,code,wParam,lParam);

	// determine if current window is a console, if so returns
	TCHAR className[128];
	className[0] = 0;
	GetClassName(GetForegroundWindow(), className, 128);
	if (_tcscmp(className, _T("ConsoleWindowClass")) == 0)
		return CallNextHookEx(pShMem->keyHook,code,wParam,lParam);
	
	GetKeyboardState(KeyState);

	if (CheckSwitchKey(wParam,lParam) || CheckShortcuts(wParam, lParam))
		return 1;
	
	if (pShMem->vietKey && code == HC_ACTION && !ClipboardIsEmpty) {
		if (wParam != VK_INSERT && wParam != VK_SHIFT && !(lParam & IsReleased)) {
			// postpone this key
			keybd_event(wParam, 
						HIBYTE(LOWORD(lParam)), 
						(lParam & IsExtended)? KEYEVENTF_EXTENDEDKEY : 0,
						0);
			return 1;
		}
		if (wParam == VK_INSERT && (lParam & IsReleased)) {
			ClearClipboard();
		}
	}
		
	if (pShMem->vietKey && (code == HC_ACTION) && !(lParam & IsReleased)) {
		if (CheckBack(wParam))	return 1;
		if (PuttingBacks) {
			if (wParam != VK_BACK) {
				keybd_event(wParam, 
						HIBYTE(LOWORD(lParam)), 
						(lParam & IsExtended)? KEYEVENTF_EXTENDEDKEY : 0,
						0);
				return 1;
			}
			return CallNextHookEx(pShMem->keyHook,code,wParam,lParam);
		}

		if ((KeyState[VK_CONTROL] & 0x80) == 0 && (KeyState[VK_MENU] & 0x80) == 0) {
			keyType = ToAscii(wParam,UINT(HIWORD(lParam)),KeyState,&CharBuf,0);
			if (keyType == 1) {
				unsigned char c = (unsigned char)CharBuf;
				if (wParam >= VK_NUMPAD0 && wParam <= VK_NUMPAD9)
					VnKbd.putChar(c); // don't process numeric keypad
				else {
					VnKbd.process(c);
					if (VnKbd.backs!=0 || VnKbd.keysPushed!=0) {
						PushBacks();
						return 1;
					}
				}
				//SendMessage(GetFocus(), WM_CHAR, CharBuf, 1);
				//return 1;
			}
			else if (wParam!=VK_SHIFT && wParam!=VK_INSERT)
				ResetBuffer();
		}
		else ResetBuffer();
	}
	return CallNextHookEx(pShMem->keyHook,code,wParam,lParam);
}


//-------------------------------------------------
void PushBuffer(HWND wndFocused)
{
	SHORT scanCode;
	WPARAM lParam;

	PostMessage(wndFocused,WM_KEYUP,VK_PAUSE,
		(LPARAM(VK_PAUSE_SCAN)<<16)+LPARAM(IsReleased)+1);
	if (pShMem->keyMode == UNICODE_CHARSET && pShMem->codeTable.encoding == UNICODE_UCS2) {
		if (!pShMem->options.useUnicodeClipboard && pShMem->unicodePlatform && IsWindowUnicode(wndFocused)) {
			for (int i=0; i<VnKbd.keysPushed; i++) {
				scanCode = VkKeyScanW(VnKbd.uniPush[i]);
				lParam = (scanCode << 16) + 1;
				PostMessageW(wndFocused, WM_CHAR, VnKbd.uniPush[i], lParam);
			}
		}
		else {
			OpenClipboard(pShMem->hMainDlg);

			EmptyClipboard();

			VnKbd.uniPush[VnKbd.keysPushed] = 0; // null-terminated
			VnKbd.ansiPush[VnKbd.keysPushed] = 0;
			VnKbd.keysPushed++;

			HGLOBAL hBuf = GlobalAlloc(GMEM_MOVEABLE, sizeof(WORD) * VnKbd.keysPushed);
			HGLOBAL hBufAnsi = GlobalAlloc(GMEM_MOVEABLE, VnKbd.keysPushed);

			LPVOID pBuf = GlobalLock(hBuf);
			LPVOID pBufAnsi = GlobalLock(hBufAnsi);

			memcpy(pBuf, VnKbd.uniPush, sizeof(WORD) * VnKbd.keysPushed);
			memcpy(pBufAnsi, VnKbd.ansiPush, VnKbd.keysPushed);
			GlobalUnlock(hBuf);
			GlobalUnlock(hBufAnsi);

			SetClipboardData(CF_UNICODETEXT, hBuf);
			SetClipboardData(CF_TEXT, hBufAnsi);

			CloseClipboard();

			//GetKeyboardState(KeyState);
			keybd_event(VK_SHIFT, 0, 0, 0);
			keybd_event(VK_INSERT, 0, KEYEVENTF_EXTENDEDKEY, 0);
			keybd_event(VK_INSERT, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);

			if ((KeyState[VK_SHIFT] & 0x80) == 0 || (KeyState[VK_RSHIFT] & 0x80) != 0)
				keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
			ClipboardIsEmpty = 0;
		}
	}
	else if (pShMem->keyMode == DECOMPOSED_UNICODE_CHARSET) {
		if (pShMem->unicodePlatform && IsWindowUnicode(wndFocused)) {
			for (int i=0; i<VnKbd.keysPushed; i++) {
				scanCode = VkKeyScanW(VnKbd.uniPush[i]);
				lParam = (scanCode << 16) + 1;
				PostMessageW(wndFocused, WM_CHAR, VnKbd.uniPush[i],lParam);
			}
		}
		else {
			ActivateKeyboardLayout((HKL)VIETNAMESE_INPUT_ID, 0);
			int count = WideCharToMultiByte(1258, 0, VnKbd.uniPush, VnKbd.keysPushed, 
				                           (char *)VnKbd.ansiPush, VnKbd.keysPushed, 0, 0);
			for (int i=0; i<count; i++) {
				scanCode = VkKeyScan(VnKbd.ansiPush[i]);
				lParam = (scanCode << 16) + 1;
				PostMessage(wndFocused,WM_CHAR, VnKbd.ansiPush[i], lParam);
			}
		}
	}
	else {
		if (pShMem->keyMode == WINCP1258_CHARSET)
			ActivateKeyboardLayout((HKL)VIETNAMESE_INPUT_ID, 0);
		for (int i=0; i<VnKbd.keysPushed; i++) {
			scanCode = VkKeyScan(VnKbd.ansiPush[i]);
			lParam = (scanCode << 16) + 1;
			PostMessage(wndFocused,WM_CHAR, VnKbd.ansiPush[i], lParam);
		}
	}
	return;
}


//-------------------------------------------------
// temporarily release shift keys
// release = 1: release
// release = 0: change shifts to original state
//-------------------------------------------------
void ReleaseShift(int release)
{
	static BYTE sh = 0;
	static BYTE lsh = 0;
	static BYTE rsh = 0;
	static int TempReleased = 0;

	if (release) {
		sh = KeyState[VK_SHIFT];
		lsh = KeyState[VK_LSHIFT];
		rsh = KeyState[VK_RSHIFT];
		KeyState[VK_SHIFT] = 0;
		KeyState[VK_LSHIFT] = 0;
		KeyState[VK_RSHIFT] = 0;
		SetKeyboardState(KeyState);
		TempReleased = 1;
	}
	else if (TempReleased) {
		TempReleased = 0;
		KeyState[VK_SHIFT] = sh;
		KeyState[VK_LSHIFT] = lsh;
		KeyState[VK_RSHIFT] = rsh;
		SetKeyboardState(KeyState);
	}
}

//-------------------------------------------------
void PushBacks()
{
	if (VnKbd.backs == 0) {
		BackTracks = 0;
		PushBuffer(GetFocus());
	}
	else {
		PuttingBacks = 1;
		if (KeyState[VK_SHIFT] & 0x80)
			ReleaseShift(1);
		for (int i=0; i<VnKbd.backs; i++) {
			keybd_event(VK_BACK, VK_BACK_SCAN,0,0);
			keybd_event(VK_BACK, VK_BACK_SCAN, KEYEVENTF_KEYUP, 0);
		}
		keybd_event(VK_PAUSE, VK_PAUSE_SCAN, 0, 0);
	}
	return;
}

//-------------------------------------------------
int CheckBack(WPARAM wParam)
{
	if (wParam == VK_PAUSE && PuttingBacks) {
		PuttingBacks = 0;
		ReleaseShift(0);
		PushBuffer(GetFocus());
		return 1;
	}
	return 0;
}

//-------------------------------------------------
int CheckSwitchKey(WPARAM wParam, LPARAM lParam)
{
	if (!ClipboardIsEmpty)
		return 0;
	int ret = 0;
	// In WinXP CTRL-SHIFT is used to change text alignment. Disable this functionality

	static int ctrlPressed = 0;
	static int shiftPressed = 0;

	if (pShMem->winMajorVersion == 5 && pShMem->winMinorVersion == 1) { // WinXP
		if ((KeyState[VK_CONTROL] & 0x80) && (KeyState[VK_SHIFT] & 0x80)) {
			ctrlPressed = 1;
			shiftPressed = 1;
		}

		if (ctrlPressed && (lParam & IsReleased) && wParam == VK_CONTROL) {
			ctrlPressed = 0;
			ret = 1;
		}

		if (shiftPressed && (lParam & IsReleased) && wParam == VK_SHIFT) {
			shiftPressed = 0;
			ret = 1;
		}
	}

	// Check switch key
	UINT key1, key2;
	switch (pShMem->switchKey) {
	case CTRL_SHIFT_SW:
		key1 = VK_SHIFT;
		key2 = VK_CONTROL;
		break;
	default:
		key1 = VK_MENU;
		key2 = 'Z';
		break;
	}

	if (wParam == key1 || wParam == key2) {
		if (lParam & IsReleased) {
			if (PendingSwitch) {
				PendingSwitch = 0;
				SwitchMode();
				MessageBeep(MB_ICONASTERISK);
				return 1;
			}
		}
		else {
			if ((KeyState[key1] & 0x80) && (KeyState[key2] & 0x80))
				PendingSwitch = 1;
		}
	}
	else PendingSwitch = 0;

	return ret;
}

//-------------------------------------------------
LRESULT CALLBACK MyMouseHook(int code, WPARAM wParam, LPARAM lParam)
{
	if (pShMem->Initialized && code == HC_ACTION && wParam == WM_LBUTTONDOWN)
			ResetBuffer();
	return CallNextHookEx(pShMem->mouseHook,code,wParam,lParam);
}

//-------------------------------------------------
void SwitchMode()
{
	if (pShMem->vietKey) pShMem->vietKey = 0;
	else pShMem->vietKey = 1;
	if (pShMem->iconShown) ModifyStatusIcon();
	ClearClipboard();
}


//-------------------------------------------------
void ModifyStatusIcon()
{
    NOTIFYICONDATA tnid; 

    tnid.cbSize = sizeof(NOTIFYICONDATA);
    tnid.hWnd = pShMem->hMainDlg;
	tnid.uID = ID_VIETKEY_ICON;
	if (pShMem->vietKey) {
		tnid.hIcon = pShMem->hVietIcon;
		lstrcpy(tnid.szTip,_TEXT("Click to turn off Vienamese mode"));

	} else {
		tnid.hIcon = pShMem->hEnIcon;
		lstrcpy(tnid.szTip,_TEXT("Click to turn on Vienamese mode"));
	}
    tnid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP; 
    tnid.uCallbackMessage = pShMem->iconMsgId;
	Shell_NotifyIcon(pShMem->iconShown?NIM_MODIFY:NIM_ADD, &tnid);
	pShMem->iconShown = 1;
} 

//-------------------------------------------------
void DeleteStatusIcon()
{
    NOTIFYICONDATA tnid; 
	if (pShMem->iconShown) {
		tnid.cbSize = sizeof(NOTIFYICONDATA); 
		tnid.hWnd = pShMem->hMainDlg;
		tnid.uID = ID_VIETKEY_ICON;
		Shell_NotifyIcon(NIM_DELETE, &tnid); 
		pShMem->iconShown = 0;
	}
}

//-------------------------------------------------
int IsVietnamese()
{
	return pShMem->vietKey;
}

//-------------------------------------------------
void SetKeyMode(WORD mode, int inMethod, CodeInfo *pTable)
{
	pShMem->keyMode = mode;
	pShMem->inMethod = inMethod;
	pShMem->codeTable = *pTable;
	ResetBuffer();
}

//-------------------------------------------------
void ClearClipboard()
{
	ClipboardIsEmpty = 1;
//	OpenClipboard(NULL);
//	EmptyClipboard();
//	CloseClipboard();
}


//-------------------------------------------------
void ResetBuffer()
{
	VnKbd.clearBuf();
	ClearClipboard();
}


//-------------------------------------------------
void SetSwitchKey(int swKey)
{
	pShMem->switchKey = swKey;
}

//-------------------------------------------------
void SetInputMethod(int method, DWORD *DT)
{
	pShMem->inMethod = method;
	memcpy(pShMem->codeTable.DT, DT, sizeof(pShMem->codeTable.DT));
	ResetBuffer();
}


//-------------------------------------------------
HINSTANCE GetVietHookDll()
{
	return MyInstance;
}

//-------------------------------------------------
int CheckShortcuts(WPARAM wParam, LPARAM lParam)
{
	if (!ClipboardIsEmpty)
		return 0;

	int ctrlPressed = KeyState[VK_CONTROL] & 0x80;
	int shiftPressed = KeyState[VK_SHIFT] & 0x80; 


	UINT msg = 0;
	WPARAM wNotify = 0;
	LPARAM lNotify = 0;
	static int shortcutActivated = 0;

	if (!(lParam & IsReleased) && ctrlPressed && shiftPressed) {
		switch (wParam) {
		case VK_F5:
			msg = WM_HOOK_PANEL_SHORTCUT;
			break;
		case VK_F6:
			msg = WM_HOOK_TOOLKIT_SHORTCUT;
			break;
		case VK_F1:
			// first charset, Unicode
			msg = WM_HOOK_CHANGE_CHARSET;
			wNotify = 0;
			break;
		case VK_F2:
			// second charset, TCVN3
			msg = WM_HOOK_CHANGE_CHARSET;
			wNotify = 1;
			break;
		case VK_F3:
			// second charset, TCVN3
			msg = WM_HOOK_CHANGE_CHARSET;
			wNotify = 2;
			break;
		case VK_F4:
			// second charset, TCVN3
			msg = WM_HOOK_CHANGE_CHARSET;
			wNotify = 3;
			break;
		case VK_F9: // Steal VK_F9, and the release of it will be processed.later
			return 1;
		}

		if (msg != 0) {
			if (msg == WM_HOOK_PANEL_SHORTCUT || msg == WM_HOOK_TOOLKIT_SHORTCUT) {
				BringWindowToTop(pShMem->hMainDlg);
				SetForegroundWindow(pShMem->hMainDlg);
			}
			else {
				MessageBeep(MB_ICONASTERISK);
			}
			PostMessage(pShMem->hMainDlg, msg, wNotify, lNotify);
			shortcutActivated = 1;
			return 1;
		}
		return 0;
	}

	if (lParam & IsReleased) {
		if (shortcutActivated &&
			  (wParam == VK_F1 || wParam == VK_F2 || wParam == VK_F3 || wParam == VK_F4)) {
			shortcutActivated = 0;
			return 1;
		}

		if (ctrlPressed && shiftPressed) {
			switch (wParam) {
			case VK_F9:
				msg = WM_HOOK_FLY_CONVERT;
				break;
			}
			if (msg != 0) {
				SendMessage(pShMem->hMainDlg, msg, wNotify, lNotify);
				MessageBeep(MB_ICONASTERISK);
				return 1;
			}
		}
	}
	return 0;
}

/*
HMODULE hUnicows = NULL;

HMODULE LoadUnicowsProc(void)
{
	OSVERSIONINFO osinfo;
	osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osinfo);

	if (hUnicows == NULL && osinfo.dwMajorVersion<5)
		hUnicows = LoadLibraryA("c:\\tools\\unikey\\test\\unicows.dll");
	return hUnicows;
}

void UnloadUnicowsProc()
{
	if (hUnicows != NULL)
		FreeLibrary(hUnicows);
}

*/

//-------------------------------------------------
SharedMem *GetSharedMem()
{
	return pShMem;
}

//-------------------------------------------------
void initDLL()
{
	pShMem->Initialized = 0; 
	// check unicode platform
	OSVERSIONINFO osinfo;
	osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osinfo);
	pShMem->unicodePlatform = (osinfo.dwMajorVersion > 4 || 
							   (osinfo.dwPlatformId == VER_PLATFORM_WIN32_NT && osinfo.dwMajorVersion ==4));
	pShMem->winMajorVersion = osinfo.dwMajorVersion;
	pShMem->winMinorVersion = osinfo.dwMinorVersion;
}