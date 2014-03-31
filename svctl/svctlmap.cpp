//---------------------------------------------------------------------------
// svctlmap.cpp
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
// SVCTL::ServiceMapEntry Implementation
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// ServiceMapEntry::Activate
//
// Attempts to "activate" the service class object by calling the static
// _Construct() member function
//
// Arguments :
//
//	NONE

DWORD ServiceMapEntry::Activate(void)
{
	DWORD			dwResult;				// Result from function call
	
	if(m_pService) return ERROR_SUCCESS;			// Class is already active

	_ASSERTE(pfnConstruct != NULL);					// Should never happen
	if(!pfnConstruct) return ERROR_INVALID_DATA;	// Bad function pointer

	// Attempt to create the service class object via it's custom _Construct func

	m_pService = pfnConstruct();
	if(!m_pService) return ERROR_NOT_ENOUGH_MEMORY;

	// Attempt to initialize the service class object, destroying it on failure

	dwResult = m_pService->BaseClassInit();
	if(dwResult != ERROR_SUCCESS) { delete m_pService; m_pService = NULL; }

	return dwResult;
}
	
//---------------------------------------------------------------------------
// ServiceMapEntry::DeActivate
//
// DeActivates the service class object by calling the static _Destruct func
//
// Arguments :
//
//	NONE

void ServiceMapEntry::DeActivate(void)
{
	// If the service has been activated, uninitialize and destroy it

	if(m_pService) {
		
		m_pService->BaseClassTerm();	// Uninitialize the class object
		delete m_pService;				// Release the class object
	}

	m_pService = NULL;					// Reset the class pointer to NULL
}

//---------------------------------------------------------------------------
// SVCTL::ServiceMap Implementation
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// ServiceMap::operator[] (service name)
//
// Retrieves a pointer to a ServiceMapEntry by the service's name
//
// Arguments :
//
//	pszServiceName		- Name of the service to be retrieved from the array

ServiceMapEntry* ServiceMap::operator[](LPCTSTR pszServiceName) const
{
	// Don't bother to look unless the string is non-NULL, and there is
	// at least one service entry listed in the array
	
	if((pszServiceName) && (!m_rgEntries.IsEmpty())) {

		// Loop through the array of service class objects, looking for one
		// that matches the specified service name
		
		for(size_t dwIndex = 0; dwIndex < m_rgEntries.Length(); dwIndex++) {

			_ASSERTE(m_rgEntries[dwIndex]->Name != NULL);	// <--- would be bad

			if(_tcsicmp(pszServiceName, m_rgEntries[dwIndex]->Name) == 0)
				return m_rgEntries[dwIndex];
		}
	}

	return NULL;
}

//---------------------------------------------------------------------------
// ServiceMap::CountMapEntries (private)
//
// Determines the number of entries in a SERVICE_MAP structure
//
// Arguments :
//
//	pServiceMap			- Pointer to the SERVICE_MAP to be counted

inline DWORD ServiceMap::CountMapEntries(const PSERVICE_MAP pServiceMap) const
{
	PSERVICE_MAP_ENTRY	pEntry;				// Pointer to a map entry
	DWORD				cEntries = 0;		// Number of map entries

	_ASSERTE(pServiceMap != NULL);			// Should never be NULL
	if(!pServiceMap) return 0;				// Invalid SERVICE_MAP pointer

	// Traverse the SERVICE_MAP in order to count the entries
	
	for(pEntry = pServiceMap; pEntry->pfnConstruct; pEntry++) cEntries++;

	return cEntries;				// Return the number of map entries
}

//---------------------------------------------------------------------------
// ServiceMap::Init
//
// Initializes the service map by copying and validating a SERVICE_MAP
// structure declared somewhere else
//
// Arguments :
//
//	pServiceMap		- Pointer to the SERVICE_MAP to be utilized

DWORD ServiceMap::Init(const PSERVICE_MAP pServiceMap)
{
	size_t			cEntries;			// Number of entries in the map
	size_t			dwIndex;			// Loop index variable
	
	// If the array of service map entries has been allocated, we've
	// already successfully run through the Init() function

	if(!m_rgEntries.IsEmpty()) {
		
		_RPTF0(_CRT_WARN, "Attempting to reinitialize the service map\r\n");
		return ERROR_ALREADY_INITIALIZED;
	}
	
#ifdef _DEBUG

	// Validate that the contents of the SERVICE_MAP are somewhat valid

	if(!ValidateServiceMap(pServiceMap)) return ERROR_INVALID_PARAMETER;

#endif	// _DEBUG

	// Construct an array of ServiceMapEntry objects to manage each of
	// the provided SERVICE_MAP entries

	cEntries = CountMapEntries(pServiceMap);
	if(cEntries) {

		// Allocate a buffer large enough to hold all of the entry pointers

		if(!m_rgEntries.Allocate(cEntries)) return ERROR_NOT_ENOUGH_MEMORY;

		// Loop to convert all of the SERVICE_MAP_ENTRY structures into 
		// ServiceMapEntry class objects
		
		for(dwIndex = 0; dwIndex < cEntries; dwIndex++) {

			m_rgEntries[dwIndex] = new ServiceMapEntry(pServiceMap[dwIndex]);
			if(!m_rgEntries[dwIndex]) { Term(); return ERROR_NOT_ENOUGH_MEMORY; }
		}
	}

	return ERROR_SUCCESS;
}

//---------------------------------------------------------------------------
// ServiceMap::Term
//
// DeActivates and releases all contained ServiceMapEntry class objects
//
// Arguments :
//
//	NONE

void ServiceMap::Term(void)
{
	if(!m_rgEntries) return;			// Nothing to do -- no map entries

	// Walk through the array of ServiceMapEntry objects into order to
	// deactivate and release each one of them

	for(size_t dwIndex = 0; dwIndex < m_rgEntries.Length(); dwIndex++) {

		if(m_rgEntries[dwIndex]) {

			m_rgEntries[dwIndex]->DeActivate();		// Deactivate the service
			delete m_rgEntries[dwIndex];			// Release the class object
		}
	}

	m_rgEntries.Free();					// Release the ServiceMapEntry buffer
}

//---------------------------------------------------------------------------
// ServiceMap::ValidateServiceMap (private)
//
// Performs some basic validation checking on a SERVICE_MAP structure
//
// Arguments :
//
//	pServiceMap			- Pointer to the SERVICE_MAP to be validated

#ifdef _DEBUG

const bool ServiceMap::ValidateServiceMap(const PSERVICE_MAP pServiceMap) const
{
	PSERVICE_MAP_ENTRY	pEntryOuter;	// Pointer to a service map entry
	PSERVICE_MAP_ENTRY	pEntryInner;	// Another service map entry
	DWORD				dwType;			// Service type code bitmask
	
	_ASSERTE(pServiceMap != NULL);		// Should never be NULL
	if(!pServiceMap) return false;		// Invalid service map pointer

	// Make an initial pass through the SERVICE_MAP to validate that all of the 
	// required static function pointers are in place

	for(pEntryOuter = pServiceMap; pEntryOuter->pfnConstruct; pEntryOuter++) {

		if( (!pEntryOuter->pfnGetDisplayName) || (!pEntryOuter->pfnGetServiceName) || 
			(!pEntryOuter->pfnGetServiceType) || (!pEntryOuter->pfnServiceMain) ) 
			return false;
	}

	// Walk the service map again to find bad service type codes, duplicate class 
	// objects, duplicate service name strings, or duplicate display name strings

	for(pEntryOuter = pServiceMap; pEntryOuter->pfnConstruct; pEntryOuter++) {

		dwType = pEntryOuter->pfnGetServiceType();		// Retrieve type codes
		
		// Make sure that the service type is not zero, is not a device driver,
		// and does not specify both OWN_PROCESS and SHARE_PROCESS flags

		if( (dwType == 0) ||
			((dwType & SERVICE_KERNEL_DRIVER) == SERVICE_KERNEL_DRIVER) ||
			((dwType & SERVICE_FILE_SYSTEM_DRIVER) == SERVICE_FILE_SYSTEM_DRIVER) ||
			((dwType & SERVICE_WIN32) == SERVICE_WIN32) ) {

			_RPTF1(_CRT_ASSERT, "The %s service has an invalid SERVICE_TYPE declaration",
				pEntryOuter->pfnGetServiceName());

			return false;					// Invalid service type codes
		}

		// Start the inner loop to check for duplicated stuff in the SERVICE_MAP
		
		for(pEntryInner = pServiceMap; pEntryInner->pfnConstruct; pEntryInner++) {

			if(pEntryOuter != pEntryInner) {
				
				// Make sure that the class objects are not the same

				if(pEntryOuter->pfnConstruct == pEntryInner->pfnConstruct) {

					_RPTF0(_CRT_ASSERT, "Duplicate class objects detected in SERVICE_MAP");
					return false;
				}

				// Make sure that the service names are not the same

				if(_tcsicmp(pEntryOuter->pfnGetServiceName(), 
					pEntryInner->pfnGetServiceName()) == 0) {

					_RPTF0(_CRT_ASSERT, "Duplicate service names detected in SERVICE_MAP");
					return false;
				}

				// Make sure that the service display names are not the same

				if(_tcsicmp(pEntryOuter->pfnGetDisplayName(), 
					pEntryInner->pfnGetDisplayName()) == 0) {

					_RPTF0(_CRT_ASSERT, "Duplicate display names detected in SERVICE_MAP");
					return false;
				}
			}
		}
	}

	return true;
}

#endif	// _DEBUG

//---------------------------------------------------------------------------

END_NAMESPACE(SVCTL)

#pragma warning(pop)
