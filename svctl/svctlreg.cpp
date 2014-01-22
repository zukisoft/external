//---------------------------------------------------------------------------
// svctlreg.cpp
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
// SVCTL::ParameterEntry Implementation
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// ParameterEntry Constructor
//
// Arguments :
//
//	dwType			- Registry value type being contained
//	pszName			- Name of the registry value begin contained
//	pvDefault		- Pointer to the default parameter value.  Can be NULL
//	cbDefault		- Size, in bytes, of the default parameter value

ParameterEntry::ParameterEntry(DWORD dwType, LPCTSTR pszName, const void *pvDefault, 
							   DWORD cbDefault) : m_dwType(dwType)

{
	_ASSERTE(dwType > REG_NONE);	// Should always have a data type
	_ASSERTE(pszName != NULL);		// Should never be NULL

	m_strName = pszName;			// Copy the registry key name

	// If a default value has been specified, copy it into the value buffer now

	if(pvDefault) {

		_ASSERTE(cbDefault > 0);						// Should not be zero
		_ASSERTE(!IsBadReadPtr(pvDefault, cbDefault));	// Check memory access

		if(m_puBuffer.Allocate(cbDefault)) memcpy(m_puBuffer, pvDefault, cbDefault);
	}
}

//---------------------------------------------------------------------------
// ParameterEntry::Load
//
// Loads the registry parameter from the specified registry key object
//
// Arguments :
//
//	hkey		- Handle to the parent registry key (requires KEY_READ)

DWORD ParameterEntry::Load(HKEY hkey)
{
	AutoCS			autoLock(*this);		// Automatic critical section
	DWORD			dwType;					// Type of value retrieved
	DWORD			cbValue = 0;			// Size of the registry value
	DWORD			dwResult;				// Result from function call

	// Determine if the value exists, and how much space needs to be allocated

	dwResult = RegQueryValueEx(hkey, m_strName, NULL, &dwType, NULL, &cbValue);
	if(dwResult != ERROR_SUCCESS) return dwResult;

	_ASSERTE(dwType == m_dwType);						// Type must be correct
	if(dwType != m_dwType) return ERROR_INVALID_DATA;	// Wrong data type

	do { 
	
		// Reallocate the contained value buffer to accomodate the registry data,
		// and re-query the registry to retrieve the value itself

		if(!m_puBuffer.ReAllocate(cbValue)) return ERROR_NOT_ENOUGH_MEMORY;
		dwResult = RegQueryValueEx(hkey, m_strName, NULL, &dwType, m_puBuffer, &cbValue);
	
	} while(dwResult == ERROR_MORE_DATA);

	return dwResult;
}

//---------------------------------------------------------------------------
// ParameterEntry::Save
//
// Saves the registry value back to the specified registry key
//
// Arguments :
//
//	hkey		- Handle to the parent registry key (requires KEY_WRITE)

DWORD ParameterEntry::Save(HKEY hkey) const
{
	AutoCS			autoLock(*this);		// Automatic critical section

	// Attempt to save the contained registry value buffer to the registry

	return RegSetValueEx(hkey, m_strName, NULL, m_dwType, m_puBuffer, 
		static_cast<DWORD>(m_puBuffer.Size()));
}

//---------------------------------------------------------------------------
// ParameterEntry::SetData
//
// Alters the contents of the cached registry value buffer.  Does not
// automatically save the value back to the registry
//
// Arguments :
//
//	pvData		- Pointer to the new value data, NULL to erase contents
//	cbData		- Size of the data passed in pvData, zero if pvData is NULL

DWORD ParameterEntry::SetData(const void *pvData, DWORD cbData)
{
	AutoCS			autoLock(*this);		// Automatic critical section

	// If a NULL data pointer has been passed in, release the cached value

	if((pvData == NULL) || (cbData == 0)) m_puBuffer.Free();

	else {

		// Attempt to reallocate our contained value buffer, and copy
		// in the new value contents as specified by the caller
		
		_ASSERTE(!IsBadReadPtr(pvData, cbData));	// Test memory access

		if(!m_puBuffer.ReAllocate(cbData)) return ERROR_NOT_ENOUGH_MEMORY;
		else memcpy(m_puBuffer, pvData, cbData);
	}

	return ERROR_SUCCESS;
}

//---------------------------------------------------------------------------
// SVCTL::ServiceParametersBase Implementation
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// ServiceParametersBase::FindParameterEntry
//
// Locates a mapped registry parameter object by index number
//
// Arguments :
//
//	dwIndex			- Index value assigned to the registry parameter

ParameterEntry* ServiceParametersBase::FindParameterEntry(DWORD dwIndex) const
{
	PSERVICE_PARAMETER_MAP	pParamMap;		// Pointer to the parameter map

	_ASSERTE(dwIndex > 0);					// Index value cannot be zero

	pParamMap = GetParameterMap();			// Retrieve the PARAMETER_MAP
	_ASSERTE(pParamMap != NULL);			// Should never be NULL

	// Loop through all of the service parameters to locate the requested one
	
	while(pParamMap->pEntry) {

		if(pParamMap->dwIndex == dwIndex) return pParamMap->pEntry;
		else pParamMap++;
	}

	return NULL;				// <--- Parameter not found
};

//---------------------------------------------------------------------------
// ServiceParametersBase::FindParameterEntry
//
// Locates a mapped registry parameter object by name (slower than index)
//
// Arguments :
//
//	pszName			- Name of the registry value to locate

ParameterEntry* ServiceParametersBase::FindParameterEntry(LPCTSTR pszName) const
{
	PSERVICE_PARAMETER_MAP	pParamMap;		// Pointer to the parameter map

	_ASSERTE(pszName != NULL);				// Name string cannot be NULL

	pParamMap = GetParameterMap();			// Retrieve the PARAMETER_MAP
	_ASSERTE(pParamMap != NULL);			// Should never be NULL

	// Loop through all of the service parameters to locate the requested one
	
	while(pParamMap->pEntry) {

		if(_tcsicmp(pszName, pParamMap->pEntry->Name) == 0)	return pParamMap->pEntry;
		else pParamMap++;
	}

	return NULL;				// <--- Parameter not found
};

//---------------------------------------------------------------------------
// ServiceParametersBase::ServiceParamInit (protected)
//
// Initializes the service registry parameters
//
// Arguments :
//
//	NONE

DWORD ServiceParametersBase::ServiceParamInit(void)
{
	PSERVICE_PARAMETER_MAP	pParamMap;		// Pointer to a SERVICE_PARAMETER_ENTRY
	DWORD					dwResult;		// Result from function call

	// Attempt to open the specified service parameters registry key with
	// KEY_READ and KEY_WRITE access rights
	
	dwResult = m_regKey.Open(HKEY_LOCAL_MACHINE, GetParametersKeyName(), 
		KEY_READ | KEY_WRITE);

	if(dwResult != ERROR_SUCCESS) return dwResult;

	pParamMap = GetParameterMap();			// Retrieve the PARAMETER_MAP
	_ASSERTE(pParamMap != NULL);			// Should never be NULL

	// Loop through all of the service parameters and attempt to load each of them.
	// Don't worry too much about failure here, since defaults are provided
	
	while(pParamMap->pEntry) {

		pParamMap->pEntry->Load(m_regKey);	// Load this parameter value
		pParamMap++;						// Move to the next parameter
	}

	return ERROR_SUCCESS;
}

//---------------------------------------------------------------------------
// ServiceParametersBase::ServiceParamInstall (protected)
//
// Installs the default service registry parameters
//
// Arguments :
//
//	NONE

DWORD ServiceParametersBase::ServiceParamInstall(void)
{
	RegistryKey				regKey;		// Registry key wrapper object
	PSERVICE_PARAMETER_MAP	pParamMap;	// Pointer to a SERVICE_PARAMETER_ENTRY
	DWORD					dwResult;	// Result from function call

	// Attempt to create the service parameters registry key object, and
	// open it with both KEY_READ and KEY_WRITE access

	dwResult = regKey.Create(HKEY_LOCAL_MACHINE, GetParametersKeyName(), 
		KEY_READ | KEY_WRITE);

	if(dwResult != ERROR_SUCCESS) return dwResult;

	pParamMap = GetParameterMap();		// Retrieve the PARAMETER_MAP
	_ASSERTE(pParamMap != NULL);		// Should never be NULL

	// Loop through the PARAMETER_MAP and add any parameters that are not already
	// set up in the registry key.  We don't worry about failure too much in here,
	// since the Parameter class will always provide a default value if need be
	
	while(pParamMap->pEntry) {

		if(pParamMap->pEntry->Load(regKey) != ERROR_SUCCESS) 
			pParamMap->pEntry->Save(regKey);

		pParamMap++;					// Move to the next parameter
	}

	return ERROR_SUCCESS;
}

//---------------------------------------------------------------------------
// SVCTL::Parameter Implementation
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Parameter Constructor
//
// Arguments :
//
//	pParent				- Pointer to the parent ParametersBase class
//	dwIndex				- Index value of the requested registry entry
//	dwType				- Expected registry type code (for debugging)

Parameter::Parameter(const ServiceParametersBase *pParent, DWORD dwIndex, DWORD dwType)
: m_pBase(pParent)
{
	_ASSERTE(pParent != NULL);			// Should never be a NULL pointer
	_ASSERTE(dwIndex > 0);				// Zero indexes are not allowed
	_ASSERTE(dwType > REG_NONE);		// Type code must be valid

	m_pEntry = pParent->FindParameterEntry(dwIndex);
	if(m_pEntry) {

		// Check that the correct type code has been specified for the parameter

		if(dwType != m_pEntry->Type) {

			m_pEntry = NULL;		// Reset the parameter pointer to NULL
			
			_RPTF1(_CRT_ASSERT, "Incorrect registry value type code specified "
				"for parameter with index of %d", dwIndex);
		}
	}
		
	else _RPTF1(_CRT_ASSERT, "Unable to locate registry value with index %d", dwIndex);
}

//---------------------------------------------------------------------------
// Parameter Constructor
//
// Arguments :
//
//	pParent				- Pointer to the parent ParametersBase class
//	pszName				- Name of the requested registry entry
//	dwType				- Expected registry type code (for debugging)

Parameter::Parameter(const ServiceParametersBase *pParent, LPCTSTR pszName, DWORD dwType)
: m_pBase(pParent)
{
	_ASSERTE(pParent != NULL);			// Should never be a NULL pointer
	_ASSERTE(pszName != NULL);			// Should never be NULL
	_ASSERTE(dwType > REG_NONE);		// Type code must be valid

	m_pEntry = pParent->FindParameterEntry(pszName);
	if(m_pEntry) {

		// Check that the correct type code has been specified for the parameter

		//////// NEED ASSERTIONS OR SOMETHING IN HERE ////////////

		if(dwType != m_pEntry->Type) m_pEntry = NULL;
	}
}

//---------------------------------------------------------------------------
// BoolParameter Implementation
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// BoolParameter::operator bool
//
// Returns a copy of the value's DWORD, converted to a boolean value
//
// Arguments :
//
//	NONE

BoolParameter::operator bool()
{
	bool			bValue = false;			// Copy of the parameter value

	// If our entry pointer is non-NULL, the requested ParameterEntry object
	// was successfully located in the Parameter constructor

	if(m_pEntry) {

		_ASSERTE(m_pEntry->Type == REG_DWORD);			// Must be REG_DWORD type
		if(m_pEntry->Type != REG_DWORD) return false;	// Invalid data type
		
		if(m_pEntry->Data) {

			_ASSERTE(m_pEntry->Size == sizeof(DWORD));			// Must be 32 bits
			if(m_pEntry->Size != sizeof(DWORD)) return false;	// Invalid data size

			// Atomically retrieve the value contained in the ParameterEntry object
			
			m_pEntry->Lock();
			bValue = (*(reinterpret_cast<DWORD*>(m_pEntry->Data)) != 0);
			m_pEntry->Unlock();
		}
	}
	
	return bValue;
}

//---------------------------------------------------------------------------
// BoolParameter::operator=
//
// Assigns a new value to the underlying registry entry cache.  Does not
// save the value back to the registry itself
//
// Arguments :
//
//	rhs			- Right hand side of the assignment operation

const BoolParameter& BoolParameter::operator=(const bool &rhs) const
{
	DWORD			dwValue;			// New value to be set as a DWORD

	dwValue = (rhs) ? 1 : 0;			// Convert into a DWORD value

	// If our contained entry pointer is non-NULL, alter it's contents
	
	if(m_pEntry) m_pEntry->SetData(&dwValue, sizeof(DWORD));

	return *this;						// Return a reference to ourselves
}
//---------------------------------------------------------------------------

END_NAMESPACE(SVCTL)

#pragma warning(pop)
