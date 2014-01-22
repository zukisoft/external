//---------------------------------------------------------------------------
// svctlmui.cpp
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
#pragma warning(disable:4127)			// "conditional expression is constant"
#pragma warning(disable:4244)			// "conversion from x to x, possible loss of data"
#pragma warning(disable:4312)			// "conversion from x to x of greater size"

BEGIN_NAMESPACE(SVCTL)

//---------------------------------------------------------------------------
// Constants

const TCHAR		MINUI_LIST_BULLET[]		= _T("- ");
const TCHAR		MINUI_CRLF[]			= _T("\n");
const TCHAR		MINUI_DOUBLE_CRLF[]		= _T("\n\n");

//---------------------------------------------------------------------------
// SVCTL::MinimalUI Implementation
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// MinimalUI::List
//
// Displays a list box as a message box with bulleted items
//
// Arguments :
//
//	uType			- MessageBox() style button codes
//	bOptimistic		- Flag indicating optimistic response when unavailable
//	pszHeader		- Listbox header text (above the list items)
//	rgszItems		- Array of list item strings
//	cItems			- Number of list item pointers in rgszItems
//	pszFooter		- Optional footer text (below the list items)

const INT_PTR MinimalUI::List(UINT uType, bool bOptimistic, LPCTSTR pszHeader,
							  LPCTSTR *rgszItems, size_t cItems, LPCTSTR pszFooter)
{
	String		strMessage;				// String for the message box
	size_t		dwIndex;				// Loop index variable

	// If the user interface has not been activated, return a default response

	if(!m_bActive) return SVCTL::DefaultUserResponse(uType, bOptimistic);
	
	strMessage = pszHeader;				// Start with the header

	// If rgszItems is non-NULL and cItems is > 0, there are list items to display

	if(rgszItems && cItems) {
		
		strMessage += MINUI_DOUBLE_CRLF;			// Skip a blank line first

		// Loop through the array of list items and add them to the message string

		for(dwIndex = 0; dwIndex < cItems; dwIndex++) {

			if(rgszItems[dwIndex]) {

				strMessage += MINUI_LIST_BULLET;	// Append the bullet and whitespace
				strMessage += rgszItems[dwIndex];	// Add the list item text string
				strMessage += MINUI_CRLF;			// Append a newline character
			}
		}
	}

	// If a footer text string was specified, add that to the message string

	if(pszFooter) {

		strMessage += MINUI_DOUBLE_CRLF;		// Skip a blank line
		strMessage += pszFooter;				// Append the footer text
	}

	// Use our very own Message() function to display the list to the user

	return Message(uType, bOptimistic, strMessage);
}

//---------------------------------------------------------------------------
// MinimalUI::Message
//
// Displays a message box on the user's desktop (no parent window)
//
// Arguments :
//
//	uType			- MessageBox() style button codes
//	bOptimistic		- Flag indicating optimistic response when unavailable
//	pszText			- Text string to be displayed in the message box

const INT_PTR MinimalUI::Message(UINT uType, bool bOptimistic, LPCTSTR pszText)
{
	INT_PTR		nResult = 0;			// Result from user input
	
	// If the user interface is active, attempt to display the message box

	if(m_bActive) nResult = MessageBox(0, pszText, m_strTitle, uType);

	// If MessageBox() succeeded, return that result.  Otherwise, select a default
	// response based on the button type and optimism information

	if(nResult != 0) return nResult;
	else return SVCTL::DefaultUserResponse(uType, bOptimistic);
}

//---------------------------------------------------------------------------
// MinimalUI::ProgressDialogProc (static, private)
//
// Dialog box callback procedure for the MinimalUI progress dialog box
//
// Arguments :
//
//	hwnd			- Dialog box window handle
//	uMsg			- Window message to be processed
//	wParam			- Window message WPARAM parameter
//	lParam			- Window message LPARAM parameter

INT_PTR CALLBACK MinimalUI::ProgressDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam,
											   LPARAM lParam)
{
	ProgressInfo*		pDialogInfo;		// Pointer to dialog information
	HANDLE				hevtCancel;			// Cancellation event object

	switch(uMsg) {

		// WM_INITDIALOG : Initialize the dialog box
	
		case WM_INITDIALOG :

			// Cast a pointer to the dialog box initialization parameters

			pDialogInfo = reinterpret_cast<ProgressInfo*>(lParam);
			_ASSERTE(pDialogInfo != NULL);

			// Change the dialog box caption to the provided caption string

			if(pDialogInfo->pszCaption) SetWindowText(hwnd, pDialogInfo->pszCaption);

			// Change the text message to the provided text message string

			if(pDialogInfo->pszText) SetWindowText(GetDlgItem(hwnd, 
				SVCTL_IDCMUI_PROGRESS_TEXT), pDialogInfo->pszText);

			// If a cancellation event was provided, enable the Cancel button
			// and save the event handle in the user-defined window long_ptr
			
			if(pDialogInfo->hevtCancel) {

				EnableWindow(GetDlgItem(hwnd, IDCANCEL), TRUE);
				SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pDialogInfo->hevtCancel));
			}

			return TRUE;				// Dialog box is initialized

		// WM_COMMAND : A dialog box control command has been issued

		case WM_COMMAND :

			if(LOWORD(wParam) == IDCANCEL) {

				// Immediately disable the Cancel button so it can't be pushed again

				EnableWindow(GetDlgItem(hwnd, IDCANCEL), FALSE);
				
				// Retrieve the cancellation event object handle from the window
				// data, and signal it if it's non-NULL

				hevtCancel = reinterpret_cast<HANDLE>(GetWindowLongPtr(hwnd, 
					GWLP_USERDATA));
				if(hevtCancel) SetEvent(hevtCancel);
			}

			return 0;
	}

	return FALSE;				// Did not process the dialog box message
}

//---------------------------------------------------------------------------
// MinimalUI::ProgressThreadProc (static, private)
//
// Implements the worker thread required to display the progress dialog box
//
// Arguments :
//
//	pvArg			- Argument passed into CreateThread()

DWORD WINAPI MinimalUI::ProgressThreadProc(void *pvArg)
{
	ProgressInfo*		pProgressInfo;		// Pointer to initialization info
	ProgressInfo		dialogInfo;			// Information to pass to dialog box
	String				strCaption;			// Dialog box caption string
	String				strText;			// Dialog box text message
	MSG					msg;				// Windows messaging structure
	HWND				hDialog;			// Dialog box window handle

	// Cast a pointer to the ProgressInfo structure passed as the argument

	pProgressInfo = reinterpret_cast<ProgressInfo*>(pvArg);
	_ASSERTE(pProgressInfo != NULL);

	// Copy the information to be passed to the dialog box out from the
	// initialization data, maintaining local copies of the strings

	dialogInfo.pszCaption = strCaption = pProgressInfo->pszCaption;
	dialogInfo.pszText = strText = pProgressInfo->pszText;
	dialogInfo.hevtCancel = pProgressInfo->hevtCancel;

	// Initialize the message queue for this thread and release the calling thread

	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
	if(pProgressInfo->hevtReady) SetEvent(pProgressInfo->hevtReady);

	// Create the modeless dialog box object, passing along the necessary information

	hDialog = CreateDialogParam(GetModuleHandle(NULL), 
		MAKEINTRESOURCE(SVCTL_IDDMUI_PROGRESS), 0, ProgressDialogProc, 
		reinterpret_cast<LPARAM>(&dialogInfo));

	// Continually process window messages until WM_QUIT is received
	
	while(GetMessage(&msg, NULL, 0, 0)) {

		if((hDialog == NULL) || (!IsDialogMessage(hDialog, &msg))) {

			TranslateMessage(&msg);			// Translate the window message
			DispatchMessage(&msg);			// Dispatch the window message
		}
	}

	if(hDialog) DestroyWindow(hDialog);		// Destroy the dialog box

	return ERROR_SUCCESS;
}

//---------------------------------------------------------------------------
// MinimalUI::StartProgress
//
// Launches a progress dialog box with the specified message text
//
// Arguments :
//
//	pszText			- Progress box text message
//	hevtCancel		- Optional event object used to cancel the operation

const bool MinimalUI::StartProgress(LPCTSTR pszText, HANDLE hevtCancel)
{
	WinHandle		hevtReady;			// Event thread signals when ready
	ProgressInfo	progressInfo;		// Progress thread information

	if(!m_bActive) return false;		// User interface is not active
	if(m_hProgress) return false;		// Progress is already active

	// Create the kernel event that the thread will signal when it's ready

	hevtReady = CreateEvent(NULL, FALSE, FALSE, NULL);
	if(!hevtReady) return false;

	progressInfo.hevtReady = hevtReady;		// Copy the ready event handle
	progressInfo.pszCaption = m_strTitle;	// Copy pointer to the caption
	progressInfo.pszText = pszText;			// Copy pointer to the text
	progressInfo.hevtCancel = hevtCancel;	// Copy cancellation event handle

	// Attempt to launch the progress dialog box worker thread, passing the
	// ProgressInfo structure as the initialization parameter

	m_hProgress = SVCTL::CreateThread(NULL, 0, ProgressThreadProc, &progressInfo,
		0, &m_dwProgressId);
	if(!m_hProgress) return false;

	WaitForSingleObject(hevtReady, INFINITE);	// Wait for thread to signal us

	return true;
}

//---------------------------------------------------------------------------
// MinimalUI::StopProgress
//
// Terminates a progress dialog box constructed with StartProgress
//
// Arguments :
//
//	NONE

void MinimalUI::StopProgress(void)
{
	if(m_hProgress) {

		_ASSERTE(m_dwProgressId != 0);		// Should never be zero here
		
		// Post a WM_QUIT message to the progress dialog box thread and wait
		// for the thread to terminate
		
		PostThreadMessage(m_dwProgressId, WM_QUIT, 0, 0);
		WaitForSingleObject(m_hProgress, INFINITE);

		CloseHandle(m_hProgress);			// Close the thread handle

		m_hProgress = NULL;					// Reset the thread handle to NULL
		m_dwProgressId = 0;					// Reset the thread ID code to zero
	}
}

//---------------------------------------------------------------------------

END_NAMESPACE(SVCTL)

#pragma warning(pop)