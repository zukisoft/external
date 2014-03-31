//---------------------------------------------------------------------------
// svctlins.cpp
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

const TCHAR	INSTALLER_LOCALSYSTEM_ACCOUNT[]		= _T(".\\LocalSystem");
const TCHAR INSTALLER_BLANK_STRING[]			= _T("");
const TCHAR INSTALLER_SERVICE_ARGUMENT[]		= _T(" -service");

//---------------------------------------------------------------------------
// SVCTL::InstallStatus
//
// Helper function used to convert ERROR_SUCCESS_REBOOT_REQUIRED
// and ERROR_SUCCESS_RESTART_REQUIRED into a status code and a boolean flag
//
// Arguments :
//
//	dwStatus		- Status code returned from an installation function
//	pbRestart		- Pointer to flag to receive restart status

DWORD InstallStatus(DWORD dwStatus, bool *pbRestart)
{
	// If the status code is either REBOOT_REQUIRED or RESTART_REQUIRED,
	// set the boolean flag and change the status code back to ERROR_SUCCESS
	
	if((dwStatus == ERROR_SUCCESS_REBOOT_REQUIRED) || 
		(dwStatus == ERROR_SUCCESS_RESTART_REQUIRED)) {

		*pbRestart = true;			// Set the boolean restart flag
		return ERROR_SUCCESS;		// Change status code to ERROR_SUCCESS
	}

	else return dwStatus;			// Return the unmodified status code
}

//---------------------------------------------------------------------------
// SVCTL::ServiceInstall Implementation
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// ServiceInstall::ConfigureService (private)
//
// Configures a service in the registry of the local machine
//
// Arguments :
//
//	pService			- Pointer to the service to be installed locally
//	pszUserName			- Optional service logon account user name
//	pszPassword			- Optional service logon account password
//	bNewInstallation	- Flag to indicate a new service installation

DWORD ServiceInstall::ConfigureService(const ServiceBase *pService, 
			LPCTSTR pszUserName, LPCTSTR pszPassword, bool bNewInstallation) const
{
	DWORD			dwServiceType;		// Service type code bitmask
	String			strUserName;		// Service logon account name
	String			strPassword;		// Service logon account password
	String			strBinaryPath;		// Service executable pathname
	SvcHandle		hSCM;				// Handle to the service control manager
	SvcHandle		hService;			// Handle to the newly created service
	DWORD			dwResult;			// Result from function call

	_ASSERTE(pService != NULL);						// Can never be NULL
	if(!pService) return ERROR_INVALID_PARAMETER;	// NULL service pointer
	
	dwServiceType = pService->Type;		// Copy the sevrice type codes
	strUserName = pszUserName;			// Copy the username string
	strPassword = pszPassword;			// Copy the password string

	// <--- SETUP SERVICE BINARY PATH -------------------------------

	if(!strBinaryPath.LoadModuleName()) return GetLastError();
	strBinaryPath += INSTALLER_SERVICE_ARGUMENT;

	// OWN_PROCESS services require the service key name be appended to the
	// "-service" argument so ServiceManager knows how to dispatch it

	if((dwServiceType & SERVICE_WIN32_OWN_PROCESS) == SERVICE_WIN32_OWN_PROCESS) {

		strBinaryPath += _T(":\"");				// Append a colon and quote
		strBinaryPath += pService->Name;		// Append service key name
		strBinaryPath += _T("\"");				// Append closing quote
	}
	
	// <--- SETUP SERVICE LOGON ACCOUNT -----------------------------

	// If the service logon account name is NULL or zero length, use the SYSTEM
	// account, and enable desktop interaction in _DEBUG (for assertions)

	if((!strUserName) || (strUserName.Length() == 0)) {

		strUserName = INSTALLER_LOCALSYSTEM_ACCOUNT;	// ".\LocalSystem"
		strPassword = INSTALLER_BLANK_STRING;			// ""

#ifdef _DEBUG

		dwServiceType |= SERVICE_INTERACTIVE_PROCESS;	// Interactive in _DEBUG

#endif	// _DEBUG

	}

	// Otherwise, the specified account must be translated into DOMAIN\ACCOUNT,
	// have the SE_SERVICE_LOGON_NAME privilege granted, and desktop interaction
	// must be disabled for this service
	
	else {

		if(!strPassword) strPassword = INSTALLER_BLANK_STRING;		// ""

		dwResult = SetupUserAccount(strUserName);			// Setup the account
		if(dwResult != ERROR_SUCCESS) return dwResult;		// Failed account setup

		dwServiceType &= ~SERVICE_INTERACTIVE_PROCESS;		// Disable interaction
	}
	
	// <--- CREATE OR OPEN THE SERVICE OBJECT -----------------------
	
	// Attempt to open the service control manager with both SC_MANAGER_CONNECT
	// and SC_MANAGER_CREATE_SERVICE access rights

	hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE);
	if(!hSCM) return GetLastError();

	if(bNewInstallation) {

		// Attempt to create the new service object, using the SERVICE_INSTALL_PARAMS.
		// We need both CHANGE_CONFIG and START access for ChangeServiceConfig2()

		hService = CreateService(hSCM, pService->Name, pService->DisplayName,
			SERVICE_CHANGE_CONFIG | SERVICE_START, dwServiceType, pService->StartType, 
			pService->ErrorControl, strBinaryPath, pService->Group, NULL, 
			pService->Dependencies, strUserName, strPassword);

		if(!hService) return GetLastError();		// Unable to create the service
	}

	else {

		// Attempt to open the existing service with both SERVICE_CHANGE_CONFIG 
		// and SERVICE_START access rights (START is for ChangeServiceConfig2())

		hService = OpenService(hSCM, pService->Name, SERVICE_CHANGE_CONFIG | SERVICE_START);
		if(!hService) return GetLastError();

		// Attempt to change the basic service configuration information to match
		// whatever has been specified in the SERVICE_INSTALL_PARAMS

		if(!ChangeServiceConfig(hService, dwServiceType, pService->StartType,
			pService->ErrorControl, strBinaryPath, pService->Group, NULL,
			pService->Dependencies, strUserName, strPassword, pService->DisplayName))
			return GetLastError();
	}

	// <--- SET DESCRIPTION AND FAILURE ACTIONS ---------------------

	LPSERVICE_FAILURE_ACTIONS	pActions;		// Service failure action structure

	SetServiceDescription(hService, pService->Description);		// Set description

	pActions = pService->GetServiceFailureActions();			// Retrieve actions
	SetServiceFailureActions(hService, pActions);				// Set actions
	if(pActions) pService->FreeServiceFailureActions(pActions);	// Release actions

	return ERROR_SUCCESS;
}

//---------------------------------------------------------------------------
// ServiceInstall::DeleteService (private)
//
// Removes a service object from the local system
//
// Arguments :
//
//	pService		- Pointer to the service to be removed from the system

DWORD ServiceInstall::DeleteService(const ServiceBase *pService) const
{
	SvcHandle		hSCM;				// Service control manager handle
	SvcHandle		hService;			// Service object handle

	// Attempt to open the service control manager with SC_MANAGER_CONNECT access

	hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
	if(!hSCM) return GetLastError();
	
	// Attempt to open the service object with DELETE access rights

	hService = OpenService(hSCM, pService->Name, DELETE);
	if(!hService) return GetLastError();

	// Attempt to remove the service object from the SCM database

	return (::DeleteService(hService)) ? ERROR_SUCCESS : GetLastError();
}

//---------------------------------------------------------------------------
// ServiceInstall::ErrorMessage (private)
//
// Displays the standard "Abort,Retry,Ignore" error message used throughout
// the ServiceInstall class
//
// Arguments :
//
//	uFormatId		- Resource ID of error message format string
//	pszDisplayName	- Service display name string
//	dwErrorCode		- Installation error code

const INT_PTR ServiceInstall::ErrorMessage(UINT uFormatId, LPCTSTR pszDisplayName,
										   DWORD dwErrorCode) const
{
	Message			msgError;				// Error message object

	_ASSERTE(uFormatId != 0);				// Invalid Resource ID code
	_ASSERTE(pszDisplayName != NULL);		// Invalid service display name
	
	// Load the message box format string, and set the error code

	msgError.Format(uFormatId, pszDisplayName);
	msgError.ErrorCode = dwErrorCode;

#if WINVER >= 0x0500

	// WINVER >= 0x0500 -- Use the newer "Cancel, Try Again, Continue" message
	// instead of "Abort, Retry, Ignore" and map the result codes for the caller

	msgError.Type = MB_ICONERROR | MB_CANCELTRYCONTINUE;

	switch(msgError.Display()) {

		case IDCANCEL : return IDABORT;			// IDCANCEL   --> IDABORT
		case IDTRYAGAIN : return IDRETRY;		// IDTRYAGAIN --> IDRETRY
		case IDCONTINUE : return IDIGNORE;		// IDCONTINUE --> IDIGNORE
		default : return 0;						// Error
	}
	
#else

	// WINVER < 0x0500 -- Use the old "Abort, Retry, Ignore" message box

	msgError.Type = MB_ICONERROR | MB_ABORTRETRYIGNORE;
	return msgError.Display();

#endif	// WINVER

}
		
//---------------------------------------------------------------------------
// ServiceInstall::GrantServiceLogon (private)
//
// Attempts to activate the "Log on as a service" account privilege for the
// specified user account
//
// Arguments :
//
//	psidAccount			- SID of the account to grant the privilege to

inline DWORD ServiceInstall::GrantServiceLogon(PSID psidAccount) const
{
	LSA_HANDLE				hLsaPolicy;			// Lsa policy handle
	LSA_OBJECT_ATTRIBUTES	lsaObjectAttrs;		// Lsa object attributes
	LSA_UNICODE_STRING		lucPrivilege;		// Lsa privilege string
	NTSTATUS				ntStatus;			// Result from Lsa function

	_ASSERTE(psidAccount != NULL);				// Should never be NULL

	// Initialize the LSA structures that we need to work with in here

	ZeroMemory(&lsaObjectAttrs, sizeof(LSA_OBJECT_ATTRIBUTES));

	lucPrivilege.Buffer = L"SeServiceLogonRight";
	lucPrivilege.Length = static_cast<USHORT>(wcslen(lucPrivilege.Buffer) * sizeof(WCHAR));
	lucPrivilege.MaximumLength = static_cast<USHORT>(lucPrivilege.Length + sizeof(WCHAR));
	
	// Attempt to open an Lsa policy handle with CREATE_ACCOUNT and LOOKUP_NAMES

	ntStatus = LsaOpenPolicy(NULL, &lsaObjectAttrs, POLICY_CREATE_ACCOUNT | 
		POLICY_LOOKUP_NAMES, &hLsaPolicy);
	
	if(ntStatus == 0) {

		// Attempt to add the SE_SERVICE_LOGON_NAME right to the user account
		
		ntStatus = LsaAddAccountRights(hLsaPolicy, psidAccount, &lucPrivilege, 1);
		LsaClose(hLsaPolicy);
	}

	return LsaNtStatusToWinError(ntStatus);		// Return ultimate status
}

//---------------------------------------------------------------------------
// ServiceInstall::Install
//
// Installs a ServiceBase-based service onto the local system
//
// Arguments :
//
//	pService		- Pointer to the service to be installed
//	pszUserName		- Optional service logon account name
//	pszPassword		- Optional service logon account password

DWORD ServiceInstall::Install(const ServiceBase *pService, LPCTSTR pszUserName,
									LPCTSTR pszPassword) const
{
	Message			msgInstall;			// Installation message box object
	bool			bInstall = true;	// Flag indicating new installation 
	DWORD			dwResult;			// Result from function call
	INT_PTR			nResult;			// Result from user input
	
	_ASSERTE(pService != NULL);						// Bad ServiceBase pointer
	if(!pService) return ERROR_INVALID_PARAMETER;	// Bad ServiceBase pointer

	while(true) {

		// Attempt to install or reconfigure the service (based on bInstall)
		
		dwResult = ConfigureService(pService, pszUserName, pszPassword, bInstall);
		
		if(dwResult == ERROR_SUCCESS) return dwResult;		// <--- Success
		
		// If the error code was SERVICE_EXISTS and a new installation was
		// attempted, ask the user if they want to reconfigure instead
		
		else if((dwResult == ERROR_SERVICE_EXISTS) && bInstall) {
		
			msgInstall.Format(SVCTL_IDSINST_RECONFIGURE_FMT, pService->DisplayName);
			msgInstall.Type = MB_ICONWARNING | MB_YESNO;

			if(msgInstall.Display() == IDYES) { bInstall = false; continue; }
			else return ERROR_CANCELLED;
		}

		// Any other error condition is reported to the user so they can decide
		// what course of action should be taken for this service
		
		else {

			nResult = ErrorMessage(SVCTL_IDSINST_INSTALLERR_FMT, pService->DisplayName,
				dwResult);
		
			// IDABORT  -- Cancel everything by returning the actual error code
			// IDIGNORE -- Cancel this service by returning ERROR_CANCELLED
			// IDRETRY  -- Loop back up and try it again
			
			if(nResult == IDABORT) return dwResult;
			else if(nResult == IDIGNORE) return ERROR_CANCELLED;
		}
	}
}

//---------------------------------------------------------------------------
// ServiceInstall::Remove
//
// Removes a ServiceBase service from the local system.
//
// Arguments :
//
//	pService		- Pointer to the service class to be installed

DWORD ServiceInstall::Remove(const ServiceBase *pService) const
{
	ServiceControl			svcControl;				// Service control object
	LPENUM_SERVICE_STATUS	pDependents = NULL;		// Services that depend on this one
	DWORD					cDependents = 0;		// Number of dependent services
	List					listDependents;			// List of dependent services
	bool					bRestart = false;		// Flag indicating system restart
	DWORD					dwIndex;				// Loop index variable
	INT_PTR					nResult;				// Result from user input
	DWORD					dwResult;				// Result from function call

	_ASSERTE(pService != NULL);						// Bad ServiceBase pointer
	if(!pService) return ERROR_INVALID_PARAMETER;	// Bad ServiceBase pointer
	
	// <--- TEST EXISTANCE AND ENUMERATE DEPENDENTS -----------------
	
	dwResult = svcControl.Open(pService->Name);		// Open the target service
	
	// If the service could not be opened, check for a DOES_NOT_EXIST status
	// and map that into ERROR_CANCELLED for the caller

	if(dwResult != ERROR_SUCCESS)
		return (dwResult == ERROR_SERVICE_DOES_NOT_EXIST) ? ERROR_CANCELLED : dwResult;
	
	// If the service accepts SERVICE_CONTROL_STOP, enumerate it's dependents
	
	if(svcControl.AcceptsControlMask(SERVICE_ACCEPT_STOP)) {

		dwResult = svcControl.GetDependents(&pDependents, &cDependents);
		if(dwResult != ERROR_SUCCESS) return dwResult;
	}

	svcControl.Close();						// Close the service object

	// <--- STOP DEPENDENT SERVICES ---------------------------------
	
	if(cDependents) {

		// Add the display names of all dependent services into the listbox object

		for(dwIndex = 0; dwIndex < cDependents; dwIndex++)
			listDependents.AddItem(pDependents[dwIndex].lpDisplayName);

		// Configure the listbox object header text, footer text, and type codes

		listDependents.Format(SVCTL_IDSINST_STOPDEPENDENTS_FMT, pService->DisplayName);
		listDependents.Footer.LoadResource(SVCTL_IDSINST_STOPDEPENDENTS_FOOTER);
		listDependents.Type = MB_ICONWARNING | MB_YESNO;		

		// Prompt the user to make certain they wish to stop all the services
		
		if(listDependents.Display() != IDYES) {
			
			svcControl.FreeDependents(pDependents);		// Release the dependents
			return ERROR_CANCELLED;						// Operation cancelled
		}

		// Attempt to stop each of the dependent services individually

		for(dwIndex = 0; dwIndex < cDependents; dwIndex++) {

			dwResult = StopService(pDependents[dwIndex].lpServiceName, 
				pDependents[dwIndex].lpDisplayName);

			// ERROR_CANCELLED -- Cancel the entire uninstallation operation
			
			if(dwResult == ERROR_CANCELLED) {

				svcControl.FreeDependents(pDependents);		// Release dependents
				return dwResult;							// Operation cancelled
			}
			
			// Any other failure code means that the system will need to be
			// restarted after the service is removed, so stop trying
			
			else if(dwResult != ERROR_SUCCESS) {

				bRestart = true;		// The system will need to be restarted
				break;					// Break out of the for() loop
			}
		}

		svcControl.FreeDependents(pDependents);			// Release the dependents
	}
	
	// <--- STOP THE TARGET SERVICE ---------------------------------

	// If restart has already been triggered, the attempt will fail due to a
	// dependent service, so there's no need to stop the service

	if(!bRestart) {
	
		dwResult = StopService(pService->Name, pService->DisplayName);

		// ERROR_CANCELLED -- Cancel the entire uninstallation process
		
		if(dwResult == ERROR_CANCELLED) return dwResult;

		// Any other failure code means the system will need to be restarted

		else if(dwResult != ERROR_SUCCESS) bRestart = true;
	}

	// <--- REMOVE THE TARGET SERVICE -------------------------------

	while(true) {
	
		dwResult = DeleteService(pService);		// Attempt to delete the service
		if(dwResult == ERROR_SUCCESS) break;	// Break the loop on success

		else {

			// The service could not be deleted -- ask the user what to do next
			
			nResult = ErrorMessage(SVCTL_IDSINST_REMOVEERR_FMT, pService->DisplayName,
				dwResult);

			// IDABORT  -- Break the loop and let the error code bubble up
			// IDIGNORE -- Cancel this service's uninstallation process
			// IDRETRY  -- Cycle back up and attempt the deletion again
			
			if(nResult == IDABORT) break;
			else if(nResult == IDIGNORE) return ERROR_CANCELLED;
		}
	}

	// If an error occurred, return the error code.  Otherwise, indicate a status
	// of RESTART_REQUIRED to the caller if the system needs to be restarted
	
	if(dwResult != ERROR_SUCCESS) return dwResult;
	else return (bRestart) ? ERROR_SUCCESS_RESTART_REQUIRED : ERROR_SUCCESS;
}

//---------------------------------------------------------------------------
// ServiceInstall::SetServiceDescription (private)
//
// Changes a service's description string in the registry
//
// Arguments :
//
//	hService		- Handle to a service with CHANGE_CONFIG access
//	pszDescription	- New service description string to be set, NULL to clear

inline void ServiceInstall::SetServiceDescription(SC_HANDLE hService, 
												  LPCTSTR pszDescription) const
{
	Buffer<TCHAR>			rgszBuffer;		// Buffer for description string
	SERVICE_DESCRIPTION		description;	// The new service description
	
	_ASSERTE(hService != NULL);				// Should never be NULL

	// If a service description string has been specified, allocate a non-const
	// buffer, and copy the string into it (to avoid access violations)
	
	if(pszDescription) {

		if(!rgszBuffer.Allocate(1025)) return;			// Allocate buffer
		_tcsncpy_s(rgszBuffer, rgszBuffer.Length(), pszDescription, 1024);		// Copy the string

		description.lpDescription = rgszBuffer;			// Set up the pointer
	}

	// Otherwise, indicate a NULL description to erase any current description

	else description.lpDescription = NULL;

	// Change the service's description string in the registry

	ChangeServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, &description);
}

//---------------------------------------------------------------------------
// ServiceInstall::SetFailureActions (private)
//
// Changes a service's failure actions information in the registry
//
// Arguments :
//
//	hService		- Handle to a service with CHANGE_CONFIG access
//	pActions		- Pointer to SERVICE_FAILURE_ACTIONS, NULL to clear

inline void ServiceInstall::SetServiceFailureActions(SC_HANDLE hService,
									LPSERVICE_FAILURE_ACTIONS pActions) const
{
	SERVICE_FAILURE_ACTIONS		defActions;		// Default failure actions
	SC_ACTION					scAction;		// SC_ACTION structure
	
	_ASSERTE(hService != NULL);			// Should never be NULL

	// If the provided pointer is non-NULL, set up the custom failure actions

	if(pActions) {
		ChangeServiceConfig2(hService, SERVICE_CONFIG_FAILURE_ACTIONS, pActions);
	}

	// Otherwise, reset (delete) the service failure actions in the registry

	else {

		scAction.Delay = 0;						// No delay value
		scAction.Type = SC_ACTION_NONE;			// Take no failure action
		
		// Set up the FAILURE_ACTIONS in such a way that the SCM will reset
		// everything back to the normal default values (see the SDK)

		defActions.dwResetPeriod = 0;
		defActions.lpRebootMsg = const_cast<LPTSTR>(INSTALLER_BLANK_STRING);
		defActions.lpCommand = const_cast<LPTSTR>(INSTALLER_BLANK_STRING);
		defActions.cActions = 0;
		defActions.lpsaActions = &scAction;

		ChangeServiceConfig2(hService, SERVICE_CONFIG_FAILURE_ACTIONS, &defActions);
	}
}

//---------------------------------------------------------------------------
// ServiceInstall::SetupUserAccount (private)
//
// Sets up the user account for the service by translating the name provided
// into Domain\UserName format, and granting the account "Log on as a service"
//
// Arguments :
//
//	strAccountName		- Name of the user account used for the service logon

DWORD ServiceInstall::SetupUserAccount(String &strAccountName) const
{
	Buffer<SID, BYTE>		psidAccount;			// Account SID
	DWORD					cbSidAccount = 1024;	// Size of account SID buffer
	Buffer<TCHAR>			rgszRefDomain;			// Referenced account domain
	DWORD					cchRefDomain = 1024;	// Size of domain buffer
	Buffer<TCHAR>			rgszUserName;			// Translated user name
	DWORD					cchUserName = 1024;		// Size of username buffer
	SID_NAME_USE			sidType;				// Type of account SID
	DWORD					dwResult;				// Result from function call

	if(strAccountName.Length() == 0) return ERROR_INVALID_PARAMETER;

	// Attempt to allocate the necessary temporary buffers for this function
	
	if(!psidAccount.Allocate(cbSidAccount)) return ERROR_NOT_ENOUGH_MEMORY;
	if(!rgszUserName.Allocate(cchUserName)) return ERROR_NOT_ENOUGH_MEMORY;
	if(!rgszRefDomain.Allocate(cchRefDomain)) return ERROR_NOT_ENOUGH_MEMORY;
	
	// Attempt to look up the SID for the provided service logon account name

	if(!LookupAccountName(NULL, strAccountName, psidAccount, &cbSidAccount,
		rgszRefDomain, &cchRefDomain, &sidType)) {
		
		// If the LookAccountName() error code is ERROR_NONE_MAPPED, change that into
		// ERROR_INVALID_SERVICE_ACCOUNT instead, since it's more appropriate here
		
		dwResult = GetLastError();
		return (dwResult == ERROR_NONE_MAPPED) ? ERROR_INVALID_SERVICE_ACCOUNT : dwResult;
	}

	dwResult = GrantServiceLogon(psidAccount);		// Grant SE_SERVICE_LOGON_NAME
	if(dwResult != ERROR_SUCCESS) return dwResult;	// Could not grant privilege

	cchUserName = cchRefDomain = 1024;				// Reset buffer lengths

	// Translate the SID back into Domain\UserName to keep the SCM happy.  This
	// really shouldn't ever fail, since LookupAccountName() gave us this SID

	if(!LookupAccountSid(NULL, psidAccount, rgszUserName, &cchUserName, rgszRefDomain,
		&cchRefDomain, &sidType)) return GetLastError();

	// Change the provided account name into Domain\UserName format, which will
	// always be accepted by the Service Control Manager
	
	strAccountName = rgszRefDomain;				// Start with the domain name
	strAccountName += _T("\\");					// Append a backslash character
	strAccountName += rgszUserName;				// Append the user name string

	return ERROR_SUCCESS;
}

//---------------------------------------------------------------------------
// ServiceInstall::StopService (private)
//
// Stops a single service on the local system
//
// Arguments :
//
//	pszService			- Name of the service to be stopped
//	pszDisplayName		- Display name of the service to be stopped

DWORD ServiceInstall::StopService(LPCTSTR pszService, LPCTSTR pszDisplayName) const
{
	ServiceControl		svcControl;			// Service control class object
	WinHandle			hevtCancel;			// Operation cancellation event
	Progress			pgsStop;			// Service stop progress box
	INT_PTR				nResult;			// Result from function call
	DWORD				dwResult;			// Result from function call

	// Attempt to open the service object with SERVICE_STOP access rights

	dwResult = svcControl.Open(pszService, SERVICE_STOP);
	if(dwResult != ERROR_SUCCESS) return dwResult;

	if(svcControl.IsStopped()) return ERROR_SUCCESS;	// Service is stopped

	// Check to see if the service will even accept SERVICE_CONTROL_STOP first
	// (This is done first so the progress will not be displayed)
	
	if(!svcControl.AcceptsControlMask(SERVICE_ACCEPT_STOP)) 
		return ERROR_INVALID_SERVICE_CONTROL;

	// Create the kernel event object that will be used to stop the operation.
	// (If this fails, the CANCEL option will just not be present)
	
	hevtCancel = CreateEvent(NULL, FALSE, FALSE, NULL);
	
	while(true) {
	
		// Set up the "stopping service" progress box and display it to the user

		pgsStop.Format(SVCTL_IDSINST_STOPSERVICE_FMT, pszDisplayName);
		pgsStop.CancelEvent = hevtCancel;
		pgsStop.Start();
		
		dwResult = svcControl.Stop(hevtCancel);		// Stop the service
		pgsStop.Stop();								// Kill the progress box
		
		// A status of ERROR_SUCCESS or ERROR_CANCELLED means we're finished

		if((dwResult == ERROR_SUCCESS) || (dwResult == ERROR_CANCELLED)) return dwResult;
		
		// Any other status code is shown to the user so they can decide what to do

		nResult = ErrorMessage(SVCTL_IDSINST_STOPSERVICE_FMT, pszDisplayName, dwResult);

		// IDABORT  - Cancel the service stop operation
		// IDIGNORE - Let the error code bubble back up to the caller
		// IDRETRY	- Attempt to stop the service again by cyccling back up

		if(nResult == IDABORT) return ERROR_CANCELLED;
		else if(nResult == IDIGNORE) return dwResult;
	}
}

//---------------------------------------------------------------------------

END_NAMESPACE(SVCTL)

#pragma warning(pop)