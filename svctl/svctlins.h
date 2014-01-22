//---------------------------------------------------------------------------
// svctlins.h
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

#ifndef __SVCTLINS_H_
#define __SVCTLINS_H_
#pragma once

#include <ntsecapi.h>					// Include LSA declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

//---------------------------------------------------------------------------
// Classes declared in this unit:
//
//	SVCTL::ServiceInstall
//---------------------------------------------------------------------------

BEGIN_NAMESPACE(SVCTL)

class	ServiceBase;			// Forward class declaration (svctlsvc.h)

//---------------------------------------------------------------------------
// Function Prototypes

DWORD InstallStatus(DWORD dwStatus, bool *pbRestart);

//---------------------------------------------------------------------------
// Class SVCTL::ServiceInstall
//
// ServiceInstall provides the functionality required to install and remove
// SVCTL::ServiceBase services on the local system
//---------------------------------------------------------------------------

class ServiceInstall
{
public:

	//-----------------------------------------------------------------------
	// Constructor / Destructor

	ServiceInstall() {}
	~ServiceInstall() {}

	//-----------------------------------------------------------------------
	// Member Functions

	DWORD Install(const ServiceBase *pService, LPCTSTR pszUserName = NULL,
		LPCTSTR pszPassword = NULL) const;

	DWORD Remove(const ServiceBase *pService) const;

private:

	ServiceInstall(const ServiceInstall &rhs);				// Disable copy
	ServiceInstall& operator=(const ServiceInstall &rhs);	// Disable assignment

	//-----------------------------------------------------------------------
	// Private Member Functions

	DWORD ConfigureService(const ServiceBase *pService, LPCTSTR pszUserName,
		LPCTSTR pszPassword, bool bNewInstallation) const;

	DWORD DeleteService(const ServiceBase *pService) const;

	const INT_PTR ErrorMessage(UINT uFormatId, LPCTSTR pszDisplayName,
		DWORD dwErrorCode) const;

	DWORD GrantServiceLogon(PSID psidAccount) const;

	void SetServiceDescription(SC_HANDLE hService, LPCTSTR pszDescription) const;

	void SetServiceFailureActions(SC_HANDLE hService, LPSERVICE_FAILURE_ACTIONS 
		pActions) const;

	DWORD SetupUserAccount(String &strAccountName) const;

	DWORD StopService(LPCTSTR pszService, LPCTSTR pszDisplayName) const;
};

//---------------------------------------------------------------------------

END_NAMESPACE(SVCTL)

#pragma warning(pop)

#endif	// _SVCTLINS_H_