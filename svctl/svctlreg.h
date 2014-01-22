//---------------------------------------------------------------------------
// svctlreg.h
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

#ifndef __SVCTLREG_H_
#define __SVCTLREG_H_
#pragma once

#pragma warning(push, 4)				// Enable maximum compiler warnings
#pragma warning(disable:4127)			// "conditional expression is constant"

//---------------------------------------------------------------------------
// Classes declared in this unit:
//
//	SVCTL::ParameterEntry			(abstract base class)
//	SVCTL::ServiceParametersBase	(abstract base class)
//	SVCTL::ServiceParameters		(template)
//	SVCTL::Parameter				(abstract base class)
//	SVCTL::BoolParameter
//	SVCTL::DWordParameterT			(template)
//	SVCTL::QWordParameterT			(template)
//	SVCTL::StringParameterT			(template)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// SVCTL_DONT_LEAK_PARAMETERS
//
// By default, the SERVICE_PARAMETER_MAP leaks at process exit, which can
// make it dificult to use some debugging tools properly.  DONT_LEAK_PARAMS
// registers a callback with _onexit() to make sure that they get released

#ifdef SVCTL_DONT_LEAK_HANDLERS			// If we can't leak control handers,
#define SVCTL_DONT_LEAK_PARAMETERS		// don't leak parameter objects either
#endif

//---------------------------------------------------------------------------
// Macros

// BEGIN_PARAMETER_MAP

#define BEGIN_PARAMETER_MAP() \
static SVCTL::PSERVICE_PARAMETER_MAP WINAPI _GetParameterMap(void) { \
static SVCTL::SERVICE_PARAMETER_ENTRY pParameterMap[] = {

// BEGIN_PARAMETER_MAP_KEY - Accepts a custom registry key name string

#define BEGIN_PARAMETER_MAP_KEY(x) \
LPCTSTR GetParametersKey(void) const { return _T(x); } \
static SVCTL::PSERVICE_PARAMETER_MAP WINAPI _GetParameterMap(void) \
{ static SVCTL::SERVICE_PARAMETER_ENTRY pParameterMap[] = {

// BEGIN_PARAMETER_MAP_KEY_ID - Accepts a custom registry key name resource ID

#define BEGIN_PARAMETER_MAP_KEY_ID(x) \
LPCTSTR GetParametersKey(void) const { static SVCTL:ResString rstr(x); return rstr; } \
static SVCTL::PSERVICE_PARAMETER_MAP WINAPI _GetParameterMap(void) \
{ static SVCTL::SERVICE_PARAMETER_ENTRY pParameterMap[] = {

// In _DEBUG and SVCTL_DONT_LEAK_HANDLERS builds, END_PARAMETER_MAP() adds the code
// necessary to register the _ReleaseParameterMap callback with _onexit() as well

#if defined(_DEBUG) || defined(SVCTL_DONT_LEAK_PARAMETERS)

#define END_PARAMETER_MAP() { 0, NULL }}; static bool bDoOnExit = true; \
if(bDoOnExit) { _onexit(_ReleaseParameterMap); bDoOnExit = false; } \
return pParameterMap; }

#else	// OK to leak the control handler objects

#define END_PARAMETER_MAP() { 0, NULL }}; return pParameterMap; }

#endif	// _DEBUG || SVCTL_DONT_LEAK_PARAMETERS

// EXPAND_STRING_PARAMETER ------------------------------------------

#define EXPAND_STRING_PARAMETER(id, name, def) \
{ id, new SVCTL::StringParameterEntry(REG_EXPAND_SZ, _T(name), _T(def)) },

#define EXPAND_STRING_PARAMETER_ID(resid, def) \
{ resid, new SVCTL::StringParameterEntry(REG_EXPAND_SZ, \
SVCTL::ResString(resid), _T(def)) },

// DWORD_PARAMETER --------------------------------------------------

#define DWORD_PARAMETER(id, name, def) \
{ id, new SVCTL::DWordParameterEntry(_T(name), static_cast<DWORD>(def)) },

#define DWORD_PARAMETER_ID(resid, def) \
{ resid, new SVCTL::DWordParameterEntry(SVCTL::ResString(resid), \
static_cast<DWORD>(def)) },

// QWORD_PARAMETER --------------------------------------------------

#define QWORD_PARAMETER(id, name, def) \
{ id, new SVCTL::QWordParameterEntry(_T(name), static_cast<ULONGLONG>(def)) },

#define QWORD_PARAMETER_ID(id, resid, def) \
{ resid, new SVCTL::QWordParameterEntry(SVCTL::ResString(resid), \
static_cast<ULONGLONG>(def)) },

// STRING_PARAMETER -------------------------------------------------

#define STRING_PARAMETER(id, name, def) \
{ id, new SVCTL::StringParameterEntry(REG_SZ, _T(name), _T(def)) },

#define STRING_PARAMETER_ID(resid, def) \
{ resid, new SVCTL::StringParameterEntry(REG_SZ, SVCTL::ResString(resid), \
_T(def)) },

// BOOLEAN_PARAMETER = DWORD_PARAMETER

#define BOOLEAN_PARAMETER(id, name, def)	DWORD_PARAMETER(id, name, def)
#define BOOLEAN_PARAMETER_ID(resid, def)	DWORD_PARAMETER_ID(resid, def)

// INT_PARAMETER = DWORD_PARAMETER

#define INT_PARAMETER(id, name, def)		DWORD_PARAMETER(id, name, def)
#define INT_PARAMETER_ID(resid, def)		DWORD_PARAMETER_ID(resid, def)

// LONG_PARAMETER = DWORD_PARAMETER

#define LONG_PARAMETER(id, name, def)		DWORD_PARAMETER(id, name, def)
#define LONG_PARAMETER_ID(resid, def)		DWORD_PARAMETER_ID(resid, def)

// LONGLONG_PARAMETER = QWORD_PARAMETER

#define LONGLONG_PARAMETER(id, name, def)	QWORD_PARAMETER(id, name, def)
#define LONGLONG_PARAMETER_ID(resid, def)	QWORD_PARAMETER_ID(resid, def)

// UINT_PARAMETER = DWORD_PARAMETER

#define UINT_PARAMETER(id, name, def)		DWORD_PARAMETER(id, name, def)
#define UINT_PARAMETER_ID(resid, def)		DWORD_PARAMETER_ID(resid, def)

// ULONG_PARAMETER = DWORD_PARAMETER

#define ULONG_PARAMETER(id, name, def)		DWORD_PARAMETER(id, name, def)
#define ULONG_PARAMETER_ID(resid, def)		DWORD_PARAMETER_ID(resid, def)

// ULONGLONG_PARAMETER = QWORD_PARAMETER

#define ULONGLONG_PARAMETER(id, name, def)	QWORD_PARAMETER(id, name, def)
#define ULONGLONG_PARAMETER_ID(resid, def)	QWORD_PARAMETER_ID(resid, def)

//---------------------------------------------------------------------------
// USES_XXXX_PARAMETER Macros
//
// Used to assist in delcaring parameter accessor classes in the service's
// member functions

#define USES_BOOLEAN_PARAMETER(var, param) \
SVCTL::BoolParameter var(const_cast<SVCTL::ServiceParametersBase*> \
(static_cast<const SVCTL::ServiceParametersBase*>(this)), param); 

#define USES_DWORD_PARAMETER(var, param) \
SVCTL::DWordParameter var(const_cast<SVCTL::ServiceParametersBase*> \
(static_cast<const SVCTL::ServiceParametersBase*>(this)), param); 

#define USES_EXPAND_STRING_PARAMETER(var, param) \
SVCTL::ExpandStringParameter var(const_cast<SVCTL::ServiceParametersBase*> \
(static_cast<const SVCTL::ServiceParametersBase*>(this)), param); 

#define USES_INT_PARAMETER(var, param) \
SVCTL::IntStringParameter var(const_cast<SVCTL::ServiceParametersBase*> \
(static_cast<const SVCTL::ServiceParametersBase*>(this)), param); 

#define USES_LONG_PARAMETER(var, param) \
SVCTL::LongParameter var(const_cast<SVCTL::ServiceParametersBase*> \
(static_cast<const SVCTL::ServiceParametersBase*>(this)), param); 

#define USES_LONGLONG_PARAMETER(var, param) \
SVCTL::LongLongParameter var(const_cast<SVCTL::ServiceParametersBase*> \
(static_cast<const SVCTL::ServiceParametersBase*>(this)), param); 

#define USES_QWORD_PARAMETER(var, param) \
SVCTL::QWordParameter var(const_cast<SVCTL::ServiceParametersBase*> \
(static_cast<const SVCTL::ServiceParametersBase*>(this)), param); 

#define USES_STRING_PARAMETER(var, param) \
SVCTL::StringParameter var(const_cast<SVCTL::ServiceParametersBase*> \
(static_cast<const SVCTL::ServiceParametersBase*>(this)), param); 

#define USES_UINT_PARAMETER(var, param) \
SVCTL::UIntParameter var(const_cast<SVCTL::ServiceParametersBase*> \
(static_cast<const SVCTL::ServiceParametersBase*>(this)), param); 

#define USES_ULONG_PARAMETER(var, param) \
SVCTL::ULongParameter var(const_cast<SVCTL::ServiceParametersBase*> \
(static_cast<const SVCTL::ServiceParametersBase*>(this)), param); 

#define USES_ULONGLONG_PARAMETER(var, param) \
SVCTL::ULongLongParameter var(const_cast<SVCTL::ServiceParametersBase*> \
(static_cast<const SVCTL::ServiceParametersBase*>(this)), param); 

BEGIN_NAMESPACE(SVCTL)

//---------------------------------------------------------------------------
// Constants

const TCHAR DEFAULT_PARAMKEY_PREFIX[] = _T("System\\CurrentControlSet\\Services\\");
const TCHAR DEFAULT_PARAMKEY_SUFFIX[] = _T("\\Parameters");

//---------------------------------------------------------------------------
// Class ParameterEntry
//
// ParameterEntry serves as the abstract base class from which all of the
// registry parameter entry classes derive
//
// WARNING : GetData() is not thread safe -- the caller must manually Lock()
// and Unlock() the public CritSec object when accessing this member
//---------------------------------------------------------------------------

class __declspec(novtable) ParameterEntry : public CritSec
{
protected:

	//-----------------------------------------------------------------------
	// Constructor (protected)

	explicit ParameterEntry(DWORD dwType, LPCTSTR pszName, 
		const void* pvDefault = NULL, DWORD cbDefault = 0);

public:

	//-----------------------------------------------------------------------
	// Member Functions

	void* GetData(void) const { return m_puBuffer; }	// <-- Not thread safe!

	LPCTSTR GetName(void) const { return m_strName; }

	size_t GetSize(void) const { return m_puBuffer.Size(); }

	DWORD GetType(void) const { return m_dwType; }
	
	DWORD Load(HKEY hkey);

	DWORD Save(HKEY hkey) const;

	DWORD SetData(const void* pvData, DWORD cbData);

	//-----------------------------------------------------------------------
	// Properties

	__declspec(property(get=GetData))	void*		Data;
	__declspec(property(get=GetName))	LPCTSTR		Name;
	__declspec(property(get=GetSize))	size_t		Size;
	__declspec(property(get=GetType))	DWORD		Type;

private:

	ParameterEntry(const ParameterEntry &rhs);
	ParameterEntry& operator=(const ParameterEntry &rhs);

	//-----------------------------------------------------------------------
	// Member Variables

	DWORD				m_dwType;			// Registry value type code
	String					m_strName;			// Registry value name string
	Buffer<BYTE>			m_puBuffer;			// Registry value buffer
};

//---------------------------------------------------------------------------
// Class DWordParameterEntry
// Class QWordParameterEntry
// Class StringParameterEntry
//
// Specializes construction of a ParameterEntry class object for all of the
// various data types supported by the SVCTL
//---------------------------------------------------------------------------

#pragma warning(disable:4511)			// "copy constructor could not be generated"
#pragma warning(disable:4512)			// "assignment oper could not be generated"

struct DWordParameterEntry : public ParameterEntry
{
	explicit DWordParameterEntry(LPCTSTR pszName, DWORD dwDefault = 0)
		: ParameterEntry(REG_DWORD, pszName, &dwDefault, sizeof(DWORD)) {}
};

struct QWordParameterEntry : public ParameterEntry
{
	explicit QWordParameterEntry(LPCTSTR pszName, ULONGLONG qwDefault = 0)
		: ParameterEntry(REG_QWORD, pszName, &qwDefault, sizeof(ULONGLONG)) {}
};

struct StringParameterEntry : public ParameterEntry
{
	explicit StringParameterEntry(DWORD dwType, LPCTSTR pszName, 
		LPCTSTR pszDefault = NULL) : ParameterEntry(dwType, pszName, pszDefault, 
		(pszDefault) ? static_cast<DWORD>(_tcslen(pszDefault) + 1) * sizeof(TCHAR) : 0) {}
};

#pragma warning(default:4512)			// "assignment oper could not be generated"
#pragma warning(default:4511)			// "copy constructor could not be generated"

//---------------------------------------------------------------------------
// SERVICE_PARAMETER_MAP
//
// Used to define the contents of the service's PARAMETER_MAP information

typedef struct {

	DWORD				dwIndex;	// Optional index code for the parameter
	ParameterEntry*		pEntry;		// Pointer to the ParameterEntry object

} SERVICE_PARAMETER_ENTRY, *PSERVICE_PARAMETER_ENTRY, *PSERVICE_PARAMETER_MAP;

//---------------------------------------------------------------------------
// Class SVCTL::ServiceParametersBase
//
// ServiceParametersBase is the shared base implementation behind the template 
// class ServiceParameters<>
//---------------------------------------------------------------------------

class __declspec(novtable) ServiceParametersBase
{
public:

	//-----------------------------------------------------------------------
	// Constructor / Destructor

	ServiceParametersBase() {}
	virtual ~ServiceParametersBase() {}

	//-----------------------------------------------------------------------
	// Public Member Functions

	ParameterEntry* FindParameterEntry(DWORD dwIndex) const;

	ParameterEntry* FindParameterEntry(LPCTSTR pszName) const;

	const HKEY GetParametersKey(void) const { return m_regKey; }

protected:

	//-----------------------------------------------------------------------
	// Protected Member Functions

	virtual const PSERVICE_PARAMETER_MAP GetParameterMap(void) const = 0;

	virtual LPCTSTR GetParametersKeyName(void) const = 0;

	virtual LPCTSTR GetServiceName(void) const = 0;

	//-----------------------------------------------------------------------
	// Auxiliary Service Class Member Functions

	DWORD ServiceParamInit(void);

	DWORD ServiceParamInstall(void);

	void ServiceParamTerm(void) { m_regKey.Close(); }

private:

	ServiceParametersBase(const ServiceParametersBase &rhs);
	ServiceParametersBase& operator=(const ServiceParametersBase &rhs);

	//-----------------------------------------------------------------------
	// Member Variables

	RegistryKey			m_regKey;			// Parameters registry key
};

//---------------------------------------------------------------------------
// Class SVCTL::ServiceParameters
//
// ServiceParamters provides the specialization necessary to use the
// ParametersBase class in conjunction with a derived service class object
//---------------------------------------------------------------------------

template<class _derived>
class __declspec(novtable) ServiceParameters : public ServiceParametersBase, 
	virtual private AuxServiceBase<_derived>
{
public:

	//-----------------------------------------------------------------------
	// Constructor / Destructor

	ServiceParameters();
	virtual ~ServiceParameters() {}

protected:

	//-----------------------------------------------------------------------
	// Default PARAMETER_MAP

	BEGIN_PARAMETER_MAP()
	END_PARAMETER_MAP()

	//-----------------------------------------------------------------------
	// Protected Member Functions

	virtual LPCTSTR GetParametersKeyName(void) const;

	static int __cdecl _ReleaseParameterMap(void);

private:

	ServiceParameters(const ServiceParameters &rhs);
	ServiceParameters& operator=(const ServiceParameters &rhs);

	//-----------------------------------------------------------------------
	// Private Member Functions

	const PSERVICE_PARAMETER_MAP GetParameterMap(void) const
		{ return _derived::_GetParameterMap(); }
};

//---------------------------------------------------------------------------
// ServiceParameters Constructor
//
// Arguments :
//
//	NONE

template<class _derived>
ServiceParameters<_derived>::ServiceParameters()
{
	// Hook into the service class initialization and termination code
	RegisterAuxInit(&ServiceParameters::ServiceParamInit, &ServiceParameters::ServiceParamTerm);

	// Hook into the service class installation code (not removal)
	RegisterAuxInstall(&ServiceParameters::ServiceParamInstall, NULL);
}

//---------------------------------------------------------------------------
// ServiceParameters::_ReleaseParameterMap (static, protected)
//
// Registered with _onexit() to ensure the destruction of the dynamically
// allocated Parameter objects from the _GetParameterMap() function
//
// Arguments :
//
//	NONE

#if defined(_DEBUG) || defined(SVCTL_DONT_LEAK_PARAMETERS)

template<class _derived>
int __cdecl ServiceParameters<_derived>::_ReleaseParameterMap(void)
{
	PSERVICE_PARAMETER_ENTRY	pMapEntry;		// Pointer to a map entry

	pMapEntry = _derived::_GetParameterMap();	// Get the parameter map pointer
	_ASSERTE(pMapEntry != NULL);				// Should never be NULL

	// Walk the parameter map in order to release all of the Parameter objects

	while(pMapEntry->pEntry) { delete pMapEntry->pEntry; pMapEntry++; }

	return 0;
}

#endif	// _DEBUG || SVCTL_DONT_LEAK_PARAMETERS

//---------------------------------------------------------------------------
// ServiceParameters::GetParametersKeyName (protected)
//
// Returns the name of the registry key to be used for service parameters
//
// Arguments :
//
//	NONE

template<class _derived>
LPCTSTR ServiceParameters<_derived>::GetParametersKeyName(void) const
{
	// Generate a static default registry key string for the service

	static String strKey(DEFAULT_PARAMKEY_PREFIX, GetServiceName(),
		DEFAULT_PARAMKEY_SUFFIX);
	
	return strKey;			// Return the default parameters key name
}

//---------------------------------------------------------------------------
// Class Parameter
//
// Parameter is the abstract base class from which all of the parameter
// access classes derive from for their common implementation
//---------------------------------------------------------------------------

class __declspec(novtable) Parameter
{
protected:

	//-----------------------------------------------------------------------
	// Constructors (protected)

	explicit Parameter(const ServiceParametersBase* pParent, DWORD dwIndex, 
		DWORD dwType);

	explicit Parameter(const ServiceParametersBase* pParent, LPCTSTR pszName, 
		DWORD dwType);

public:

	//-----------------------------------------------------------------------
	// Member Functions

	DWORD Reload(void) const 
		{ return (m_pEntry) ? m_pEntry->Load(m_pBase->GetParametersKey()) : 
			ERROR_FILE_NOT_FOUND; }

	DWORD Save(void) const
		{ return (m_pEntry) ? m_pEntry->Save(m_pBase->GetParametersKey()) : 
			ERROR_FILE_NOT_FOUND; }

protected:

	//-----------------------------------------------------------------------
	// Protected Member Variables

	const ServiceParametersBase*	m_pBase;	// Pointer to our parent class
	ParameterEntry*					m_pEntry;	// Pointer to the parameter object

private:

	Parameter(const Parameter &rhs);				// Disable copy constructor
	Parameter& operator=(const Parameter &rhs);		// Disable assignment operator

	void* operator new(size_t cb);			// Disable heap-based objects
};

//---------------------------------------------------------------------------
// Class BoolParameter
//
// BoolParameter is used to represent boolean registry value types
//---------------------------------------------------------------------------

class BoolParameter : public Parameter
{
public:

	//-----------------------------------------------------------------------
	// Constructors

	explicit BoolParameter(const ServiceParametersBase *pParent, DWORD dwIndex)
		: Parameter(pParent, dwIndex, REG_DWORD) {}

	explicit BoolParameter(const ServiceParametersBase *pParent, LPCTSTR pszName)
		: Parameter(pParent, pszName, REG_DWORD) {}

	//-----------------------------------------------------------------------
	// Overloaded Operators

	operator bool();

	const BoolParameter& operator=(const bool &rhs) const;

private:

	BoolParameter(const BoolParameter &rhs);
	BoolParameter& operator=(const BoolParameter &rhs);
};

//---------------------------------------------------------------------------
// Template Class DWordParameterT
//
// DWordParameterT is used to represent all 32-bit numeric registry types
//---------------------------------------------------------------------------

template <typename _type>
class DWordParameterT : public Parameter
{
public:

	//-----------------------------------------------------------------------
	// Constructors

	explicit DWordParameterT(const ServiceParametersBase *pParent, DWORD dwIndex)
		: Parameter(pParent, dwIndex, REG_DWORD) {}

	explicit DWordParameterT(const ServiceParametersBase *pParent, LPCTSTR pszName)
		: Parameter(pParent, pszName, REG_DWORD) {}

	//-----------------------------------------------------------------------
	// Overloaded Operators

	operator _type() const;

	const DWordParameterT& operator=(const _type &rhs) const;

private:

	DWordParameterT(const DWordParameterT &rhs);
	DWordParameterT& operator=(const DWordParameterT &rhs);
};

//---------------------------------------------------------------------------
// DWordParameterT::operator _type
//
// Returns a copy of the value's DWORD, converted to the specified type
//
// Arguments :
//
//	NONE

template<typename _type>
DWordParameterT<_type>::operator _type() const
{
	DWORD				dwValue = 0;		// Copy of the parameter value

	// If our entry pointer is non-NULL, the requested ParameterEntry object
	// was successfully located in the Parameter constructor

	if(m_pEntry) {

		_ASSERTE(m_pEntry->Type == REG_DWORD);		// Must be REG_DWORD type
		if(m_pEntry->Type != REG_DWORD) return 0;	// Invalid data type
		
		if(m_pEntry->Data) {

			_ASSERTE(m_pEntry->Size == sizeof(DWORD));		// Must be 32 bit data
			if(m_pEntry->Size != sizeof(DWORD)) return 0;	// Invalid data size

			// Atomically retrieve the value contained in the ParameterEntry object
			
			m_pEntry->Lock();
			dwValue = *(reinterpret_cast<DWORD*>(m_pEntry->Data));
			m_pEntry->Unlock();
		}
	}
	
	return static_cast<_type>(dwValue);		// Return in the proper type
}

//---------------------------------------------------------------------------
// DWordParameterT::operator=
//
// Assigns a new value to the underlying registry entry cache.  Does not
// save the value back to the registry itself
//
// Arguments :
//
//	rhs			- Right hand side of the assignment operation

template<typename _type>
const DWordParameterT<_type>& DWordParameterT<_type>::operator=(const _type &rhs) const
{
	DWORD			dwValue;				// New value to be set as a DWORD

	dwValue = static_cast<DWORD>(rhs);		// Convert to DWORD

	// If our contained entry pointer is non-NULL, alter it's contents
	
	if(m_pEntry) m_pEntry->SetData(&dwValue, sizeof(DWORD));

	return *this;							// Return a reference to ourselves
}

//---------------------------------------------------------------------------
// IntParameter
// UIntParameter
// LongParameter
// ULongParameter
// DWordParameter
//
// Specializations of the DWordParameterT template for the basic 32-bit
// registry parameter value types
//---------------------------------------------------------------------------

typedef DWordParameterT<INT>		IntParameter;
typedef DWordParameterT<UINT>		UIntParameter;
typedef DWordParameterT<LONG>		LongParameter;
typedef DWordParameterT<ULONG>		ULongParameter;
typedef ULongParameter				DWordParameter;

//---------------------------------------------------------------------------
// Template Class QWordParameterT
//
// QWordParameterT is used to represent all 64-bit numeric registry types
//---------------------------------------------------------------------------

template <typename _type>
class QWordParameterT : public Parameter
{
public:

	//-----------------------------------------------------------------------
	// Constructors

	explicit QWordParameterT(const ServiceParametersBase *pParent, DWORD dwIndex)
		: Parameter(pParent, dwIndex, REG_QWORD) {}

	explicit QWordParameterT(const ServiceParametersBase *pParent, LPCTSTR pszName)
		: Parameter(pParent, pszName, REG_QWORD) {}

	//-----------------------------------------------------------------------
	// Overloaded Operators

	operator _type() const;

	const QWordParameterT& operator=(const _type &rhs) const;

private:

	QWordParameterT(const QWordParameterT &rhs);
	QWordParameterT& operator=(const QWordParameterT &rhs);
};

//---------------------------------------------------------------------------
// QWordParameterT::operator _type
//
// Returns a copy of the value's QWORD, converted to the specified type
//
// Arguments :
//
//	NONE

template<typename _type>
QWordParameterT<_type>::operator _type() const
{
	ULONGLONG			qwValue = 0;		// Copy of the parameter value

	// If our entry pointer is non-NULL, the requested ParameterEntry object
	// was successfully located in the Parameter constructor

	if(m_pEntry) {

		_ASSERTE(m_pEntry->Type == REG_QWORD);		// Must be REG_QWORD type
		if(m_pEntry->Type != REG_QWORD) return 0;	// Invalid data type
		
		if(m_pEntry->Data) {

			_ASSERTE(m_pEntry->Size == sizeof(ULONGLONG));		// Must be 64 bits
			if(m_pEntry->Size != sizeof(ULONGLONG)) return 0;	// Invalid data size

			// Atomically retrieve the value contained in the ParameterEntry object
			
			m_pEntry->Lock();
			qwValue = *(reinterpret_cast<ULONGLONG*>(m_pEntry->Data));
			m_pEntry->Unlock();
		}
	}
	
	return static_cast<_type>(qwValue);			// Return in the proper type
}

//---------------------------------------------------------------------------
// QWordParameterT::operator=
//
// Assigns a new value to the underlying registry entry cache.  Does not
// save the value back to the registry itself
//
// Arguments :
//
//	rhs			- Right hand side of the assignment operation

template<typename _type>
const QWordParameterT<_type>& QWordParameterT<_type>::operator=(const _type &rhs) const
{
	ULONGLONG			qwValue;			// New value to be set as a QWORD

	qwValue = static_cast<ULONGLONG>(rhs);	// Convert to QWORD

	// If our contained entry pointer is non-NULL, alter it's contents
	
	if(m_pEntry) m_pEntry->SetData(&qwValue, sizeof(ULONGLONG));

	return *this;							// Return a reference to ourselves
}

//---------------------------------------------------------------------------
// LongLongParameter
// ULongLongParameter
// QWordParameter
//
// Specializations of the QWordParameterT template for the basic 64-bit
// registry parameter value types
//---------------------------------------------------------------------------

typedef QWordParameterT<LONGLONG>		LongLongParameter;
typedef QWordParameterT<ULONGLONG>		ULongLongParameter;
typedef ULongLongParameter				QWordParameter;

//---------------------------------------------------------------------------
// Template Class StringParameterT
//
// StringParameterT is used to represent all supported string registry types
//---------------------------------------------------------------------------

template <DWORD _regtype>
class StringParameterT : public Parameter
{
public:

	//-----------------------------------------------------------------------
	// Constructors

	explicit StringParameterT(const ServiceParametersBase *pParent, DWORD dwIndex)
		: Parameter(pParent, dwIndex, _regtype) {}

	explicit StringParameterT(const ServiceParametersBase *pParent, LPCTSTR pszName)
		: Parameter(pParent, pszName, _regtype) {}

	//-----------------------------------------------------------------------
	// Overloaded Operators

	operator LPCTSTR();

	const StringParameterT& operator=(LPCTSTR rhs);

private:

	StringParameterT(const StringParameterT &rhs);
	StringParameterT& operator=(const StringParameterT &rhs);

	//-----------------------------------------------------------------------
	// Member Variables

	String				m_strValue;				// Local copy of the value
};

//---------------------------------------------------------------------------
// StringParameterT::operator LPCTSTR
//
// Returns a pointer to the value's string buffer
//
// Arguments :
//
//	NONE

template<DWORD _regtype>
StringParameterT<_regtype>::operator LPCTSTR()
{
	// If our entry pointer is non-NULL, the requested ParameterEntry object
	// was successfully located in the Parameter constructor

	if(m_pEntry) {

		_ASSERTE(m_pEntry->Type == _regtype);			// Must be same data type
		if(m_pEntry->Type != _regtype) return NULL;		// Invalid data type
		
		if(m_pEntry->Data) {

			// Atomically retrieve the value contained in the ParameterEntry object,
			// and store it inside our local member string buffer
			
			m_pEntry->Lock();
			m_strValue = reinterpret_cast<LPTSTR>(m_pEntry->Data);
			m_pEntry->Unlock();
		}
	}
	
	return m_strValue;				// Return a pointer to our string buffer
}

//---------------------------------------------------------------------------
// StringParameterT::operator=
//
// Assigns a new value to the underlying registry entry cache.  Does not
// save the value back to the registry itself
//
// Arguments :
//
//	rhs			- Right hand side of the assignment operation

template<DWORD _regtype>
const StringParameterT<_regtype>& StringParameterT<_regtype>::operator=(LPCTSTR rhs)
{
	LPCTSTR				pszValue;		// Pointer to the new value to save

	pszValue = m_strValue = rhs;		// Copy the provided string locally first

	// If our contained entry pointer is non-NULL, alter it's contents
	
	if(m_pEntry) m_pEntry->SetData(pszValue, (m_strValue.Length() + 1) * sizeof(TCHAR));

	return *this;							// Return a reference to ourselves
}

//---------------------------------------------------------------------------
// ExpandStringParameter
// StringParameter
//
// Specializations of the StringParameterT template for the basic string
// registry parameter value types
//---------------------------------------------------------------------------

typedef StringParameterT<REG_EXPAND_SZ>		ExpandStringParameter;
typedef StringParameterT<REG_SZ>			StringParameter;

//---------------------------------------------------------------------------

END_NAMESPACE(SVCTL)

#pragma warning(pop)

#endif		// __SVCTLREG_H_

