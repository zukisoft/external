//---------------------------------------------------------------------------
// svctlgui.h
//
// The contents of this file are subject to the Mozilla Public License
// Version 1.1 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/
//
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License.
//
// The Original Code is Windows Service Template Library.
//
// The Initial Developer of the Original Code is Michael G. Brehm.
// Portions created by the Initial Developer(s) are Copyright (C)2001-2007
// Michael G. Brehm. All Rights Reserved.
//
// Contributor(s):
//	Michael G. Brehm <michael.brehm@verizon.net> (original author)
//---------------------------------------------------------------------------

#ifndef __SVCTLGUI_H_
#define __SVCTLGUI_H_
#pragma once

//---------------------------------------------------------------------------
// Classes declared in this unit:
//
//	SVCTL::GraphicalUI
//---------------------------------------------------------------------------

BEGIN_NAMESPACE(SVCTL)

//---------------------------------------------------------------------------
// Class SVCTL::GraphicalUI
//
// GraphicalUI provides the GUI-based user interface services for SVCTL
//---------------------------------------------------------------------------

class GraphicalUI : public UserInterface
{
public:

	//-----------------------------------------------------------------------
	// Constructor / Destructor

	GraphicalUI();
	virtual ~GraphicalUI() { StopProgress(); DeActivate(); }

	//-----------------------------------------------------------------------
	// SVCTL::UserInterface

	DWORD Activate(LPCTSTR pszTitle = NULL);

	void DeActivate(void);

	const INT_PTR List(UINT uType, LPCTSTR pszHeader, LPCTSTR *rgszItems,
		DWORD cItems, LPCTSTR pszFooter);

	const INT_PTR Message(UINT uType, LPCTSTR pszText);

	const bool StartProgress(LPCTSTR pszText, HANDLE hevtCancel = NULL);

	void StopProgress(void);

private:

	GraphicalUI(const GraphicalUI &rhs);				// Disable copy
	GraphicalUI& operator=(const GraphicalUI &rhs);		// Disable assignment

	//-----------------------------------------------------------------------
	// Private Member Functions

	DWORD BaseWindowThread(void);
	
	const bool LockWindow(bool bMaster = false);

	void UnlockWindow(void);

	//---------------------------------------------------------------------------
	// Static Member Functions

	static DWORD WINAPI BaseWindowThreadProc(void *pv)
		{ return reinterpret_cast<GraphicalUI*>(pv)->BaseWindowThread(); }

	//-----------------------------------------------------------------------
	// Member Variables

	WinHandle       m_hevtLock;			// Window activation lock object
	HANDLE			m_hThread;			// Hidden window thread handle
	DWORD			m_dwThreadId;		// Hidden window thread ID code
	WinHandle		m_hevtReady;		// Thread ready notification event
	HWND			m_hwnd;				// Hidden window object handle
	String			m_strTitle;			// Dialog box caption string
};

//---------------------------------------------------------------------------

END_NAMESPACE(SVCTL)

#endif