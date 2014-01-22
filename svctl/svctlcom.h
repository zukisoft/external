//---------------------------------------------------------------------------
// svctlcom.h
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

#ifndef __SVCTLCOM_H_
#define __SVCTLCOM_H_
#pragma once

#pragma warning(push, 4)				// Enable maximum compiler warnings
#pragma warning(disable:4127)			// "conditional expression is constant"

//---------------------------------------------------------------------------
// DECLARE_COM_SERVICE Macros
//
// Most of these are used solely for COM registration, except TYPELIB info
// which is also used for the default IDispatch implementation as well

#define DECLARE_COM_SERVICE_APPID(appid) \
const GUID* GetComServiceAppId(void) const { return &appid; }

#define DECLARE_COM_SERVICE_CLASSID(clsid) \
const GUID* GetComServiceClassId(void) const { return &clsid; }

#define DECLARE_COM_SERVICE_PARAMETERS(x) \
LPCTSTR GetComServiceParameters(void) const { return _T(x); }

#define DECLARE_COM_SERVICE_PARAMETERS_ID(x) \
LPCTSTR GetComServiceParameters(void) const \
{ static SVCTL::ResString rstr(x); return rstr; }

#define DECLARE_COM_SERVICE_PROGID(x) \
LPCTSTR GetComServiceProgId(void) const { return _T(x); }

#define DECLARE_COM_SERVICE_PROGID_ID(x) \
LPCTSTR GetComServiceProgId(void) const \
{ static SVCTL::ResString rstr(x); return rstr; }

#define DECLARE_COM_SERVICE_CURRENT_PROGID(x) \
LPCTSTR GetComServiceProgIdCurVer(void) const { return _T(x); }

#define DECLARE_COM_SERVICE_CURRENT_PROGID_ID(x) \
LPCTSTR GetComServiceProgIdCurVer(void) const \
{ static SVCTL::ResString rstr(x); return rstr; }

#define DECLARE_COM_SERVICE_INDEPENDENT_PROGID(x) \
LPCTSTR GetComServiceProgIdIndependent(void) const { return _T(x); }

#define DECLARE_COM_SERVICE_INDEPENDENT_PROGID_ID(x) \
LPCTSTR GetComServiceProgIdIndependent(void) const \
{ static SVCTL::ResString rstr(x); return rstr; }

#define DECLARE_COM_SERVICE_TYPELIB(libid) \
const GUID* GetComServiceTypeLib(void) const { return &libid; }

#define DECLARE_COM_SERVICE_TYPELIB_VERSION(x) \
LPCTSTR GetComServiceTypeLibVer(void) const { return _T(x); }

#define DECLARE_COM_SERVICE_TYPELIB_VERSION_ID(x) \
LPCTSTR GetComServiceTypeLibVer(void) const \
{ static SVCTL::ResString rstr(x); return rstr; }

//---------------------------------------------------------------------------
// DECLARE_COM_ELEVATION Macros
//
// New macros to support inclusion of User Access Control elevation

#define DECLARE_COM_ELEVATION_ENABLED() \
bool GetComServiceElevationEnabled(void) const { return true; }

#define DECLARE_COM_ELEVATION_DISPLAYNAME_ID(x) \
UINT GetComServiceElevationDisplayNameId(void) const { return x; }

#define DECLARE_COM_ELEVATION_ICON_ID(x) \
UINT GetComServiceElevationIconId(void) const { return x; }

//---------------------------------------------------------------------------
// COM_INTERFACE_MAP Macros
//
// The COM_INTERFACE_MAP defines the IUnknown implementation for the derived
// COM service class object
//
// These are somewhat difficult to follow, so below is an example INTERFACE_MAP
// and the code that it would represent in a class declaration:
//
//	BEGIN_COM_INTERFACE_MAP()
//		COM_INTERFACE(IMyService)
//	END_COM_INTERFACE_MAP()
//
//	--------------------------------------------------
//
//	STDMETHOD_(ULONG, AddRef)(void)	{ return ComServiceAddRef(); }
//
//	STDMETHOD(QueryInterface)(REFIID riid, void **ppv)
//		{ return ComServiceQueryInterface(riid, ppv); }
//
//	STDMETHOD_(ULONG, Release)(void) { return ComServiceRelease(); }
//
//	const HRESULT ComServiceQueryInterface(REFIID riid, void **ppv)
//	{
//		if(!ppv) return E_POINTER;			// Invalid [out] pointer
//		*ppv = NULL;						// Initialize [out] pointer
//
//		// IID_IUnknown -> IClassFactory
//		// IID_IClassFactory -> IClassFactory
//
//		if((riid == IID_IUnknown) || (riid == IID_IClassFactory))
//			*ppv = static_cast<IClassFactory*>(this);
//
//		// IID_IExternalConnection -> IExternalConnection
//
//		else if(riid == IID_IExternalConnection)
//			*ppv = static_cast<IExternalConnection*>(this);
//
//		// IID_IMyService -> IMyService
//
//		else if(riid == IID_IMyService)
//			*ppv = static_cast<IMyService*>(this);
//
//		else return E_NOINTERFACE;		// Unknown interface IID
//
//		ComServiceAddRef();				// Increment refcount on success
//		return S_OK;
//	}

// BEGIN_COM_INTERFACE_MAP() -- Begins the IUnknown implementation

#define BEGIN_COM_INTERFACE_MAP() \
STDMETHOD_(ULONG, AddRef)(void) { return ComServiceAddRef(); } \
STDMETHOD(QueryInterface)(REFIID riid, void **ppv) \
{ return ComServiceQueryInterface(riid, ppv); } \
STDMETHOD_(ULONG, Release)(void) { return ComServiceRelease(); } \
const HRESULT ComServiceQueryInterface(REFIID riid, void **ppv) { \
if(!ppv) return E_POINTER; *ppv = NULL; \
if((riid == IID_IUnknown) || (riid == IID_IClassFactory)) \
*ppv = static_cast<IClassFactory*>(this); \
else if(riid == IID_IExternalConnection) \
*ppv = static_cast<IExternalConnection*>(this);

// COM_INTERFACE - Used to declare a "normal" COM interface.  
// Comparable to ATL's COM_INTERFACE_ENTRY() macro

#define COM_INTERFACE(iface) \
else if(riid == IID_##iface) *ppv = static_cast<iface*>(this);

// COM_INTERFACE_EX - Used to "disambiguate" inheritance branchs.  
// Comperable to ATL's COM_INTERFACE_ENTRY2() macro

#define COM_INTERFACE_EX(iface, impl) \
else if(riid == IID_##iface) *ppv = static_cast<impl*>(this);

// COM_INTERFACE_IID - Used to expose a base class vtable via an interface IID.
// Comparable to ATL's COM_INTERFACE_ENTRY_IID() macro

#define COM_INTERFACE_IID(iid, impl) \
else if(riid == iid) *ppv = static_cast<impl*>(this);

// COM_INTERFACE_IID_EX - Used to both disambiguate and expose a base class vtable.
// Comarable to ATL's COM_INTERFACE_ENTRY2_IID() macro

#define COM_INTERFACE_IID_EX(iid, iface, impl) \
else if(riid == iid) *ppv = static_cast<iface*>(static_cast<impl*>(this));

// END_COM_INTERFACE_MAP - Finishes off the IUnknown implementation

#define END_COM_INTERFACE_MAP() \
else return E_NOINTERFACE; ComServiceAddRef(); return S_OK; }

//---------------------------------------------------------------------------
// Classes declared in this unit:
//
//	SVCTL::ComClassFactory
//	SVCTL::ComRegistrar
//	SVCTL::ComService				(template)
//---------------------------------------------------------------------------

BEGIN_NAMESPACE(SVCTL)

//---------------------------------------------------------------------------
// Constants

typedef enum { ExternalReference, InternalReference } COM_REFERENCE_TYPE;

//---------------------------------------------------------------------------
// Class SVCTL::ComServiceBase
//
// ComServiceBase implements all of the base functionality that gets inherited
// by the template-based COM service classes.
//---------------------------------------------------------------------------

class __declspec(novtable) ComServiceBase : public IClassFactory,
	public IExternalConnection
{
public:

	//-----------------------------------------------------------------------
	// Constructor / Destructor

	ComServiceBase() : m_dwCoRegister(0), m_csRefCount(5000), m_cIntRefCount(0), 
		m_cExtRefCount(0) {}

	virtual ~ComServiceBase() {}

	//-----------------------------------------------------------------------
	// IUnknown

	const ULONG ComServiceAddRef(void);

	virtual const HRESULT ComServiceQueryInterface(REFIID riid, void **ppv) = 0;

	const ULONG ComServiceRelease(void);

	//-----------------------------------------------------------------------
	// IClassFactory

	STDMETHOD(CreateInstance)(IUnknown *punkOuter, REFIID riid, void **ppv);

	STDMETHOD(LockServer)(BOOL bLock)
		{ return CoLockObjectExternal(ComServiceUnknown(), bLock, TRUE); }

	//-----------------------------------------------------------------------
	// IExternalConnection

	STDMETHOD_(DWORD, AddConnection)(DWORD dwConnType, DWORD dwReserved);

	STDMETHOD_(DWORD, ReleaseConnection)(DWORD dwConnType, DWORD dwReserved, 
		BOOL bFinalReleaseCloses);

	//-----------------------------------------------------------------------
	// COM Registrar Informational Functions

	virtual const GUID* GetComServiceAppId(void) const 
		{ return GetComServiceClassId(); }

	virtual const GUID* GetComServiceClassId(void) const = 0;
	
	virtual LPCTSTR GetComServiceParameters(void) const { return NULL; }

	virtual LPCTSTR GetComServiceProgId(void) const { return NULL; }

	virtual LPCTSTR GetComServiceProgIdCurrent(void) const 
		{ return GetComServiceProgId(); }

	virtual LPCTSTR GetComServiceProgIdIndependent(void) const { return NULL; }
	
	virtual const GUID* GetComServiceTypeLib(void) const { return NULL; }

	virtual LPCTSTR GetComServiceTypeLibVer(void) const { return NULL; }
	
	virtual LPCTSTR GetServiceDisplayName(void) const = 0;

	virtual LPCTSTR GetServiceName(void) const = 0;

	// New entries to support User Access Control enabled services

	virtual bool GetComServiceElevationEnabled(void) const { return false; }
	virtual UINT GetComServiceElevationDisplayNameId(void) const { return 0; }
	virtual UINT GetComServiceElevationIconId(void) const { return 0; }

protected:

	//-----------------------------------------------------------------------
	// Protected Member Functions

	virtual DWORD ComServiceRegister(void);

	DWORD ComServiceStart(void);

	void ComServiceStop(void);

	virtual void ComServiceUnregister(void);

private:

	ComServiceBase(const ComServiceBase &rhs);				// Disable copy constructor
	ComServiceBase& operator=(const ComServiceBase &rhs);	// Disable assignment operator

	//-----------------------------------------------------------------------
	// Private Member Functions

	IUnknown* ComServiceUnknown(void) { return static_cast<IClassFactory*>(this); }

	DWORD ComRegisterAPPID(void) const;

	DWORD ComRegisterCLSID(void) const;

	DWORD ComRegisterPROGID(void) const;

	DWORD CreateKeyAndValue(HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, LPCTSTR psz) const;
	DWORD CreateKeyAndValue(HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, DWORD dw) const;

#ifdef SVCTL_TRACE_COM_REFCOUNT

	void TraceReferenceCount(COM_REFERENCE_TYPE nType, bool bIncreased) const;

#endif	// SVCTL_TRACE_COM_REFCOUNT

	//-----------------------------------------------------------------------
	// Member Variables

	DWORD			m_dwCoRegister;			// Class registration cookie
	CritSec			m_csRefCount;			// Reference count critical section
	volatile long	m_cIntRefCount;			// Internal COM references
	volatile long	m_cExtRefCount;			// External COM references
	WinHandle		m_hevtRefCount;			// COM references event object
};

//---------------------------------------------------------------------------
// Template SVCTL::ComService
//
// ComService is the template that specializes the ComServiceBase class for
// each COM-based service class object in the module
//---------------------------------------------------------------------------

template<class _derived>
class __declspec(novtable) ComService : public ComServiceBase, 
	public Service<_derived>, virtual private AuxServiceBase<_derived>
{
protected:

	//-----------------------------------------------------------------------
	// Constructor / Destructor (protected)
	
	ComService();
	virtual ~ComService() {}

public:

	//-----------------------------------------------------------------------
	// Default COM Dependency Map

	BEGIN_DEPENDENCY_MAP()
		SERVICE_DEPENDENCY("RPCSS")
	END_DEPENDENCY_MAP()

	//-----------------------------------------------------------------------
	// Service<> Static Member Overrides
	
	static ServiceBase* WINAPI _Construct(void);

	static void WINAPI _ServiceMain(DWORD dwArgc, LPTSTR *rgszArgv);

private:

	ComService(const ComService &rhs);				// Disable copy constructor
	ComService& operator=(const ComService &rhs);	// Disable assignment
};

//---------------------------------------------------------------------------
// ComService Constructor
//
// Arguments :
//
//	NONE

template<class _derived>
ComService<_derived>::ComService()
{
	// Hook into service class start / stop logic
	RegisterAuxRun(&ComService::ComServiceStart, &ComService::ComServiceStop);
	
	// Hook into service class installation / removal logic
	RegisterAuxInstall(&ComService::ComServiceRegister, &ComService::ComServiceUnregister);
}

//---------------------------------------------------------------------------
// ComService::_Construct (static)
//
// Used to construct a new derived service class object instance
//
// Arguments :
//
//	NONE

template<class _derived>
ServiceBase* WINAPI ComService<_derived>::_Construct(void)
{
	_ASSERTE(s_pThis == NULL);			// Should always be NULL here
	
	// Construct a new derived service class object and return a pointer
	// to it's ServiceBase implementation to the caller
	
	s_pThis = new _derived;
	return static_cast<ServiceBase*>(s_pThis);
}

//---------------------------------------------------------------------------
// ComService::_ServiceMain (static)
//
// Main service entry point for the derived service class object
//
// Arguments :
//
//	dwArgc			- Number of command line arguments passed in rgszArgv
//	rgszArgv		- Array of command line argument strings

template<class _derived>
void WINAPI ComService<_derived>::_ServiceMain(DWORD dwArgc, LPTSTR *rgszArgv)
{
	SVCTL::ComInit		coInit;			// CoInitializeEx() wrapper
	HRESULT				hResult;		// Result from function call

	_ASSERTE(s_pThis != NULL);			// _Construct() has not been called

	// Initialize this thread into the COM multi-threaded apartment

	hResult = coInit.Initialize(COINIT_MULTITHREADED);
	_ASSERTE(SUCCEEDED(hResult));

	// Call into ServiceBase::ServiceMain to do all the actual work here

	s_pThis->ServiceMain(dwArgc, rgszArgv);
}

//---------------------------------------------------------------------------

END_NAMESPACE(SVCTL)

#pragma warning(pop)

#endif		// __SVCTLCOM_H_