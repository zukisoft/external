//---------------------------------------------------------------------------
// svctlsts.h
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
// The Original Code is Windows Service Template Library
//
// The Initial Developer of the Original Code is Michael G. Brehm.
// Portions created by the Initial Developer are Copyright (C)2001-2007
// Michael G. Brehm. All Rights Reserved.
//
// Contributor(s):
//	Michael G. Brehm <michael.brehm@verizon.net> (original author)
//---------------------------------------------------------------------------

#ifndef __SVCTLSTS_H_
#define __SVCTLSTS_H_
#pragma once

#pragma warning(push, 4)				// Enable maximum compiler warnings
#pragma warning(disable:4100)			// "unreferenced formal parameter"

//---------------------------------------------------------------------------
// Classes declared in this unit:
//
//	SVCTL::ServiceStatus
//---------------------------------------------------------------------------

BEGIN_NAMESPACE(SVCTL)

//---------------------------------------------------------------------------
// Class SVCTL::ServiceStatus
//
// Provides the management of a service's status information
//---------------------------------------------------------------------------

class ServiceStatus
{
public:

	//-----------------------------------------------------------------------
	// Constructor / Destructor

	ServiceStatus();
	~ServiceStatus() { Term(); }

	//-----------------------------------------------------------------------
	// Overloaded Operators

	operator SERVICE_STATUS_HANDLE() const { return m_hStatus; }

	//-----------------------------------------------------------------------
	// Member Functions

	DWORD AddControls(DWORD dwControls);
	
	DWORD Change(DWORD dwStatus);

	DWORD ChangeControls(DWORD dwControls);

	DWORD Init(LPCTSTR pszServiceName, LPHANDLER_FUNCTION_EX pfnHandler,
		void* pvContext, DWORD dwServiceType, DWORD dwAcceptControls,
		DWORD dwPendingControlMask);
	
	DWORD Query(void) const;

	DWORD RemoveControls(DWORD dwControls);
	
	DWORD Report(void) const;

	void Term(DWORD dwWin32ExitCode = 0, DWORD dwSpecificExitCode = 0);

private:

	ServiceStatus(const ServiceStatus &rhs);				// Disable copy
	ServiceStatus& operator=(const ServiceStatus &rhs);		// Disable assignment

	//-----------------------------------------------------------------------
	// Private Member Functions

	void AdvanceCheckpoint(void);

	static void CALLBACK AdvanceCheckpointProc(void *pvArg, BOOLEAN bFired)
		{ reinterpret_cast<ServiceStatus*>(pvArg)->AdvanceCheckpoint(); }
	
	const bool IsPendingStatus(DWORD dwStatus) const;

	const bool IsRedundantStatus(DWORD dwStatus) const;

	DWORD SetNonPendingStatus(DWORD dwStatus, DWORD dwWin32Exit = 0,
		DWORD dwSpecificExit = 0);

	DWORD SetPendingStatus(DWORD dwStatus, DWORD dwCheckpoint = 1);

	//-----------------------------------------------------------------------
	// Member Variables

	WinHandle		m_hmtxSync;			// Mutex to serialize status changes
	WinHandle		m_hsemPending;		// Semaphore to control pending statuses
	HANDLE			m_hPending;			// Timer queue handle for pending work
	DWORD			m_dwAccept;			// Standard service accepted controls
	DWORD			m_dwPendingMask;	// Pending state accepted control mask
	
	CritSec					m_csStatus;		// Critical section for SERVICE_STATUS
	SERVICE_STATUS_HANDLE	m_hStatus;		// Registered service status handle
	SERVICE_STATUS			m_svcStatus;	// Contained SERVICE_STATUS structure

#ifdef SVCTL_TRACE_STATUS

	String			m_strServiceName;	// Copy of the service name for tracing

#endif	// SVCTL_TRACE_STATUS

};

//---------------------------------------------------------------------------

END_NAMESPACE(SVCTL)

#pragma warning(pop)

#endif	// _SVCTLSTS_H_