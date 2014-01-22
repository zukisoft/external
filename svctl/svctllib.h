//---------------------------------------------------------------------------
// svctllib.h
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

#ifndef __SVCTLLIB_H_
#define __SVCTLLIB_H_
#pragma once

#pragma warning(push, 4)				// Enable maximum compiler warnings
#pragma warning(disable:4100)			// "unreferenced formal parameter"
#pragma warning(disable:4127)			// "conditional expression is constant"

//---------------------------------------------------------------------------
// Classes declared in this unit:
//
//	SVCTL::BufferBase
//	SVCTL::Buffer			(template)
//	SVCTL::ComInit
//	SVCTL::CritSec
//	SVCTL::AutoCS
//	SVCTL::RegKey
//	SVCTL::ScmLock
//	SVCTL::String
//	SVCTL::ResString
//	SVCTL::SvcHandle
//	SVCTL::WinHandle
//---------------------------------------------------------------------------

BEGIN_NAMESPACE(SVCTL)

//---------------------------------------------------------------------------
// Class SVCTL::BufferBase
//
// BufferBase implements a simple heap buffer base class used by the Buffer<>
// template class for SVCTL dynamic memory
//---------------------------------------------------------------------------

class __declspec(novtable) BufferBase
{
protected:

	//-----------------------------------------------------------------------
	// Constructors / Destructor

	BufferBase() : m_pv(NULL), m_cb(0) {}
	
	BufferBase(const BufferBase &rhs) : m_pv(NULL), m_cb(0) { operator=(rhs); }

	virtual ~BufferBase() { Free(); }

	//-----------------------------------------------------------------------
	// Overloaded Operators

	const BufferBase& operator= (const BufferBase &rhs);

//	void** operator &() { return &m_pv; }

	//-----------------------------------------------------------------------
	// Protected Member Functions

	const bool Allocate(size_t cb);

	void* Buffer(void) const { return m_pv; }
	
	void Free(void);

	void* GetAt(size_t cbOffset) const;

	const bool IsEmpty(void) const { return (m_pv == NULL); }

	const bool ReAllocate(size_t cb);
	
	size_t Size(void) const { return m_cb; }

	void ZeroOut(void) { if(m_pv) memset(m_pv, 0, m_cb); }

private:

	//-----------------------------------------------------------------------
	// Member Variables

	void*			m_pv;				// Pointer to the allocated heap buffer
	size_t			m_cb;				// Size of the allocated heap buffer
};

//---------------------------------------------------------------------------
// Template Class SVCTL::Buffer
//
// Buffer is a type-casting template around the basic functionality provided
// by the BufferBase class
//
// Arguments :
//
//	_type			- Any valid C++ base type or class object
//	_alloc_units	- C++ type used to determine allocation unit size
//---------------------------------------------------------------------------

template <typename _type, typename _alloc_units = _type>
class Buffer : private BufferBase
{
public:

	//-----------------------------------------------------------------------
	// Constructors / Destructor

	Buffer() {}
	Buffer(const Buffer &buffer) : BufferBase(buffer) {}
	
	virtual ~Buffer() {}

	//-----------------------------------------------------------------------
	// Overloaded Operators
	
	operator bool() const { return !(BufferBase::IsEmpty()); }

	const bool operator !() const { return BufferBase::IsEmpty(); }

	const Buffer& operator= (const Buffer &rhs) 
		{ BufferBase::operator=(rhs); return *this; }

	operator void*() const { return BufferBase::Buffer(); }

	operator _type*() const
		{ return reinterpret_cast<_type*>(BufferBase::Buffer()); }

//	_type** operator &()
//		{ return reinterpret_cast<_type**>(BufferBase::operator&()); }

	_type& operator[] (int index) const
		{ return operator[](static_cast<size_t>(index)); }

	_type& operator[] (size_t index) const
		{ return *(reinterpret_cast<_type*>(BufferBase::GetAt(index * sizeof(_type)))); }

	//-----------------------------------------------------------------------
	// Public Member Functions

	const bool Allocate(size_t dwUnits)
		{ return BufferBase::Allocate(dwUnits * sizeof(_alloc_units)); }

	void Free(void) { BufferBase::Free(); }

	const bool IsEmpty(void) const { return BufferBase::IsEmpty(); }

	// LENGTH IS THE NUMBER OF ELEMENTS
	size_t Length(void) const { return (BufferBase::Size() / sizeof(_type)); }

	const bool ReAllocate(size_t dwUnits)
		{ return BufferBase::ReAllocate(dwUnits * sizeof(_alloc_units)); }

	// SIZE IS THE SIZE IN BYTES
	size_t Size(void) const { return BufferBase::Size(); }

	void ZeroOut(void) { BufferBase::ZeroOut(); }
};

//---------------------------------------------------------------------------
// Class SVCTL::ComInit
//
// ComInit wraps around CoInitializeEx() to ensure that CoUninitialize()
// is called when the object falls out of scope
//---------------------------------------------------------------------------

class ComInit
{
public:

	//-----------------------------------------------------------------------
	// Constructor / Destructor

	ComInit() : m_bInit(false) {}
	~ComInit() { Uninitialize(); }

	//-----------------------------------------------------------------------
	// Member Functions

	const HRESULT Initialize(COINIT coInit = COINIT_APARTMENTTHREADED)
		{ HRESULT hr = CoInitializeEx(NULL, coInit); m_bInit = SUCCEEDED(hr); return hr; }

	void Uninitialize(void) 
		{ if(m_bInit) CoUninitialize(); m_bInit = false; }

private:

	ComInit(const ComInit &rhs);				// Disable copy constructor
	ComInit& operator=(const ComInit &rhs);		// Disable assignment operator

	//-----------------------------------------------------------------------
	// Member Variables

	bool			m_bInit;		// Flag if CoInitializeEx() was successful
};

//---------------------------------------------------------------------------
// Class SVCTL::CritSec
//
// CritSec is a very thin wrapper around a Win32 CRITICAL_SECTION object
//---------------------------------------------------------------------------

class CritSec
{
public:

	//-----------------------------------------------------------------------
	// Constructors / Destructor

	CritSec() { InitializeCriticalSection(&m_cs); }

	CritSec(DWORD dwSpinCount) 
		{ InitializeCriticalSectionAndSpinCount(&m_cs, dwSpinCount); }

	~CritSec() { DeleteCriticalSection(&m_cs); }

	//-----------------------------------------------------------------------
	// Public Member Functions

	void Lock(void) const { EnterCriticalSection(&m_cs); }

	void SetSpinCount(DWORD dwSpinCount) const
		{ SetCriticalSectionSpinCount(&m_cs, dwSpinCount); }

	const bool TryLock(void) const
		{ return (TryEnterCriticalSection(&m_cs) == TRUE); }

	void Unlock(void) const { LeaveCriticalSection(&m_cs); }

private:

	CritSec(const CritSec &rhs);				// Disable copy
	CritSec& operator=(const CritSec &rhs);		// Disable assignment

	//-----------------------------------------------------------------------
	// Member Variables

	mutable CRITICAL_SECTION	m_cs;		// The Win32 critical section object
};

//---------------------------------------------------------------------------
// Class SVCTL::AutoCS
//
// AutoCS is used to provide an automatic locking/unlocking critical section
// object, normally used on the stack in member functions
//---------------------------------------------------------------------------

class AutoCS
{
public:

	//-----------------------------------------------------------------------
	// Constructor / Destructor

	explicit AutoCS(const CritSec &rcs) : m_rcs(rcs) { m_rcs.Lock(); }
	~AutoCS() { m_rcs.Unlock(); }

private:

	AutoCS(const AutoCS &rhs);					// Disable copy
	AutoCS& operator=(const AutoCS &rhs);		// Disable assignment

	//-----------------------------------------------------------------------
	// Member Variables

	const CritSec&			m_rcs;			// CritSec object reference
};

//---------------------------------------------------------------------------
// Class SVCTL::RegistryKey
//
// RegistryKey is a decent registry key manipulation class used by most of
// the auxiliary service base class implementations.
//---------------------------------------------------------------------------

class RegistryKey
{
public:

	//-----------------------------------------------------------------------
	// Constructors / Destructor

	RegistryKey() : m_hKey(NULL) {}
	RegistryKey(const RegistryKey &rhs) : m_hKey(NULL) { Copy(rhs.m_hKey); }

	~RegistryKey() { Close(); }

	//-----------------------------------------------------------------------
	// Overloaded Operators

	const RegistryKey& operator=(const RegistryKey &rhs)
		{ return operator=(rhs.m_hKey); }

	const RegistryKey& operator=(HKEY hkey) { Copy(hkey); return *this; }

	operator HKEY() const { return m_hKey; }

	HKEY* operator &() { return &m_hKey; }

	operator bool() const { return (m_hKey != NULL); }

	const bool operator !() { return (m_hKey == NULL); }

	//-----------------------------------------------------------------------
	// Member Functions

	DWORD Close(void);

	DWORD Copy(HKEY hkeyExisting, REGSAM samDesired = KEY_ALL_ACCESS)
		{ return Open(hkeyExisting, NULL, samDesired); }

	DWORD Create(HKEY hkeyParent, LPCTSTR pszKeyName, 
		REGSAM samDesired = KEY_ALL_ACCESS, PSECURITY_ATTRIBUTES pSecAttr = NULL,
		LPTSTR pszClass = REG_NONE, DWORD dwOptions = REG_OPTION_NON_VOLATILE,
		DWORD *pdwDisposition = NULL);

	// Want to change this to accept a RegistryKey* or something, to avoid
	// multiple opens
	
	DWORD CreateSubKey(LPCTSTR pszKeyName, REGSAM samDesired = KEY_ALL_ACCESS,
		PSECURITY_ATTRIBUTES pSecAttr = NULL, LPTSTR pszClass = REG_NONE,
		DWORD dwOptions = REG_OPTION_NON_VOLATILE, DWORD *pdwDisposition = NULL);

	DWORD DeleteSubKey(LPCTSTR pszKeyName) const;

	DWORD DeleteValue(LPCTSTR pszValueName) const
		{ _ASSERTE(m_hKey != NULL); return RegDeleteValue(m_hKey, pszValueName); }

	DWORD Flush(void) const
		{ _ASSERTE(m_hKey != NULL); return RegFlushKey(m_hKey); }

	DWORD GetMaxSubKeyNameLength(void) const;

	DWORD GetMaxValueLength(void) const;

	DWORD GetMaxValueNameLength(void) const;

	DWORD GetNumSubKeys(void) const;

	DWORD GetNumValues(void) const;

	const bool IsOpen(void) const { return (m_hKey != NULL); }

	DWORD Open(HKEY hkeyParent, LPCTSTR pszKeyName, 
		REGSAM samDesired = KEY_ALL_ACCESS);

	DWORD OpenCurrentUser(REGSAM samDesired = KEY_ALL_ACCESS);

	DWORD OpenRemote(HKEY hkeyPredef, LPCTSTR pszMachineName);

	DWORD OpenUserClassesRoot(HANDLE hToken, REGSAM samDesired = KEY_ALL_ACCESS);

	DWORD SetBinary(LPCTSTR pszValue, const BYTE* puData, DWORD cbData) const;

	DWORD SetBoolean(LPCTSTR pszValue, bool bData) const
		{ return SetDWORD(pszValue, (bData) ? 1 : 0); }

	DWORD SetDWORD(LPCTSTR pszValue, DWORD dwData) const;

	DWORD SetExpandString(LPCTSTR pszValue, LPCTSTR pszData) const;
	
	DWORD SetLong(LPCTSTR pszValue, LONG lData) const
		{ return SetDWORD(pszValue, static_cast<DWORD>(lData)); }

	DWORD SetLongLong(LPCTSTR pszValue, LONGLONG llData) const
		{ return SetQWORD(pszValue, static_cast<ULONGLONG>(llData)); }

	DWORD SetQWORD(LPCTSTR pszValue, ULONGLONG qwData) const;

	DWORD SetString(LPCTSTR pszValue, LPCTSTR pszData) const;
	
	DWORD SetULong(LPCTSTR pszValue, ULONG ulData) const
		{ return SetDWORD(pszValue, static_cast<DWORD>(ulData)); }

	DWORD SetULongLong(LPCTSTR pszValue, ULONGLONG ullData) const
		{ return SetQWORD(pszValue, static_cast<ULONGLONG>(ullData)); }

	//-----------------------------------------------------------------------
	// Properties

	__declspec(property(get=GetMaxSubKeyNameLength))	DWORD MaxSubKeyName;
	__declspec(property(get=GetMaxValueLength))			DWORD MaxValue;
	__declspec(property(get=GetMaxValueNameLength))		DWORD MaxValueName;
	__declspec(property(get=GetNumSubKeys))				DWORD NumSubKeys;
	__declspec(property(get=GetNumValues))				DWORD NumValues;

private:

	//-----------------------------------------------------------------------
	// Private Member Functions

	DWORD QueryInfoKey(void);

	//-----------------------------------------------------------------------
	// Member Variables

	HKEY			m_hKey;				// Contained registry key handle
};

// RegistryKey::GetMaxSubKeyNameLength() ------------------------------------

inline DWORD RegistryKey::GetMaxSubKeyNameLength(void) const
{
	DWORD		cchKeyLen = 0;		// Maximum subkey name length

	_ASSERTE(m_hKey != NULL);		// Registry key must be open
	
	RegQueryInfoKey(m_hKey, NULL, NULL, NULL, NULL, &cchKeyLen, NULL, NULL, 
		NULL, NULL, NULL, NULL);

	return cchKeyLen;				// Return the maximum subkey name length
}

// RegistryKey::GetMaxValueLength() -----------------------------------------

inline DWORD RegistryKey::GetMaxValueLength(void) const
{
	DWORD		cchValLen = 0;		// Maximum value length

	_ASSERTE(m_hKey != NULL);		// Registry key must be open
	
	RegQueryInfoKey(m_hKey, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
		NULL, &cchValLen, NULL, NULL);

	return cchValLen;				// Return the maximum value length
}

// RegistryKey::GetMaxValueNameLength() -------------------------------------

inline DWORD RegistryKey::GetMaxValueNameLength(void) const
{
	DWORD		cchValLen = 0;		// Maximum value name length

	_ASSERTE(m_hKey != NULL);		// Registry key must be open
	
	RegQueryInfoKey(m_hKey, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
		&cchValLen, NULL, NULL, NULL);

	return cchValLen;				// Return the maximum value name length
}

// RegistryKey::GetNumSubKeys() ---------------------------------------------

inline DWORD RegistryKey::GetNumSubKeys(void) const
{
	DWORD		cSubKeys = 0;		// Holds number of subkeys

	_ASSERTE(m_hKey != NULL);		// Registry key must be open
	
	RegQueryInfoKey(m_hKey, NULL, NULL, NULL, &cSubKeys, NULL, NULL, NULL, 
		NULL, NULL, NULL, NULL);

	return cSubKeys;				// Return the number of subkeys
}

// RegistryKey::GetNumValues() ----------------------------------------------

inline DWORD RegistryKey::GetNumValues(void) const
{
	DWORD		cValues = 0;		// Holds number of values

	_ASSERTE(m_hKey != NULL);		// Registry key must be open
	
	RegQueryInfoKey(m_hKey, NULL, NULL, NULL, NULL, NULL, NULL, &cValues, 
		NULL, NULL, NULL, NULL);

	return cValues;					// Return the number of values
}

// RegistryKey::SetBinary() -------------------------------------------------

inline DWORD RegistryKey::SetBinary(LPCTSTR pszValue, const BYTE *puData,
										  DWORD cbData) const
{
	_ASSERTE(m_hKey != NULL);		// Registry key must be open

	return RegSetValueEx(m_hKey, pszValue, 0, REG_BINARY, puData, cbData);
}

// RegistryKey::SetDWORD() --------------------------------------------------

inline DWORD RegistryKey::SetDWORD(LPCTSTR pszValue, DWORD dwData) const
{
	_ASSERTE(m_hKey != NULL);		// Registry key must be open

	return RegSetValueEx(m_hKey, pszValue, 0, REG_DWORD, 
		reinterpret_cast<BYTE*>(&dwData), sizeof(DWORD));
}

// RegistryKey::SetQWORD() --------------------------------------------------

inline DWORD RegistryKey::SetQWORD(LPCTSTR pszValue,
										 unsigned __int64 qwData) const
{
	_ASSERTE(m_hKey != NULL);		// Registry key must be open

	return RegSetValueEx(m_hKey, pszValue, 0, REG_QWORD, 
		reinterpret_cast<BYTE*>(&qwData), sizeof(unsigned __int64));
}

//---------------------------------------------------------------------------
// Class SVCTL::ScmLock
//
// Provides an automatically unlocking Service Control Manager lock class
// used when a group of services are installed or removed from the system
//---------------------------------------------------------------------------

class ScmLock
{
public:

	//-----------------------------------------------------------------------
	// Constructor / Destructor

	explicit ScmLock() : m_hSCM(NULL), m_hLock(NULL) {}
	~ScmLock() { Unlock(); }

	//-----------------------------------------------------------------------
	// Member Functions

	DWORD Lock(LPCTSTR pszServer = NULL);

	void Unlock(void);

private:

	ScmLock(const ScmLock& rhs);				// Disable copy constructor
	ScmLock& operator=(const ScmLock& rhs);		// Disable assignment

	//-----------------------------------------------------------------------
	// Member Variables

	SC_HANDLE			m_hSCM;		// Handle to the service control manager
	SC_LOCK				m_hLock;	// Handle returned from LockDatabase()
};

//---------------------------------------------------------------------------
// Class SVCTL::String
//
// String provides a fairly standard dynamically allocated C++ string class 
// used throughout the SVCTL library.
//---------------------------------------------------------------------------

class String
{
public:

	//-----------------------------------------------------------------------
	// Constructors / Destructor

	String() {}
	String(const String &string) : m_strBuffer(string.m_strBuffer) {}

	String(LPCTSTR psz);
	String(LPCTSTR psz1, LPCTSTR psz2, LPCTSTR psz3 = NULL, LPCTSTR psz4 = NULL);

	virtual ~String() {}

	//-----------------------------------------------------------------------
	// Overloaded Operators

	operator bool() const { return !(m_strBuffer.IsEmpty()); }
	
	const bool operator !() const { return m_strBuffer.IsEmpty(); }

	const String& operator= (const String &rhs);
	const String& operator= (LPCTSTR rhs);
	const String& operator= (TCHAR rhs);

	const String& operator= (const GUID *pguid);	// <--- For COM registrar

	const String& operator+= (const String& rhs) { return Append(rhs.m_strBuffer); }
	const String& operator+= (LPCTSTR rhs) { return Append(rhs); }
	const String& operator+= (TCHAR rhs) { return Append(rhs); }

	const bool operator== (const String& rhs) const 
		{ return Compare(rhs.m_strBuffer) == 0; }

	const bool operator== (LPCTSTR rhs) const { return Compare(rhs) == 0; }
	
	const bool operator!= (const String& rhs) const 
		{ return Compare(rhs.m_strBuffer) != 0; }

	const bool operator!= (LPCTSTR rhs) const { return Compare(rhs) != 0; }

	const bool operator< (const String& rhs) const 
		{ return Compare(rhs.m_strBuffer) < 0; }

	const bool operator< (LPCTSTR rhs) const { return Compare(rhs) < 0; }

	const bool operator> (const String& rhs) const 
		{ return Compare(rhs.m_strBuffer) > 0; }

	const bool operator> (LPCTSTR rhs) const { return Compare(rhs) > 0; }

	const bool operator<= (const String& rhs) const 
		{ return Compare(rhs.m_strBuffer) <= 0; }

	const bool operator<= (LPCTSTR rhs) const { return Compare(rhs) <= 0; }

	const bool operator>= (const String& rhs) const 
		{ return Compare(rhs.m_strBuffer) >= 0; }

	const bool operator>= (LPCTSTR rhs) const { return Compare(rhs) >= 0; }

	const TCHAR& operator[] (int index) const 
		{ return operator[](static_cast<size_t>(index)); }
		
	const TCHAR& operator[] (size_t index) const
		{ _ASSERTE(index < Length()); return m_strBuffer[index]; }

	operator LPCTSTR() const { return m_strBuffer; }

	//-----------------------------------------------------------------------
	// Member Functions

	const String& Append(const String &string) { return Append(string.m_strBuffer); }
	const String& Append(LPCTSTR psz);
	const String& Append(TCHAR ch);

	const String& __cdecl AppendFormat(LPCTSTR pszFormat, ...);
	const String& __cdecl AppendFormat(UINT uResID, ...);
	const String& __cdecl AppendFormatV(LPCTSTR pszFormat, va_list varArgs);

	void Clear(void) { m_strBuffer.Free(); }

	const int Compare(const String& string, bool bIgnoreCase = false) const
		{ return Compare(string.m_strBuffer, bIgnoreCase); }

	const int Compare(LPCTSTR psz, bool bIgnoreCase = false) const;

	const String& ExpandVariables(void);

	const String& __cdecl Format(LPCTSTR pszFormat, ...);
	const String& __cdecl Format(UINT uResID, ...);
	const String& __cdecl FormatV(LPCTSTR pszFormat, va_list varArgs);

	const bool IsNull(void) const { return m_strBuffer.IsEmpty(); }

	size_t Length(void) const
		{ return m_strBuffer.IsEmpty() ? 0 : m_strBuffer.Length() - 1; }

	const bool LoadMessage(DWORD dwMsg, LPCTSTR pszModuleName = NULL);

	const bool LoadModuleName(HMODULE hModule = NULL);

	const bool LoadResource(UINT uID, HINSTANCE hInstance = GetModuleHandle(NULL));
	
	const String& ToLower(void) 
		{ if(!m_strBuffer.IsEmpty()) _tcslwr_s(m_strBuffer, m_strBuffer.Length()); return *this; }

	const String& ToUpper(void) 
		{ if(!m_strBuffer.IsEmpty()) _tcsupr_s(m_strBuffer, m_strBuffer.Length()); return *this; }

private:

	//-----------------------------------------------------------------------
	// Member Variables

	Buffer<TCHAR>			m_strBuffer;		// The string buffer object
};

inline const bool operator==(LPCTSTR lhs, const String &rhs)
{
	return rhs.Compare(lhs) == 0;
}

inline const bool operator!=(LPCTSTR lhs, const String &rhs)
{
	return rhs.Compare(lhs) != 0;
}

inline const bool operator<(LPCTSTR lhs, const String &rhs)
{
	return (rhs.Compare(lhs) > 0);
}

inline const bool operator>(LPCTSTR lhs, const String &rhs)
{
	return (rhs.Compare(lhs) < 0);
}

inline const bool operator<=(LPCTSTR lhs, const String &rhs)
{
	return (rhs.Compare(lhs) >= 0);
}

inline const bool operator>=(LPCTSTR lhs, const String& rhs)
{
	return (rhs.Compare(lhs) <= 0);
}

//---------------------------------------------------------------------------
// Class SVCTL::ResString
//
// ResString provides a very simplified String object used solely for constant
// resource strings.  Unlike String, ResString will never return a NULL pointer.
//---------------------------------------------------------------------------

class ResString : private String
{
public:

	//-----------------------------------------------------------------------
	// Constructor / Destructor

	explicit ResString(UINT uID, HINSTANCE hInstance = GetModuleHandle(NULL))
	{ 
		if(!String::LoadResource(uID, hInstance)) {
			_RPTF1(_CRT_WARN, "Unable to load string with resource id %d\r\n", uID);
		}
	}

	ResString(const ResString& rhs) : String(rhs) {}
	
	virtual ~ResString() {}

	//-----------------------------------------------------------------------
	// Overloaded Operators

	operator LPCTSTR (void) const
		{ return IsNull() ? s_pszNullString : String::operator LPCTSTR(); }

private:

	ResString& operator=(const ResString& rhs);		// Disable assignment

	//-----------------------------------------------------------------------
	// Member Variables

	static LPCTSTR		s_pszNullString;	// A constant zero-length string
};

// ResString static member variable initializations

__declspec(selectany) LPCTSTR ResString::s_pszNullString = _T("");

//---------------------------------------------------------------------------
// Class SVCTL::SvcHandle
//
// Provides a simple container for a Win32 service handle that will close
// the handle when this class object is unwound
//---------------------------------------------------------------------------

class SvcHandle
{
public:

	//-----------------------------------------------------------------------
	// Constructor / Destructor

	SvcHandle() : m_hService(NULL) {}
	~SvcHandle() { Close(); }

	//-----------------------------------------------------------------------
	// Overloaded Operators

	const SvcHandle& operator=(const SC_HANDLE hService) 
		{ Close(); m_hService = hService; return *this; }

	operator bool() const { return (m_hService != NULL); }

	const bool operator!() const { return (m_hService == NULL); }

	operator SC_HANDLE() const { return m_hService; }

	//-----------------------------------------------------------------------
	// Member Functions

	void Close(void)
	{ 
		if(m_hService) CloseServiceHandle(m_hService);	// Close the handle
		m_hService = NULL;								// Reset the handle
	}

private:

	SvcHandle(const SvcHandle& rhs);				// Disable copy
	SvcHandle& operator=(const SvcHandle& rhs);		// Disable assignment

	//-----------------------------------------------------------------------
	// Member Variables

	SC_HANDLE		m_hService;				// The contained service handle
};

//---------------------------------------------------------------------------
// Class SVCTL::WinHandle
//
// Provides a simple container for a standard Win32 handle that will close
// the handle when the class object is unwound
//---------------------------------------------------------------------------

class WinHandle
{
public:

	//-----------------------------------------------------------------------
	// Constructor / Destructor

	WinHandle() : m_handle(NULL) {}
	~WinHandle() { Close(); }

	//-----------------------------------------------------------------------
	// Overloaded Operators

	const WinHandle& operator=(const HANDLE handle) 
		{ Close(); m_handle = handle; return *this; }

	operator bool() const { return (m_handle != NULL); }

	const bool operator!() const { return (m_handle == NULL); }

	operator HANDLE() const { return m_handle; }

	//-----------------------------------------------------------------------
	// Member Functions

	void Close(void)
	{ 
		if(m_handle) CloseHandle(m_handle);		// Close the handle
		m_handle = NULL;						// Reset the handle
	}

private:

	WinHandle(const WinHandle& rhs);				// Disable copy
	WinHandle& operator=(const WinHandle& rhs);		// Disable assignment

	//-----------------------------------------------------------------------
	// Member Variables

	HANDLE			m_handle;			// The contained object handle
};

//---------------------------------------------------------------------------

END_NAMESPACE(SVCTL)

#pragma warning(pop)

#endif		// __SVCTLLIB_H_