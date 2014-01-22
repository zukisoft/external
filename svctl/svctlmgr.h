//---------------------------------------------------------------------------
// svctlmgr.h
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

#ifndef __SVCTLMGR_H_
#define __SVCTLMGR_H_
#pragma once

#pragma warning(push, 4)				// Enable maximum compiler warnings
#pragma warning(disable:4127)			// "conditional expression is constant"

//---------------------------------------------------------------------------
// Classes declared in this unit:
//
//	SVCTL::ServiceTracker
//	SVCTL::ServiceManager
//---------------------------------------------------------------------------

BEGIN_NAMESPACE(SVCTL)

//---------------------------------------------------------------------------
// Class SVCTL::ServiceHolder
//
// Implements a simple buffer that holds ServiceMapEntry class pointers
// that ServiceManager uses to keep track of operations performed upon
// multiple services
//---------------------------------------------------------------------------

class ServiceHolder
{
public:

	//-----------------------------------------------------------------------
	// Constructors / Destructor

	ServiceHolder() {}
	~ServiceHolder() {}

	//-----------------------------------------------------------------------
	// Overloaded Operators

	ServiceMapEntry* operator[] (int index) const
		{ return operator[](static_cast<size_t>(index)); }

	ServiceMapEntry* operator[] (size_t index) const
		{ _ASSERTE(index < m_rgEntries.Length()); return m_rgEntries[index]; }

	//-----------------------------------------------------------------------
	// Member Functions

	void Clear(void) { m_rgEntries.Free(); }

	size_t GetCount(void) const { return m_rgEntries.Length(); }

	const bool Push(ServiceMapEntry* pEntry);

	//-----------------------------------------------------------------------
	// Properties

	__declspec(property(get=GetCount))	size_t Count;

private:

	ServiceHolder(const ServiceHolder &rhs);				// Disable copy
	ServiceHolder& operator=(const ServiceHolder &rhs);		// Disable assignment

	//-----------------------------------------------------------------------
	// Member Variables

	Buffer<ServiceMapEntry*>	m_rgEntries;		// Array of entries
};

//---------------------------------------------------------------------------
// Class SVCTL::ServiceManager
//
// Provides the top-level management for the module's SERVICE_MAP
//---------------------------------------------------------------------------

class ServiceManager
{
public:

	//-----------------------------------------------------------------------
	// Constructor / Destructor

	ServiceManager() {}
	~ServiceManager() { Term(); }

	//-----------------------------------------------------------------------
	// Member Functions

	DWORD Dispatch(LPCTSTR pszServiceName = NULL) const;
	
	DWORD Init(const PSERVICE_MAP pServiceMap)
		{ _ASSERTE(pServiceMap != NULL); return m_serviceMap.Init(pServiceMap); }

	DWORD Install(LPCTSTR pszServiceName = NULL, LPCTSTR pszLogonUser = NULL,
		LPCTSTR pszLogonPassword = NULL) const;

	DWORD Install(LPCTSTR *rgszServiceNames, DWORD cServiceNames, 
		LPCTSTR pszLogonUser = NULL, LPCTSTR pszLogonPassword = NULL) const;

	DWORD Remove(LPCTSTR pszServiceName = NULL) const;

	DWORD Remove(LPCTSTR *rgszServiceNames, DWORD cServiceNames) const;

	void Term(void) { m_serviceMap.Term(); }

private:

	ServiceManager(const ServiceManager &rhs);				// Disable copy
	ServiceManager& operator=(const ServiceManager &rhs);	// Disable assignment

	//-----------------------------------------------------------------------
	// Private Member Functions
	
	DWORD ActivateServices(const ServiceHolder &svcHolder) const;

	void DeActivateServices(const ServiceHolder &svcHolder) const;

	DWORD DispatchServices(const ServiceHolder &svcHolder) const;

	DWORD InitiateRestart(void) const;
	
	DWORD InstallServices(const ServiceHolder &svcHolder,
		LPCTSTR pszUserName, LPCTSTR pszPassword) const;

	DWORD LoadServiceHolder(ServiceHolder &svcHolder) const;
	
	DWORD LoadServiceHolder(ServiceHolder &svcHolder, LPCTSTR *rgszServices,
		DWORD cServices) const;

	DWORD LoadServiceHolder(ServiceHolder &svcHolder, DWORD dwType) const;

	void PromptToRestartSystem(UINT uFormatId) const;

	DWORD RemoveServices(const ServiceHolder &svcHolder) const;

	//-----------------------------------------------------------------------
	// Member Variables

	ServiceMap			m_serviceMap;			// The service map object
};

//---------------------------------------------------------------------------

END_NAMESPACE(SVCTL)

#pragma warning(pop)

#endif	// _SVCTLMGR_H_