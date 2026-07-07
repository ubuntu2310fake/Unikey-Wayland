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

#ifndef __EXPANDING_DIALOG_BOX_H
#define __EXPANDING_DIALOG_BOX_H


/*--------------------------------------------------------------------------
  Class:    CExpandDialog

  Summary:  Allows dialogs to be expanded/contracted
----------------------------------------------------------------------------*/
class CExpandDialog: virtual public CVirDialog
{
public:
	virtual BOOL onInitDialog();

	int init(
		HINSTANCE hInst,
		LPCTSTR lpszTemplate,
		HWND hWndOwner, // parent window, could be NULL
		int nIDFrame,
		int nIDButton,
		int nIDFooter,
		LPCTSTR strExpand   = _T("Advanced >>"),
		LPCTSTR strContract = _T("Advanced <<"),
		BOOL bAllowContract = TRUE
		);

public:
	// a public function so that the dialog can query whether or not
	// we are in "advanced" mode.
	BOOL isExpanded() const {return m_bExpanded;};
	
	// allow the user to expand or contract the dialog whenever they
	// please.  This is only needed in extreme circumstances, when the
	// dialog should expand or contract based on something else besides
	// the "Advanced" button.
	BOOL expand(BOOL bExpand);

	// a virtual notification function so that the expanding or contracting
	// can be aborted if need be.  The function should return TRUE if the
	// expanding should happen, or FALSE if it should not.
	virtual BOOL onDialogExpanding(BOOL bExpanded);

	// a virtual notification function that is called after the expansion
	// takes place
	virtual void onDialogExpanded(BOOL bExpanded);

	// Overrides
	virtual BOOL onCommand(WPARAM wParam, LPARAM lParam);

protected:
	// property data
	TCHAR m_strExpand[64];		// string to put on the expand/contract button, when
								// the dialog is currently contracted.
	TCHAR m_strContract[64];	// string to pu on the expand/contract button, when
								// the dialog is currently expanded.
	int m_nIDFrame;			// resource ID of the frame that defines the contracted
							// dimensions of the dialog.
	int m_nIDButton;		// resource ID of the expand/contract button on the 
							// dialog
	BOOL m_bAllowContract;	// whether or not to allow the user to contract the dialog
							// after expanding it for the first time.

	SIZE m_tsize;
	BOOL m_bExpanded;		// records whether the dialog is currently expanded or not.
	BOOL m_wasShown;		// records whether the dialog was shown before

	int m_nIDFooter;		// ID of the footer control

private:
	void expandBox(BOOL fExpand);
	void onClickAdvanced();
};

#endif
