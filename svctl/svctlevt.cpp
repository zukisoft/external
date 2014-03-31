//---------------------------------------------------------------------------
// svctlevt.cpp
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

BEGIN_NAMESPACE(SVCTL)

//---------------------------------------------------------------------------
// SVCTL::EventStrings Implementation
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// EventStrings Destructor

EventStrings::~EventStrings()
{
	size_t				dwIndex;			// Loop index variable
	
	// Loop to release each of the String objects contained in the buffer

	for(dwIndex = 0; dwIndex < m_rgstrBuffer.Length(); dwIndex++)
		if(m_rgstrBuffer[dwIndex]) delete m_rgstrBuffer[dwIndex];
}

//---------------------------------------------------------------------------
// EventStrings::operator[]

LPCTSTR EventStrings::operator[](size_t index) const
{
	String*				pstrInsert;			// Pointer to the insertion string
	
	// If the specified index is less than the size of the buffer,
	// return the String::LPCTSTR() pointer at that location

	if(index < m_rgstrBuffer.Length()) {

		pstrInsert = m_rgstrBuffer[index];				// Get the String pointer
		return (pstrInsert) ? *pstrInsert : NULL;		// Return if not NULL
	}

	else return NULL;						// Invalid index value
}

//---------------------------------------------------------------------------
// EventStrings::Add
//
// Inserts a new insertion string into the next available location
//
// Arguments :
//
//	pszString			- Insertion string to be added to the array

const bool EventStrings::Add(LPCTSTR pszString)
{
	String*				pstrInsert;			// Pointer to the new insertion string

	pstrInsert = new String(pszString);		// Construct the new insertion string
	if(!pstrInsert) return false;			// Could not allocate memory

	// Attempt to reallocate the local buffer to make room for the new string

	if(m_rgstrBuffer.ReAllocate(m_rgstrBuffer.Length() + 1)) {

		m_rgstrBuffer[m_rgstrBuffer.Length() - 1] = pstrInsert;
		return true;
	}

	delete pstrInsert;						// Release the insertion string
	return false;							// Failed to allocate memory
}

//---------------------------------------------------------------------------
// EventStrings::Add
//
// Inserts a new insertion string at a specified offset in the array
//
// Arguments :
//
//	dwIndex				- Index into the array to place the string
//	pszString			- Insertion string to be added into the array

const bool EventStrings::Add(size_t dwIndex, LPCTSTR pszString)
{
	String*				pstrInsert;			// Pointer to the insertion string

	pstrInsert = new String(pszString);		// Construct the new insertion string
	if(!pstrInsert) return false;			// Could not allocate memory

	// Make sure that the buffer has been allocated large enough to accomodate

	if(m_rgstrBuffer.Length() <= dwIndex)
		if(!m_rgstrBuffer.ReAllocate(dwIndex + 1)) return false;

	// If the specified index location is NULL, just stick in the String pointer

	if(m_rgstrBuffer[dwIndex] == NULL) m_rgstrBuffer[dwIndex] = pstrInsert;

	// Otherwise, overwrite the existing insertion string object

	else { *m_rgstrBuffer[dwIndex] = *pstrInsert; delete pstrInsert; }
		
	return true;
}

//---------------------------------------------------------------------------
// SVCTL::Event Implementation
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Event::SetBinaryData
//
// Sets the binary data to be logged in the event log
//
// Arguments :
//
//	pvData			- Pointer to the data buffer (NULL to erase current)
//	cbData			- Size of the data buffer (zero to erase current)

void Event::SetBinaryData(const void *pvData, DWORD cbData)
{
	// If the caller has specified NULL or zero length, erase current data

	if((pvData == NULL) || (cbData == 0)) { m_binaryData.Free(); return; }

	// Attempt to resize the contained buffer and copy in the provided buffer

	if(m_binaryData.ReAllocate(cbData)) memcpy(m_binaryData, pvData, cbData);
}

//---------------------------------------------------------------------------
// Event::SetUserSid
//
// Sets the user SID to be logged in the event log
//
// Arguments :
//
//	psidUser		- Pointer to the user's SID (NULL to erase current)

void Event::SetUserSid(PSID psidUser)
{
	DWORD				cbSid;				// Size of the provided SID
	
	// If the caller has specified NULL or an invalid SID, clear the current data

	if((psidUser == NULL) || (!IsValidSid(psidUser))) { m_sidUser.Free(); return; }

	cbSid = GetLengthSid(psidUser);			// Determine the length of the SID

	// Attempt to resize the contained buffer and copy in the provided SID

	if(m_sidUser.ReAllocate(cbSid)) memcpy(m_sidUser, psidUser, cbSid);
}

//---------------------------------------------------------------------------
// Event::SetUserSidFromThread
//
// Sets the user SID to be logged in the event log from the current thread
//
// Arguments :
//
//	bOpenAsSelf			- Flag to pass to OpenThreadToken() -- see the SDK

void Event::SetUserSidFromThread(bool bOpenAsSelf)
{
	BOOL				bOpenSelf;			// "Open as self" flag to pass in
	HANDLE				hToken;				// Current thread token handle
	DWORD				cbTokenUser = 0;	// Length of TOKEN_USER information

	Buffer<TOKEN_USER, BYTE>	tokenUser;	// TOKEN_USER information buffer

	bOpenSelf = (bOpenAsSelf) ? TRUE : FALSE;		// Set up the BOOL flag
	
	// Attempt to open the current thread token, or the process' if not available
	
	if(!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, bOpenSelf, &hToken)) {

		if(GetLastError() == ERROR_NO_TOKEN) {

			if(!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) return;
		}

		else return;						// Unable to open the thread token
	}

	// Determine the length of the buffer we need to allocate for TOKEN_USER

	GetTokenInformation(hToken, TokenUser, NULL, cbTokenUser, &cbTokenUser);
	if(GetLastError() != ERROR_INSUFFICIENT_BUFFER) { CloseHandle(hToken); return; }

	// Attempt to allocate the Buffer<> that will hold the TOKEN_USER data

	if(!tokenUser.Allocate(cbTokenUser)) { CloseHandle(hToken); return; }

	// Retrieve the TOKEN_USER information about the current thread for real,
	// and set the user's SID using the normal SetUserSid() function

	if(GetTokenInformation(hToken, TokenUser, tokenUser, cbTokenUser, &cbTokenUser))
		SetUserSid(static_cast<PTOKEN_USER>(tokenUser)->User.Sid);

	CloseHandle(hToken);					// Close the thread token handle
}

//---------------------------------------------------------------------------
// SVCTL::Win32Event Implementation
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Win32Event Constructor
//
// Arguments :
//
//	dwEventId			- Optional event id code to be logged
//	dwStatus			- Optional Win32 status code to be logged
//	wSeverity			- Optional severity code to be logged
//	wCategory			- Optional category code to be logged

Win32Event::Win32Event(DWORD dwEventId, DWORD dwStatus, WORD wSeverity, 
					   WORD wCategory) : Event(dwEventId, wSeverity, wCategory)
{
	String			strInsertion;			// Win32 insertion string

	strInsertion.LoadMessage(dwStatus);		// Load the Win32 error code

	InsertionStrings.Add(strInsertion);			// Add the insertion string
	SetBinaryData(&dwStatus, sizeof(DWORD));	// Set the error code as the data
}

//---------------------------------------------------------------------------
// SVCTL::ServiceEventsBase Implementation
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// ServiceEventsBase::CategoryFromEventId (private)
//
// Extracts the category code from an event id (EVENTLOG_AUTO_CATEGORY)
//
// Arguments :
//
//	dwEventId		- Event id containing the embedded category code

inline 
const WORD ServiceEventsBase::CategoryFromEventId(DWORD dwEventId) const
{
	// The AUTO_CATEGORY code is nothing more than the facility code

	return static_cast<WORD>((dwEventId >> 16) & 0xFFF);
}

//---------------------------------------------------------------------------
// ServiceEventsBase::FindMessageModule (private)
//
// Attempts to locate the full path name for a message module file
//
// Arguments :
//
//	pszModule		- Name of the module to be located
//	strModule		- SVCTL::String to receive the full module name

void ServiceEventsBase::FindMessageModule(LPCTSTR pszModule, String &strModule) const
{
	HMODULE				hModule;			// Loaded DLL module handle
	
	// Attempt to find the DLL module using the standard search path
	
	hModule = LoadLibraryEx(pszModule, NULL, DONT_RESOLVE_DLL_REFERENCES);
	if(hModule) {

		strModule.LoadModuleName(hModule);		// Load the actual module name
		FreeLibrary(hModule);					// Release the module handle
	}

	else strModule = pszModule;			// Unable to locate the DLL module
}

//---------------------------------------------------------------------------
// ServiceEventsBase::LogEvent (SVCTL::Event object)
//
// Reports an event to the service's event log
//
// Arguments :
//
//	event			- Reference to a pre-loaded SVCTL::Event object

DWORD ServiceEventsBase::LogEvent(const Event &event) const
{
	size_t				cInserts;			// Number of insertion strings
	LPCTSTR*			rgszInserts;		// Array of insertion string pointers
	size_t				dwIndex;			// Loop index variable

	_ASSERTE(m_hEventLog != NULL);					// Invalid event log handle
	if(!m_hEventLog) return ERROR_INVALID_HANDLE;	// Invalid event log handle
	
	cInserts = event.InsertionStrings.Count;		// Get number of inserts
	
	// Allocate a stack buffer to hold pointers to all the insertion strings

	if(cInserts > 0) {

		rgszInserts = reinterpret_cast<LPCTSTR*>(_alloca(cInserts * sizeof(LPCTSTR)));

		for(dwIndex = 0; dwIndex < cInserts; dwIndex++)
			rgszInserts[dwIndex] = event.GetInsertionStrings()[dwIndex];
	}
	
	else rgszInserts = NULL;				// No insertion strings to be reported

	// Report the event, using the do-everything version of LogEvent()
	
	return LogEvent(event.EventId, rgszInserts, cInserts, event.BinaryData,
		static_cast<DWORD>(event.BinaryDataSize), event.Severity, event.Category, 
		event.UserSid);
}

//---------------------------------------------------------------------------
// ServiceEventsBase::LogEvent (No insertion strings)
//
// Reports an event to the service's event log
//
// Arguments :
//
//	dwEventId			- Event message ID number (error code)
//	wSeverity			- Event severity code
//	wCategory			- Event category code
//	psidUser			- Pointer to the referenced user's SID

DWORD ServiceEventsBase::LogEvent(DWORD dwEventId, WORD wSeverity, 
										WORD wCategory, PSID psidUser) const
{
	// Call through the "anything and everything" version of LogEvent()

	return LogEvent(dwEventId, NULL, 0, NULL, 0, wSeverity, wCategory, psidUser);
}

//---------------------------------------------------------------------------
// ServiceEventsBase::LogEvent (Single insertion string)
//
// Reports an event to the service's event log
//
// Arguments :
//
//	dwEventId			- Event message ID number (error code)
//	pszInsert			- Pointer to the single insertion string
//	wSeverity			- Event severity code
//	wCategory			- Event category code
//	psidUser			- Pointer to the referenced user's SID

DWORD ServiceEventsBase::LogEvent(DWORD dwEventId, LPCTSTR pszInsert, 
										WORD wSeverity, WORD wCategory, PSID psidUser) const
{
	LPCTSTR rgszInserts[] = { pszInsert, NULL };		// Create an LPCTSTR array

	// Call through the "anything and everything" version of LogEvent()

	return LogEvent(dwEventId, rgszInserts, 1, NULL, 0, wSeverity, wCategory, 
		psidUser);
}

//---------------------------------------------------------------------------
// ServiceEventsBase::LogEvent (Multiple insertion strings)
//
// Reports an event to the service's event log
//
// Arguments :
//
//	dwEventId			- Event message ID number (error code)
//	rgszInserts			- Array of event insertion strings
//	cInserts			- Size of the rgszInserts array
//	wSeverity			- Event severity code
//	wCategory			- Event category code
//	psidUser			- Pointer to the referenced user's SID

DWORD ServiceEventsBase::LogEvent(DWORD dwEventId, LPCTSTR *rgszInserts, 
										size_t cInserts, WORD wSeverity, WORD wCategory, 
										PSID psidUser) const
{
	// Call through the "anything and everything" version of LogEvent()

	return LogEvent(dwEventId, rgszInserts, cInserts, NULL, 0, wSeverity, wCategory, 
		psidUser);
}

//---------------------------------------------------------------------------
// ServiceEventsBase::LogEvent (Binary Data)
//
// Reports an event to the service's event log
//
// Arguments :
//
//	dwEventId			- Event message ID number (error code)
//	pvBinaryData		- Pointer to the binary data to attach to the event
//	cbBinaryData		- Length of the binary data, in bytes
//	wSeverity			- Event severity code
//	wCategory			- Event category code
//	psidUser			- Pointer to the referenced user's SID

DWORD ServiceEventsBase::LogEvent(DWORD dwEventId, const void *pvBinaryData, 
										DWORD cbBinaryData, WORD wSeverity, WORD wCategory, 
										PSID psidUser) const
{
	// Call through the "anything and everything" version of LogEvent()

	return LogEvent(dwEventId, NULL, 0, pvBinaryData, cbBinaryData, wSeverity, 
		wCategory, psidUser);
}

//---------------------------------------------------------------------------
// ServiceEventsBase::LogEvent (Anything and everything)
//
// Reports an event to the service's event log
//
// Arguments :
//
//	dwEventId			- Event message ID number (error code)
//	rgszInserts			- Array of event insertion strings
//	cInserts			- Size of the rgszInserts array
//	pvBinaryData		- Pointer to the binary data to attach to the event
//	cbBinaryData		- Length of the binary data, in bytes
//	wSeverity			- Event severity code
//	wCategory			- Event category code
//	psidUser			- Pointer to the referenced user's SID

DWORD ServiceEventsBase::LogEvent(DWORD dwEventId, LPCTSTR *rgszInserts, 
										size_t cInserts, const void *pvBinaryData, 
										DWORD cbBinaryData, WORD wSeverity, 
										WORD wCategory, PSID psidUser) const
{
	WORD				wSeverityCode;			// Severity code to use
	WORD				wCategoryCode;			// Category code to use
	BOOL				bResult;				// Result from function call

	// Check access to all of the pointer-based data in _DEBUG builds
	if(psidUser)     { _ASSERTE(IsValidSid(psidUser)); }

	// Convert the severity and category codes if caller is using auto types

	wSeverityCode = (wSeverity == EVENTLOG_AUTO_SEVERITY) ? 
		SeverityFromEventId(dwEventId) : wSeverity;

	wCategoryCode = (wCategory == EVENTLOG_AUTO_CATEGORY) ?
		CategoryFromEventId(dwEventId) : wCategory;

	// Report the event to the service's event log registry hive
	
	bResult = ReportEvent(m_hEventLog, wSeverityCode, wCategoryCode, dwEventId,
		psidUser, static_cast<WORD>(cInserts), cbBinaryData, rgszInserts, 
		const_cast<void*>(pvBinaryData));

	return (bResult) ? ERROR_SUCCESS : GetLastError();
}

//---------------------------------------------------------------------------
// ServiceEventsBase::ServiceEventsInit (protected)
//
// Initializes the auxiliary service class object
//
// Arguments :
//
//	NONE

DWORD ServiceEventsBase::ServiceEventsInit(void)
{
	// Open a handle to the event log for this service application

	m_hEventLog = RegisterEventSource(NULL, GetServiceName());
	
	return (m_hEventLog) ? ERROR_SUCCESS : GetLastError();
}

//---------------------------------------------------------------------------
// ServiceEventsBase::ServiceEventsInstall
//
// Registers the service's event logging information with the local system
//
// Arguments :
//
//	NONE

DWORD ServiceEventsBase::ServiceEventsInstall(void)
{
	String				strKeyPath;			// Registration key path name
	RegistryKey			regEventLog;		// Parent event log registry key
	String				strMessageFile;		// MessageFile registry string
	String				strCategoryFile;	// CategoryFile registry string
	String				strParameterFile;	// ParameterFile registry string
	DWORD				dwResult;			// Result from function call

	strKeyPath = EVENTLOG_REG_PREFIX;		// Start with the common prefix
	strKeyPath += GetEventLogTarget();		// Append the target logfile name
	strKeyPath += _T('\\');					// Append a backslash character
	strKeyPath += GetServiceName();			// Append the derived service's name

	// Load the complete path name strings for all of the module file names

	if(GetEventLogMessageFile() == NULL) strMessageFile.LoadModuleName();
	else FindMessageModule(GetEventLogMessageFile(), strMessageFile);

	if(GetEventLogCategoryFile() == NULL) strCategoryFile = strMessageFile;
	else FindMessageModule(GetEventLogCategoryFile(), strCategoryFile);

	if(GetEventLogParameterFile() == NULL) strParameterFile = strMessageFile;
	else FindMessageModule(GetEventLogParameterFile(), strParameterFile);

	// Attempt to create/open the specified target event log registry key object

	dwResult = regEventLog.Create(HKEY_LOCAL_MACHINE, strKeyPath, KEY_READ | KEY_WRITE);
	if(dwResult != ERROR_SUCCESS) return dwResult;

	// CategoryCount

	dwResult = regEventLog.SetDWORD(_T("CategoryCount"), GetEventLogCategoryCount());
	if(dwResult != ERROR_SUCCESS) { ServiceEventsRemove(); return dwResult; }

	// CategoryMessageFile

	dwResult = regEventLog.SetExpandString(_T("CategoryMessageFile"), strCategoryFile);
	if(dwResult != ERROR_SUCCESS) { ServiceEventsRemove(); return dwResult; }

	// EventMessageFile

	dwResult = regEventLog.SetExpandString(_T("EventMessageFile"), strMessageFile);
	if(dwResult != ERROR_SUCCESS) { ServiceEventsRemove(); return dwResult; }

	// ParameterMessageFile

	dwResult = regEventLog.SetExpandString(_T("ParameterMessageFile"), strParameterFile);
	if(dwResult != ERROR_SUCCESS) { ServiceEventsRemove(); return dwResult; }

	// TypesSupported

	dwResult = regEventLog.SetDWORD(_T("TypesSupported"), GetEventLogTypesSupported());
	if(dwResult != ERROR_SUCCESS) { ServiceEventsRemove(); return dwResult; }

	return ERROR_SUCCESS;
}

//---------------------------------------------------------------------------
// ServiceEventsBase::ServiceEventsRemove (protected)
//
// Removes the service's event logging registry information from the local system
//
// Arguments :
//
//	NONE

void ServiceEventsBase::ServiceEventsRemove(void)
{
	String				strKeyPath;			// Registration key path name
	RegistryKey			regEventLog;		// Parent event log registry key
	DWORD				dwResult;			// Result from function call

	strKeyPath = EVENTLOG_REG_PREFIX;		// Start with the common prefix
	strKeyPath += GetEventLogTarget();		// Append the target logfile name

	// Attempt to open the parent event log registry key object
	
	dwResult = regEventLog.Open(HKEY_LOCAL_MACHINE, strKeyPath, KEY_WRITE | DELETE);
	if(dwResult != ERROR_SUCCESS) return;

	regEventLog.DeleteSubKey(GetServiceName());		// Remove service entries
}

//---------------------------------------------------------------------------
// ServiceEventsBase::ServiceEventsTerm (protected)
//
// Uninitializes the service auxiliary class object
//
// Arguments :
//
//	NONE

void ServiceEventsBase::ServiceEventsTerm(void)
{
	// If the event log handle has been registered, unregister it

	if(m_hEventLog) DeregisterEventSource(m_hEventLog);
	
	m_hEventLog = NULL;						// Reset the handle to NULL
}

//---------------------------------------------------------------------------
// ServiceEventsBase::SeverityFromEventId (private)
//
// Extracts the severity code from an event id (EVENTLOG_AUTO_SEVERITY)
//
// Arguments :
//
//	dwEventId		- Event id containing the embedded severity code

inline 
const WORD ServiceEventsBase::SeverityFromEventId(DWORD dwEventId) const
{	
	// Break out the severity code from the highest two bits of the event id

	BYTE uSeverity = static_cast<BYTE>((dwEventId >> 30) & 0x3);
	
	// Translate the event code's severity code into the Win32 constant
	// value to be passed to ReportEvent()

	switch(uSeverity) {

		case 0x0 : return EVENTLOG_SUCCESS;
		case 0x1 : return EVENTLOG_INFORMATION_TYPE;
		case 0x2 : return EVENTLOG_WARNING_TYPE;
		default  : return EVENTLOG_ERROR_TYPE;
	}
}

//---------------------------------------------------------------------------

END_NAMESPACE(SVCTL)

#pragma warning(pop)
