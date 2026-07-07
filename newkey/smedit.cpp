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
#include "smedit.h"

LRESULT CALLBACK GenEditProc(
			HWND hWnd,      // handle to window
			UINT uMsg,      // message identifier
			WPARAM wParam,  // first message parameter
			LPARAM lParam   // second message parameter
			)
{
	CSmartEdit *pEdit = (CSmartEdit *)GetWindowLong(hWnd, GWL_USERDATA);
	if (pEdit)
		return pEdit->wndProc(hWnd, uMsg, wParam, lParam);
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

//-----------------------------------------------
// Attach the edit control to this CSmartEdit object
//-----------------------------------------------
void CSmartEdit::attachWindow(HWND hWnd)
{
	m_hWnd = hWnd;
	SetWindowLong(hWnd, GWL_USERDATA, (LONG)this);
	// Subclass the edit control
	m_prevWndProc = SetWindowLong(hWnd, GWL_WNDPROC, (LONG)GenEditProc);
}

//-----------------------------------------------
void CSmartEdit::init(HINSTANCE hInst, HWND hOwner, HWND hWnd)
{
	m_hInstance = hInst;
	m_hOwner = hOwner;
	attachWindow(hWnd);
}

//-------------------------------------------------------
LRESULT CSmartEdit::wndProc(
				HWND hWnd,      // handle to window
				UINT uMsg,      // message identifier
				WPARAM wParam,  // first message parameter
				LPARAM lParam)  // second message parameter
{
	return CallWindowProc((WNDPROC)m_prevWndProc, hWnd, uMsg, wParam, lParam);
}
