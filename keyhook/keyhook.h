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
#ifndef _KEYHOOK_H
#define _KEYHOOK_H 1
#include <windows.h>
#include "keycons.h"

struct CodeInfo {
	DWORD DT[256];
	unsigned char BD[12][6];
	unsigned char BK[8];
	unsigned char BW[6];
	unsigned char BT[6];
	WORD ToUniH[256];
	WORD ToUniL[256];
	int multiBytes; // 2 bytes charset (Unicode, VNI, ....)
	int encoding; //UCS-2, UTF-8, Reference
	int singleBackspace;
//	unsigned char remapLayout[256]; //used to remap Vietnamese layout to US layout
};

#define CTRL_SHIFT_SW 0
#define ALT_Z_SW 1

struct KeyboardOptions
{
	int freeMarking;
	int toneNextToVowel;
	int modernStyle;
	int macroEnabled;
	int useUnicodeClipboard;
	int alwaysMacro;
	int useIME;
};

struct HookMacroDef
{
	int keyOffset;
	int textOffset;
};

struct SharedMem {
	//states
	int Initialized;
	int vietKey;
	int iconShown;
	int switchKey;

	KeyboardOptions options;

	WORD keyMode;
	int inMethod;
	CodeInfo codeTable;

	// system
	HHOOK keyHook,mouseHook;
	HWND hMainDlg;
	UINT iconMsgId;
	HICON hVietIcon,hEnIcon;
	int unicodePlatform;
	DWORD winMajorVersion, winMinorVersion;

	// macro table
//	MacroDef macroTable[MAX_MACRO_ITEMS];
	HookMacroDef macroTable[MAX_MACRO_ITEMS];
	char macroMem[MACRO_MEM_SIZE];
	int macroCount;
};

//----------------------------------------------------------------
// Interface functions 
LRESULT CALLBACK MyKeyHook(int code, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MyMouseHook(int code, WPARAM wParam, LPARAM lParam);
HINSTANCE GetVietHookDll();

void SwitchMode();
void ModifyStatusIcon();
void DeleteStatusIcon();

int IsVietnamese();
//void InitSharedMem(SharedMem *pPara);
void SetInputMethod(int method, DWORD *DT);
void SetKeyMode(WORD mode, int inMethod, CodeInfo *pTable);
void SetSwitchKey(int swKey);
//void SetKeyOptions(int freeMarking, int toneNextToVowel, int modernStyle);

SharedMem *GetSharedMem();

#define WM_HOOK_TOOLKIT_SHORTCUT (WM_USER+102)
#define WM_HOOK_PANEL_SHORTCUT (WM_USER+103)
#define WM_HOOK_FLY_CONVERT (WM_USER+104)
#define WM_HOOK_CHANGE_CHARSET (WM_USER+105)

#endif
