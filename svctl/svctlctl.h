//---------------------------------------------------------------------------
// svctlctl.h
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

#ifndef __SVCTLCTL_H_
#define __SVCTLCTL_H_
#pragma once

#pragma warning(push, 4)				// Enable maximum compiler warnings

//---------------------------------------------------------------------------
// Classes declared in this unit:
//
//	SVCTL::ServiceControl
//---------------------------------------------------------------------------

BEGIN_NAMESPACE(SVCTL)

//---------------------------------------------------------------------------
// Class SVCTL::ServiceControl
//
// ServiceControl wraps most of the functionality required by a Service
// Control Program (SCP) to manipulate the status of service objects
//---------------------------------------------------------------------------

class ServiceControl
{
public:

	//-----------------------------------------------------------------------
	// Constructor / Destructor

	ServiceControl() {}
	~ServiceControl() { Close(); UnlockDatabase(); }

	//---------------------------------------------------------------------------
	// Overloaded Operators

	operator SC_HANDLE() const { return m_hService; }

	//-----------------------------------------------------------------------
	// Member Functions

	const bool AcceptsControlMask(DWORD dwMask) const;

	void Close(void) { m_hService.Close(); m_hSCM.Close(); }

	void FreeDependents(LPENUM_SERVICE_STATUS pDependents) const
		{ SVCTL::FreeMem(pDependents); }
	
	DWORD GetDependents(LPENUM_SERVICE_STATUS *ppDependents, DWORD *pcDependents,
		DWORD dwState = SERVICE_ACTIVE) const;

	LPCTSTR GetServerName(void) { return m_strServerName; }

	const bool IsPaused(void) const { return IsServiceStatus(SERVICE_PAUSED); }

	const bool IsRunning(void) const { return IsServiceStatus(SERVICE_RUNNING); }

	const bool IsStopped(void) const { return IsServiceStatus(SERVICE_STOPPED); }

	DWORD LockDatabase(void) { return m_scmLock.Lock(m_strServerName); }

	DWORD NetBindAdd(void) const
		{ return UserControl(SERVICE_CONTROL_NETBINDADD); }

	DWORD NetBindDisable(void) const
		{ return UserControl(SERVICE_CONTROL_NETBINDDISABLE); }

	DWORD NetBindEnable(void) const
		{ return UserControl(SERVICE_CONTROL_NETBINDENABLE); }

	DWORD NetBindRemove(void) const
		{ return UserControl(SERVICE_CONTROL_NETBINDREMOVE); }

	DWORD Open(LPCTSTR pszService, DWORD dwAccess = SERVICE_ALL_ACCESS);

	DWORD ParamChange(void) const
		{ return UserControl(SERVICE_CONTROL_PARAMCHANGE); }

	DWORD Pause(HANDLE hevtCancel = NULL) const;

	void PutServerName(LPCTSTR pszServerName) { Close(); m_strServerName = pszServerName; }

	DWORD Resume(HANDLE hevtCancel = NULL) const;

	DWORD Start(DWORD dwArgc, LPCTSTR *rgszArgv, HANDLE hevtCancel = NULL) const;

	DWORD Stop(HANDLE hevtCancel = NULL) const
		{ return StopService(m_hService, hevtCancel); }

	DWORD StopEx(HANDLE hevtCancel = NULL) const;

	void UnlockDatabase(void) { m_scmLock.Unlock(); }

	DWORD UserControl(DWORD dwControl) const;

	//-----------------------------------------------------------------------
	// Properties

	__declspec(property(get=GetServerName,put=PutServerName)) LPCTSTR ServerName;

private:

	ServiceControl(const ServiceControl &rhs);				// Disable copy
	ServiceControl& operator=(const ServiceControl &rhs);	// Disable assignment

	//-----------------------------------------------------------------------
	// Private Member Functions

	const bool IsServiceStatus(DWORD dwStatus) const;

	DWORD StopService(SC_HANDLE hService, HANDLE hevtCancel) const;
	
	DWORD WaitForService(SC_HANDLE hService, DWORD dwDesiredState, 
		LPSERVICE_STATUS pSvcStatus, HANDLE hevtCancel = NULL) const;

	//-----------------------------------------------------------------------
	// Member Variables

	String			m_strServerName;	// Target server name string
	ScmLock			m_scmLock;			// Service Control Manager lock object
	SvcHandle		m_hSCM;				// Handle to the SCM database
	SvcHandle		m_hService;			// Handle to the target service
};

//---------------------------------------------------------------------------

END_NAMESPACE(SVCTL)

#pragma warning(pop)

#endif	// _SVCTLCTL_H_