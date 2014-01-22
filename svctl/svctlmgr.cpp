//---------------------------------------------------------------------------
// svctlmgr.cpp
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
// SVCTL::ServiceHolder Implementation
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// ServiceHolder::Push
//
// Pushes another ServiceMapEntry class pointer into the pointer array
//
// Arguments :
//
//	pEntry			- Pointer to the ServiceMapEntry class to be pushed

const bool ServiceHolder::Push(ServiceMapEntry *pEntry)
{
	size_t			cEntries;			// Number of elements in the array
	
	_ASSERTE(pEntry != NULL);			// NULL pointers are not allowed
	if(!pEntry) return false;			// Nothing to do - NULL pointer

	cEntries = m_rgEntries.Length();	// Get current array size

	// Attempt to allocate enough space to hold the new class pointer

	if(!m_rgEntries.ReAllocate(cEntries + 1)) return false;

	m_rgEntries[cEntries] = pEntry;		// Push the new pointer in
	return true;
}

//---------------------------------------------------------------------------
// SVCTL::ServiceManager Implementation
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// ServiceManager::ActivateServices (private)
//
// Activates all of the service classes contained in a ServiceHolder
//
// Arguments :
//
//	svcHolder		- ServiceHolder class containing the services

DWORD ServiceManager::ActivateServices(const ServiceHolder &svcHolder) const
{
	ServiceMapEntry*		pEntry;			// Pointer to a service map entry
	size_t					dwIndex;		// Loop index variable
	DWORD					dwResult;		// Result from function call

	// Iterate through the service holder and activate all of the services

	for(dwIndex = 0; dwIndex < svcHolder.Count; dwIndex++) {

		pEntry = svcHolder[dwIndex];		// Get the map entry pointer
		_ASSERTE(pEntry != NULL);			// Should never be NULL

		// Attempt to activate the service, and deactivate all if any fail

		dwResult = pEntry->Activate();
		if(dwResult != ERROR_SUCCESS) {

			_RPTF2(_CRT_ASSERT, "Unable to activate the %s service, result code = %d", 
				pEntry->Name, dwResult);

			DeActivateServices(svcHolder);		// Deactivate all services
			return dwResult;					// Return the failure code
		}
	}

	return ERROR_SUCCESS;
}

//---------------------------------------------------------------------------
// ServiceManager::DeActivateServices (private)
//
// Deactivates all of the service classes contained in a ServiceHolder
//
// Arguments :
//
//	svcHolder		- ServiceHolder containing services to deactivate

void ServiceManager::DeActivateServices(const ServiceHolder &svcHolder) const
{
	ServiceMapEntry*		pEntry;			// Pointer to a service map entry
	size_t					dwIndex;		// Loop index variable

	// Iterate through the service holder to deactivate all of the services

	for(dwIndex = 0; dwIndex < svcHolder.Count; dwIndex++) {

		pEntry = svcHolder[dwIndex];		// Get the map entry pointer
		_ASSERTE(pEntry != NULL);			// Should never be NULL

		pEntry->DeActivate();				// Deactivate the service
	}
}

//---------------------------------------------------------------------------
// ServiceManager::Dispatch
//
// Dispatches either an OWN_PROCESS service or all SHARE_PROCESS services
// to the service control manager.  Does not return until all services
// have reported SERVICE_STOPPED
//
// Arguments :
//
//	pszServiceName		- Optional name of an OWN_PROCESS service to dispatch

DWORD ServiceManager::Dispatch(LPCTSTR pszServiceName) const
{
	ServiceHolder		services;			// Active services pointer holder
	DWORD				dwResult;			// Result from function call

	// Attempt to load up a ServiceHolder object with all of the 
	// SHARE_PROCESS services, or just the single OWN_PROCESS service
	
	dwResult = (pszServiceName == NULL) ? 
		LoadServiceHolder(services, SERVICE_WIN32_SHARE_PROCESS) :
		LoadServiceHolder(services, &pszServiceName, 1);

	// Attempt to activate all of the service classes contained in the holder

	dwResult = ActivateServices(services);
	if(dwResult != ERROR_SUCCESS) return dwResult;

	// Attempt to dispatch all of the service classes contained in the holder

	dwResult = DispatchServices(services);	// Dispatch the services
	DeActivateServices(services);			// Deactivate all services

	return dwResult;						// Return result from Dispatch
}

//---------------------------------------------------------------------------
// ServiceManager::DispatchServices (private)
//
// Dispatches all of the service classes contained in a ServiceHolder
//
// Arguments :
//
//	svcHolder		- ServiceHolder containing services to dispatch

DWORD ServiceManager::DispatchServices(const ServiceHolder &svcHolder) const
{
	Buffer<SERVICE_TABLE_ENTRY>	rgTable;	// SERVICE_TABLE for the SCM
	ServiceMapEntry*			pEntry;		// Pointer to a service map entry
	size_t						dwIndex;	// Loop index variable

	// If there are no services to dispatch (not likely), we can bail out

	if(svcHolder.Count == 0) return ERROR_SUCCESS;

	// Attempt to allocate the SERVICE_TABLE_ENTRY array for the SCM

	if(!rgTable.Allocate(svcHolder.Count + 1)) return ERROR_NOT_ENOUGH_MEMORY;

	// Iterate through the services in the service holder, and load the
	// SERVICE_TABLE with the required information on each one

	for(dwIndex = 0; dwIndex < svcHolder.Count; dwIndex++) {

		pEntry = svcHolder[dwIndex];		// Grab the next service pointer
		_ASSERTE(pEntry != NULL);			// Should never be NULL

		rgTable[dwIndex].lpServiceName = const_cast<LPTSTR>(pEntry->Name);
		rgTable[dwIndex].lpServiceProc = pEntry->ServiceMain;
	}

	// Attach this thread to the service control manager in order to
	// dispatch the requested services

	return (StartServiceCtrlDispatcher(rgTable)) ? ERROR_SUCCESS : GetLastError();
}

//---------------------------------------------------------------------------
// ServiceManager::InitiateRestart (private)
//
// Initiates a system restart if the calling user has privileges to do so
//
// Arguments :
//
//	NONE

DWORD ServiceManager::InitiateRestart(void) const
{
	HANDLE				hToken;			// Current process token handle
	TOKEN_PRIVILEGES	tokenPriv;		// Token privilege information
	DWORD				dwResult;		// Result from function call

	// Open the token for the current process with ADJUST_PRIVILEGES access

	if(!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | 
		TOKEN_QUERY, &hToken)) return GetLastError();

	// Lookup the privilege value for the passed in string

	if(!LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tokenPriv.Privileges[0].Luid)) {

		CloseHandle(hToken);			// Close the process token handle
		return GetLastError();			// Return the failure code
	}

	// Initialize the rest of the TOKEN_PRIVILEGES structure to enable the privilege

	tokenPriv.PrivilegeCount = 1;
	tokenPriv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	
	// Attempt to adjust the privileges of the process token to allow shutdown

	dwResult = (AdjustTokenPrivileges(hToken, FALSE, &tokenPriv, 0, NULL, 0)) ?
		ERROR_SUCCESS : GetLastError();

	CloseHandle(hToken);				// Finished with the process token

	// If the privilege was successfully enabled, attempt to restart the system

	if(dwResult == ERROR_SUCCESS) 
		dwResult = (ExitWindowsEx(EWX_REBOOT, 0)) ? ERROR_SUCCESS : GetLastError();

	return dwResult;	
}

//---------------------------------------------------------------------------
// ServiceManager::Install
//
// Installs a single service onto the local system, or all of the services
// on the local system, if the pszServiceName parameter is NULL
//
// Arguments :
//
//	pszServiceName		- Optional name of the service to be installed
//	pszLogonUser		- Optional service logon account user name
//	pszLogonPassword	- Optional service logon account password

DWORD ServiceManager::Install(LPCTSTR pszServiceName, LPCTSTR pszLogonUser,
									LPCTSTR pszLogonPassword) const
{
	// If no service name was specified, install all services

	if(!pszServiceName) return Install(NULL, 0, pszLogonUser, pszLogonPassword);

	// Otherwise, install only the service matching the specified name

	else return Install(&pszServiceName, 1, pszLogonUser, pszLogonPassword);
}

//---------------------------------------------------------------------------
// ServiceManager::Install
//
// Installs multiple services from this module onto the local system
//
// Arguments :
//
//	rgszServiceNames	- Array containing the names of the services to install
//	cServiceNames		- Number of service names contained in rgszServiceNames
//	pszLogonUser		- Optional service logon account user name
//	pszPassword			- Optional service logon account password

DWORD ServiceManager::Install(LPCTSTR *rgszServiceNames, DWORD cServiceNames,
	LPCTSTR pszLogonUser, LPCTSTR pszLogonPassword) const
{
	ServiceHolder		services;			// Active services pointer holder
	DWORD				dwResult;			// Result from function call

	// Attempt to load up a ServiceHolder object with all of the services
	// that we are going to attempt installation for
	
	dwResult = (rgszServiceNames == NULL) ? LoadServiceHolder(services) :
		LoadServiceHolder(services, rgszServiceNames, cServiceNames);

	// Attempt to activate all of the service classes contained in the holder

	dwResult = ActivateServices(services);
	if(dwResult != ERROR_SUCCESS) return dwResult;

	// Attempt to install all of the service classes contained in the holder

	dwResult = InstallServices(services, pszLogonUser, pszLogonPassword);
	
	DeActivateServices(services);			// Deactivate all services
	return dwResult;						// Return result from Install
}

//---------------------------------------------------------------------------
// ServiceManager::InstallServices (private)
//
// Installs all of the services contained in a ServiceHolder object
//
// Arguments :
//
//	svcHolder		- ServiceHolder containing all services to be installed
//	pszUserName		- Service logon account name to register with the SCM
//	pszPassword		- Service logon account password to register with the SCM

DWORD ServiceManager::InstallServices(const ServiceHolder &svcHolder,
	LPCTSTR pszUserName, LPCTSTR pszPassword) const
{
	ScmLock				scmLock;			// Service control manager lock
	ServiceHolder		svcsInstalled;		// Holds installed services
	ServiceMapEntry*	pEntry;				// Pointer to a service map entry
	size_t				dwIndex;			// Loop index variable
	bool				bRestart = false;	// Flag used to restart the system
	DWORD				dwResult;			// Result from function call

	// Lock the service database so no services can be started while we're
	// installing the services in this module

	dwResult = scmLock.Lock();
	if(dwResult != ERROR_SUCCESS) return dwResult;

	// Iterate through and attempt to install all of the services

	for(dwIndex = 0; dwIndex < svcHolder.Count; dwIndex++) {

		pEntry = svcHolder[dwIndex];		// Get the next service entry
		_ASSERTE(pEntry != NULL);			// Should never be NULL

		dwResult = InstallStatus(pEntry->Install(pszUserName, pszPassword), &bRestart);

		// If the installation was successful, track the service.  Otherwise,
		// prompt the user to see if they want to remove the services that
		// have already been installed

		if(dwResult == ERROR_SUCCESS) svcsInstalled.Push(pEntry);
		else if(dwResult != ERROR_CANCELLED) {

			if(svcsInstalled.Count > 0) {

				Message		msgPrompt;		// SVCTL messagebox object

				// Prompt the user, and try to remove the installed services
				// if they say that's what they want to do
				
				msgPrompt.LoadResource(SVCTL_IDSMGR_REMOVE_PROMPT);
				msgPrompt.Type = MB_ICONWARNING | MB_YESNO;

				if(msgPrompt.Display() == IDYES) RemoveServices(svcsInstalled);
			}

			return dwResult;				// Installation failure code
		}
	}

	// Generate and display a list box with the display names of all the
	// services that were successfully installed

	if(svcsInstalled.Count > 0) {
	
		List		listInstalled;			// SVCTL listbox object
		
		listInstalled.LoadResource(SVCTL_IDSMGR_INSTALL_SUCCESS);
		listInstalled.Type = MB_ICONINFORMATION | MB_OK;
		
		for(dwIndex = 0; dwIndex < svcsInstalled.Count; dwIndex++)
			listInstalled.AddItem(svcsInstalled[dwIndex]->DisplayName);
		
		listInstalled.Display();
	}

	// If the system restart flag has been triggered, prompt the user to
	// restart the system before exiting from the uninstallation process

	if(bRestart) PromptToRestartSystem(SVCTL_IDSMGR_INSTALL_RESTART);
	return ERROR_SUCCESS;
}

//---------------------------------------------------------------------------
// ServiceManager::LoadServiceHolder (private)
//
// Loads the contents of a service holder class with all mapped services
//
// Arguments :
//
//	svcHolder			- The service holder object to be loaded

DWORD ServiceManager::LoadServiceHolder(ServiceHolder &svcHolder) const
{
	ServiceMapEntry*	pEntry;			// Pointer to a service map entry
	size_t				dwIndex;		// Loop index variable

	svcHolder.Clear();					// Remove all previous entries
	
	// Iterate through all of the services in the service map and add them

	for(dwIndex = 0; dwIndex < m_serviceMap.Count; dwIndex++) {

		pEntry = m_serviceMap[dwIndex];		// Get the next service entry
		_ASSERTE(pEntry != NULL);			// Should never be NULL

		if(!svcHolder.Push(pEntry)) return ERROR_NOT_ENOUGH_MEMORY;
	}

	return ERROR_SUCCESS;
}

//---------------------------------------------------------------------------
// ServiceManager::LoadServiceHolder (private)
//
// Loads the contents of a service holder class from an array of service names
//
// Arguments :
//
//	svcHolder			- The service holder object to be loaded
//	rgszServices		- Array of service names to be loaded
//	cServices			- Number of service names in rgszServices

DWORD ServiceManager::LoadServiceHolder(ServiceHolder &svcHolder, 
	LPCTSTR *rgszServices, DWORD cServices) const
{
	ServiceMapEntry*		pEntry;			// Pointer to a service map entry
	DWORD					dwIndex;		// Loop index variable

	svcHolder.Clear();							// Clear all entries
	if(cServices == 0) return ERROR_SUCCESS;	// Nothing to do

	// Make sure that the provided array pointer is non-NULL
	
	_ASSERTE(!IsBadReadPtr(rgszServices, cServices * sizeof(LPCTSTR)));
	if(!rgszServices) return ERROR_INVALID_PARAMETER;

	// Iterate through the service names, and attempt to push them all
	// into the service holder object
	
	for(dwIndex = 0; dwIndex < cServices; dwIndex++) {

		pEntry = m_serviceMap[rgszServices[dwIndex]];
		if(!pEntry) {
			
			_RPTF1(_CRT_ASSERT, "Attempting to load unmapped service %s", 
				rgszServices[dwIndex]);

			return ERROR_INVALID_PARAMETER;		// Service is not mapped
		}

		if(!svcHolder.Push(pEntry)) return ERROR_NOT_ENOUGH_MEMORY;
	}

	return ERROR_SUCCESS;
}

//---------------------------------------------------------------------------
// ServiceManager::LoadServiceHolder (private)
//
// Loads the contents of a service holder based on service type flags
//
// Arguments :
//
//	svcHolder			- The service holder object to be loaded
//	dwType				- Service type flags used for criteria

DWORD ServiceManager::LoadServiceHolder(ServiceHolder &svcHolder, 
											  DWORD dwType) const
{
	ServiceMapEntry*	pEntry;			// Pointer to a service map entry
	size_t				dwIndex;		// Loop index variable

	svcHolder.Clear();					// Remove all previous entries
	
	// Iterate through all of the services in the service map, and add
	// any that match the service type criteria

	for(dwIndex = 0; dwIndex < m_serviceMap.Count; dwIndex++) {

		pEntry = m_serviceMap[dwIndex];		// Get the next service entry
		_ASSERTE(pEntry != NULL);			// Should never be NULL

		// If the service type matches, try to add the service entry

		if((dwType & pEntry->Type) == dwType)
			if(!svcHolder.Push(pEntry)) return ERROR_NOT_ENOUGH_MEMORY;
	}

	return ERROR_SUCCESS;
}

//---------------------------------------------------------------------------
// ServiceManager::PromptToRestartSystem (private)
//
// Prompts the user to restart the system when required by an installation
// or removal procedure
//
// Arguments :
//
//	uFormatId			- Resource string ID to use for the prompt

void ServiceManager::PromptToRestartSystem(UINT uFormatId) const
{
	Message			msgRestart;			// Restart system message box
	DWORD			dwResult;			// Result from function call

	// Initialize the restart message box to be displayed to the user
	
	msgRestart.LoadResource(uFormatId);
	msgRestart.Type = MB_ICONWARNING | MB_YESNO;

	if(msgRestart.Display() == IDYES) {

		// The user chose to restart the system now ... initiate shutdown

		dwResult = InitiateRestart();
		if(dwResult != ERROR_SUCCESS) {

			// Could not initiate the system shutdown and restart process

			msgRestart.LoadResource(SVCTL_IDSMGR_RESTART_FAILED);
			msgRestart.Type = MB_ICONERROR | MB_OK;
			msgRestart.ErrorCode = dwResult;
			msgRestart.Display();
		}
	}
}

//---------------------------------------------------------------------------
// ServiceManager::Remove
//
// Uninstalls a single service from the local system, or all of the services
// if the pszServiceName parameter is NULL
//
// Arguments :
//
//	pszServiceName		- Optional name of a single service to uninstall

DWORD ServiceManager::Remove(LPCTSTR pszServiceName) const
{
	// If a service name was specified, just uninstall that service,
	// otherwise uninstall all of the services from this module

	return (pszServiceName) ? Remove(&pszServiceName, 1) : Remove(NULL, 0);
}

//---------------------------------------------------------------------------
// ServiceManager::Remove
//
// Uninstalls multiple services from the local system
//
// Arguments :
//
//	rgszServiceNames	- Array of service names to be uninstalled
//	cServiceNames		- Number of services names in rgszServiceNames

DWORD ServiceManager::Remove(LPCTSTR *rgszServiceNames, DWORD cServiceNames) const
{
	ServiceHolder		services;			// Active services pointer holder
	DWORD				dwResult;			// Result from function call

	// Attempt to load up a ServiceHolder object with all of the services
	// that we are going to attempt to uninstall from this system
	
	dwResult = (rgszServiceNames == NULL) ? LoadServiceHolder(services) :
		LoadServiceHolder(services, rgszServiceNames, cServiceNames);

	// Attempt to activate all of the service classes contained in the holder

	dwResult = ActivateServices(services);
	if(dwResult != ERROR_SUCCESS) return dwResult;

	dwResult = RemoveServices(services);	// Uninstall the services
	DeActivateServices(services);			// Deactivate all services

	return dwResult;						// Return result from Remove
}

//---------------------------------------------------------------------------
// ServiceManager::RemoveServices (private)
//
// Uninstalls all of the services contained in a ServiceHolder object
//
// Arguments :
//
//	svcHolder		- ServiceHolder containing all services to be removed
//	bSilent			- Flag used to supress the list box at the end

DWORD ServiceManager::RemoveServices(const ServiceHolder &svcHolder) const
{
	ScmLock				scmLock;			// Service control manager lock
	ServiceHolder		svcsRemoved;		// Holds uninstalled services
	ServiceMapEntry*	pEntry;				// Pointer to a service map entry
	LONG				lIndex;				// Signed loop index variable
	bool				bRestart = false;	// Flag to restart the system
	size_t				dwIndex;			// Loop index variable
	DWORD				dwResult;			// Result from function call

	// Lock the service database so no services can be started while we're
	// uninstalling the services in this module

	dwResult = scmLock.Lock();
	if(dwResult != ERROR_SUCCESS) return dwResult;

	// Iterate through and attempt to uninstall all of the services in reverse

	for(lIndex = (static_cast<LONG>(svcHolder.Count) - 1); lIndex >= 0; lIndex--) {

		pEntry = svcHolder[lIndex];				// Get the next service entry
		_ASSERTE(pEntry != NULL);				// Should never be NULL

		dwResult = InstallStatus(pEntry->Remove(), &bRestart);

		// If the uninstallation was successful, track the service,
		// otherwise abort the entire process
		
		if(dwResult == ERROR_SUCCESS) svcsRemoved.Push(pEntry);
		else if(dwResult != ERROR_CANCELLED) return dwResult;
	}

	// Generate and display a list box with the display names of all the
	// services that were successfully uninstalled

	if(svcsRemoved.Count > 0) {
	
		List	listRemoved;				// SVCTL listbox object
		
		listRemoved.LoadResource(SVCTL_IDSMGR_REMOVE_SUCCESS);
		listRemoved.Type = MB_ICONINFORMATION | MB_OK;
		
		for(dwIndex = 0; dwIndex < svcsRemoved.Count; dwIndex++)
			listRemoved.AddItem(svcsRemoved[dwIndex]->DisplayName);
		
		listRemoved.Display();
	}
	
	// If the system restart flag has been triggered, prompt the user to
	// restart the system before exiting from the uninstallation process

	if(bRestart) PromptToRestartSystem(SVCTL_IDSMGR_REMOVE_RESTART);
	return ERROR_SUCCESS;
}

//---------------------------------------------------------------------------

END_NAMESPACE(SVCTL)

#pragma warning(pop)
