//---------------------------------------------------------------------------
// svctlmui.h
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

#ifndef __SVCTLMUI_H_
#define __SVCTLMUI_H_
#pragma once

#pragma warning(push, 4)				// Enable maximum compiler warnings

//---------------------------------------------------------------------------
// Classes declared in this unit:
//
//	SVCTL::MinimalUI
//---------------------------------------------------------------------------

BEGIN_NAMESPACE(SVCTL)

//---------------------------------------------------------------------------
// Class SVCTL::MinimalUI
//
// MinimalUI provides the minimal user interface services for SVCTL, which
// consists of message boxes and an unremarkably boring progress dialog box
//---------------------------------------------------------------------------

class MinimalUI : public UserInterface
{
public:

	//-----------------------------------------------------------------------
	// Constructor / Destructor

	MinimalUI() : m_bActive(false), m_hProgress(NULL), m_dwProgressId(0) {}
	~MinimalUI() { DeActivate(); }

	//-----------------------------------------------------------------------
	// SVCTL::UserInterface

	DWORD Activate(LPCTSTR pszTitle = NULL) 
		{ m_strTitle = pszTitle; m_bActive = true; return ERROR_SUCCESS; }

	void DeActivate(void) { StopProgress(); m_bActive = false; }

	const INT_PTR List(UINT uType, bool bOptimistic, LPCTSTR pszHeader, 
		LPCTSTR *rgszItems, size_t cItems, LPCTSTR pszFooter);

	const INT_PTR Message(UINT uType, bool bOptimistic, LPCTSTR pszText);

	const bool StartProgress(LPCTSTR pszText, HANDLE hevtCancel = NULL);

	void StopProgress(void);

private:

	MinimalUI(const MinimalUI &rhs);				// Disable copy
	MinimalUI& operator=(const MinimalUI &rhs);		// Disable assignment

	//-----------------------------------------------------------------------
	// Private Type Declarations

	typedef struct ProgressInfo {

		HANDLE		hevtReady;			// Event to signal when queue is ready
		LPCTSTR		pszCaption;			// Progress dialog box caption
		LPCTSTR		pszText;			// Text to be displayed in the dialog
		HANDLE		hevtCancel;			// Optional cancellation event object
	};

	//-----------------------------------------------------------------------
	// Private Member Functions

	static INT_PTR CALLBACK ProgressDialogProc(HWND hwnd, UINT uMsg, 
		WPARAM wParam, LPARAM lParam);
	
	static DWORD WINAPI ProgressThreadProc(void *pvArg);

	//-----------------------------------------------------------------------
	// Member Variables

	bool			m_bActive;			// Flag if Activate() has been called
	String			m_strTitle;			// Dialog box caption string
	HANDLE			m_hProgress;		// Progress dialog box thread handle
	DWORD			m_dwProgressId;		// Progress dialog box thread ID
};

//---------------------------------------------------------------------------

END_NAMESPACE(SVCTL)

#pragma warning(pop)

#endif