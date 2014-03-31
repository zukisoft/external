//---------------------------------------------------------------------------
// svctlsts.cpp
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
// Constants

DWORD STATUS_CHECKPOINT_PERIOD	= 2000;			// 2 seconds
DWORD STATUS_CHECKPOINT_WAITHINT	= 5000;			// 5 seconds
DWORD STATUS_STARTUP_WAITHINT		= 30000;		// 30 seconds

//---------------------------------------------------------------------------
// SVCTL::ServiceStatus Implementation
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// ServiceStatus Constructor
//
// Arguments :
//
// NONE

ServiceStatus::ServiceStatus() : m_hStatus(NULL), m_hPending(NULL), m_dwAccept(0),
m_dwPendingMask(0)
{
	// Initialize the synchronization kernel objects, which are checked
	// for validity in the Init() function and closed automatically

	m_hmtxSync = CreateMutex(NULL, FALSE, NULL);
	m_hsemPending = CreateSemaphore(NULL, 1, 1, NULL);
}

//---------------------------------------------------------------------------
// ServiceStatus::AddControls
//
// Adds accepted service controls to the SERVICE_STATUS structure
//
// Arguments :
//
//	dwControls		- Accepted control mask to be added

DWORD ServiceStatus::AddControls(DWORD dwControls)
{
	AutoCS		autoLock(m_csStatus);		// Automatic critical section

	m_svcStatus.dwControlsAccepted |= dwControls;	// Add in the controls
	return Report();								// Report updated status
}

//---------------------------------------------------------------------------
// ServiceStatus::AdvanceCheckpoint (private)
//
// Entry point for the timer queue thread that automatically increments
// the SERVICE_STATUS checkpoint value
//
// Arguments :
//
//	NONE

void ServiceStatus::AdvanceCheckpoint(void)
{
	AutoCS		autoLock(m_csStatus);		// Automatic critical section

	m_svcStatus.dwCheckPoint++;				// Advance the checkpoint value
	Report();								// Report the updated status
}

//---------------------------------------------------------------------------
// ServiceStatus::Change
//
// Changes the service status information and reports it to the SCM.
//
// Arguments :
//
//	dwStatus			- New service state to be reported to the SCM

DWORD ServiceStatus::Change(DWORD dwStatus)
{
	if(!m_hStatus) return ERROR_INVALID_HANDLE;		// Init() not called

	// SERVICE_START_PENDING and SERVICE_STOPPED are reserved for use by us

	if((dwStatus == SERVICE_START_PENDING) || (dwStatus == SERVICE_STOPPED))
		return ERROR_INVALID_PARAMETER;	
	
	// Delegate the call to the appropriate private member function
	
	return (IsPendingStatus(dwStatus)) ? SetPendingStatus(dwStatus) : 
		SetNonPendingStatus(dwStatus);
}

//---------------------------------------------------------------------------
// ServiceStatus::ChangeControls
//
// Changes the control codes accepted by the service
//
// Arguments :
//
//	dwControls		- Updated accepted control bitmask for the service

DWORD ServiceStatus::ChangeControls(DWORD dwControls)
{
	AutoCS		autoLock(m_csStatus);		// Automatic critical section

	m_svcStatus.dwControlsAccepted = dwControls;	// Change the controls
	return Report();								// Report updated status
}

//---------------------------------------------------------------------------
// ServiceStatus::Init
//
// Registers the service's HandlerEx() callback function, and automatically
// starts a SERVICE_START_PENDING status for the service
//
// Arguments :
//
//	pszServiceName			- Service key name string
//	pfnHandler				- Pointer to the service's HandlerEx() callback
//	pvContext				- Context data to pass into HandlerEx()
//	dwServiceType			- Service type code bitmask
//	dwAcceptControls		- Normal (non-pending) controls accepted by service
//	dwPendingControlMask	- Mask used to block accepted controls while pending

DWORD ServiceStatus::Init(LPCTSTR pszServiceName, LPHANDLER_FUNCTION_EX pfnHandler,
								void *pvContext, DWORD dwServiceType, 
								DWORD dwAcceptControls, DWORD dwPendingControlMask)
{
	DWORD				dwResult;				// Result from function call
	
	_ASSERTE(pszServiceName != NULL);
	_ASSERTE(dwServiceType != 0);
	
	// Check to make sure that the required kernel objects were successfully
	// created in the constructor, and that Init() has not already been called
	
	if((!m_hmtxSync) || (!m_hsemPending)) return ERROR_INVALID_HANDLE;
	if(m_hStatus) return ERROR_ALREADY_INITIALIZED;

	// Attempt to register the service's HandlerEx() callback function

	m_hStatus = RegisterServiceCtrlHandlerEx(pszServiceName, pfnHandler, pvContext);
	if(!m_hStatus) return GetLastError();

	m_dwAccept = dwAcceptControls;			// Copy the accepted controls bitmask
	m_dwPendingMask = dwPendingControlMask;	// Copy the pending controls bitmask

#ifdef SVCTL_TRACE_STATUS

	m_strServiceName = pszServiceName;		// Copy the service name string

#endif	// SVCTL_TRACE_STATUS
	
	// Initialize the SERVICE_STATUS structure as SERVICE_START_PENDING, and
	// quickly make an initial call into Report() to keep the SCM happy
	
	ZeroMemory(&m_svcStatus, sizeof(SERVICE_STATUS));

	m_svcStatus.dwServiceType = dwServiceType;
	m_svcStatus.dwCurrentState = SERVICE_START_PENDING;
	m_svcStatus.dwCheckPoint = 1;
	m_svcStatus.dwWaitHint = STATUS_STARTUP_WAITHINT;

	dwResult = Report();
	if(dwResult != ERROR_SUCCESS) { Term(dwResult); return dwResult; }

	// Initial status report was successful -- launch SERVICE_START_PENDING
	
	return SetPendingStatus(SERVICE_START_PENDING, 2);
}

//---------------------------------------------------------------------------
// ServiceStatus::IsPendingStatus (private)
//
// Determines if a service status code indicates a pending status or not
//
// Arguments :
//
//	dwStatus			- Service status code to test

inline const bool ServiceStatus::IsPendingStatus(DWORD dwStatus) const
{
	// Test the status code against all available pending status codes
	
	return ((dwStatus == SERVICE_START_PENDING) || 
			(dwStatus == SERVICE_PAUSE_PENDING) ||
			(dwStatus == SERVICE_CONTINUE_PENDING) || 
			(dwStatus == SERVICE_STOP_PENDING));
}

//---------------------------------------------------------------------------
// ServiceStatus::IsRedundantStatus (private)
//
// Determines if a given status code matches the current service status code
//
// Arguments :
//
//	dwStatus		- Status code to check against the current service status

inline const bool ServiceStatus::IsRedundantStatus(DWORD dwStatus) const
{
	bool		bRedundant;				// Flag if status code is redundant

	// Lock SERVICE_STATUS just long enough to check the current state code
	// (Using an AutoCS object in an inline function bloats the code a bit)

	m_csStatus.Lock();
	bRedundant = (dwStatus == m_svcStatus.dwCurrentState);
	m_csStatus.Unlock();

	return bRedundant;
}

//---------------------------------------------------------------------------
// ServiceStatus::Query
//
// Returns the current service state code, or 0xFFFFFFFF if uninitialized
//
// Arguments :
//
//	NONE

DWORD ServiceStatus::Query(void) const
{
	AutoCS		autoLock(m_csStatus);			// Automatic critical section

	if(!m_hStatus) return 0xFFFFFFFF;			// Init() was not called
	else return m_svcStatus.dwCurrentState;		// Return the current state
}

//---------------------------------------------------------------------------
// ServiceStatus::RemoveControls
//
// Removes accepted service controls from the SERVICE_STATUS structure
//
// Arguments :
//
//	dwControls		- Bitmask of accepted controls to be removed

DWORD ServiceStatus::RemoveControls(DWORD dwControls)
{
	AutoCS		autoLock(m_csStatus);		// Automatic critical section

	m_svcStatus.dwControlsAccepted &= ~dwControls;	// Remove the controls
	return Report();								// Report updated status
}

//---------------------------------------------------------------------------
// ServiceStatus::Report
//
// Reports the current service status to the Service Control Manager
//
// Arguments :
//
//	NONE

DWORD ServiceStatus::Report(void) const
{
	AutoCS			autoLock(m_csStatus);			// Automatic critical section

	if(!m_hStatus) return ERROR_INVALID_HANDLE;		// Init() has not been called
	
#ifdef SVCTL_TRACE_STATUS

	String	strTraceMsg(m_strServiceName);		// Trace message for debugger

	strTraceMsg.ToUpper();						// Convert into all caps
	strTraceMsg += _T(": ");					// Add colon delimiter

	// Append the textual constant associated with the current service status

	switch(m_svcStatus.dwCurrentState) {

		case SERVICE_STOPPED: strTraceMsg += _T("SERVICE_STOPPED"); break;
		case SERVICE_START_PENDING: strTraceMsg += _T("SERVICE_START_PENDING"); break;
		case SERVICE_STOP_PENDING: strTraceMsg += _T("SERVICE_STOP_PENDING"); break;
		case SERVICE_RUNNING: strTraceMsg += _T("SERVICE_RUNNING"); break;
		case SERVICE_CONTINUE_PENDING: strTraceMsg += _T("SERVICE_CONTINUE_PENDING"); break;
		case SERVICE_PAUSE_PENDING: strTraceMsg += _T("SERVICE_PAUSE_PENDING"); break;
		case SERVICE_PAUSED: strTraceMsg += _T("SERVICE_PAUSED"); break;
		default: strTraceMsg += _T("<<Unknown Service State>>");
	}

	strTraceMsg += _T("\r\n");					// Append required CRLF terminator
	OutputDebugString(strTraceMsg);				// Send trace message to debugger

#endif // SVCTL_TRACE_STATUS
	
	// Update the Service Control Manager with the current service status data
	
	return (SetServiceStatus(m_hStatus, const_cast<LPSERVICE_STATUS>(&m_svcStatus))) 
		? ERROR_SUCCESS : GetLastError();
}

//---------------------------------------------------------------------------
// ServiceStatus::SetNonPendingStatus (private)
//
// Changes the service status to reflect a non-pending state.  Non-pending
// states cancel out any currently pending auto-update operations
//
// Arguments :
//
//	dwStatus			- Non-pending status to be reported
//	dwWin32Exit			- Optional Win32 exit code to report on SERVICE_STOPPED
//	dwSpecificExit		- Optional service-specific exit code to report on STOPPED

DWORD ServiceStatus::SetNonPendingStatus(DWORD dwStatus, DWORD dwWin32Exit,
											   DWORD dwSpecificExit)
{
	DWORD			dwResult;				// Result from function call

	_ASSERTE(m_hStatus != NULL);			// Init() has not been called
	_ASSERTE(!IsPendingStatus(dwStatus));	// Make sure status is correct
	
	// Test for a redundant status code, and bail out if necessary

	if(IsRedundantStatus(dwStatus)) return ERROR_SUCCESS;
	
	// Non-pending status changes only require us to wait for the mutex

	dwResult = WaitForSingleObject(m_hmtxSync, INFINITE);
	if(dwResult == WAIT_FAILED) return GetLastError();

	// If a pending status is currently taking place, cancel the timer queue
	// and release the pending semaphore object

	if(m_hPending) {

		DeleteTimerQueueTimer(NULL, m_hPending, INVALID_HANDLE_VALUE);
		m_hPending = NULL;

		ReleaseSemaphore(m_hsemPending, 1, NULL);	// Release pending semaphore
	}

	m_csStatus.Lock();						// Lock down SERVICE_STATUS

	// Configure the SERVICE_STATUS data to reflect the non-pending state
	
	m_svcStatus.dwCurrentState = dwStatus;
	m_svcStatus.dwControlsAccepted = m_dwAccept;
	m_svcStatus.dwWin32ExitCode = dwWin32Exit;
	m_svcStatus.dwServiceSpecificExitCode = dwSpecificExit;
	m_svcStatus.dwCheckPoint = 0;
	m_svcStatus.dwWaitHint = 0;

	dwResult = Report();					// Report the updated status

	m_csStatus.Unlock();					// Release SERVICE_STATUS lock
	ReleaseMutex(m_hmtxSync);				// Release synchronization mutex

	return dwResult;
}

//---------------------------------------------------------------------------
// ServiceStatus::SetPendingStatus (private)
//
// Changes the service status to reflect a pending state.  Pending states
// are automatically updated by a timer queue worker thread
//
// Arguments :
//
//	dwStatus			- Pending status to be reported
//	dwCheckpoint		- Optional initial checkpoint value to use

DWORD ServiceStatus::SetPendingStatus(DWORD dwStatus, DWORD dwCheckpoint)
{
	DWORD			dwResult;				// Result from function call
	
	_ASSERTE(m_hStatus != NULL);			// Init() has not been called
	_ASSERTE(IsPendingStatus(dwStatus));	// Make sure status is correct

	// Test for a redundant status code, and bail out if necessary

	if(IsRedundantStatus(dwStatus)) return ERROR_SUCCESS;
	
	// Pending status changes require a wait on both the primary sync mutex
	// as well as the pending semaphore before they can be posted to the SCM
	
	HANDLE phWait[] = { m_hmtxSync, m_hsemPending };
	
	dwResult = WaitForMultipleObjects(2, phWait, TRUE, INFINITE);
	if(dwResult == WAIT_FAILED) return GetLastError();

	m_csStatus.Lock();						// Lock down SERVICE_STATUS

	// Configure the SERVICE_STATUS data to reflect the pending state
	
	m_svcStatus.dwCurrentState = dwStatus;
	m_svcStatus.dwControlsAccepted = (m_dwAccept & ~m_dwPendingMask);
	m_svcStatus.dwWin32ExitCode = 0;
	m_svcStatus.dwServiceSpecificExitCode = 0;
	m_svcStatus.dwCheckPoint = dwCheckpoint;
	m_svcStatus.dwWaitHint = STATUS_CHECKPOINT_WAITHINT;
	
	dwResult = Report();					// Report the updated status
	m_csStatus.Unlock();					// Release SERVICE_STATUS

	if(dwResult == ERROR_SUCCESS) {

		_ASSERTE(m_hPending == NULL);		// This should be NULL here
		
		// The initial status update was successful.  Create a timer queue entry
		// that will call into AdvanceCheckpoint() every second or so
		
		dwResult = (CreateTimerQueueTimer(&m_hPending, NULL, AdvanceCheckpointProc,
			this, 0, STATUS_CHECKPOINT_PERIOD, 0)) ? ERROR_SUCCESS : GetLastError();
	}
	
	ReleaseMutex(m_hmtxSync);				// Release synchronization mutex
	
	return dwResult;
}

//---------------------------------------------------------------------------
// ServiceStatus::Term
//
// Terminates the service status management, and reports SERVICE_STOPPED
// to the Service Control Manager
//
// Arguments :
//
//	dwWin32ExitCode		- Win32 exit code to report to the SCM
//	dwSpecificExitCode	- Service-specific exit code to report to the SCM 

void ServiceStatus::Term(DWORD dwWin32ExitCode, DWORD dwSpecificExitCode)
{
	// Inform the Service Control Manager that the service has been stopped
	
	if(m_hStatus)
		SetNonPendingStatus(SERVICE_STOPPED, dwWin32ExitCode, dwSpecificExitCode);

	m_hStatus = NULL;			// Reset service status handle
	m_dwAccept = 0;				// Reset accepted controls to none
	m_dwPendingMask = 0;		// Reset pending controls mask to none

#ifdef SVCTL_TRACE_STATUS

	m_strServiceName.Clear();	// Erase the stored service name string

#endif	// SVCTL_TRACE_STATUS

}

//---------------------------------------------------------------------------

END_NAMESPACE(SVCTL)

#pragma warning(pop)
