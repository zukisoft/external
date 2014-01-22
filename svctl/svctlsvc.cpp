//---------------------------------------------------------------------------
// svctlsvc.cpp
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
// SVCTL::ServiceBase Implementation
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// ServiceBase::_HandlerEx (static)
//
// Static handler callback function registered for each service that derives
// from ServiceBase
//
// Arguments :
//
//	dwControl		- Control code sent from the Service Control Manager
//	dwEventType		- For extended controls, the type of event that occurred
//	pvEventData		- For extended controls, additional event data
//	pvContext		- Service-specific context information

DWORD WINAPI ServiceBase::_HandlerEx(DWORD dwControl, DWORD dwEventType, 
									 void *pvEventData, void *pvContext)
{
	_ASSERTE(pvContext != NULL);		// Should never be NULL

	// Cast the context pointer back into a ServiceBase class pointer,
	// and call through to the HandlerEx() member function
	
	ServiceBase* pThis = reinterpret_cast<ServiceBase*>(pvContext);

	return pThis->HandlerEx(dwControl, dwEventType, pvEventData);
}

//---------------------------------------------------------------------------
// ServiceBase::BaseClassInit (private)
//
// Called by ServiceMapEntry to initialize a newly constructed service object
//
// Arguments :
//
//	NONE

DWORD ServiceBase::BaseClassInit(void)
{
	DWORD			dwResult;			// Result from function call
	
	// Initialize any auxiliary service base classes first
	
	dwResult = AuxiliaryClassInit();
	if(dwResult == ERROR_SUCCESS) {

		// Initialize the derived service class object, making sure to Term()
		// the auxiliary classes upon failure

		dwResult = ClassInit();
		if(dwResult != ERROR_SUCCESS) AuxiliaryClassTerm();
	}

	return dwResult;
}

//---------------------------------------------------------------------------
// ServiceBase::BaseInstall (private)
//
// Called by ServiceMapEntry to install the service on the local system
//
// Arguments :
//
//	pszUserName		- Optional service logon account user name
//	pszPassword		- Optional service logon account password

DWORD ServiceBase::BaseInstall(LPCTSTR pszUserName, LPCTSTR pszPassword)
{
	ServiceInstall		svcInstall;			// Service installation class
	bool				bRestart = false;	// Flag indicating system restart
	DWORD				dwResult;			// Result from function call
	
	// Perform the basic SCM installation first (never requires a restart)
	
	dwResult = svcInstall.Install(this, pszUserName, pszPassword);
	if(dwResult != ERROR_SUCCESS) return dwResult;

	// Install any auxiliary classes with a registered installation hook

	dwResult = InstallStatus(AuxiliaryInstall(), &bRestart);
	if(dwResult == ERROR_SUCCESS) {

		// Call into the derived service's Install() function to let it do
		// any custom installations it requires
		
		dwResult = InstallStatus(Install(), &bRestart);
		if(dwResult != ERROR_SUCCESS) AuxiliaryRemove();
	}

	// If the auxiliary or custom installations failed, delete the service from
	// Service Control Manager before exiting
	
	if(dwResult != ERROR_SUCCESS) svcInstall.Remove(this);

	// If installation was successful, and the system needs to be restarted, 
	// return the special RESTART_REQUIRED status back to ServiceMapEntry

	if((dwResult == ERROR_SUCCESS) && bRestart) return ERROR_SUCCESS_RESTART_REQUIRED;
	else return dwResult;
}

//---------------------------------------------------------------------------
// ServiceBase::BaseRemove (private)
//
// Called by ServiceMapEntry to remove the service from the local system
//
// Arguments :
//
//	NONE

DWORD ServiceBase::BaseRemove(void)
{
	ServiceInstall		svcInstall;			// Service installation class
	bool				bRestart = false;	// Flag indicating system restart
	DWORD				dwResult;			// Result from function call
	
	// Removal takes place in the reverse order of the installation, starting
	// with the custom service uninstallation
	
	dwResult = InstallStatus(Remove(), &bRestart);
	if(dwResult == ERROR_SUCCESS) {

		// If the custom removal succeeded, remove the auxiliary classes
		// and then the service itself from the Service Control Manager

		AuxiliaryRemove();
		dwResult = InstallStatus(svcInstall.Remove(this), &bRestart);
	}

	// If uninstallation was successful, and the system needs to be restarted, 
	// return the special RESTART_REQUIRED status back to ServiceMapEntry

	if((dwResult == ERROR_SUCCESS) && bRestart) return ERROR_SUCCESS_RESTART_REQUIRED;
	else return dwResult;
}

//---------------------------------------------------------------------------
// ServiceBase::DeriveAcceptedControls (private)
//
// Generates the bitmask of control codes accepted by a service class,
// based on the contents of it's SERVICE_CONTROL_MAP structure
//
// Arguments :
//
//	pControlMap		- Pointer to the SERVICE_CONTROL_MAP to work with

DWORD ServiceBase::DeriveAcceptedControls(PSERVICE_CONTROL_MAP pControlMap) const
{
	PSERVICE_CONTROL_ENTRY	pEntry;			// Pointer to a service control entry
	DWORD					dwAccept = 0;	// Generated accepted controls bitmask

	_ASSERTE(pControlMap != NULL);			// Verify pointer is non-NULL

	// Walk through the SERVICE_CONTROL_MAP to examine each control entry
	// and map that into an accepted control bitmask element

	for(pEntry = pControlMap; pEntry->dwControl; pEntry++) {

		switch(pEntry->dwControl) {

			// SERVICE_ACCEPT_STOP
		
			case SERVICE_CONTROL_STOP:

				dwAccept |= SERVICE_ACCEPT_STOP; 
				break;

			// SERVICE_ACCEPT_PAUSE_CONTINUE

			case SERVICE_CONTROL_PAUSE:
			case SERVICE_CONTROL_CONTINUE: 

				dwAccept |= SERVICE_ACCEPT_PAUSE_CONTINUE; 
				break;

			// SERVICE_ACCEPT_SHUTDOWN

			case SERVICE_CONTROL_SHUTDOWN:

				dwAccept |= SERVICE_ACCEPT_SHUTDOWN; 
				break;

			// SERVICE_ACCEPT_PARAMCHANGE

			case SERVICE_CONTROL_PARAMCHANGE:

				dwAccept |= SERVICE_ACCEPT_PARAMCHANGE; 
				break;

			// SERVICE_ACCEPT_NETBINDCHANGE

			case SERVICE_CONTROL_NETBINDADD:
			case SERVICE_CONTROL_NETBINDREMOVE:
			case SERVICE_CONTROL_NETBINDENABLE:
			case SERVICE_CONTROL_NETBINDDISABLE:

				dwAccept |= SERVICE_ACCEPT_NETBINDCHANGE; 
				break;

			// SERVICE_ACCEPT_HARDWAREPROFILECHANGE

			case SERVICE_CONTROL_HARDWAREPROFILECHANGE:

				dwAccept |= SERVICE_ACCEPT_HARDWAREPROFILECHANGE; 
				break;

			// SERVICE_ACCEPT_POWEREVENT

			case SERVICE_CONTROL_POWEREVENT:

				dwAccept |= SERVICE_ACCEPT_POWEREVENT; 
				break;
		}
	}

	return dwAccept;					// Return the generated bitmask
}

//---------------------------------------------------------------------------
// ServiceBase::HandlerEx
//
// HandlerEx is used as the control handler function for all services that
// derive from the ServiceBase class
//
// Arguments :
//
//	dwControl		- Control code sent from the Service Control Manager
//	dwEventType		- For extended controls, the type of event that occurred
//	pvEventData		- For extended controls, additional event data

DWORD ServiceBase::HandlerEx(DWORD dwControl, DWORD dwEventType, void* pvEventData)
{
	DWORD					dwStatus;			// Current service state code
	PSERVICE_CONTROL_ENTRY	pControl;			// Service control handler entry

	// For SERVICE_CONTROL_PAUSE and SERVICE_CONTROL_CONTINUE, we need to block
	// redundant control requests ourselves, since the SCM does not
	
	if((dwControl == SERVICE_CONTROL_PAUSE) || (dwControl == SERVICE_CONTROL_CONTINUE)) {
	
		dwStatus = m_status.Query();			// Retrieve the current service status
		_ASSERTE(dwStatus != 0xFFFFFFFF);		// Should never happen

		// Check if the SERVICE_CONTROL_PAUSE control is redundant
		
		if((dwControl == SERVICE_CONTROL_PAUSE) && ((dwStatus == SERVICE_PAUSED) ||
			(dwStatus == SERVICE_PAUSE_PENDING))) return ERROR_SUCCESS;

		// Check if the SERVICE_CONTROL_CONTINUE control is redundant
		
		else if((dwControl == SERVICE_CONTROL_CONTINUE) && ((dwStatus == SERVICE_RUNNING) ||
			(dwStatus == SERVICE_CONTINUE_PENDING))) return ERROR_SUCCESS;
	}

	// Walk the service's SERVICE_CONTROL_MAP structure to determine if the
	// control is actually implemented by the service
	
	_ASSERTE(GetControlMap() != NULL);				// Should never be NULL

	for(pControl = GetControlMap(); pControl->dwControl; pControl++) {

		if(pControl->dwControl == dwControl) {

			_ASSERTE(pControl->pHandler != NULL);	// Should never be NULL

			// If the control handler requires an automatic PENDING state change
			// before the control handler is invoked, set that up first
			
			if(pControl->bSetPending) {

				switch(dwControl) {

					case SERVICE_CONTROL_STOP: 
						m_status.Change(SERVICE_STOP_PENDING); break;

					case SERVICE_CONTROL_PAUSE:
						m_status.Change(SERVICE_PAUSE_PENDING); break;

					case SERVICE_CONTROL_CONTINUE:
						m_status.Change(SERVICE_CONTINUE_PENDING); break;
				}
			}
			
			// Invoke the control handler function for the specified service control
			
			return InvokeControlHandler(pControl->pHandler, dwEventType, pvEventData);
		}
	}

	return ERROR_CALL_NOT_IMPLEMENTED;			// Control not handled by service
}

//---------------------------------------------------------------------------
// ServiceBase::ServiceMain (protected)
//
// Default ServiceMain() entry point for all services that derive from the
// ServiceBase class object
//
// Arguments :
//
//	dwArgc				- Number of command line arguments
//	rgszArgv			- Array of command line argument strings

void ServiceBase::ServiceMain(DWORD dwArgc, LPTSTR *rgszArgv)
{
	DWORD			dwResult;				// Result from function call

	// Verify that the correct ServiceMain function has been invoked
	
	_ASSERTE(_tcsicmp(GetServiceName(), rgszArgv[0]) == 0);
	
	// Initialize the service status manager, which includes deriving what controls
	// the service will respond to, as well as those blocked during a pending state

	dwResult = m_status.Init(GetServiceName(), _HandlerEx, this, GetServiceType(),
		DeriveAcceptedControls(GetControlMap()), GetBlockedPendingControls());

	if(dwResult != ERROR_SUCCESS) {

		_RPTF2(_CRT_ASSERT, "%s: Unable to initalize ServiceStatus class : EC = %d",
			GetServiceName(), GetLastError());
		return;
	}

	// Initialize the auxiliary service base classes (params, events, etc)

	dwResult = AuxiliaryInit();
	if(dwResult != ERROR_SUCCESS) { m_status.Term(dwResult); return; }

	dwResult = Init(dwArgc, rgszArgv);		// Perform custom initializations
	if(dwResult == ERROR_SUCCESS) {

		dwResult = AuxiliaryStart();		// Start all auxiliary classes
		if(dwResult == ERROR_SUCCESS) {

			// OWN_PROCESS services can normally benefit from swapping out all of the
			// initialization gobbly gook associated with using the SVCTL

			if((GetServiceType() & SERVICE_WIN32_OWN_PROCESS) == SERVICE_WIN32_OWN_PROCESS)
				SetProcessWorkingSetSize(GetCurrentProcess(), -1, -1);
			
			m_status.Change(SERVICE_RUNNING);	// The service is now running
			dwResult = Run();					// Execute the service's main thread
			AuxiliaryStop();					// Stop all auxiliary classes
		}

		Term();								// Perform custom uninitializations
	}

	AuxiliaryTerm();						// Terminate auxiliary base classes
	m_status.Term(dwResult);				// Set SERVICE_STOPPED status
}

//---------------------------------------------------------------------------

END_NAMESPACE(SVCTL)