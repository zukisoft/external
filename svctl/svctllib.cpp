//---------------------------------------------------------------------------
// svctllib.cpp
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
// SVCTL::BufferBase Implementation
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// BufferBase::operator= (protected)

const BufferBase& BufferBase::operator= (const BufferBase &rhs)
{
	if(&rhs == this) return *this;		// Assignment to self

	Free();								// Make sure old buffer is released

	// If the object being assigned has allocated memory, allocate a buffer
	// of the same size and copy that memory into our local buffer
	
	if(rhs.m_pv) {

		m_cb = rhs.m_cb;				// Copy the allocated memory size
		m_pv = SVCTL::AllocMem(m_cb);	// Allocate the same size buffer

		// If the buffer allocation was successful, copy the buffer contents

		if(m_pv) memcpy(m_pv, rhs.m_pv, m_cb);
		else m_cb = 0;
	}

	return *this;
}

//---------------------------------------------------------------------------
// BufferBase::Allocate (protected)
//
// Attempts to allocate the specified number of BYTEs from the heap
//
// Arguments :
//
//	cb			- Number of bytes to be allocated for this buffer

const bool BufferBase::Allocate(size_t cb)
{
	// Attempt to allocate the requested amount of data with the SVCTL API

	void* pvAlloc = SVCTL::AllocMem(cb);
	if(!pvAlloc) return false;

	Free();								// Release any allocated memory
	m_pv = pvAlloc;						// Copy the new buffer pointer
	m_cb = cb;							// Copy the new buffer size

	return true;						// Successfully allocated the buffer
}

//---------------------------------------------------------------------------
// BufferBase::Free (protected)
//
// Releases the memory held by the class object
//
// Arguments :
//
//	NONE

void BufferBase::Free(void)
{
	if(m_pv) SVCTL::FreeMem(m_pv);		// Release any allocated heap memory
	
	m_pv = NULL;						// Reset the buffer pointer to NULL
	m_cb = 0;							// Reset the buffer size to zero
}

//---------------------------------------------------------------------------
// BufferBase::GetAt (protected)
//
// Arguments :
//
//	cbOffset			- Index into the buffer to retrieve the pointer from

void* BufferBase::GetAt(size_t cbOffset) const
{
	_ASSERTE(cbOffset < m_cb);				// Index is out of range
	if(cbOffset >= m_cb) return NULL;		// Index is out of range

	// Return the address of the requested element in the buffer
	
	return reinterpret_cast<BYTE*>(m_pv) + cbOffset;
}

//---------------------------------------------------------------------------
// BufferBase::ReAllocate (protected)
//
// Attempts to reallocate the buffer to the specified number of BYTEs
//
// Arguments :
//
//	cb			- Desired reallocated buffer size, in BYTEs

const bool BufferBase::ReAllocate(size_t cb)
{
	if(!m_pv) return Allocate(cb);		// Nothing has been allocated yet
	if(cb == m_cb) return true;			// Reallocation size is the same
	
	// Attempt to reallocate the memory block with the SVCTL API

	void* pvReAlloc = SVCTL::ReAllocMem(m_pv, cb);
	if(!pvReAlloc) return false;
	
	m_pv = pvReAlloc;					// Copy the new buffer pointer
	m_cb = cb;							// Copy the new buffer size

	return true;						// Successfully reallocated the buffer
}

//---------------------------------------------------------------------------
// SVCTL::RegistryKey Implementation
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// RegistryKey::Close
//
// Closes the contained registry key handle, if one exists
//
// Arguments :
//
//	NONE

DWORD RegistryKey::Close(void)
{
	DWORD			dwResult;			// Result from function call

	// If a registry key has been opened, close it out and reset the key
	// handle back to NULL
	
	dwResult = (m_hKey) ? RegCloseKey(m_hKey) : ERROR_SUCCESS;
	m_hKey = NULL;

	///// RESET OTHER MEMBER VARIABLES HERE AS WELL ///////////

	return dwResult;
}

//---------------------------------------------------------------------------
// RegistryKey::Create
//
// Creates a new registry key, or opens an existing registry key
//
// Arguments :
//
//	hkeyParent		- Handle to the parent registry key, or NULL for self
//	pszKeyName		- Name of the registry key to be created
//	samDesired		- Desired registry key access rights
//	pSecAttr		- Optional SECURITY_ATTRIBUTES to assign to the key
//	pszClass		- Registry key class.  Must be set to REG_NONE 
//	dwOptions		- Key volatility and backup options
//	pdwDisposition	- Option pointer to receive creation disposition flags

DWORD RegistryKey::Create(HKEY hkeyParent, LPCTSTR pszKeyName, REGSAM samDesired,
								PSECURITY_ATTRIBUTES pSecAttr, LPTSTR pszClass,
								DWORD dwOptions, DWORD *pdwDisposition)
{
	HKEY			hkeyNew;			// The newly created registry key
	DWORD			dwDisposition;		// Creation disposition flags
	DWORD			dwResult;			// Result from function call

	if(!hkeyParent) hkeyParent = m_hKey;	// NULL parent = use ourselves
	
	// Attempt to create the new registry key, using the provided parameters

	dwResult = RegCreateKeyEx(hkeyParent, pszKeyName, 0, pszClass, dwOptions,
		samDesired, pSecAttr, &hkeyNew, &dwDisposition);
	if(dwResult != ERROR_SUCCESS) return dwResult;

	Close();							// Close the current registry key handle
	m_hKey = hkeyNew;					// Copy the new registry key handle
	
	if(pdwDisposition) *pdwDisposition = dwDisposition;		// Copy create flags
	
	return ERROR_SUCCESS;
}

//---------------------------------------------------------------------------
// RegistryKey::CreateSubKey
//
// Creates a new registry key, or opens an existing registry key under the
// currently open key
//
// Arguments :
//
//	pszKeyName		- Name of the registry key to be created
//	samDesired		- Desired registry key access rights
//	pSecAttr		- Optional SECURITY_ATTRIBUTES to assign to the key
//	pszClass		- Registry key class.  Must be set to REG_NONE 
//	dwOptions		- Key volatility and backup options
//	pdwDisposition	- Option pointer to receive creation disposition flags

DWORD RegistryKey::CreateSubKey(LPCTSTR pszKeyName, REGSAM samDesired, 
									  PSECURITY_ATTRIBUTES pSecAttr, LPTSTR pszClass, 
									  DWORD dwOptions, DWORD *pdwDisposition)
{
	HKEY			hkeyNew;			// The newly created registry key
	DWORD			dwDisposition;		// Creation disposition flags
	DWORD			dwResult;			// Result from function call
	
	_ASSERTE(m_hKey != NULL);			// Shouldn't be NULL for this method

	// Attempt to create the new registry key, using the provided parameters

	dwResult = RegCreateKeyEx(m_hKey, pszKeyName, 0, pszClass, dwOptions,
		samDesired, pSecAttr, &hkeyNew, &dwDisposition);
	if(dwResult != ERROR_SUCCESS) return dwResult;

	RegCloseKey(hkeyNew);				// Close the registry key handle

	if(pdwDisposition) *pdwDisposition = dwDisposition;		// Copy create flags
	
	return ERROR_SUCCESS;
}

//---------------------------------------------------------------------------
// RegistryKey::DeleteSubKey
//
// Deletes the specified subkey, and all subkeys contained underneath it
//
// Arguments :
//
//	pszKeyName		- Name of the subkey to be deleted

DWORD RegistryKey::DeleteSubKey(LPCTSTR pszKeyName) const
{
	RegistryKey			regKey;				// Child registry key object
	DWORD				cchMaxSubKey;		// Length of longest subkey name
	DWORD				cchSubKey;			// Length of subkey name string
	LPTSTR				pszSubKey;			// Buffer for subkey name string
	DWORD				dwResult;			// Result from function call

	// Attempt to open the specified subkey with KEY_READ and DELETE access

	dwResult = regKey.Open(m_hKey, pszKeyName, KEY_READ | DELETE);
	if(dwResult != ERROR_SUCCESS) return dwResult;

	// Determine the length of the longest subkey name underneath this key,
	// and allocate a memory buffer large enough to hold such a string
	
	cchMaxSubKey = (regKey.MaxSubKeyName + 1);

	pszSubKey = reinterpret_cast<LPTSTR>(AllocMem(cchMaxSubKey * sizeof(TCHAR)));
	if(!pszSubKey) return ERROR_NOT_ENOUGH_MEMORY;

	do {

		// Recursively walk through each subkey in order to delete them.  Note that
		// zero is used as the index each time to always get the first entry

		cchSubKey = cchMaxSubKey;		// Set up the buffer length variable
		
		dwResult = RegEnumKeyEx(regKey, 0, pszSubKey, &cchSubKey, NULL,	NULL, 
			NULL, NULL);

		if(dwResult == ERROR_SUCCESS) dwResult = regKey.DeleteSubKey(pszSubKey);
		
	} while(dwResult == ERROR_SUCCESS);

	FreeMem(pszSubKey);					// Finished with the subkey name string

	// At this point, dwResult should be set to ERROR_NO_MORE_ITEMS, unless an
	// actual error occurred in RegEnumKeyEx(), or a recursive subkey call
	
	if(dwResult != ERROR_NO_MORE_ITEMS) return dwResult;

	regKey.Close();						// Close the key handle before deletion

	return RegDeleteKey(m_hKey, pszKeyName);
}

//---------------------------------------------------------------------------
// RegistryKey::Open
//
// Opens an existing registry key and associates it with this class object
//
// Arguments:
//
//	hkeyParent		- Handle to the parent registry key
//	pszKeyName		- Name of the subkey to be opened
//	samDesired		- Desired security access rights for the opened key

DWORD RegistryKey::Open(HKEY hkeyParent, LPCTSTR pszKeyName, REGSAM samDesired)
{
	HKEY			hKey;				// Newly opened registry key handle
	DWORD			dwResult;			// Result from function call

	dwResult = RegOpenKeyEx(hkeyParent, pszKeyName, 0, samDesired, &hKey);
	if(dwResult != ERROR_SUCCESS) return dwResult;

	Close();							// Close any existing key information
	m_hKey = hKey;						// Assign the new registry key handle

	return ERROR_SUCCESS;
}

//---------------------------------------------------------------------------
// RegistryKey::OpenCurrentUser
//
// Opens HKEY_CURRENT_USER for the user currently being impersonated
//
// Arguments:
//
//	samDesired		- Desired security access rights for the opened key

DWORD RegistryKey::OpenCurrentUser(REGSAM samDesired)
{
	HKEY			hKey;				// Newly opened registry key handle
	DWORD			dwResult;			// Result from function call

	dwResult = RegOpenCurrentUser(samDesired, &hKey);
	if(dwResult != ERROR_SUCCESS) return dwResult;

	Close();							// Close any existing key information
	m_hKey = hKey;						// Assign the new registry key handle

	return ERROR_SUCCESS;
}

//---------------------------------------------------------------------------
// RegistryKey::OpenRemote
//
// Opens one of the pre-defined registry keys on another system
//
// Arguments :
//
//	hkeyPredef		- Predefined registry key handle to be opened
//	pszMachineName	- Name of the remote computer to open the key on

DWORD RegistryKey::OpenRemote(HKEY hkeyPredef, LPCTSTR pszMachineName)
{
	HKEY			hKey;				// Newly opened registry key handle
	DWORD			dwResult;			// Result from function call

	dwResult = RegConnectRegistry(pszMachineName, hkeyPredef, &hKey);
	if(dwResult != ERROR_SUCCESS) return dwResult;

	Close();							// Close any existing key information
	m_hKey = hKey;						// Assign the new registry key handle

	return ERROR_SUCCESS;
}

//---------------------------------------------------------------------------
// RegistryKey::OpenUserClassesRoot
//
// Opens the HKEY_CLASSES_ROOT for a specific user account
//
// Arguments:
//
//	hToken			- Open security token for the user (TOKEN_QUERY access)
//	samDesired		- Desired security access rights for the opened key

DWORD RegistryKey::OpenUserClassesRoot(HANDLE hToken, REGSAM samDesired)
{
	HKEY			hKey;				// Newly opened registry key handle
	DWORD			dwResult;			// Result from function call

	dwResult = RegOpenUserClassesRoot(hToken, 0, samDesired, &hKey);
	if(dwResult != ERROR_SUCCESS) return dwResult;

	Close();							// Close any existing key information
	m_hKey = hKey;						// Assign the new registry key handle

	return ERROR_SUCCESS;
}

//---------------------------------------------------------------------------
// RegistryKey::SetExpandString
//
// Sets a REG_EXPAND_SZ registry value underneath the current registry key
//
// Arguments :
//
//	pszValue		- Name of the value to be set

DWORD RegistryKey::SetExpandString(LPCTSTR pszValue, LPCTSTR pszData) const
{
	_ASSERTE(m_hKey != NULL);			// Registry key is not open

	// If the provided string is non-NULL, perform some ugly casting
	// of the string pointer and stick it into the registry
	
	if(pszData) return RegSetValueEx(m_hKey, pszValue, 0, REG_EXPAND_SZ,
		reinterpret_cast<BYTE*>(const_cast<LPTSTR>(pszData)),
		static_cast<DWORD>(_tcslen(pszData) + 1) * sizeof(TCHAR));

	// A NULL string pointer requires us to put a NULL string in it's place

	TCHAR chNull = _T('\0');			// NULL character

	return RegSetValueEx(m_hKey, pszValue, 0, REG_EXPAND_SZ, 
		reinterpret_cast<BYTE*>(&chNull), sizeof(TCHAR));
}

//---------------------------------------------------------------------------
// RegistryKey::SetString
//
// Sets a REG_SZ registry value underneath the current registry key
//
// Arguments :
//
//	pszValue		- Name of the value to be set

DWORD RegistryKey::SetString(LPCTSTR pszValue, LPCTSTR pszData) const
{
	_ASSERTE(m_hKey != NULL);			// Registry key is not open

	// If the provided string is non-NULL, perform some ugly casting
	// of the string pointer and stick it into the registry
	
	if(pszData) return RegSetValueEx(m_hKey, pszValue, 0, REG_SZ,
		reinterpret_cast<BYTE*>(const_cast<LPTSTR>(pszData)),
		static_cast<DWORD>(_tcslen(pszData) + 1) * sizeof(TCHAR));

	// A NULL string pointer requires us to put a NULL string in it's place

	TCHAR chNull = _T('\0');			// NULL character

	return RegSetValueEx(m_hKey, pszValue, 0, REG_SZ, 
		reinterpret_cast<BYTE*>(&chNull), sizeof(TCHAR));
}

//---------------------------------------------------------------------------
// SVCTL::ScmLock Implementation
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// ScmLock::Lock
//
// Attempts to lock the service control manager database, which prevents
// other processes from starting services
//
// Arguments :
//
//	pszServer		- Optional target server name string

DWORD ScmLock::Lock(LPCTSTR pszServer)
{
	DWORD			dwResult;				// Result from function call
	
	if(m_hLock) return ERROR_SUCCESS;		// Database is already locked

	_ASSERTE(m_hSCM == NULL);				// Should be NULL right now
	_ASSERTE(m_hLock == NULL);				// Should be NULL right now

	// Attempt to open the service control manager with LOCK access rights

	m_hSCM = OpenSCManager(pszServer, NULL, SC_MANAGER_CONNECT | SC_MANAGER_LOCK);
	if(!m_hSCM) return GetLastError();

	// Attempt to acquire the service database lock from the SCM
	
	m_hLock = LockServiceDatabase(m_hSCM);
	if(!m_hLock) {

		dwResult = GetLastError();			// Save the returned error code
		
		CloseServiceHandle(m_hSCM);			// Close the handle to the SCM
		m_hSCM = NULL;						// Reset the handle to NULL
		
		return dwResult;					// Return the failure code
	}

	return ERROR_SUCCESS;
}

//---------------------------------------------------------------------------
// ScmLock::Unlock
//
// Releases the service database lock acquired with Lock()
//
// Arguments :
//
//	NONE

void ScmLock::Unlock(void)
{
	if(m_hLock) UnlockServiceDatabase(m_hLock);		// Unlock the database
	if(m_hSCM) CloseServiceHandle(m_hSCM);			// Close the SCM

	m_hLock = NULL;					// Reset handle to NULL
	m_hSCM = NULL;					// Reset handle to NULL
}

//---------------------------------------------------------------------------
// SVCTL::String Implementation
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// String Constructor
//
// Accepts a single c-style string pointer for initialization
//
// Arguments :
//
//	psz			- Pointer to a constant c-style string

String::String(LPCTSTR psz)
{
	// If the provided string pointer is non-NULL, attempt to allocate
	// enough heap space for the string and then copy it locally

	if(psz) {

		size_t cch = _tcslen(psz);
		if(m_strBuffer.Allocate(cch + 1)) _tcscpy_s(m_strBuffer, m_strBuffer.Length(), psz);
	}
}

//---------------------------------------------------------------------------
// String Constructor
//
// Accepts multiple c-style strings and concatenates all of them
//
// Arguments :
//
//	psz1		- Pointer to a constant c-style string
//	psz2		- Pointer to another c-style string, gets appended to psz1
//	psz3		- Pointer to another c-style string, gets appended to psz2
//	psz4		- Pointer to another c-style string, gets appended to psz3

String::String(LPCTSTR psz1, LPCTSTR psz2, LPCTSTR psz3, LPCTSTR psz4)
{
	size_t			cch = 0;			// Character count of complete string
	
	if(psz1) cch += _tcslen(psz1);		// Calculate first string length
	if(psz2) cch += _tcslen(psz2);		// Calculate second string length
	if(psz3) cch += _tcslen(psz3);		// Calculate third string length
	if(psz4) cch += _tcslen(psz4);		// Calculate fourth string length

	// If either string was non-NULL, and we can allocate the memory,
	// combine the two provided strings into a single string
	
	if((cch > 0) && (m_strBuffer.Allocate(cch + 1))) {

		// Buffer<> initializes everything to zeros, so we can use _tcscat()

		if(psz1) _tcscat_s(m_strBuffer, m_strBuffer.Length(), psz1);
		if(psz2) _tcscat_s(m_strBuffer, m_strBuffer.Length(), psz2);
		if(psz3) _tcscat_s(m_strBuffer, m_strBuffer.Length(), psz3);
		if(psz4) _tcscat_s(m_strBuffer, m_strBuffer.Length(), psz4);
	}
}

//---------------------------------------------------------------------------
// String::operator=
//
// Arguments :
//
//	rhs			- String object from the right hand side of the expression

const String& String::operator=(const String &rhs)
{
	if(&rhs == this) return *this;				// Assignment to self

	m_strBuffer.operator=(rhs.m_strBuffer);		// Copy the string's buffer
	return *this;								// Return ourselves by reference
}

//---------------------------------------------------------------------------
// String::operator=
//
// Arguments :
//
//	rhs			- C-style string from the right hand side of the expression

const String& String::operator=(LPCTSTR rhs)
{
	m_strBuffer.Free();					// Release any already allocated data

	// If the provided string pointer is non-NULL, attempt to allocate
	// enough heap space for the string and then copy it locally

	if(rhs) {

		size_t cch = _tcslen(rhs);
		if(m_strBuffer.Allocate(cch + 1)) _tcscpy_s(m_strBuffer, m_strBuffer.Length(), rhs);
	}

	return *this;						// Return ourselves by reference
}

//---------------------------------------------------------------------------
// String::operator=
//
// Arguments :
//
//	rhs			- Single character to initialize the string with

const String& String::operator=(TCHAR rhs)
{
	m_strBuffer.Free();					// Release any already allocated data

	// If the provided character is non-NULL, attempt to allocate enough heap
	// space for it and a trailing NULL, and then copy in the character

	if(rhs) { if(m_strBuffer.Allocate(2)) m_strBuffer[0] = rhs; }

	return *this;						// Return ourselves by reference
}

//---------------------------------------------------------------------------
// String::operator=
//
// Arguments :
//
//	pguid		- Pointer to a GUID to be converted and stored

const String& String::operator=(const GUID *pguid)
{
	WCHAR		rgwchGuid[40];			// Buffer for GUID string conversion
	LPTSTR		pszGuid = NULL;			// TCHAR pointer to the GUID string

	// Attempt to convert the GUID into a string by using the OLE libraries
	
	if(StringFromGUID2(*pguid, rgwchGuid, 39)) {

#ifndef _UNICODE

		// Calculate the length of, and allocate, an ANSI string buffer off of the
		// stack.  Then, convert the UNICODE CLSID string into an ANSI string

		DWORD	cbAnsiString = wcslen(rgwchGuid) + 1;
		pszGuid = reinterpret_cast<LPSTR>(_alloca(cbAnsiString * sizeof(CHAR)));
		WideCharToMultiByte(CP_ACP, 0, rgwchGuid, -1, pszGuid, cbAnsiString, NULL, NULL);

#else

		pszGuid = rgwchGuid;			// _UNICODE, no conversion required

#endif	// _UNICODE

	}

	return operator=(pszGuid);			// Assign the GUID string
}



//---------------------------------------------------------------------------
// String::Append
//
// Appends the contents of another string to this object's string
//
// Arguments :
//
//	psz			- Pointer to a constant c-style string

const String& String::Append(LPCTSTR psz)
{
	if(!m_strBuffer) return operator=(psz);		// Use assignment when NULL

	// If the provided string pointer is non-NULL, attempt to reallocate
	// the buffer to accomodate the new data, and append the string
	
	if(psz) {

		if(m_strBuffer.ReAllocate(Length() + _tcslen(psz) + 1)) 
			_tcscat_s(m_strBuffer, m_strBuffer.Length(), psz);
	}
	
	return *this;
}

//---------------------------------------------------------------------------
// String::Append
//
// Appends a character to this object's string
//
// Arguments :
//
//	ch			- The character to be appended

const String& String::Append(TCHAR ch)
{
	// If the provided charcater is non-NULL, attempt to reallocate
	// the buffer to accomodate it, and tack it on the end of the string
	
	if(ch) {

		size_t cch = Length();
		if(m_strBuffer.ReAllocate(cch + 2)) m_strBuffer[cch] = ch;
	}
	
	return *this;
}

//---------------------------------------------------------------------------
// String::AppendFormat
//
// Appends a printf-style formatting to the contents of this object
//
// Arguments :
//
//	pszFormat		- printf-style format specified string
//	...				- Variable argument list with parameters for pszFormat

const String& __cdecl String::AppendFormat(LPCTSTR pszFormat, ...)
{
	va_list		varArgs;				// The variable argument list

	_ASSERTE(pszFormat != NULL);		// Format string cannot be NULL
	
	va_start(varArgs, pszFormat);		// Initialize the variable arguments
	AppendFormatV(pszFormat, varArgs);	// Format using the va_list version
	va_end(varArgs);					// Reset the variable arguments

	return *this;						// Return a reference to ourselves
}

//---------------------------------------------------------------------------
// String::AppendFormat
//
// Appends a printf-style formatting to the contents of this object
//
// Arguments :
//
//	uResID			- printf-style format specified string from resources
//	...				- Variable argument list with parameters for pszFormat

const String& __cdecl String::AppendFormat(UINT uResID, ...)
{
	String			strResource;			// The loaded resource string
	va_list			varArgs;				// The variable argument list

	// Only attempt the formatting operation if the resource load succeeds

	if(strResource.LoadResource(uResID)) {

		va_start(varArgs, uResID);				// Initialize the variable arguments
		AppendFormatV(strResource, varArgs);	// Format using the va_list version
		va_end(varArgs);						// Reset the variable arguments
	}

	return *this;					// Return a reference to ourselves
}

//---------------------------------------------------------------------------
// String::AppendFormatV
//
// Appends a printf-style formatting to the contents of this object, using
// an already constructed variable argument list
//
// Arguments :
//
//	pszFormat		- printf-style format specified string
//	varArgs			- Variable arguments with parameters for pszFormat

const String& __cdecl String::AppendFormatV(LPCTSTR pszFormat, va_list varArgs)
{
	Buffer<TCHAR>		strBuffer;		// Temporary formatted string buffer
	int					nResult;		// Result from function call

	_ASSERTE(pszFormat != NULL);		// Format string cannot be NULL

	// Allocate a temporary buffer large enough to hold 1024 characters

	if(!strBuffer.Allocate(1025)) return *this;

	// Format the string with the o-so-handy _vsntprintf() function, 
	// and if successful, append that string to ourselves

	nResult = _vsntprintf_s(strBuffer, strBuffer.Length(), 1024, pszFormat, varArgs);
	if(nResult > 0) Append(strBuffer);

	return *this;					// Return a reference to ourselves
}

//---------------------------------------------------------------------------
// String::Compare
//
// Performs a comparison of this string with another, optionally without
// case sensitivity
//
// Arguemnts :
//
//	psz				- The c-style string to be compared with
//	bIgnoreCase		- Flag controlling case sensitivity

const int String::Compare(LPCTSTR psz, bool bIgnoreCase) const
{
	// If both strings are non-NULL, compare the two of them.  Our string is
	// the left-hand operand in the comparison

	if(psz && Length()) return (bIgnoreCase) ?
		_tcsicmp(m_strBuffer, psz) : _tcscmp(m_strBuffer, psz);

	else if((!psz) && (!m_strBuffer)) return 0;		// Both are NULL
	else return (!m_strBuffer) ? -1 : 1;			// One string is NULL
}

//---------------------------------------------------------------------------
// String::ExpandVariables
//
// Expands any environment variables contained in the string
//
// Arguments :
//
//	NONE

const String& String::ExpandVariables(void)
{
	Buffer<TCHAR>		strBuffer;		// Temporary string data buffer
	DWORD				cchBuffer;		// Size of the temporary string buffer

	if(IsNull()) return *this;			// Nothing to do -- no contained string

	// Determine the size of the buffer that needs to be allocated

	cchBuffer = ExpandEnvironmentStrings(m_strBuffer, NULL, 0);
	
	if((cchBuffer > 0) && (strBuffer.Allocate(cchBuffer))) {

		// Expand the environment variables for real this time, and
		// assign the resultant string to ourselves
		
		ExpandEnvironmentStrings(m_strBuffer, strBuffer, cchBuffer);
		operator=(strBuffer);
	}

	return *this;
}

//---------------------------------------------------------------------------
// String::Format
//
// Uses printf-style formatting to load the contents of this object.
//
// Arguments :
//
//	pszFormat		- printf-style format specified string
//	...				- Variable argument list with parameters for pszFormat

const String& __cdecl String::Format(LPCTSTR pszFormat, ...)
{
	va_list		varArgs;			// The variable argument list

	_ASSERTE(pszFormat != NULL);	// Format string cannot be NULL
	
	va_start(varArgs, pszFormat);	// Initialize the variable arguments
	FormatV(pszFormat, varArgs);	// Format using the va_list version
	va_end(varArgs);				// Reset the variable arguments

	return *this;					// Return a reference to ourselves
}

//---------------------------------------------------------------------------
// String::Format
//
// Uses printf-style formatting to load the contents of this object.
//
// Arguments :
//
//	uResID			- printf-style format specified string from resources
//	...				- Variable argument list with parameters for pszFormat

const String& __cdecl String::Format(UINT uResID, ...)
{
	String		strResource;		// The loaded resource string
	va_list		varArgs;			// The variable argument list

	// Only perform the formatting operation if the resource load is successful

	if(strResource.LoadResource(uResID)) {

		va_start(varArgs, uResID);			// Initialize the variable arguments
		FormatV(strResource, varArgs);		// Format using the va_list version
		va_end(varArgs);					// Reset the variable arguments
	}

	return *this;					// Return a reference to ourselves
}

//---------------------------------------------------------------------------
// String::FormatV
//
// Uses printf-style formatting to load the contents of this object, using
// an already constructed variable argument list
//
// Arguments :
//
//	pszFormat		- printf-style format specified string
//	varArgs			- Variable arguments with parameters for pszFormat

const String& __cdecl String::FormatV(LPCTSTR pszFormat, va_list varArgs)
{
	Buffer<TCHAR>		strBuffer;		// Temporary formatted string buffer
	int					nResult;		// Result from function call

	_ASSERTE(pszFormat != NULL);		// Format string cannot be NULL
	
	// Allocate a temporary buffer large enough to hold 1024 characters

	if(!strBuffer.Allocate(1025)) return *this;

	// Format the string with the o-so-handy _vsntprintf() function, 
	// and if successful, assign that string to ourselves

	nResult = _vsntprintf_s(strBuffer, strBuffer.Length(), 1024, pszFormat, varArgs);
	if(nResult > 0) operator=(strBuffer);

	return *this;					// Return a reference to ourselves
}

//---------------------------------------------------------------------------
// String::LoadMessage
//
// Loads a message resource from the system, or a specified module
//
// Arguments :
//
//	dwMsg			- Message identifier code
//	pszModuleName	- Optional module name to load the message from

const bool String::LoadMessage(DWORD dwMsg, LPCTSTR pszModuleName)
{
	HMODULE			hModule = NULL;			// Loaded message module handle
	bool			bSuccess = false;		// Flag indicating operational success
	LPTSTR			pszMessage;				// Pointer to the message string
	LPTSTR			pszWhack;				// Used to whack CRLF characters

	// Load the specified message module if one has been specified

	if(pszModuleName) {
		
		hModule = LoadLibraryEx(pszModuleName, NULL, LOAD_LIBRARY_AS_DATAFILE);
		if(!hModule) {
			
			_RPTF1(_CRT_WARN, "Unable to load message library %s\r\n", pszModuleName);
			return false;
		}
	}

	// Call FormatMessage to let Windows do all the real work involved here

	if(FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS |
		FORMAT_MESSAGE_FROM_SYSTEM | ((hModule) ? FORMAT_MESSAGE_FROM_HMODULE : 0),
		((hModule) ? hModule : NULL), dwMsg, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPTSTR>(&pszMessage), 0, NULL)) {

		bSuccess = true;				// Successfully retrieved the message

		// Whack any of those annoying CRLF characters from the end of the string

		pszWhack = &pszMessage[_tcslen(pszMessage) - 1];

		while((pszWhack != pszMessage) && ((*pszWhack == _T('\r')) || 
			(*pszWhack == _T('\n')))) { *pszWhack = 0; pszWhack--; }

		operator=(pszMessage);			// Use the assignment operator
		LocalFree(pszMessage);			// Release the system message
	}

	if(hModule) FreeLibrary(hModule);	// Release the loaded message module

	return bSuccess;
}

//---------------------------------------------------------------------------
// String::LoadModuleName
//
// Loads the contents of this string object with the pathname of a module
//
// Arguments :
//
//	hModule			- Optional module handle to get the pathname for

const bool String::LoadModuleName(HMODULE hModule)
{
	Buffer<TCHAR>	pathBuffer;		// Temporary module pathname buffer
	DWORD			dwResult;		// Result from function call

	// Allocate a temporary buffer large enough to hold _MAX_PATH characters,
	// and ask the system to get us the pathname for the module

	if(!pathBuffer.Allocate(_MAX_PATH + 1)) return false;
	dwResult = GetModuleFileName(hModule, pathBuffer, _MAX_PATH);

	// On success, assign the pathname string to this String object

	if(dwResult > 0) { operator=(pathBuffer); return true; }
	else return false;
}

//---------------------------------------------------------------------------
// String::LoadResource
//
// Loads the contents of this string object from a resource string.
// (Limited to a maximum of 1024 characters)
//
// Arguments :
//
//	uID				- Resource identifier
//	hInstance		- Optional module instance handle to load resource from

const bool String::LoadResource(UINT uID, HINSTANCE hInstance)
{
	Buffer<TCHAR>	resBuffer;		// Temporary resource string buffer
	int				nResult;		// Result from function call

	// Allocate a temporary buffer large enough to hold 1024 characters

	if(!resBuffer.Allocate(1025)) return false;

	// Load the requested resource string into the temporary buffer

	nResult = LoadString(hInstance, uID, resBuffer, 1024);

	// On success, assign the resource string to this String object

	if(nResult > 0) { operator=(resBuffer); return true; }
	else { _RPTF1(_CRT_WARN, "Unable to load string resource ID %d\r\n", uID); }

	return false;
}

//---------------------------------------------------------------------------

END_NAMESPACE(SVCTL)

#pragma warning(pop)
