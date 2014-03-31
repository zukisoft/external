//---------------------------------------------------------------------------
// svctlmap.h
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

#ifndef __SVCTLMAP_H_
#define __SVCTLMAP_H_
#pragma once

#pragma warning(push, 4)				// Enable maximum compiler warnings
#pragma warning(disable:4127)			// "conditional expression is constant"

//---------------------------------------------------------------------------
// Classes declared in this unit:
//
//	SVCTL::ServiceMapEntry
//	SVCTL::ServiceMap
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// SVCTL_MAP Macros
//
// Used to define the global map of service objects implemented in the module.
// Note : this was called SERVICE_MAP, but that conflicted with ATL

#define BEGIN_SVCTL_MAP(pmap) static SVCTL::SERVICE_MAP_ENTRY pmap[] = {

#define SVCTL_MAP_ENTRY(_class) { _class::_Construct, _class::_ServiceMain, \
_class::_GetServiceName, _class::_GetDisplayName, _class::_GetServiceType },

#define END_SVCTL_MAP() { NULL, NULL, NULL, NULL }};

//---------------------------------------------------------------------------

BEGIN_NAMESPACE(SVCTL)

//---------------------------------------------------------------------------
// Type Declarations

typedef ServiceBase*	(WINAPI * PFN_CONSTRUCT)(void);
typedef void			(WINAPI * PFN_SERVICEMAIN)(DWORD, LPTSTR*);
typedef LPCTSTR			(WINAPI * PFN_GETSERVICENAME)(void);
typedef LPCTSTR			(WINAPI * PFN_GETDISPLAYNAME)(void);
typedef DWORD			(WINAPI * PFN_GETSERVICETYPE)(void);

// SERVICE_MAP_ENTRY - Structure containing pointers to all of the service
// class object's static member functions used by the SVCTL

typedef struct {

	PFN_CONSTRUCT			pfnConstruct;		// Pointer to _Construct()
	PFN_SERVICEMAIN			pfnServiceMain;		// Pointer to _ServiceMain()
	PFN_GETSERVICENAME		pfnGetServiceName;	// Pointer to _GetServiceName()
	PFN_GETDISPLAYNAME		pfnGetDisplayName;	// Pointer to _GetDisplayName()
	PFN_GETSERVICETYPE		pfnGetServiceType;	// Pointer to _GetServiceType()

} SERVICE_MAP_ENTRY, *PSERVICE_MAP_ENTRY, *PSERVICE_MAP;

//---------------------------------------------------------------------------
// Class SVCTL::ServiceMapEntry
//
// Wraps the SERVICE_MAP_ENTRY structure into a managed class object
//---------------------------------------------------------------------------

class ServiceMapEntry : private SERVICE_MAP_ENTRY
{
public:

	//-----------------------------------------------------------------------
	// Constructor / Destructor

	explicit ServiceMapEntry(const SERVICE_MAP_ENTRY &rhs) : m_pService(NULL)
	{
		// Copy the contents of the SERVICE_MAP_ENTRY into our class object
		
		memcpy(static_cast<PSERVICE_MAP_ENTRY>(this), &rhs, sizeof(SERVICE_MAP_ENTRY));
	}

	~ServiceMapEntry() { DeActivate(); }

	//-----------------------------------------------------------------------
	// Member Functions

	DWORD Activate(void);

	void DeActivate(void);

	const LPCTSTR GetDisplayName(void) const { return pfnGetDisplayName(); }
	
	const LPCTSTR GetName(void) const { return pfnGetServiceName(); }

	const PFN_SERVICEMAIN GetServiceMain(void) const { return pfnServiceMain; }

	DWORD GetType(void) const { return pfnGetServiceType(); }

	DWORD Install(LPCTSTR pszUserName, LPCTSTR pszPassword) const;

	const bool IsActive(void) const { return (m_pService != NULL); }

	DWORD Remove(void) const; 

	//-----------------------------------------------------------------------
	// Properties

	__declspec(property(get=GetDisplayName))	LPCTSTR			DisplayName;
	__declspec(property(get=GetName))			LPCTSTR			Name;
	__declspec(property(get=GetServiceMain))	PFN_SERVICEMAIN	ServiceMain;
	__declspec(property(get=GetType))			DWORD			Type;

private:

	ServiceMapEntry(const ServiceMapEntry &rhs);
	ServiceMapEntry& operator=(const ServiceMapEntry &rhs);

	//-----------------------------------------------------------------------
	// Member Variables

	ServiceBase*		m_pService;		// Pointer to activated service object
};

//---------------------------------------------------------------------------
// ServiceMapEntry::Install
//
// Installs the service onto the local system
//
// Arguments :
//
//	pszUserName			- Optional service logon account name
//	pszPassword			- Optional service logon account password

inline DWORD ServiceMapEntry::Install(LPCTSTR pszUserName, 
											LPCTSTR pszPassword) const
{
	// If the service class has been activated, attempt to install it

	if(m_pService) return m_pService->BaseInstall(pszUserName, pszPassword);

	// Otherwise, issue a debug warning and bail with INVALID_FUNCTION

	else {
		
		_RPTF1(_CRT_WARN, "Attempting to install inactive service %s", Name);
		return ERROR_INVALID_FUNCTION;
	}
}

//---------------------------------------------------------------------------
// ServiceMapEntry::Remove
//
// Uninstalls the service from the local system
//
// Arguments :
//
//	NONE

inline DWORD ServiceMapEntry::Remove(void) const
{
	// If the service class has been activated, attempt to uninstall it

	if(m_pService) return m_pService->BaseRemove();

	// Otherwise, issue a debug warning and bail with INVALID_FUNCTION

	else {
		
		_RPTF1(_CRT_WARN, "Attempting to remove inactive service %s", Name);
		return ERROR_INVALID_FUNCTION;
	}
}

//---------------------------------------------------------------------------
// Class SVCTL::ServiceMap
//
// Wraps the SERVICE_MAP structure into a managed class object
//---------------------------------------------------------------------------

class ServiceMap
{
public:

	//-----------------------------------------------------------------------
	// Constructor / Destructor

	ServiceMap() {}
	~ServiceMap() { Term(); }

	//-----------------------------------------------------------------------
	// Overloaded Operators

	operator bool() const { return m_rgEntries.Length() > 0; }

	const bool operator!() { return m_rgEntries.Length() == 0; }
	
	ServiceMapEntry* operator[] (int index) const
		{ return operator[](static_cast<size_t>(index)); }

	ServiceMapEntry* operator[] (size_t index) const
		{ _ASSERTE(index < m_rgEntries.Length()); return m_rgEntries[index]; }

	ServiceMapEntry* operator[] (LPCTSTR pszServiceName) const;

	//-----------------------------------------------------------------------
	// Member Functions

	size_t GetCount(void) const { return m_rgEntries.Length(); }

	DWORD Init(const PSERVICE_MAP pServiceMap);

	void Term(void);

	//-----------------------------------------------------------------------
	// Properties

	__declspec(property(get=GetCount)) size_t Count;

private:

	ServiceMap(const ServiceMap &rhs);					// Disable copy
	ServiceMap& operator=(const ServiceMap &rhs);		// Disable assignment
	
	//-----------------------------------------------------------------------
	// Private Member Functions

	DWORD CountMapEntries(const PSERVICE_MAP pServiceMap) const;

	const bool ValidateServiceMap(const PSERVICE_MAP pServiceMap) const;

	//---------------------------------------------------------------------------
	// Member Variables

	Buffer<ServiceMapEntry*>	m_rgEntries;	// Array of service map entries
};

//---------------------------------------------------------------------------

END_NAMESPACE(SVCTL)

#pragma warning(pop)

#endif	// _SVCTLMAP_H_