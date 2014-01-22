//---------------------------------------------------------------------------
// svctlui.cpp
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

#include "stdafx.h"						// Include project pre-compiled headers

#pragma warning(push, 4)				// Enable maximum compiler warnings

BEGIN_NAMESPACE(SVCTL)

//---------------------------------------------------------------------------
// Global Variables

DWORD		g_dwDisplayMode = SVCTL_DISPLAY_VERBOSE;	// Display mode flags
UIClass		g_uiClass;									// UI class object

//---------------------------------------------------------------------------
// SVCTL::DefaultUserResponse
//
// Determines what the user would have responded with when a UI component
// is not available to display
//
// Arguments :
//
//	uMsgType		- MessageBox() type codes that would have been used
//	bOptimistic		- Response optimism flag

const INT_PTR DefaultUserResponse(UINT uMsgType, bool bOptimistic)
{
	// Determine what type of buttons the message box would have had, and
	// select a default response for it.

	// NOTE : IDRETRY and IDTRYAGAIN are never returned -- doing so may
	// result in an infinite loop that the user could never terminate

	switch(uMsgType & 0x0000000F) {

		case MB_OK : return IDOK;

		case MB_OKCANCEL : return (bOptimistic) ? IDOK : IDCANCEL;

		case MB_ABORTRETRYIGNORE : return (bOptimistic) ? IDIGNORE : IDABORT;

		case MB_YESNOCANCEL : return (bOptimistic) ? IDYES : IDCANCEL;

		case MB_YESNO : return (bOptimistic) ? IDYES : IDNO;

		case MB_RETRYCANCEL : return IDCANCEL;

#if WINVER >= 0x0500

		case MB_CANCELTRYCONTINUE : return (bOptimistic) ? IDCONTINUE : IDCANCEL;

#endif // WINVER

	}

	return 0;				// <--- Invalid MessageBox() button code(s)
}

//---------------------------------------------------------------------------
// SVCTL::UseDefaultResponse
//
// Determines if a default user response should be used instead of
// displaying a user interface component
//
// Arguments :
//
//	uMsgType		- MessageBox() style flags indicating component options

const bool UseDefaultResponse(UINT uMsgType)
{
	DWORD dwDisplayLevel = SVCTL::GetDisplayMode();		// Get display level
	DWORD dwSeverity = (uMsgType & 0x000000F0);			// Get severity code

	// If the level is SVCTL_DISPLAY_SILENT, never display anything

	if((dwDisplayLevel & SVCTL_DISPLAY_SILENT) == SVCTL_DISPLAY_SILENT)
		return true;

	// If the level is SVCTL_DISPLAY_QUIET, only display warnings and errors

	else if((dwDisplayLevel & SVCTL_DISPLAY_QUIET) == SVCTL_DISPLAY_QUIET)
		return !((dwSeverity == MB_ICONERROR) || (dwSeverity == MB_ICONWARNING));

	else return false;				// Show the user interface component
}

//---------------------------------------------------------------------------
// SVCTL::List Implementation
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// List::AddItem
//
// Adds another string ponter to the contained array of list box items
//
// Arguments :
//
//	pszItem			- Pointer to the string to be added to the list box

const bool List::AddItem(LPCTSTR pszItem)
{
	size_t			cItems;				// Number of strings already in the list
	
	if(!pszItem) return false;			// Nothing to do -- NULL string ptr
	cItems = m_rgszItems.Length();		// Get current number of items

	// Attempt to reallocate the array so it can hold another string pointer

	if(!m_rgszItems.ReAllocate(cItems + 1)) return false;

	m_rgszItems[cItems] = pszItem;		// Copy the item string pointer
	return true;
}

//---------------------------------------------------------------------------
// List::Display
//
// Displays the list box, using the selected global SVCTL UI class object
//
// Arguments :
//
//	NONE

const INT_PTR List::Display(void) const
{
	// If the global SVCTL display level is set too high, use a default response

	if(SVCTL::UseDefaultResponse(m_uType)) 
		return SVCTL::DefaultUserResponse(m_uType, m_bOptimistic);

	// Otherwise, call through to the user interface class to show the listbox
	
	else return g_uiClass.List(m_uType, m_bOptimistic, *this, m_rgszItems, 
		m_rgszItems.Length(), m_strFooter);
}

//---------------------------------------------------------------------------
// List::Reset
//
// Resets the contents of the List object back to the initial state
//
// Arguments :
//
//	NONE

void List::Reset(void)
{
	Clear();					// Clear the contents of the header string
	ClearItems();				// Release the contents of the list items
	m_uType = MB_OK;			// Reset type code back to MB_OK;
	m_bOptimistic = false;		// Reset back to a pessimistic default
}

//---------------------------------------------------------------------------
// SVCTL::Message Implementation
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Message::Display
//
// Displays a message box by passing the contained information to the global
// SVCTL user interface class object
//
// Arguments :
//
//	NONE

const INT_PTR Message::Display(void) const
{
	// If the SVCTL display level is set too high, just use a default response
	
	if(SVCTL::UseDefaultResponse(m_uType)) 
		return SVCTL::DefaultUserResponse(m_uType, m_bOptimistic);

	// If there is no error code to be reported, just pass our text message
	// and the type codes along to the SVCTL user interface module

	if(m_dwError == ERROR_SUCCESS) 
		return g_uiClass.Message(m_uType, m_bOptimistic, *this);

	// Otherwise, add the error message text (if possible) to the end of
	// the message text before sending it along to the UI class

	else {

		String		strError;				// Error description text
		String		strMessage;				// Generated message text

		strError.LoadMessage(m_dwError);	// Load the error description
		strMessage = *this;					// Start with our current text

		// If the error code is smaller than 0x10000, use a decimal code,
		// otherwise convert the error code into hexadecimal
		
		if(m_dwError <= 0xFFFF) strMessage.AppendFormat(_T("\n\n%d: "), m_dwError);
		else strMessage.AppendFormat(_T("\n\n0x%08X: "), m_dwError);

		strMessage.Append(strError);		// Append the error description

		return g_uiClass.Message(m_uType, m_bOptimistic, strMessage);
	}
}

//---------------------------------------------------------------------------
// SVCTL::Progress Implementation
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Progress::Start
//
// Launches the progress box, based on the current contained information
//
// Arguments :
//
//	NONE

const bool Progress::Start(void) const
{ 
	DWORD dwDisplayLevel = SVCTL::GetDisplayMode();		// Get display level

	// We don't display progress boxes unless the display level is set
	// to SVCTL_DISPLAY_VERBOSE

	if((dwDisplayLevel & SVCTL_DISPLAY_VERBOSE) == SVCTL_DISPLAY_VERBOSE)
		return g_uiClass.StartProgress(*this, m_hevtCancel);

	else return true;
}

//---------------------------------------------------------------------------

END_NAMESPACE(SVCTL)

#pragma warning(pop)
