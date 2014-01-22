//---------------------------------------------------------------------------
// svctlcui.cpp
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
// Global Variables

// Console Prompt Data Structure

CONSOLE_PROMPT_ENTRY g_rgConsolePrompts[] = {

	{ MB_OK,				g_pszOK,				g_rgvkOK },
	{ MB_OKCANCEL,			g_pszOKCANCEL,			g_rgvkOKCANCEL },
	{ MB_ABORTRETRYIGNORE,	g_pszABORTRETRYIGNORE,	g_rgvkABORTRETRYIGNORE },
	{ MB_YESNOCANCEL,		g_pszYESNOCANCEL,		g_rgvkYESNOCANCEL },
	{ MB_YESNO,				g_pszYESNO,				g_rgvkYESNO },
	{ MB_RETRYCANCEL,		g_pszRETRYCANCEL,		g_rgvkRETRYCANCEL },

#if WINVER >= 0x0500

	{ MB_CANCELTRYCONTINUE,	g_pszCANCELTRYCONTINUE,	g_rgvkCANCELTRYCONTINUE },

#endif	// WINVER
};

// Console Prompt Data Structure Size

DWORD g_cConsolePrompts = 
	(sizeof(g_rgConsolePrompts) / sizeof(CONSOLE_PROMPT_ENTRY));

// Console Response Data Structure

CONSOLE_RESPONSE_ENTRY g_rgConsoleResponses[] = {

	{ 0x41,			g_pszIDABORT,		IDABORT },		// VK_A
	{ 0x58,			g_pszIDCANCEL,		IDCANCEL },		// VK_X
	{ VK_ESCAPE,	g_pszIDCANCEL,		IDCANCEL },
	{ 0x49,			g_pszIDIGNORE,		IDIGNORE },		// VK_I
	{ 0x4E,			g_pszIDNO,			IDNO },			// VK_N
	{ 0x4F,			g_pszIDOK,			IDOK },			// VK_O
	{ VK_RETURN,	g_pszIDOK,			IDOK },
	{ 0x52,			g_pszIDRETRY,		IDRETRY },		// VK_R
	{ 0x59,			g_pszIDYES,			IDYES },		// VK_Y

#if WINVER >= 0x0500

	{ 0x43,			g_pszIDCONTINUE,	IDCONTINUE },	// VK_C
	{ 0x54,			g_pszIDTRYAGAIN,	IDTRYAGAIN },	// VK_T

#endif // WINVER
};

// Console Response Data Structure Size

DWORD g_cConsoleResponses = 
	(sizeof(g_rgConsoleResponses) / sizeof(CONSOLE_RESPONSE_ENTRY));

//---------------------------------------------------------------------------
// SVCTL::ConsoleUI Implementation
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// ConsoleUI Constructor
//
// Arguments :
//
//	NONE

ConsoleUI::ConsoleUI() : m_hin(INVALID_HANDLE_VALUE), m_hout(INVALID_HANDLE_VALUE),
	m_uSavedWidth(0), m_hThread(NULL), m_hevtCancel(NULL)
{
	// Create the kernel event objects required by the reset of the class
	
	m_hevtLock = CreateEvent(NULL, FALSE, TRUE, NULL);
	m_hevtStop = CreateEvent(NULL, FALSE, FALSE, NULL);
}

//---------------------------------------------------------------------------
// ConsoleUI::Activate
//
// Activates the console user interface by retrieving handles to STDIN and
// STDOUT, and optionally creating a new console if necessary
//
// Arguments :
//
//	pszTitle		- Application name, used as the caption for the console

DWORD ConsoleUI::Activate(LPCTSTR pszTitle)
{
	CONSOLE_SCREEN_BUFFER_INFO	conInfo;			// Console screen buffer info
	Buffer<TCHAR>				strConTitle;		// Console title string buffer
	
	// Attempt to lock the console handles.  The only way this can fail is if
	// the underlying kernel object(s) were not properly created
	
	if(!LockConsole(true)) return ERROR_INVALID_HANDLE;
	
	// Check to see if the input/output handles have already been initialized

	if(m_hin != m_hout) { UnlockConsole(); return ERROR_ALREADY_INITIALIZED; }
	
	// Retrieve the STDIN/STDOUT handles for this process.  If they are both set
	// to INVALID_HANDLE_VALUE, this process is not attached to a console
	
	m_hin = GetStdHandle(STD_INPUT_HANDLE);
	m_hout = GetStdHandle(STD_OUTPUT_HANDLE);
	
	if(m_hin == m_hout) { UnlockConsole(); return ERROR_INVALID_HANDLE; }

	// There are some assumptions made about the width of the console,
	// so ensure that the screen buffer is at least CONSOLE_MIN_WIDTH wide 
	
	if(GetConsoleScreenBufferInfo(m_hout, &conInfo)) {

		if(conInfo.dwSize.X < CONSOLE_MIN_WIDTH) {

			m_uSavedWidth = conInfo.dwSize.X;
			conInfo.dwSize.X = CONSOLE_MIN_WIDTH;
			SetConsoleScreenBufferSize(m_hout, conInfo.dwSize);
		}
	}

	// Change the console window title to reflect the application name

	if((pszTitle) && (strConTitle.Allocate(1025))) {

		if(GetConsoleTitle(strConTitle, 1024) > 0) {

			m_strSavedTitle = strConTitle;		// Save the original title
			SetConsoleTitle(pszTitle);			// Set the new title
		}
	}

	// Attempt to create the CTRL+C handler event object, and register the control
	// handler function to be used for this process

	s_hevtCtrlC = CreateEvent(NULL, FALSE, FALSE, NULL);
	SetConsoleCtrlHandler(ConsoleControlHandler, TRUE);

	BlankLines();							// Start out with a blank line
	UnlockConsole();						// Release the console lock

	return ERROR_SUCCESS;
}

//---------------------------------------------------------------------------
// ConsoleUI::BlankLines (private)
//
// Forces the insertion of some blank lines at the current cursor position
//
// Arguments :
//
//	nAmount			- Number of blank console lines to add

const bool ConsoleUI::BlankLines(int nAmount) const
{
	CONSOLE_SCREEN_BUFFER_INFO	conInfo;		// Console information

	// Attempt to retrieve the screen buffer information for the console,
	// and if the cursor is not at zero X offset, add another line feed

	if(!GetConsoleScreenBufferInfo(m_hout, &conInfo)) return false;
	if(conInfo.dwCursorPosition.X) nAmount++;

	// Loop to send the specified number of line feed characters to the console

	while(nAmount--) { if (!WriteChar(CONSOLE_NEWLINE)) return false; }

	return true;
}

//---------------------------------------------------------------------------
// ConsoleUI::ClearCurrentLine (private)
//
// Clears the contents of the current console line
//
// Arguments :
//
//	NONE

const bool ConsoleUI::ClearCurrentLine(void) const
{
	CONSOLE_SCREEN_BUFFER_INFO	conInfo;		// Console information

	// Retrieve the console information so we know what the width is

	if(!GetConsoleScreenBufferInfo(m_hout, &conInfo)) return false;

	// Move the cursor to position 0 on the currently active line

	conInfo.dwCursorPosition.X = 0;
	SetConsoleCursorPosition(m_hout, conInfo.dwCursorPosition);

	// Erase the current line by spitting out a bunch of space characters,
	// and move the cursor back to position 0 when we're finished
		
	while(conInfo.dwSize.X > 1) { WriteChar(CONSOLE_SPACE); conInfo.dwSize.X--; }
	SetConsoleCursorPosition(m_hout, conInfo.dwCursorPosition);

	return true;
}

//---------------------------------------------------------------------------
// ConsoleUI::ConsoleControlHandler (private, static)
//
// Implements the console control handler callback function
//
// Arguments :
//
//	dwControl		- Control code for the console event

BOOL WINAPI ConsoleUI::ConsoleControlHandler(DWORD dwControl)
{
	// If the notification is for a CTRL+C event, and the required event object
	// has been successfully created, pulse the CTRL+C event object

	if((dwControl == CTRL_C_EVENT) && (s_hevtCtrlC)) {

		PulseEvent(s_hevtCtrlC);		// Pulse the CTRL+C handler event
		return TRUE;					// Processed the notification
	}

	return FALSE;						// Did not process the notification
}
	
//---------------------------------------------------------------------------
// ConsoleUI::DeActivate
//
// Deactivates the console user interface and closes the console handles
//
// Arguments :
//
//	NONE

void ConsoleUI::DeActivate(void)
{
	CONSOLE_SCREEN_BUFFER_INFO	conInfo;	// Console screen buffer information

	if(!LockConsole(true)) return;						// Wait for the console lock
	if(m_hin == m_hout) { UnlockConsole(); return; }	// Console is not active

	// Revoke the console control handler, and destroy the CTRL+C event object

	SetConsoleCtrlHandler(ConsoleControlHandler, FALSE);
	if(s_hevtCtrlC) CloseHandle(s_hevtCtrlC); 
	s_hevtCtrlC = NULL;

	// Restore the original console window caption if it had been altered

	if(!m_strSavedTitle.IsNull()) SetConsoleTitle(m_strSavedTitle);
	m_strSavedTitle.Clear();

	// Restore the original console screen buffer width if it had been altered

	if(m_uSavedWidth) {

		GetConsoleScreenBufferInfo(m_hout, &conInfo);
		conInfo.dwSize.X = m_uSavedWidth;
		SetConsoleScreenBufferSize(m_hout, conInfo.dwSize);
		m_uSavedWidth = 0;
	}
	
	m_hin = m_hout = INVALID_HANDLE_VALUE;		// Reset the STDIN/STDOUT handles
	UnlockConsole();							// Release the console lock
}

//---------------------------------------------------------------------------
// ConsoleUI::GetVkResponse (private)
//
// Retrieves a virtual key code response from the user.  This is only called
// by the PromptUser() function, so it has been marked as inline
//
// Arguments :
//
//	rgvkCriteria		- Array of valid virtual key response codes

inline const VKEY ConsoleUI::GetVkResponse(PCVKEY rgvkCriteria) const
{
	CONSOLE_CURSOR_INFO	cursorInfo;			// Console cursor information
	DWORD				dwInputMode;		// Console input mode flags
	bool				bValid = false;		// Loop termination flag
	INPUT_RECORD		recInput;			// INPUT_RECORD from console input
	DWORD				cEvents;			// Number of events read from console
	VKEY				vkInput;			// Virtual key code read from the console
	PCVKEY				pvk;				// Pointer to a virtual key code

	if(!rgvkCriteria) return NULL;			// No criteria codes to work with
	
	GetConsoleCursorInfo(m_hout, &cursorInfo);		// Get console cursor info
	
	cursorInfo.bVisible = FALSE;					// Turn off the cursor
	SetConsoleCursorInfo(m_hout, &cursorInfo);		// Set new cursor info
	
	// Remove the LINE_INPUT and ECHO_INPUT flags from the console input handle

	GetConsoleMode(m_hin, &dwInputMode);
	SetConsoleMode(m_hin, dwInputMode & ~ENABLE_LINE_INPUT & ~ENABLE_ECHO_INPUT);

	FlushConsoleInputBuffer(m_hin);					// Flush the console input buffer

	// Loop until the user presses one of the valid virtual keys
	
	while(!bValid) {

		vkInput = NULL;

		// Read in the next event from the console's input buffer, and discard
		// it if it's not a KEY_EVENT indicating a key down signal

		if(!ReadConsoleInput(m_hin, &recInput, 1, &cEvents)) break;
		
		if(recInput.EventType != KEY_EVENT) continue;		// Not a KEY_EVENT
		if(!recInput.Event.KeyEvent.bKeyDown) continue;		// Not a keydown event

		vkInput = recInput.Event.KeyEvent.wVirtualKeyCode;

		// Check to see if this virtual key code matches any of the specified
		// valid codes, and break the while() loop if it does

		for(pvk = rgvkCriteria; *pvk; pvk++) 
			if(vkInput == *pvk) { bValid = true; break; }
	};

	SetConsoleMode(m_hin, dwInputMode);				// Restore input mode flags
	
	cursorInfo.bVisible = TRUE;						// Turn the cursor back on
	SetConsoleCursorInfo(m_hout, &cursorInfo);		// Restore the cursor state
		
	return vkInput;
}

//---------------------------------------------------------------------------
// ConsoleUI::InsertHeader (private)
//
// Inserts a pseudo-icon header, based on MessageBox() icon type codes
//
// Arguments :
//
//	uType		- MessageBox() style icon type code for the header

inline const bool ConsoleUI::InsertHeader(UINT uType) const
{
	uType &= 0x000000F0;				// Strip all but the icon code

	// Determine which header string will be used at the beginning of the
	// separator bar
	
	switch(uType) {

		// MB_ICONWARNING : Display the "WARNING" separator bar

		case MB_ICONWARNING : return Separator(CONSOLE_HEADER_WARNING);

		// MB_ICONERROR : Display the "ERROR" separator bar

		case MB_ICONERROR : return Separator(CONSOLE_HEADER_ERROR);

		// Everything else just gets a default separator bar

		default : return Separator();
	}
}

//---------------------------------------------------------------------------
// ConsoleUI::List
//
// Displays a list box in the console window
//
// Arguments :
//
//	uType		- Type codes as specified for the MessageBox() API
//	pszHeader	- Header text (appears before the item list)
//	rgszItems	- Array of item text string pointers
//	cItems		- Number of pointers contained in the rgszItems array
//	pszFooter	- Optional footer text (appears after the item list)

const INT_PTR ConsoleUI::List(UINT uType, LPCTSTR pszHeader, LPCTSTR *rgszItems,
							  DWORD cItems, LPCTSTR pszFooter)
{
	String			strItem;				// Item text string object
	DWORD			dwIndex;				// Loop index variable
	INT_PTR			nResult;				// Result from function call

	// Attempt to lock the console, and default the response code if we cannot

	if(!LockConsole()) return DefaultUserResponse(uType, false);

	// Display the optional faux icon header string, and the list box header text
	
	if(InsertHeader(uType)) BlankLines();
	if(pszHeader) { if(WriteText(pszHeader)) BlankLines(); }

	// Loop through the array of list items to display them in the console
	
	if(rgszItems) {
		
		for(dwIndex = 0; dwIndex < cItems; dwIndex++) {

			strItem = CONSOLE_BULLET;		// Start with the bullet
			strItem += CONSOLE_SPACE;		// Add in a space
			strItem += rgszItems[dwIndex];	// Finish with the text itself

			WriteText(strItem, CONSOLE_LIST_LMARGIN);
		}

		BlankLines();						// Finish up with a blank line
	}

	// Display the optional list box footer text before prompting the user
	
	if(pszFooter) { if(WriteText(pszFooter)) BlankLines(); }
		
	nResult = PromptUser(uType);			// Prompt the user for their response
	BlankLines();							// Skip a line for neatness

	UnlockConsole();						// Release the console handle lock
	return nResult;							// Return the user's response
}

//---------------------------------------------------------------------------
// ConsoleUI::LockConsole (private)
//
// Locks the console input/output handles during the execution of a function
//
// Arguments :
//
//	bWait			- Flag to wait for the console lock.  Default is to fail

inline const bool ConsoleUI::LockConsole(bool bWait)
{
	// Either wait forever, or not at all, for the console lock to become
	// available.  If m_hevtLock is not valid, this will also fail

	return (WaitForSingleObject(m_hevtLock, (bWait) ? INFINITE : 0) == WAIT_OBJECT_0);
}
	
//---------------------------------------------------------------------------
// ConsoleUI::Message
//
// Displays MessageBox() style information in the console window
//
// Arguments :
//
//	uType		- Type codes as specified for the MessageBox() API
//	pszText		- Text message to be displayed

const INT_PTR ConsoleUI::Message(UINT uType, LPCTSTR pszText)
{
	INT_PTR			nResult;				// Result from function call

	// Attempt to lock the console, and default the response code if we cannot

	if(!LockConsole()) return DefaultUserResponse(uType, false);

	// Display the icon header and the message text in the console window
	
	if(InsertHeader(uType)) BlankLines();
	if(WriteText(pszText)) BlankLines();	

	nResult = PromptUser(uType);			// Prompt the user for their response
	BlankLines();							// Skip a line for neatness

	UnlockConsole();						// Release the console handle lock
	return nResult;							// Return the user's response
}

//---------------------------------------------------------------------------
// ConsoleUI::ProgressProc
//
// Main progress worker thread entry point
//
// Arguments :
//
//	NONE

DWORD ConsoleUI::ProgressProc(void)
{
	CONSOLE_SCREEN_BUFFER_INFO	conInfo;		// Console information
	int							nPosition;		// Current cursor position
	DWORD						dwWait;			// Result from wait operation

	// If a cancellation event handle is available, display the CTRL+C message,
	// otherwise just set up the left margin by writing the NULL string

	if(m_hevtCancel) Write(CONSOLE_CANCEL_PROGRESS, CONSOLE_NORM_LMARGIN, false);
	else Write(CONSOLE_NULL_STRING, CONSOLE_NORM_LMARGIN, false);
	
	GetConsoleScreenBufferInfo(m_hout, &conInfo);		// Retrieve dimensions
	conInfo.dwSize.X -= CONSOLE_NORM_RMARGIN;			// Adjust for the right margin
	
	nPosition = conInfo.dwCursorPosition.X;				// Set initial position
	HANDLE rghWait[] = { m_hevtStop, s_hevtCtrlC };		// Initialize handle array
	
	do {

		// Continually write progress dots to the console until we are told to stop,
		// or the user cancels the operation via the CTRL+C handler

		dwWait = WaitForMultipleObjects((m_hevtCancel) ? 2 : 1, rghWait, FALSE,
			CONSOLE_PROGRESS_INTERVAL);

		switch(dwWait) {

			// WAIT_TIMEOUT : Display another progress dot on the console

			case WAIT_TIMEOUT :

				WriteChar(CONSOLE_PROGRESS);

				// If the current cursor position has exceeded the right margin,
				// clear any of the already displayed dots and reset the cursor

				if(++nPosition >= conInfo.dwSize.X) {

					SetConsoleCursorPosition(m_hout, conInfo.dwCursorPosition);
					nPosition = conInfo.dwCursorPosition.X;

					while(nPosition++ < conInfo.dwSize.X) WriteChar(CONSOLE_SPACE);
					
					SetConsoleCursorPosition(m_hout, conInfo.dwCursorPosition);
					nPosition = conInfo.dwCursorPosition.X;
				}

				break;

			// WAIT_OBJECT_0 + 1 : The CTRL+C handler has been invoked

			case WAIT_OBJECT_0 + 1 :

				if(m_hevtCancel) SetEvent(m_hevtCancel);	// Set cancellation event
				
				break;

			// WAIT_FAILED : Something bad has happened

			case WAIT_FAILED : return GetLastError();
		}
	
	} while(dwWait == WAIT_TIMEOUT);

	// After the loop has been broken, display the final status of the progress
	// on the current line after it has been cleared off
	
	ClearCurrentLine();
	Write((dwWait == WAIT_OBJECT_0) ? g_pszIDOK : g_pszIDCANCEL);
	
	return ERROR_SUCCESS;
}

//---------------------------------------------------------------------------
// ConsoleUI::PromptUser (private)
//
// Displays one of the canned MessageBox() style prompts to the user, 
// and retrieves their response via the GetVkResponse() helper
//
// Arguments :
//
//	uType			- MessageBox() style type codes

const INT_PTR ConsoleUI::PromptUser(UINT uType) const
{
	LPCTSTR			pszPromptText = NULL;		// Pointer to the prompt text
	PCVKEY			rgvkResponses = NULL;		// Pointer to valid responses
	VKEY			vkInput = NULL;				// User's response character
	DWORD			dwIndex;					// Loop index variable

	uType &= 0x0000000F;			// Strip all but button codes from uType

	// Locate the prompt text and valid response key codes in the global array

	for(dwIndex = 0; dwIndex < g_cConsolePrompts; dwIndex++) {
		if(uType == g_rgConsolePrompts[dwIndex].uButtonCode) {

			pszPromptText = g_rgConsolePrompts[dwIndex].pszPrompt;
			rgvkResponses = g_rgConsolePrompts[dwIndex].rgvkRepsonses;
			
			break;					// Found the prompt information
		}
	}

	// If the button code did not exist, fall back to a default response

	if((!pszPromptText) || (!rgvkResponses)) return DefaultUserResponse(uType, false);

	// Display the prompt, wait for a response, and erase everything when done
	
	Write(pszPromptText, CONSOLE_NORM_LMARGIN, false);
	vkInput = GetVkResponse(rgvkResponses);
	ClearCurrentLine();

	// If we were unable to retrieve the user's response use a default
	
	if(vkInput == NULL) return DefaultUserResponse(uType, false);

	// Map the response virtual key code into both the response text and the
	// associated MessageBox() INT_PTR return value
	
	for(dwIndex = 0; dwIndex < g_cConsoleResponses; dwIndex++) {

		if(g_rgConsoleResponses[dwIndex].vkResponse == vkInput) {

			Write(g_rgConsoleResponses[dwIndex].pszResponse);
			return g_rgConsoleResponses[dwIndex].nResponse;
		}
	}

	return DefaultUserResponse(uType, false);		// Response key code was not mapped
}

//---------------------------------------------------------------------------
// ConsoleUI::Separator (private)
//
// Displays a separator bar in the console, optionally adding a text
// string into the center of it
//
// Arguments :
//
//	pszText			- Character string to be displayed
//	nLeftMargin		- Left margin to be used for the separator
//	nRightMargin	- Right margin to be used for the separator

const bool ConsoleUI::Separator(LPCTSTR pszText, int nLeftMargin, int nRightMargin) const
{
	CONSOLE_SCREEN_BUFFER_INFO	conInfo;		// Console information
	Buffer<TCHAR>				lineBuffer;		// Console output buffer
	int							nIndex = 0;		// Offset into line buffer
	int							nLength = 0;	// Length of the string
	int							nTextPos;		// Position of separator text

	if(pszText) nLength = static_cast<int>(_tcslen(pszText));	// Determine length

	// Attempt to retrieve the screen buffer information for the console,
	// and allocate a line buffer large enough for the maximum output

	if(!GetConsoleScreenBufferInfo(m_hout, &conInfo)) return false;
	if(!lineBuffer.Allocate(conInfo.dwSize.X + 1)) return false;

	// Calculate both the length of the separator bar, and the position that
	// the inserted text will appear in the line buffer
	
	conInfo.dwSize.X -= (nLeftMargin + nRightMargin);
	nTextPos = (conInfo.dwSize.X - nLength) / 2;

	// Construct the separator bar line buffer by first adding the hyphens, then
	// the centered text, and the trailing hyphens ...
	
	while(nIndex < nTextPos) lineBuffer[nIndex++] = CONSOLE_SEPARATOR;
	if(nLength > 0) { while(*pszText) { lineBuffer[nIndex++] = *pszText; pszText++; } }
	while(nIndex < conInfo.dwSize.X) lineBuffer[nIndex++] = CONSOLE_SEPARATOR;

	return Write(lineBuffer, nLeftMargin);		// Display the separator bar
}

//---------------------------------------------------------------------------
// ConsoleUI::StartProgress
//
// Starts up a progress box, optionally with a cancellation feature
//
// Arguments :
//
//	pszText			- Text to display in the progress box message
//	hevtCancel		- Optional kernel event object used to signal cancellation

const bool ConsoleUI::StartProgress(LPCTSTR pszText, HANDLE hevtCancel)
{
	AutoCS		autoLock(m_csProgress);		// Automatic critical section object
	DWORD		dwThreadId;					// Launched progress thread id code
	
	if(!LockConsole()) return false;		// Attempt to lock the console handles

	if(!m_hevtStop) { UnlockConsole(); return false; }	// Test STOP event object
	m_hevtCancel = hevtCancel;							// Store CANCEL event object
	
	// Display the standard PLEASE WAIT separator bar, and the progress message
	
	if(Separator(CONSOLE_HEADER_PROGRESS)) BlankLines();
	if(WriteText(pszText)) BlankLines();

	// Attempt to launch the progress worker thread that actually does all of 
	// the real work for us
	
	m_hThread = SVCTL::CreateThread(NULL, 0, ProgressThreadProc, this, 0, &dwThreadId);
	if(!m_hThread) { UnlockConsole(); return false; }

	// NOTE : The call to UnlockConsole() happens in the StopProgress() function

	return true;
}

//---------------------------------------------------------------------------
// ConsoleUI::StopProgress
//
// Stops a progress box started up with the StartProgress() function
//
// Arguments :
//
//	NONE

void ConsoleUI::StopProgress(void)
{
	AutoCS		autoLock(m_csProgress);		// Automatic critical section

	// If the progress worker thread was successfully started, it needs to be
	// shut down, and the progress related variables need to be reset
	
	if(m_hThread) {

		// Signal the STOP event object, and wait for the thread to terminate
		
		SetEvent(m_hevtStop);
		WaitForSingleObject(m_hThread, INFINITE);

		m_hThread = NULL;				// Reset the thread object handle
		m_hevtCancel = NULL;			// Reset the event object handle

		BlankLines();					// Append a blank line to the output
		UnlockConsole();				// Unlock the lock acquired in StartProgress()
	}	
}

//---------------------------------------------------------------------------
// ConsoleUI::UnlockConsole (private)
//
// Unlocks the console input/output handles after a successful call into the
// LockConsole() function
//
// Arguments :
//
//	NONE

inline void ConsoleUI::UnlockConsole(void)
{
	if(m_hevtLock) SetEvent(m_hevtLock);	// Reset the event to signaled state
}
	
//---------------------------------------------------------------------------
// ConsoleUI::Write (private)
//
// Displays text left-justified on the current console line
//
// Arguments :
//
//	pszText			- Character string to be displayed
//	nMargin			- Left margin to be applied to the output
//	bLineFeed		- Flag to add an optional line feed character to the output

const bool ConsoleUI::Write(LPCTSTR pszText, int nMargin, bool bLineFeed) const
{
	DWORD		cchOutput;				// Characters actually displayed
	int			nLength;				// Length of the output string
	
	if(!pszText) return false;						// NULL string pointer
	nLength = static_cast<int>(_tcslen(pszText));	// Determine string length

	// Apply the specified left-hand margin by spitting out blank characters

	while(nMargin--) WriteChar(CONSOLE_SPACE);

	// Write the text string to the console at the current cursor position

	if(!WriteConsole(m_hout, pszText, static_cast<DWORD>(_tcslen(pszText)), &cchOutput, NULL)) 
		return false;

	// Finally, apply the optional line feed by spitting out a NEWLINE character

	return (bLineFeed) ? WriteChar(CONSOLE_NEWLINE) : true;
}

//---------------------------------------------------------------------------
// ConsoleUI::WriteChar (private)
//
// Outputs a single character to the current console position
//
// Arguments :
//
//	ch			- Character to be written to the console window

inline const bool ConsoleUI::WriteChar(TCHAR ch) const
{
	DWORD		cchOutput;		// Characters actually displayed in console

	// Display the single character at the current cursor position

	return (WriteConsole(m_hout, &ch, 1, &cchOutput, NULL) == TRUE);
}

//---------------------------------------------------------------------------
// ConsoleUI::WriteText (private)
//
// Outputs a block of text to the console, ensuring that smaller words do not
// wrap lines, and larger words get hyphenated when they must be broken up
//
// Arguments :
//
//	pszText			- Text to be displayed in the console
//	nLeftMargin		- Left margin to be used for output
//	nRightMargin	- Right margin to be used for output

const bool ConsoleUI::WriteText(LPCTSTR pszText, int nLeftMargin, int nRightMargin) const
{
	CONSOLE_SCREEN_BUFFER_INFO	conInfo;		// Console information
	Buffer<TCHAR>				lineBuffer;		// Console line buffer
	int							nIndex = 0;		// Index into the buffer
	int							nBlanks;		// Used to add spaces to the string
	int							nBackup;		// Line break char counter

	if(!pszText) return false;					// NULL string pointer

	// Validate the margin values, and adjust them if necessary
	
	if(nLeftMargin > CONSOLE_MAX_LMARGIN) nLeftMargin = CONSOLE_MAX_LMARGIN;
	if(nRightMargin > CONSOLE_MAX_RMARGIN) nRightMargin = CONSOLE_MAX_RMARGIN;

	// Determine the size of the buffer we need to allocate, and allocate it

	if(!GetConsoleScreenBufferInfo(m_hout, &conInfo)) return false;
	if(!lineBuffer.Allocate(conInfo.dwSize.X + 1)) return false;
	
	nRightMargin = (conInfo.dwSize.X - nRightMargin);	// Calculate right margin
	
	// Loop through this mess until we hit the NULL string terminator ...
	
	while(*pszText) {
		
		nIndex = 0;							// Reset the loop index variable
		lineBuffer.ZeroOut();				// Zero out the buffer contents

		// Apply the left margin to the buffer by adding space characters

		for(nBlanks = 0; (nBlanks < nLeftMargin) && (nIndex < nRightMargin); nBlanks++)
			lineBuffer[nIndex++] = CONSOLE_SPACE;

		// Loop to add the maximum number of characters we can to the buffer,
		// stopping if we run into the end of the string or a line feed char
		
		while((nIndex < nRightMargin) && (*pszText != CONSOLE_NEWLINE) && (*pszText)) {

			if(*pszText != CONSOLE_TAB) lineBuffer[nIndex++] = *pszText++;
			else { lineBuffer[nIndex++] = CONSOLE_SPACE; pszText++; }
		}

		// CONDITION 1 : The line buffer has been filled up.  If the next character
		// in the string is not a SPACE, we need to break up the line neatly

		if((nIndex == nRightMargin) && (*pszText != CONSOLE_SPACE)) {

			pszText--;									// Back off one character
			lineBuffer[--nIndex] = CONSOLE_HYPHEN;		// Assume we need a hyphen

			// Backup up to CONSOLE_HYPHEN_LIMIT characters, looking for a 
			// whitespace character we can break on instead of using the hyphen
			
			nBackup = 0;

			while((nBackup < nIndex) && (nBackup < CONSOLE_HYPHEN_LIMIT)) {

				// If a space is located, break the line and back off the pointers

				if(lineBuffer[nIndex - nBackup] == CONSOLE_SPACE) {
					
					lineBuffer[nIndex - nBackup] = CONSOLE_NEWLINE;
					lineBuffer[nIndex - nBackup + 1] = 0;

					pszText -= nBackup;			// Back off the string pointer
					break;						// We're done
				}

				nBackup++;			// <--- Keep looking for a SPACE character
			}
		}

		// CONDITION 2 : A line feed character has been located.  Just add it
		// to the buffer like any other character

		else if(*pszText == CONSOLE_NEWLINE) lineBuffer[nIndex] = *pszText++;
		
		// CONDITION 3 : The end of the string was reached.  If the entire
		// console line has not been filled, append a line feed character
		
		else if(nIndex < conInfo.dwSize.X) lineBuffer[nIndex] = CONSOLE_NEWLINE;
		
		if(!Write(lineBuffer, 0, false)) return false;	// Display the text
		while(*pszText == CONSOLE_SPACE) pszText++;		// Skip whitespace
	}

	return true;
}

//---------------------------------------------------------------------------

END_NAMESPACE(SVCTL)