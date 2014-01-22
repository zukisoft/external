//---------------------------------------------------------------------------
// svctlcui.h
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

#ifndef __SVCTLCUI_H_
#define __SVCTLCUI_H_
#pragma once

//#pragma warning(push, 4)				// Enable maximum compiler warnings
//#pragma warning(disable:4100)			// "unreferenced formal parameter"
//#pragma warning(disable:4127)			// "conditional expression is constant"

//---------------------------------------------------------------------------
// Classes declared in this unit:
//
//	SVCTL::ConsoleUI
//---------------------------------------------------------------------------

BEGIN_NAMESPACE(SVCTL)

//---------------------------------------------------------------------------
// Type Declarations

typedef USHORT		VKEY;			// A virtual key code
typedef VKEY*		PVKEY;			// Pointer to a virtual key code
typedef const VKEY*	PCVKEY;			// Pointer to a constant virtual key code

// CONSOLE_PROMPT_ENTRY
// Used to provide MessageBox() style prompts for the user

typedef struct {

	UINT		uButtonCode;		// MessageBox() button style code
	LPCTSTR		pszPrompt;			// Prompt text string
	PCVKEY		rgvkRepsonses;		// Array of valid VK response codes

} CONSOLE_PROMPT_ENTRY, *PCONSOLE_PROMPT_ENTRY;

// CONSOLE_RESPONSE_ENTRY
// Maps a user's character based response into a MessageBox() return value

typedef struct {

	VKEY		vkResponse;			// Repsonse virtual key code
	LPCTSTR		pszResponse;		// Textual version of the response
	INT_PTR		nResponse;			// Response code MessageBox() value

} CONSOLE_RESPONSE_ENTRY, *PCONSOLE_RESPONSE_ENTRY;

//---------------------------------------------------------------------------
// Constants

// Numerics

const int CONSOLE_MIN_WIDTH = 40;			// Minimum console screen buffer width
const int CONSOLE_NORM_LMARGIN = 2;			// Standard console left margin
const int CONSOLE_NORM_RMARGIN = 2;			// Standard console right margin
const int CONSOLE_LIST_LMARGIN = 5;			// List item text left margin
const int CONSOLE_MAX_LMARGIN = 20;			// Maximum console left margin
const int CONSOLE_MAX_RMARGIN = 20;			// Maximum console right margin
const int CONSOLE_HYPHEN_LIMIT = 20;		// Value controlling text hyphenation
const int CONSOLE_PROGRESS_INTERVAL = 500;	// Progress dot display interval

// Single Characters

const TCHAR CONSOLE_BULLET					= _T('-');
const TCHAR CONSOLE_CARRIAGERETURN			= _T('\r');
const TCHAR CONSOLE_HYPHEN					= CONSOLE_BULLET;
const TCHAR CONSOLE_NEWLINE					= _T('\n');
const TCHAR CONSOLE_PROGRESS				= _T('.');
const TCHAR CONSOLE_SEPARATOR				= CONSOLE_BULLET;
const TCHAR CONSOLE_SPACE					= _T(' ');
const TCHAR CONSOLE_TAB						= _T('\t');

// Generic Strings

const TCHAR CONSOLE_NULL_STRING[]			= _T("");

// Icon Header Strings

const TCHAR CONSOLE_HEADER_ERROR[]			= _T(" >>> ERROR <<< ");
const TCHAR CONSOLE_HEADER_WARNING[]		= _T(" >>> WARNING <<< ");
const TCHAR CONSOLE_HEADER_PROGRESS[]		= _T(" >>> PLEASE WAIT <<< ");
const TCHAR CONSOLE_CANCEL_PROGRESS[]		= _T("Press <Ctrl+C> to cancel ");

// Console Prompt Text Strings

const TCHAR g_pszOK[]					= _T("Press <Enter> to continue ");
const TCHAR g_pszOKCANCEL[]				= _T("OK, Cancel? [O,X] ");
const TCHAR g_pszABORTRETRYIGNORE[]		= _T("Abort, Retry, Ignore? [A,R,I] ");
const TCHAR g_pszYESNOCANCEL[]			= _T("Yes, No, Cancel? [Y,N,X] ");
const TCHAR g_pszYESNO[]				= _T("Yes, No? [Y,N] ");
const TCHAR g_pszRETRYCANCEL[]			= _T("Retry, Cancel? [R,X] ");
const TCHAR g_pszCANCELTRYCONTINUE[]	= _T("Cancel, Try Again, Continue? [X,T,C] ");

// Console Response Text Strings

const TCHAR g_pszIDABORT[]				= _T("(Abort)");
const TCHAR g_pszIDCANCEL[]				= _T("(Cancel)");
const TCHAR g_pszIDCONTINUE[]			= _T("(Continue)");
const TCHAR g_pszIDIGNORE[]				= _T("(Ignore)");
const TCHAR g_pszIDNO[]					= _T("(No)");
const TCHAR g_pszIDOK[]					= _T("(OK)");
const TCHAR g_pszIDRETRY[]				= _T("(Retry)");
const TCHAR g_pszIDTRYAGAIN[]			= _T("(Try Again)");
const TCHAR g_pszIDYES[]				= _T("(Yes)");

/// Console Prompt Virtual Key Code Arrays (NULL terminators required)

const VKEY g_rgvkOK[]					= { VK_RETURN, 0x4F, NULL };
const VKEY g_rgvkOKCANCEL[]				= { VK_RETURN, 0x4F, VK_ESCAPE, 0x58, NULL };
const VKEY g_rgvkABORTRETRYIGNORE[]		= { 0x41, 0x52, 0x49, NULL };
const VKEY g_rgvkYESNOCANCEL[]			= { 0x59, 0x4E, VK_ESCAPE, 0x58, NULL };
const VKEY g_rgvkYESNO[]				= { 0x59, 0x4E, NULL };
const VKEY g_rgvkRETRYCANCEL[]			= { 0x52, VK_ESCAPE, 0x58, NULL };
const VKEY g_rgvkCANCELTRYCONTINUE[]	= { VK_ESCAPE, 0x58, 0x54, 0x43, NULL };

//---------------------------------------------------------------------------
// Class SVCTL::ConsoleUI
//
// ConsoleUI provides the console-based user interface for the service
// module object
//---------------------------------------------------------------------------

class ConsoleUI : public UserInterface
{
public:

	//-----------------------------------------------------------------------
	// Constructor / Destructor

	ConsoleUI();
	virtual ~ConsoleUI() { StopProgress(); DeActivate(); }
	
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

	ConsoleUI(const ConsoleUI &rhs);				// Disable copy
	ConsoleUI& operator=(const ConsoleUI &rhs);		// Disable assignment

	//-----------------------------------------------------------------------
	// Private Member Functions

	const bool BlankLines(int nAmount = 1) const;

	const bool ClearCurrentLine(void) const;
	
	const VKEY GetVkResponse(PCVKEY rgvkCriteria) const;

	const bool InsertHeader(UINT uType) const;

	const bool LockConsole(bool bWait = false);

	DWORD ProgressProc(void);

	const INT_PTR PromptUser(UINT uType) const;

	const bool Separator(LPCTSTR pszText = NULL, int nLeftMargin = CONSOLE_NORM_LMARGIN,
		int nRightMargin = CONSOLE_NORM_RMARGIN) const;

	void UnlockConsole(void);

	const bool Write(LPCTSTR pszText, int nMargin = CONSOLE_NORM_LMARGIN,
		bool bLineFeed = true) const;

	const bool WriteChar(TCHAR ch) const;

	const bool WriteText(LPCTSTR pszText, int nLeftMargin = CONSOLE_NORM_LMARGIN,
		int nRightMargin = CONSOLE_NORM_RMARGIN) const;

	//-----------------------------------------------------------------------
	// Static Member Functions
	
	static BOOL WINAPI ConsoleControlHandler(DWORD dwControl);

	static DWORD WINAPI ProgressThreadProc(void *pv)
		{ return reinterpret_cast<ConsoleUI*>(pv)->ProgressProc(); }

	//-----------------------------------------------------------------------
	// Member Variables

	WinHandle		m_hevtLock;				// Console input/output handle lock
	HANDLE			m_hin;					// Console input handle
	HANDLE			m_hout;					// Console output handle
	SHORT			m_uSavedWidth;			// Saved console screen buffer width
	String			m_strSavedTitle;		// Saved console window title string

	CritSec			m_csProgress;			// Progress critical section
	WinHandle		m_hevtStop;				// Progress thread stop event object
	HANDLE			m_hThread;				// Progress thread handle
	HANDLE			m_hevtCancel;			// Progress cancellation event object

	static HANDLE	s_hevtCtrlC;			// Static CTRL+C handler event object
};

//---------------------------------------------------------------------------
// ConsoleUI Static Member Initialization

__declspec(selectany) HANDLE ConsoleUI::s_hevtCtrlC = NULL;

//---------------------------------------------------------------------------

END_NAMESPACE(SVCTL)

//#pragma warning(pop)

#endif	// __SVCTLCUI_H_