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
#include "font.h"

//---------------------------------------------------------------
// call-back function for enumerating fonts
//   lParam: the test function will set this data to font name
//---------------------------------------------------------------
static int FontFound = 0;
int CALLBACK myEnumFontCallback(
					ENUMLOGFONTEX *lpelfe,    // logical-font data
					NEWTEXTMETRICEX *lpntme,  // physical-font data
					DWORD FontType,           // type of font
					LPARAM lParam             // application-defined data
)
{
	if (_tcsicmp((const TCHAR *)lParam, lpelfe->elfLogFont.lfFaceName) == 0)
		FontFound = 1;
	return 0;
}

//----------------------------------------------------------------
int FontExists(const TCHAR *fontFace)
{
	LOGFONT lf;
	HWND hWndDesktop = GetDesktopWindow();
	HDC hDC = GetDC(hWndDesktop);
	FontFound = 0;
	lf.lfCharSet = DEFAULT_CHARSET; //any charset
	_tcscpy(lf.lfFaceName, fontFace);
	lf.lfPitchAndFamily = 0;
	EnumFontFamiliesEx(hDC, &lf, (FONTENUMPROC)myEnumFontCallback, (LPARAM)fontFace, 0);
	ReleaseDC(hWndDesktop, hDC);
	return (FontFound);
}

