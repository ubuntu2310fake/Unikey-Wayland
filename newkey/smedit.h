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

#ifndef __SMART_EDIT_CONTROL_H
#define __SMART_EDIT_CONTROL_H


LRESULT CALLBACK GenEditProc(
			HWND hWnd,      // handle to window
			UINT uMsg,      // message identifier
			WPARAM wParam,  // first message parameter
			LPARAM lParam   // second message parameter
			);


class CSmartEdit  //to enforce more control over edit control's behavior
{
protected:
	HWND m_hOwner;
	HINSTANCE m_hInstance;
	LONG m_prevWndProc; // previous window procedure


public:
	HWND m_hWnd;
	void init(HINSTANCE hInst, HWND hOwner, HWND hWnd);
	//~CSmartEdit();

	friend LRESULT CALLBACK ::GenEditProc(
			HWND hwnd,      // handle to window
			UINT uMsg,      // message identifier
			WPARAM wParam,  // first message parameter
			LPARAM lParam   // second message parameter
			);
protected:
	virtual LRESULT wndProc(
				HWND hwnd,      // handle to window
				UINT uMsg,      // message identifier
				WPARAM wParam,  // first message parameter
				LPARAM lParam   // second message parameter
				);

private:
	void attachWindow(HWND hWnd);

};



#endif
