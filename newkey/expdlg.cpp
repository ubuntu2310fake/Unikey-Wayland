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
#include "util.h"
#include "ExpDlg.h"

//------------------------------------------
int CExpandDialog::init(
		HINSTANCE hInst,
		LPCTSTR lpszTemplate,
		HWND hWndOwner, // parent window, could be NULL
		int nIDFrame,
		int nIDButton,
		int nIDFooter,
		LPCTSTR strExpand,
		LPCTSTR strContract,
		BOOL bAllowContract
		)
{
	CVirDialog::init(hInst, lpszTemplate, hWndOwner);
	_tcscpy(m_strExpand, strExpand);
	_tcscpy(m_strContract, strContract);
	m_nIDFrame = nIDFrame;
	m_nIDButton = nIDButton;
	m_nIDFooter = nIDFooter;
	m_bExpanded = TRUE;
	m_bAllowContract = bAllowContract;
	m_wasShown = FALSE;
	return TRUE;
}

//-------------------------------------------
BOOL CExpandDialog::onDialogExpanding(BOOL /*bExpanded*/) 
{
	return (TRUE);
}

//-------------------------------------------
void CExpandDialog::onDialogExpanded(BOOL /*bExpanded*/) 
{
	return;
}


//-------------------------------------------
BOOL CExpandDialog::onInitDialog() 
{
	CVirDialog::onInitDialog();
	// during initialization, before any windows are shown, shrink the
	// dialog box so that only the default portion is shown.
	HWND wndCtrl = GetDlgItem(m_hWnd, m_nIDFrame);
	ShowWindow(wndCtrl, SW_HIDE);
	wndCtrl = GetDlgItem(m_hWnd, m_nIDFooter);
	ShowWindow(wndCtrl, SW_HIDE);
	wndCtrl = GetDlgItem(m_hWnd, m_nIDButton);
	SetWindowText(wndCtrl, m_bExpanded? m_strContract : m_strExpand);

	m_wasShown = FALSE;
//	expandBox(TRUE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


//-------------------------------------------
BOOL CExpandDialog::onCommand(WPARAM wParam, LPARAM lParam) 
{
	HWND hwndCtl;
	int id;
	UINT codeNotify;

	// crack the WM_COMMAND message
	id = GET_WM_COMMAND_ID(wParam,lParam);
	hwndCtl = GET_WM_COMMAND_HWND(wParam,lParam);
	codeNotify = GET_WM_COMMAND_CMD(wParam,lParam);
	
	// if the "Advanced" button was clicked, then call the 
	// message handler.
	if ((id == m_nIDButton) && (codeNotify==BN_CLICKED))
	{
		onClickAdvanced();
	}

	return CVirDialog::onCommand(wParam, lParam);
}


//-------------------------------------------
void CExpandDialog::onClickAdvanced() 
{
	expand(!m_bExpanded);
}


//-------------------------------------------
BOOL CExpandDialog::expand(BOOL bExpand)
{
	BOOL bShouldExpand;

	// if the current state matches the flag passed into this
	// routine, then there is no need to do anything.  Note that
	// the virtual functions OnDialogExpanding and OnDialogExpanded
	// are not called.
	if (bExpand == m_bExpanded) return TRUE;

	// call the virtual function to see if it is OK to expand the dialog.
	bShouldExpand = onDialogExpanding(m_bExpanded);

	
	// if OnDialogExpanding returns OK (TRUE), then call the appropriate
	// functions to expand the dialog.
	if (bShouldExpand)
	{
		expandBox(bExpand);
		// call the notification for the post-expand
		onDialogExpanded(m_bExpanded);
	}

	return(m_bExpanded == bExpand);
}


//-------------------------------------------
void CExpandDialog::expandBox(BOOL fExpand)
{
	// if the dialog is already in the requested state, return
	// immediately.
	if (fExpand == m_bExpanded) return;

	RECT rcWnd, rcDefaultBox, rcChild, rcIntersection, rcFooter, rcTempFooter;
	HWND wndChild = NULL;
	HWND wndDefaultBox = NULL;

	GetWindowRect(m_hWnd, &rcWnd);

	// get the window of the button 
	HWND wndCtrl = GetDlgItem(m_hWnd, m_nIDButton);
	if (wndCtrl == NULL) return;

	wndDefaultBox = GetDlgItem(m_hWnd, m_nIDFrame);
	if (wndDefaultBox==NULL) return;

	// retrieve coordinates for the default child window
	GetWindowRect(wndDefaultBox, &rcDefaultBox);

	HWND wndFooter = GetDlgItem(m_hWnd, m_nIDFooter);
	if (wndFooter == NULL) return;
	GetWindowRect(wndFooter, &rcFooter);
	if (m_bExpanded)
		rcTempFooter = rcFooter;
	else {
		rcTempFooter.left = rcDefaultBox.left;
		rcTempFooter.top = rcDefaultBox.bottom;
		rcTempFooter.right = rcTempFooter.left + (rcFooter.right - rcFooter.left);
		rcTempFooter.bottom = rcTempFooter.top + (rcFooter.bottom - rcFooter.top);
	}

	// Calculate offsets to move the footer
	int dx, dy;
	dx = rcFooter.left - rcDefaultBox.left;
	dy = rcFooter.top - rcDefaultBox.bottom;
	if (!fExpand) {
		dx = -dx;
		dy = -dy;
	}
	dx = 0;

	// enable/disable all of the child window outside of the default box.
	wndChild = GetTopWindow(m_hWnd);
	
	POINT pt;
	for ( ; wndChild != NULL; wndChild = GetWindow(wndChild, GW_HWNDNEXT))
	{
		// get rectangle occupied by child window in screen coordinates.
		GetWindowRect(wndChild, &rcChild);
		if (!IntersectRect(&rcIntersection, &rcChild, &rcDefaultBox))
		{
			if ((!m_wasShown || IsWindowVisible(wndChild)) && 
				wndChild != wndFooter && 
				IntersectRect(&rcIntersection, &rcChild, &rcTempFooter)) {
				pt.x = rcChild.left + dx;
				pt.y = rcChild.top + dy;
				ScreenToClient(m_hWnd, &pt);
				SetWindowPos(
						wndChild,
						HWND_TOP,
						pt.x, //rcChild.left,
						pt.y, //rcChild.top,
						0,
						0,
						SWP_NOZORDER | SWP_NOSIZE);
				ShowWindow(wndChild, SW_SHOW);
			}
			else {
				//EnableWindow(wndChild, fExpand); //DEBUG
				ShowWindow(wndChild, fExpand? SW_SHOW : SW_HIDE);
			}
			//EnableWindow(wndChild, fExpand);
			//ShowWindow(wndChild, fExpand? SW_SHOW : SW_HIDE);
		}
	}

	if (!fExpand)  // we are contracting
	{
		if (!m_wasShown)
		{
			m_wasShown = 1;
			// this is the first time we are being called to shrink the dialog
			// box.  The dialog box is currently in its expanded size and we must
			// save the expanded width and height so that it can be restored
			// later when the dialog box is expanded.
			m_tsize.cx = rcWnd.right - rcWnd.left;
			m_tsize.cy = rcWnd.bottom - rcWnd.top;

			// we also hide the default box here so that it is not visible
			ShowWindow(wndDefaultBox, SW_HIDE);
		}
		// shrink the dialog box so that it encompasses everything from the top,
		// left up to and including the default box.
		SetWindowPos(m_hWnd, 
			HWND_TOP,
			0,
			0,
			rcDefaultBox.right - rcWnd.left, 
			rcDefaultBox.bottom - rcWnd.top + (rcFooter.bottom - rcFooter.top), // adding extra space for the footer
			SWP_NOZORDER|SWP_NOMOVE);

		SetWindowText(wndCtrl, m_strExpand);

		// record that the dialog is contracted.
		m_bExpanded = FALSE;
	}
	else // we are expanding
	{
		SetWindowPos(
			m_hWnd,
			HWND_TOP,
			0,
			0,
			m_tsize.cx,
			m_tsize.cy,
			SWP_NOZORDER|SWP_NOMOVE);

		// make sure that the entire dialog box is visible on the user's
		// screen.
		SendMessage(m_hWnd, DM_REPOSITION, 0, 0);

		// hide the footer
		ShowWindow(wndFooter, SW_HIDE);

		if (m_bAllowContract)
		{
			SetWindowText(wndCtrl, m_strContract);
		}
		else
		{
			//EnableWindow(wndCtrl, FALSE); //debug
		}

		// record that the dialog is expanded
		m_bExpanded = TRUE;
	}
}
