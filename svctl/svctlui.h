//---------------------------------------------------------------------------
// svctlui.h
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

#ifndef __SVCTLUI_H_
#define __SVCTLUI_H_
#pragma once

#pragma warning(push, 4)				// Enable maximum compiler warnings
#pragma warning(disable:4100)			// "unreferenced formal parameter"

//---------------------------------------------------------------------------
// BUILD OPTIONS
//---------------------------------------------------------------------------

// If no user interface option has been selected, go with SVCTL_MIN_UI

#if (!defined SVCTL_CONSOLE_UI) && (!defined SVCTL_GRAPHICAL_UI) && (!defined SVCTL_MIN_UI)
#define SVCTL_MIN_UI
#endif

//---------------------------------------------------------------------------
// Classes declared in this unit:
//
//	SVCTL::UserInterface		(abstract base class)
//  SVCTL::List
//	SVCTL::Message
//	SVCTL::Progress
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// SVCTL Display Level Codes

#define SVCTL_DISPLAY_VERBOSE		0x0001		// Everything pops up
#define SVCTL_DISPLAY_QUIET			0x0002		// Only warnings and errors
#define SVCTL_DISPLAY_SILENT		0x0004		// No output at all

BEGIN_NAMESPACE(SVCTL) 

//---------------------------------------------------------------------------
// Function Prototypes

DWORD		ActivateUserInterface(LPCTSTR pszTitle = NULL);
DWORD		ActivateUserInterface(UINT uResTitle);
void			DeActivateUserInterface(void);
const INT_PTR	DefaultUserResponse(UINT uMsgType, bool bOptimistic);
DWORD		GetDisplayMode(void);
DWORD		SetDisplayMode(DWORD dwNewMode);
const bool		UseDefaultResponse(UINT uMsgType);

//---------------------------------------------------------------------------
// Class SVCTL::UserInterface
//
// UserInterface provides the interface that all of the SVCTL user interface
// classes (MinimalUI, ConsoleUI, GraphicalUI) must implement
//---------------------------------------------------------------------------

class __declspec(novtable) UserInterface
{
public:

	//-----------------------------------------------------------------------
	// Virtual Member Functions

	virtual DWORD Activate(LPCTSTR pszTitle) = 0;

	virtual void DeActivate(void) = 0;

	virtual const INT_PTR List(UINT uType, bool bOptimistic, LPCTSTR pszHeader, 
		LPCTSTR *rgszItems, size_t cItems, LPCTSTR pszFooter) = 0;

	virtual const INT_PTR Message(UINT uType, bool bOptimistic, LPCTSTR pszText) = 0;

	virtual const bool StartProgress(LPCTSTR pszText, HANDLE hevtCancel) = 0;

	virtual void StopProgress(void) = 0;
};

//---------------------------------------------------------------------------
// Include User Interface Headers
//---------------------------------------------------------------------------

END_NAMESPACE(SVCTL)			// <--- Leave SVCTL namespace to #include

#include "svctlmui.h"				// Include MinimalUI class declarations
#include "svctlcui.h"				// Include ConsoleUI class declarations
#include "svctlgui.h"				// Include GraphicalUI class declarations

BEGIN_NAMESPACE(SVCTL)			// <--- Reenter the SVCTL namespace

//---------------------------------------------------------------------------
// Select User Interface Class
//---------------------------------------------------------------------------

#pragma message("TODO -- Fix SVCTL_GRAPHICAL_UI -- it's broken")

#if (defined SVCTL_MIN_UI)

//#pragma comment(lib, "svctlmrc.lib")	// Link with MinimalUI resources
typedef MinimalUI UIClass;				// Select MinimalUI as the user interface

#elif (defined SVCTL_CONSOLE_UI)

//#pragma comment(lib, "svctlcrc.lib")	// Link with ConsoleUI resources
typedef ConsoleUI UIClass;				// Select ConsoleUI as the user interface

//#elif (defined SVCTL_GRAPHICAL_UI)

//#pragma comment(lib, "svctlgrc.lib")	// Link with GraphicalUI resources
typedef GraphicalUI UIClass;			// Select GraphicalUI as the user interface

#else

#error no SVCTL user interface class can be selected

#endif	// UIClass declaration

//---------------------------------------------------------------------------
// Global Variables

extern DWORD	g_dwDisplayMode;	// Global SVCTL display mode variable
extern UIClass	g_uiClass;			// Global SVCTL user interface class object

//---------------------------------------------------------------------------
// Class SVCTL::List
//
// The List class wraps the functionality of a list box, which is ultimately
// displayed by the selected SVCTL user interface class
//---------------------------------------------------------------------------

class List : public String
{
public:

	//-----------------------------------------------------------------------
	// Constructors / Destructor

	List() : m_uType(MB_OK), m_bOptimistic(false) {}
	List(LPCTSTR rhs) : String(rhs), m_uType(MB_OK), m_bOptimistic(false) {}

	~List() {}

	//-----------------------------------------------------------------------
	// Overloaded Operators

	const List& operator=(const String &rhs) { String::operator=(rhs); return *this; }
	const List& operator=(LPCTSTR rhs) { String::operator=(rhs); return *this; }
	const List& operator=(TCHAR rhs) { String::operator=(rhs); return *this; }

	//-----------------------------------------------------------------------
	// Member Functions

	const bool AddItem(LPCTSTR pszItem);
	
	void ClearItems(void) { m_rgszItems.Free(); }

	const INT_PTR Display(void) const;

	String& GetFooter(void) { return m_strFooter; }

	void Reset(void);
	
	void SetOptimistic(bool bOptimistic) { m_bOptimistic = bOptimistic; }
	
	void SetType(UINT uType) { m_uType = uType; }

	//-----------------------------------------------------------------------
	// Properties

	__declspec(property(get=GetFooter))			String&		Footer;
	__declspec(property(put=SetOptimistic))		bool		Optimistic;
	__declspec(property(put=SetType))			UINT		Type;

private:

	List(const List& rhs);					// Disable copy
	List& operator=(const List& rhs);		// Disable assignment

	//-----------------------------------------------------------------------
	// Member Variables

	bool				m_bOptimistic;		// Default response optimism
	UINT				m_uType;			// List box type codes
	Buffer<LPCTSTR>		m_rgszItems;		// Array of item string pointers
	String				m_strFooter;		// List box footer text
};

//---------------------------------------------------------------------------
// Class SVCTL::Message
//
// The Message class wraps the functionality of a message box, which is
// ultimately displayed by the selected SVCTL user interface class
//---------------------------------------------------------------------------

class Message : public String
{
public:

	//-----------------------------------------------------------------------
	// Constructors / Destructor

	Message() : m_dwError(ERROR_SUCCESS), m_uType(MB_OK), m_bOptimistic(false) {}
	
	Message(LPCTSTR rhs) : String(rhs), m_dwError(ERROR_SUCCESS), m_uType(MB_OK),
		m_bOptimistic(false) {}

	~Message() {}

	//-----------------------------------------------------------------------
	// Overloaded Operators

	const Message& operator=(const String &rhs) { String::operator=(rhs); return *this; }
	const Message& operator=(LPCTSTR rhs) { String::operator=(rhs); return *this; }
	const Message& operator=(TCHAR rhs) { String::operator=(rhs); return *this; }

	//-----------------------------------------------------------------------
	// Member Functions

	const INT_PTR Display(void) const;

	void Reset(void);

	void SetErrorCode(DWORD dwError) { m_dwError = dwError; }

	void SetOptimistic(bool bOptimistic) { m_bOptimistic = bOptimistic; }

	void SetType(UINT uType) { m_uType = uType; }

	//-----------------------------------------------------------------------
	// Properties

	__declspec(property(put=SetErrorCode))		DWORD	ErrorCode;
	__declspec(property(put=SetOptimistic))		bool	Optimistic;
	__declspec(property(put=SetType))			UINT	Type;

private:

	Message(const Message& rhs);				// Disable copy
	Message& operator=(const Message& rhs);		// Disable assignment

	//-----------------------------------------------------------------------
	// Member Variables

	bool			m_bOptimistic;			// Default response optimism
	DWORD			m_dwError;				// Error code (optional)
	UINT			m_uType;				// Message box type
};

//---------------------------------------------------------------------------
// Class SVCTL::Progress
//
// The Progress class wraps the functionality of a progress box, which is
// ultimately displayed by the selected SVCTL user interface class
//---------------------------------------------------------------------------

class Progress : public String
{
public:

	//-----------------------------------------------------------------------
	// Constructors / Destructor

	Progress() : m_hevtCancel(NULL) {}
	Progress(LPCTSTR rhs) : String(rhs), m_hevtCancel(NULL) {}

	virtual ~Progress() {}

	//-----------------------------------------------------------------------
	// Overloaded Operators

	const Progress& operator=(const String &rhs) { String::operator=(rhs); return *this; }
	const Progress& operator=(LPCTSTR rhs) { String::operator=(rhs); return *this; }
	const Progress& operator=(TCHAR rhs) { String::operator=(rhs); return *this; }

	//-----------------------------------------------------------------------
	// Member Functions

	const HANDLE GetCancelEvent(void) const { return m_hevtCancel; }

	void SetCancelEvent(HANDLE hevtCancel) { m_hevtCancel = hevtCancel; }
	
	const bool Start(void) const; 

	void Stop(void) const { g_uiClass.StopProgress(); }

	//-----------------------------------------------------------------------
	// Properties

	__declspec(property(get=GetCancelEvent,put=SetCancelEvent)) HANDLE CancelEvent;

private:

	Progress(const Progress& rhs);					// Disable copy
	Progress& operator=(const Progress& rhs);		// Disable assignment

	//-----------------------------------------------------------------------
	// Member Variables

	HANDLE			m_hevtCancel;			// Cancellation event handle
};

//---------------------------------------------------------------------------
// Inlined Global Functions
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// SVCTL::ActivateUserInterface (inline)
//
// Activates the global SVCTL user interface class object
//
// Arguments :
//
//	pszTitle		- Optional application title string

inline DWORD ActivateUserInterface(LPCTSTR pszTitle)
{
	return g_uiClass.Activate(pszTitle);	// Activate the user interface
}

//---------------------------------------------------------------------------
// SVCTL::ActivateUserInterface (inline)
//
// Activates the global SVCTL user interface class object
//
// Arguments :
//
//	uResTitle		- Resource identifier of the application title string

inline DWORD ActivateUserInterface(UINT uResTitle)
{
	ResString	strTitle(uResTitle);		// Load the resource string
	
	return g_uiClass.Activate(strTitle);	// Activate the user interface
}

//---------------------------------------------------------------------------
// SVCTL::DeActivateUserInterface (inline)
//
// Deactivates the global SVCTL user interface class object
//
// Arguments :
//
//	NONE

inline void DeActivateUserInterface(void)
{
	g_uiClass.DeActivate();			// Deactivate the user interface class
}

//---------------------------------------------------------------------------
// SVCTL::GetDisplayMode (inline)
//
// Retrieves the module-wide user interface display mode variable
//
// Arguments :
//
//	NONE

inline DWORD GetDisplayMode(void) 
{ 
	return g_dwDisplayMode;			// Return the current display mode value
}

//---------------------------------------------------------------------------
// SVCTL::SetDisplayMode (inline)
//
// Changes the module-wide user interface display mode variable.  The previous
// value is returned to the caller, to allow for temporary changes
//
// Arguments :
//
//	dwNewMode		- Combination of SVCTL::DISPLAY_MODE_XXX constants

inline DWORD SetDisplayMode(DWORD dwNewMode)
{
	return InterlockedExchange(reinterpret_cast<LPLONG>(&g_dwDisplayMode), 
		dwNewMode);
}

//---------------------------------------------------------------------------

END_NAMESPACE(SVCTL)

#pragma warning(pop)

#endif	// __SVCTLUI_H_