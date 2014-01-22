//---------------------------------------------------------------------------
// svctlgui.cpp
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

BEGIN_NAMESPACE(SVCTL)

//---------------------------------------------------------------------------
// SVCTL::GraphicalUI Implementation
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// GraphicalUI Constructor
//
// Arguments :
//
//	NONE

GraphicalUI::GraphicalUI() : m_hThread(NULL), m_dwThreadId(0), m_hwnd(NULL)
{
	// Create the kernel objects required by the rest of the class

	m_hevtLock = CreateEvent(NULL, FALSE, TRUE, NULL);
	m_hevtReady = CreateEvent(NULL, FALSE, FALSE, NULL);
}

//---------------------------------------------------------------------------
// GraphicalUI::Activate
//
// Activates the graphical user interface module by creating the hidden
// window object and the thread that it lives on
//
// Arguments :
//
//	pszTitle		- Optional application name to use as the window title

DWORD GraphicalUI::Activate(LPCTSTR pszTitle)
{
	DWORD			dwResult;			// Result from function call
	
	// Wait for the lock object to become ready.  The only way this can 
	// fail is if the event object was not properly constructed

	if(!LockWindow(true)) return ERROR_INVALID_HANDLE;

	// If the worker thread handle exists, the base window is already active

	if(m_hThread) { UnlockWindow(); return ERROR_ALREADY_INITIALIZED; }

	// Verify that the READY event object was properly created

	if(!m_hevtReady) { UnlockWindow(); return ERROR_INVALID_HANDLE; }

	// Set up the string that will be used for all window captions

	if(pszTitle) m_strTitle = pszTitle;
	else m_strTitle.LoadResource(SVCTL_IDSGUI_DEFTITLE);
	
	// Attempt to create the worker thread that runs the hidden base window

	m_hThread = SVCTL::CreateThread(NULL, 0, BaseWindowThreadProc, this, 0,
		&m_dwThreadId);
	if(!m_hThread) { UnlockWindow(); return GetLastError(); }

	// Wait for the thread to signal the READY event, or for it to terminate early

	HANDLE rghWait[] = { m_hThread, m_hevtReady };

	if(WaitForMultipleObjects(2, rghWait, FALSE, INFINITE) == WAIT_OBJECT_0) {

		GetExitCodeThread(m_hThread, &dwResult);	// Retrieve the error code
		
		CloseHandle(m_hThread);				// Close the thread handle
		m_hThread = NULL;					// Reset the thread handle
		m_dwThreadId = 0;					// Reset the thread ID code

		UnlockWindow();						// Release the window lock
		return dwResult;					// Return the error code
	}

	UnlockWindow();							// Release the window lock
	return ERROR_SUCCESS;					// Base window successfully activated
}

//---------------------------------------------------------------------------
// GraphicalUI::BaseWindowThread (private)
//
// Main entry point for the worker thread that manages the hidden window
//
// Arguments :
//
//	NONE

DWORD GraphicalUI::BaseWindowThread(void)
{
	HINSTANCE		hInstance;			// Application instance handle
	String			strWndClass;		// Window class string
	WNDCLASS		wndclass;			// Window class information
	RECT			rect;				// Window bounding rectangle
	MSG				msg;				// Window message structure

	// Retrieve the module instance handle and load the window class string

	hInstance = GetModuleHandle(NULL);
	strWndClass.LoadResource(SVCTL_IDSGUI_WNDCLASS, hInstance);
	
	// Initialize the WNDCLASS structure used to register the hidden window

	ZeroMemory(&wndclass, sizeof(WNDCLASS));
	wndclass.style = CS_NOCLOSE;
	wndclass.lpfnWndProc = DefWindowProc;
	wndclass.hInstance = hInstance;
	wndclass.lpszClassName = strWndClass;

	// Attempt to register the window class for this application instance

	if(!RegisterClass(&wndclass)) return GetLastError();

	// Attempt to create the hidden base window object

	m_hwnd = CreateWindowEx(WS_EX_TOOLWINDOW, strWndClass, m_strTitle,
		WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL,
		NULL, NULL, NULL);
	if(!m_hwnd) return GetLastError();

	// Center the hidden window on the user's desktop so that all of our messages
	// and dialog boxes appear relative to that location

	GetWindowRect(m_hwnd, &rect);

	SetWindowPos(m_hwnd, NULL,
		((GetSystemMetrics(SM_CXSCREEN) - (rect.right - rect.left)) / 2),
		((GetSystemMetrics(SM_CYSCREEN) - (rect.bottom - rect.top)) / 2),
		0, 0, SWP_NOSIZE | SWP_NOCOPYBITS | SWP_NOREDRAW);

	SetForegroundWindow(m_hwnd);			// Try to move into the foreground

	// Get the thread's message queue up and running and signal the READY event
	// so that the main thread can now continue execution

	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
	SetEvent(m_hevtReady);

	// Continually process window messages until WM_QUIT has been received

	while(GetMessage(&msg, NULL, 0, 0)) {

		TranslateMessage(&msg);				// Translate the window message
		DispatchMessage(&msg);				// Dispatch the window message
	}

	DestroyWindow(m_hwnd);					// Destroy the hidden window object
	m_hwnd = NULL;							// Reset the window handle

	// Although it's not really necessary, de-register the WNDCLASS as well

	UnregisterClass(strWndClass, hInstance);
	
	return ERROR_SUCCESS;
}

//---------------------------------------------------------------------------
// GraphicalUI::DeActivate
//
// Deactivates the graphical user interface, and destroys the hidden base
// window object
//
// Arguments :
//
//	NONE

void GraphicalUI::DeActivate(void)
{
	if(!LockWindow(true)) return;			// Wait for the lock object

	if(!m_hThread) { UnlockWindow(); return; }	// Window not activated

	// Post a WM_QUIT message to the base window, and wait for the worker
	// thread to terminate itself

	PostThreadMessage(m_dwThreadId, WM_QUIT, 0, 0);
	WaitForSingleObject(m_hThread, INFINITE);

	CloseHandle(m_hThread);					// Close the thread handle
	m_hThread = NULL;						// Reset the thread handle
	m_dwThreadId = 0;						// Reset the thread ID code

	m_strTitle.Clear();						// Clear out the caption string

	UnlockWindow();							// Release the window lock
}

//---------------------------------------------------------------------------
// GraphicalUI::List

const INT_PTR GraphicalUI::List(UINT uType, LPCTSTR pszHeader, LPCTSTR *rgszItems,
								DWORD cItems, LPCTSTR pszFooter)
{
	return IDCANCEL;
}

//---------------------------------------------------------------------------
// GraphicalUI::LockWindow (private)
//
// Locks the underlying base window object in place so it cannot be destroyed
// in the middle of a user interface operation
//
// Arguments :
//
//	bMaster			- Flag indicating whether this is a "master" lock or not

const bool GraphicalUI::LockWindow(bool bMaster)
{
	// If the caller is requesting a master lock, we wait forever for the 
	// lock to become availble, and we don't care about m_hwnd

	if(bMaster) return (WaitForSingleObject(m_hevtLock, INFINITE) == WAIT_OBJECT_0);

	// Otherwise, we do not wait at all, and we also check m_hwnd for validity
	// before successfully granting the lock to the caller

	if(WaitForSingleObject(m_hevtLock, 0) != WAIT_OBJECT_0) return false;
	if(m_hwnd != NULL) { SetEvent(m_hevtLock); return false; }

	return true;
}

//---------------------------------------------------------------------------
// GraphicalUI::Message

const INT_PTR GraphicalUI::Message(UINT uType, LPCTSTR pszText)
{
	return MessageBox(m_hwnd, pszText, m_strTitle, uType | MB_SETFOREGROUND);

	//return IDCANCEL;
}

//---------------------------------------------------------------------------
// GraphicalUI::StartProgress

const bool GraphicalUI::StartProgress(LPCTSTR pszText, HANDLE hevtCancel)
{
	return false;
}

//---------------------------------------------------------------------------
// GraphicalUI::StopProgress

void GraphicalUI::StopProgress(void)
{
}

//---------------------------------------------------------------------------
// GraphicalUI::UnlockWindow (private)
//
// Releases a base window lock obtained with LockWindow()
//
// Arguments :
//
//	NONE

inline void GraphicalUI::UnlockWindow(void)
{
	if(m_hevtLock) SetEvent(m_hevtLock);	// Reset the event to signaled state
}

//---------------------------------------------------------------------------

END_NAMESPACE(SVCTL)