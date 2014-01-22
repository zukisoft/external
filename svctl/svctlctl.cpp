//---------------------------------------------------------------------------
// svctlctl.cpp
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
// SVCTL::ServiceControl Implementation
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// ServiceControl::AcceptsControlMask
//
// Determines if the service accepts a specified control code or not
//
// Arguments :
//
//	dwMask			- SERVICE_ACCEPT_XXX bitmask to test against the service

const bool ServiceControl::AcceptsControlMask(DWORD dwMask) const
{
	SERVICE_STATUS		svcStatus;		// Service status information

	if(!m_hService) return false;		// Service has not been opened

	// Retrieve the service's curretn status information from the SCM,
	// and compare the dwControlsAccepted to the provided bitmask

	if(!QueryServiceStatus(m_hService, &svcStatus)) return false;

	return (svcStatus.dwControlsAccepted & dwMask) == dwMask;
}

//---------------------------------------------------------------------------
// ServiceControl::GetDependents
//
// Wraps the call to EnumDependentServices() to make that a bit easier
//
// Arguments :
//
//	ppDependents		- Pointer to receive the LPENUM_SERVICE_STATUS data
//	pcDependents		- Number of dependent services returned
//	dwState				- Optional service state flag -- see the SDK
//
// NOTE : Caller must release *ppDependents with ServiceControl::FreeDependents()

DWORD ServiceControl::GetDependents(LPENUM_SERVICE_STATUS *ppDependents,
											DWORD *pcDependents, DWORD dwState) const
{
	LPENUM_SERVICE_STATUS	pEnumStatus;	// Enumeration buffer pointer
	DWORD					cbEnumStatus;	// Size of enumeration buffer
	DWORD					cDependents;	// Number of dependent services

	_ASSERTE(!IsBadWritePtr(ppDependents, sizeof(LPENUM_SERVICE_STATUS)));
	_ASSERTE(!IsBadWritePtr(pcDependents, sizeof(DWORD)));

	*ppDependents = NULL;				// Initialize [out] pointer to NULL
	*pcDependents = 0;					// Initialize [out] counter to zero
	
	if(!m_hService) return ERROR_INVALID_HANDLE;	// Service is not open
	
	// Determine the size of the buffer that needs to be allocated.  If this
	// initial call to EnumDependents() succeeds, there are no dependent services
	
	if(EnumDependentServices(m_hService, dwState, NULL, 0, &cbEnumStatus, 
		&cDependents)) return ERROR_SUCCESS;
	
	if(GetLastError() != ERROR_MORE_DATA) return GetLastError();

	// Attempt to allocate a buffer large enough to hold the enumeration data

	pEnumStatus = reinterpret_cast<LPENUM_SERVICE_STATUS>(SVCTL::AllocMem(cbEnumStatus));
	if(!pEnumStatus) return ERROR_NOT_ENOUGH_MEMORY;

	// Enumerate the running services that depend upon this service (for real)

	if(!EnumDependentServices(m_hService, dwState, pEnumStatus, cbEnumStatus, 
		&cbEnumStatus, &cDependents)) {
		
		SVCTL::FreeMem(pEnumStatus);		// Release the allocated buffer
		return GetLastError();				// Return the error code
	}

	*ppDependents = pEnumStatus;			// Copy the buffer pointer
	*pcDependents = cDependents;			// Copy the number of dependents

	return ERROR_SUCCESS;
}

//---------------------------------------------------------------------------
// ServiceControl::IsServiceStatus (private)
//
// Determines if the current status is the same as a specified status code
//
// Arguments :
//
//	dwStatus		- Status code to check against the service

const bool ServiceControl::IsServiceStatus(DWORD dwStatus) const
{
	SERVICE_STATUS		svcStatus;		// Service status information

	if(!m_hService) return false;		// Service has not been opened
	
	// Retrieve the service's current status information from the SCM,
	// and compare the dwCurrentStatus to the provided status code

	if(!QueryServiceStatus(m_hService, &svcStatus)) return false;

	return svcStatus.dwCurrentState == dwStatus;
}

//---------------------------------------------------------------------------
// ServiceControl::Open
//
// Opens the requested service on the target service so that operations may
// be performed against it
//
// Arguments :
//
//	pszService			- Name of the service being opened
//	dwAccess			- Requested service access rights

DWORD ServiceControl::Open(LPCTSTR pszService, DWORD dwAccess)
{
	DWORD			dwResult;				// Result from function call

	Close();					// Make sure any existing handles are closed
	
	// Always add SERVICE_QUERY_STATUS to the requested access mask, and also
	// add SERVICE_ENUMERATE_DEPENDENTS if SERVICE_STOP was requested

	dwAccess |= SERVICE_QUERY_STATUS;
	if((dwAccess & SERVICE_STOP) == SERVICE_STOP) dwAccess |= SERVICE_ENUMERATE_DEPENDENTS;
	
	// Attempt to open the Service Control Manager database on the target server

	m_hSCM = OpenSCManager(m_strServerName, NULL, SC_MANAGER_CONNECT);
	if(!m_hSCM) return GetLastError();
	
	// Attempt to open the requested service object with the requested access

	m_hService = OpenService(m_hSCM, pszService, dwAccess);
	if(!m_hService) {

		dwResult = GetLastError();			// Store the resultant error code
		m_hSCM.Close();						// Close the Service Control Manager
		return dwResult;					// Return error from OpenService()
	}

	return ERROR_SUCCESS;
}

//---------------------------------------------------------------------------
// ServiceControl::Pause
//
// Pauses the service
//
// Arguments :
//
//	hevtCancel		- Optional event object that will cancel the wait

DWORD ServiceControl::Pause(HANDLE hevtCancel) const
{
	SERVICE_STATUS			svcStatus;		// Service status information

	if(!m_hService) return ERROR_INVALID_HANDLE;
	
	// Retrieve the service's current status information from the SCM.
	// If the service is already paused, there is no reason to pause it

	if(!QueryServiceStatus(m_hService, &svcStatus)) return GetLastError();
	if(svcStatus.dwCurrentState == SERVICE_PAUSED) return ERROR_SUCCESS;

	// Send this service the SERVICE_CONTROL_PAUSE control and wait
	// until it reaches the SERVICE_PAUSED state

	if(!ControlService(m_hService, SERVICE_CONTROL_PAUSE, &svcStatus))
		return GetLastError();

	return WaitForService(m_hService, SERVICE_PAUSED, &svcStatus, hevtCancel);
}

//---------------------------------------------------------------------------
// ServiceControl::Resume
//
// Resumes the service, if paused
//
// Arguments :
//
//	hevtCancel		- Optional event object that will cancel the wait

DWORD ServiceControl::Resume(HANDLE hevtCancel) const
{
	SERVICE_STATUS			svcStatus;		// Service status information

	if(!m_hService) return ERROR_INVALID_HANDLE;
	
	// Retrieve the service's current status information from the SCM.
	// If the service is already running, there is no reason to resume it

	if(!QueryServiceStatus(m_hService, &svcStatus)) return GetLastError();
	if(svcStatus.dwCurrentState == SERVICE_RUNNING) return ERROR_SUCCESS;

	// Send this service the SERVICE_CONTROL_CONTINUE control and wait
	// until it reaches the SERVICE_RUNNING state

	if(!ControlService(m_hService, SERVICE_CONTROL_CONTINUE, &svcStatus))
		return GetLastError();

	return WaitForService(m_hService, SERVICE_RUNNING, &svcStatus, hevtCancel);
}

//---------------------------------------------------------------------------
// ServiceControl::Start
//
// Starts the target service object
//
// Arguments :
//
//	dwArgc			- Number of command line arguments to pass to service
//	rgszArgv		- Array of command line arguments to pass to service
//	hetvCancel		- Optional kernel event handle to cancel wait operations

DWORD ServiceControl::Start(DWORD dwArgc, LPCTSTR *rgszArgv, HANDLE hevtCancel) const
{
	SERVICE_STATUS			svcStatus;		// Service status information

	if(!m_hService) return ERROR_INVALID_HANDLE;
	
	// Retrieve the service's current status information from the SCM.
	// If the state is anything other than STOPPED, it's already started

	if(!QueryServiceStatus(m_hService, &svcStatus)) return GetLastError();
	if(svcStatus.dwCurrentState != SERVICE_STOPPED) return ERROR_SUCCESS;

	// Attempt to start the service with the specified command arguments, and
	// retrieve the initial SERVICE_STATUS data before waiting on the service

	if(!StartService(m_hService, dwArgc, rgszArgv)) return GetLastError();
	if(!QueryServiceStatus(m_hService, &svcStatus)) return GetLastError();

	return WaitForService(m_hService, SERVICE_RUNNING, &svcStatus, hevtCancel);
}

//---------------------------------------------------------------------------
// ServiceControl::StopEx
//
// Attempts to stop a service, along with any services that depend on it
//
// Arguments :
//
//	hevtCancel		- Optional kernel event handle to cancel wait operations

DWORD ServiceControl::StopEx(HANDLE hevtCancel) const
{
	LPENUM_SERVICE_STATUS	pDependents;	// Pointer to dependent services
	DWORD					cDependents;	// Number of dependent services
	DWORD					dwIndex;		// Loop index variable
	SvcHandle				hDependent;		// Handle to a dependent service
	DWORD					dwResult;		// Result from function call

	// Retrieve the list of services that depend on this service to be running
	
	dwResult = GetDependents(&pDependents, &cDependents);
	if(dwResult != ERROR_SUCCESS) return dwResult;

	// Loop through the dependent services and try to stop each of them

	for(dwIndex = 0; dwIndex < cDependents; dwIndex++) {

		// Attempt to open the service with the necessary access rights
	
		hDependent = OpenService(m_hSCM, pDependents[dwIndex].lpServiceName,
			SERVICE_QUERY_STATUS | SERVICE_STOP);
		if(!hDependent) { 
			
			FreeDependents(pDependents);	// Release dependent services
			return GetLastError();			// Return the error code
		}

		// Attempt to stop the dependent service object

		dwResult = StopService(hDependent, hevtCancel);
		if(dwResult != ERROR_SUCCESS) {

			FreeDependents(pDependents);	// Release dependent services
			return dwResult;				// Return the error code
		}
	}

	FreeDependents(pDependents);					// Release dependent services
	return StopService(m_hService, hevtCancel);		// Stop the target service
}

//---------------------------------------------------------------------------
// ServiceControl::StopService (private)
//
// Stops a single service on the target server
//
// Arguments :
//
//	hService		- Handle to the target service object
//	hevtCancel		- Optional kernel event handle to cancel operation

DWORD ServiceControl::StopService(SC_HANDLE hService, HANDLE hevtCancel) const
{
	SERVICE_STATUS			svcStatus;		// Service status information

	// Retrieve the service's current status information from the SCM.
	// If the service is already stopped, there is no reason to stop it

	if(!QueryServiceStatus(hService, &svcStatus)) return GetLastError();
	if(svcStatus.dwCurrentState == SERVICE_STOPPED) return ERROR_SUCCESS;

	// Send this service the SERVICE_CONTROL_STOP control

	if(!ControlService(hService, SERVICE_CONTROL_STOP, &svcStatus)) 
		return GetLastError();

	return WaitForService(hService, SERVICE_STOPPED, &svcStatus, hevtCancel);
}

//---------------------------------------------------------------------------
// ServiceControl::UserControl
//
// Sends a user-defined control to the service (also used for all the other
// non-waitable service controls)
//
// Arguments :
//
//	NONE

DWORD ServiceControl::UserControl(DWORD dwControl) const
{
	SERVICE_STATUS			svcStatus;		// Service status information

	if(!m_hService) return ERROR_INVALID_HANDLE;

	// We cannot wait for user controls to be processed, so all we can do
	// is rely upon the result from the ControlService() function
	
	if(!ControlService(m_hService, dwControl, &svcStatus)) return GetLastError();

	return ERROR_SUCCESS;				// Successfully controlled service
}

//---------------------------------------------------------------------------
// ServiceControl::WaitForService (private)
//
// Waits for the service to reach a specific state
//
// Arguments :
//
//	hService			- Handle to the service to be waited upon
//	dwDesiredState		- Desired ultimate state of the service
//	pSvcStatus			- Pointer to an initialized SERVICE_STATUS struct
//	hevtCancel			- Optional kernel event to trigger a cancellation

DWORD ServiceControl::WaitForService(SC_HANDLE hService, DWORD dwDesiredState, 
						LPSERVICE_STATUS pSvcStatus, HANDLE hevtCancel) const
{
	DWORD		dwCheckpoint;			// Service's previous checkpoint
	DWORD		dwTickCount;			// Checkpoint tick counter
	DWORD		dwWait;					// Time to wait between queries

	_ASSERTE(hService != NULL);			// Should never be NULL
	
	dwTickCount = GetTickCount();					// Set starting counter
	dwCheckpoint = pSvcStatus->dwCheckPoint;		// Get check point

	// Loop forever until the service reaches the desired state or fails

	while(pSvcStatus->dwCurrentState != dwDesiredState) {

		dwWait = pSvcStatus->dwWaitHint / 10;		// Calculate interval

		if(dwWait < 1000) dwWait = 1000;			// Minimum = 1 second
		else if(dwWait > 10000) dwWait = 10000;		// Maximum = 10 seconds

		// If there is no event, we can simply sleep for dwWait milliseconds.
		// Otherwise, we need to wait on the event in case it fires
		
		if(!hevtCancel) Sleep(dwWait);

		else if(WaitForSingleObject(hevtCancel, dwWait) == WAIT_OBJECT_0)
			return ERROR_CANCELLED;

		// Get the most up to date information about the service's status

		if(!QueryServiceStatus(hService, pSvcStatus)) return GetLastError();

		// If the desired state was reached, skip back up to the loop condition

		if(pSvcStatus->dwCurrentState == dwDesiredState) continue;
		
		// If the checkpoint is increasing, we reset and loop back again
		
		if(pSvcStatus->dwCheckPoint > dwCheckpoint) {

			dwTickCount = GetTickCount();				// Reset starting counter
			dwCheckpoint = pSvcStatus->dwCheckPoint;	// Save new checkpoint
		}

		// Otherwise, if the wait hint has expired the service has failed

		else if((GetTickCount() - dwTickCount) > pSvcStatus->dwWaitHint) {

			if(pSvcStatus->dwWin32ExitCode) return pSvcStatus->dwWin32ExitCode;
			else return ERROR_SERVICE_REQUEST_TIMEOUT;
		}
	}

	// Check one more time to see if the user cancelled the operation or not
	
	if((hevtCancel) && (WaitForSingleObject(hevtCancel, 0) == WAIT_OBJECT_0))
		return ERROR_CANCELLED;
	
	return ERROR_SUCCESS;			// Service reached the desired state
}

//---------------------------------------------------------------------------

END_NAMESPACE(SVCTL)

#pragma warning(pop)
