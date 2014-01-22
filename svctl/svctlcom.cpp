//---------------------------------------------------------------------------
// svctlcom.cpp
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
#pragma warning(disable:4100)			// "unreferenced formal parameter"
#pragma warning(disable:4127)			// "conditional expression is constant"

BEGIN_NAMESPACE(SVCTL)

//---------------------------------------------------------------------------
// SVCTL::ComServiceBase Implementation
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// ComServiceBase::AddConnection
//
// Increments the COM service's external reference count
//
// Arguments :
//
//	dwConnType		- External connection type.  Must be EXTCONN_STRONG
//	dwReserved		- Reserved by OLE

STDMETHODIMP_(DWORD) ComServiceBase::AddConnection(DWORD dwConnType, DWORD dwReserved)
{
	// If a STRONG connection has been added, increment the external refcount

	if((dwConnType & EXTCONN_STRONG) == EXTCONN_STRONG)
		InterlockedIncrement(const_cast<long*>(&m_cExtRefCount));

#ifdef SVCTL_TRACE_COM_REFCOUNT

	TraceReferenceCount(ExternalReference, true);

#endif	// SVCTL_TRACE_COM_REFCOUNT

	return m_cExtRefCount;			// Return current external reference count
}

//---------------------------------------------------------------------------
// ComServiceBase::ComRegisterAPPID
//
// Performs all default registration operations under HKCR\APPID
//
// Arguments :
//
//	NONE

DWORD ComServiceBase::ComRegisterAPPID(void) const
{
	String			strAppId;			// String representation of the APPID
	RegistryKey		regAppId;			// APPID registration key
	DWORD			dwResult;			// Result from function call

	_ASSERTE(GetComServiceAppId() != NULL);		// Should never be NULL
	strAppId = GetComServiceAppId();			// Convert the APPID

	// Open the HKLM\SOFTWARE\CLASSES\APPID registry key with KEY_WRITE access, 
	// and attempt to create the service's APPID sub key underneath it.  Note
	// that you cannot use HKEY_CLASSES_ROOT anymore for this
	
	dwResult = regAppId.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\CLASSES\\APPID"), KEY_WRITE);
	if(dwResult != ERROR_SUCCESS) return dwResult;

	dwResult = regAppId.Create(NULL, strAppId, KEY_WRITE);
	if(dwResult != ERROR_SUCCESS) return dwResult;

	// (Default)	= [service display name]
	// LocalService	= [service key name]

	dwResult = regAppId.SetString(NULL, GetServiceDisplayName());
	if(dwResult != ERROR_SUCCESS) return dwResult;

	dwResult = regAppId.SetString(_T("LocalService"), GetServiceName());
	if(dwResult != ERROR_SUCCESS) return dwResult;

	// ServiceParameters = [service startup parameters]
	
	if(GetComServiceParameters()) {

		dwResult = regAppId.SetString(_T("ServiceParameters"), GetComServiceParameters());
		if(dwResult != ERROR_SUCCESS) return dwResult;
	}

	return ERROR_SUCCESS;			// All APPID registrations successful
}
	
//---------------------------------------------------------------------------
// ComServiceBase::ComRegisterCLSID
//
// Performs all default registration operations under HKCR\CLSID
//
// Arguments :
//
//	NONE

DWORD ComServiceBase::ComRegisterCLSID(void) const
{
	String			strAppId;			// String representation of the APPID
	String			strClassId;			// String representation of the CLSID
	RegistryKey		regClassId;			// CLSID registration key object
	RegistryKey		regSubKey;			// Subkey underneath the CLSID key
	UINT			resid;				// Resource identifier code
	String			strModule;			// This module name
	String			strResPath;			// Resource path identifier
	DWORD			dwResult;			// Result from function call

	_ASSERTE(GetComServiceAppId() != NULL);		// Should never be NULL
	_ASSERTE(GetComServiceClassId() != NULL);	// Should never be NULL

	strAppId = GetComServiceAppId();			// Convert the APPID
	strClassId = GetComServiceClassId();		// Convert the CLSID
	strModule.LoadModuleName();					// Load the module name

	// Open the HKCR\CLSID registry key with KEY_WRITE access, and attempt to
	// create the service's CLSID sub key underneath it
	
	dwResult = regClassId.Open(HKEY_CLASSES_ROOT, _T("CLSID"), KEY_WRITE);
	if(dwResult != ERROR_SUCCESS) return dwResult;

	dwResult = regClassId.Create(NULL, strClassId, KEY_WRITE);
	if(dwResult != ERROR_SUCCESS) return dwResult;

	// @		= [service display name]
	// AppID	= [service APPID]
	
	dwResult = regClassId.SetString(NULL, GetServiceDisplayName());
	if(dwResult != ERROR_SUCCESS) return dwResult;

	dwResult = regClassId.SetString(_T("AppID"), strAppId);
	if(dwResult != ERROR_SUCCESS) return dwResult;

	// ProgId\(Default)	= [service PROGID string]

	if(GetComServiceProgId()) {

		dwResult = CreateKeyAndValue(regClassId, _T("ProgID"), NULL, 
			GetComServiceProgId());

		if(dwResult != ERROR_SUCCESS) return dwResult;
	}

	// TypeLib\(Default) = [service LIBID]

	if(GetComServiceTypeLib()) {

		String	strTypeLib;							// For LIBID conversion
		strTypeLib = GetComServiceTypeLib();		// Convert service LIBID

		dwResult = CreateKeyAndValue(regClassId, _T("TypeLib"), NULL, strTypeLib);
		if(dwResult != ERROR_SUCCESS) return dwResult;
	}

	// Version\(Default) = [service LIBID version]

	if(GetComServiceTypeLibVer()) {

		dwResult = CreateKeyAndValue(regClassId, _T("Version"), NULL, 
			GetComServiceTypeLibVer());

		if(dwResult != ERROR_SUCCESS) return dwResult;
	}

	// VersionIndependentProgId\(Default) = [service independent PROGID]

	if(GetComServiceProgIdIndependent()) {

		dwResult = CreateKeyAndValue(regClassId, _T("VersionIndependentProgId"), NULL,
			GetComServiceProgIdIndependent());

		if(dwResult != ERROR_SUCCESS) return dwResult;
	}

	// Elevation\Enabled = [elevation enabled flag]

	if(GetComServiceElevationEnabled()) {

		dwResult = CreateKeyAndValue(regClassId, _T("Elevation"), _T("Enabled"), 1);
		if(dwResult != ERROR_SUCCESS) return dwResult;
	}

	// Elevation\IconReference = [elevation icon reference]

	resid = GetComServiceElevationIconId();
	if(resid != 0) {

		// @<pathtobinary>,-<resourcenumber>
		strResPath.Format(_T("@%s,-%d"), static_cast<LPCTSTR>(strModule), resid);

		dwResult = CreateKeyAndValue(regClassId, _T("Elevation"), _T("IconReference"), strResPath);
		if(dwResult != ERROR_SUCCESS) return dwResult;
	}

	// LocalizedString = [elevation MUI display name]

	resid = GetComServiceElevationDisplayNameId();
	if(resid != 0) {

		// @<pathtobinary>,-<resourcenumber>
		strResPath.Format(_T("@%s,-%d"), static_cast<LPCTSTR>(strModule), resid);

		dwResult = regClassId.SetString(_T("LocalizedString"), strResPath);
		if(dwResult != ERROR_SUCCESS) return dwResult;
	}

	return ERROR_SUCCESS;			// All CLSID registrations successful
}
	
//---------------------------------------------------------------------------
// ComServiceBase::ComRegisterPROGID
//
// Performs all default ProgId registration operations under HKCR
//
// Arguments :
//
//	NONE

DWORD ComServiceBase::ComRegisterPROGID(void) const
{
	String			strClassId;			// String representation of the CLSID
	RegistryKey		regProgId;			// PROGID registration key
	DWORD			dwResult;			// Result from function call

	// STANDARD PROGID ------------------------------------------------------
	
	// If there is no defined PROGID for the service, we ignore the independent
	// PROGID and the current version data as well and just skip this step

	if(GetComServiceProgId() == NULL) return ERROR_SUCCESS;
	
	_ASSERTE(GetComServiceClassId() != NULL);		// Should never be NULL
	strClassId = GetComServiceClassId();			// Convert the CLSID

	// Attempt to create the PROGID registry key underneath HKEY_CLASSES_ROOT
	
	dwResult = regProgId.Create(HKEY_CLASSES_ROOT, GetComServiceProgId(), KEY_WRITE);
	if(dwResult != ERROR_SUCCESS) return dwResult;

	// (Default) = [service display name]

	dwResult = regProgId.SetString(NULL, GetServiceDisplayName());
	if(dwResult != ERROR_SUCCESS) return dwResult;

	// CLSID\(Default) = [service CLSID]

	dwResult = CreateKeyAndValue(regProgId, _T("CLSID"), NULL, strClassId);
	if(dwResult != ERROR_SUCCESS) return dwResult;

	// VERSION INDEPENDENT PROGID -------------------------------------------
	
	// If there is no defined version independent PROGID for the service,
	// we're all done with this part of registration
	
	if(GetComServiceProgIdIndependent() == NULL) return ERROR_SUCCESS;

	// Attempt to create the VI PROGID registry key underneath HKEY_CLASSES_ROOT
	
	dwResult = regProgId.Create(HKEY_CLASSES_ROOT, GetComServiceProgIdIndependent(), 
		KEY_WRITE);
	if(dwResult != ERROR_SUCCESS) return dwResult;

	// (Default) = [service display name]

	dwResult = regProgId.SetString(NULL, GetServiceDisplayName());
	if(dwResult != ERROR_SUCCESS) return dwResult;

	// CLSID\(Default) = [service CLSID]

	dwResult = CreateKeyAndValue(regProgId, _T("CLSID"), NULL, strClassId);
	if(dwResult != ERROR_SUCCESS) return dwResult;

	// CurVer\(Default) = [service current PROGID]

	dwResult = CreateKeyAndValue(regProgId, _T("CurVer"), NULL, 
		GetComServiceProgIdCurrent());

	return dwResult;				// Return final regitration status
}
	
//---------------------------------------------------------------------------
// ComServiceBase::ComServiceAddRef
//
// Increments the COM service's internal reference count
//
// Arguments :
//
//	NONE

const ULONG ComServiceBase::ComServiceAddRef(void)
{
	AutoCS		autoLock(m_csRefCount);		// Automatic critical section

	m_cIntRefCount++;				// Increment the internal reference count
	ResetEvent(m_hevtRefCount);		// Reset the reference monitor to FALSE

#ifdef SVCTL_TRACE_COM_REFCOUNT

	TraceReferenceCount(InternalReference, true);

#endif	// SVCTL_TRACE_COM_REFCOUNT

	return m_cIntRefCount;			// Return current internal reference count
}

//---------------------------------------------------------------------------
// ComServiceBase::ComServiceRegister (protected)
//
// Registers this service's CLSID and APPID into the registry
//
// Arguments :
//
//	NONE

DWORD ComServiceBase::ComServiceRegister(void)
{
	DWORD			dwResult;			// Result from function call

	// Call into each of the individual registration functions

	dwResult = ComRegisterAPPID();
	if(dwResult == ERROR_SUCCESS) dwResult = ComRegisterCLSID();
	if(dwResult == ERROR_SUCCESS) dwResult = ComRegisterPROGID();

	// If any of the registration functions failed, undo everything
	
	if(dwResult != ERROR_SUCCESS) ComServiceUnregister();

	return dwResult;
}

//---------------------------------------------------------------------------
// ComServiceBase::ComServiceRelease
//
// Decrements the COM service's internal reference count
//
// Arguments :
//
//	NONE

const ULONG ComServiceBase::ComServiceRelease(void)
{
	AutoCS		autoLock(m_csRefCount);		// Automatic critical section

	m_cIntRefCount--;				// Decrement the internal reference count
	_ASSERTE(m_cIntRefCount >= 0);	// Should never fall below zero
	
	// If the object's internal reference count has fallen to zero, signal
	// the reference count monitoring event object

	if(m_cIntRefCount == 0) SetEvent(m_hevtRefCount);

#ifdef SVCTL_TRACE_COM_REFCOUNT

	TraceReferenceCount(InternalReference, false);

#endif	// SVCTL_TRACE_COM_REFCOUNT

	return m_cIntRefCount;			// Return current internal reference count
}

//---------------------------------------------------------------------------
// ComServiceBase::ComServiceStart (protected)
//
// Starts up the COM portion of the service by creating required member
// variables and registering the service class object with the COM SCM
//
// Arguments :
//
//	NONE

DWORD ComServiceBase::ComServiceStart(void)
{
	HRESULT				hResult;		// Result from function call

	// Attempt to create the kernel event object used to monitor object
	// reference counts for a graceful service shutdown
	
	m_hevtRefCount = CreateEvent(NULL, TRUE, TRUE, NULL);
	if(!m_hevtRefCount) return GetLastError();

	// Attempt to register this service class object with the COM SCM
	
	_ASSERTE(GetComServiceClassId() != NULL);	// Should never be NULL
	
	hResult = CoRegisterClassObject(*GetComServiceClassId(), ComServiceUnknown(), 
		CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE, &m_dwCoRegister);
	if(FAILED(hResult)) return static_cast<DWORD>(hResult);
	
	return ERROR_SUCCESS;
}

//---------------------------------------------------------------------------
// ComServiceBase::ComServiceStop (protected)
//
// Stops the COM portion of the service by revoking the class object and
// disconnecting all external clients
//
// Arguments :
//
//	NONE

void ComServiceBase::ComServiceStop(void)
{
	// If the class object was successfully registered, revoke it from the SCM
	
	if(m_dwCoRegister) CoRevokeClassObject(m_dwCoRegister);
	m_dwCoRegister = 0;
	
	// Disconnect the stub manager, and wait up to 30 seconds for all the
	// client connections to actually drop off before exiting

	CoDisconnectObject(ComServiceUnknown(), 0);
	WaitForSingleObject(m_hevtRefCount, 30000);
}

//---------------------------------------------------------------------------
// ComServiceBase::ComServiceUnregister (protected)
//
// Unegisters this service's CLSID and APPID from the registry
//
// Arguments :
//
//	NONE

void ComServiceBase::ComServiceUnregister(void)
{
	String			strAppId;			// String representation of the APPID
	String			strClassId;			// String representation of the CLSID
	RegistryKey		regKey;				// HKEY_CLASSES_ROOT registry key
	DWORD			dwResult;			// Result from function call

	_ASSERTE(GetComServiceAppId() != NULL);		// Should never be NULL
	_ASSERTE(GetComServiceClassId() != NULL);	// Should never be NULL

	strAppId = GetComServiceAppId();			// Convert the APPID
	strClassId = GetComServiceClassId();		// Convert the CLSID
	
	// Remove the service APPID key from HKEY_CLASSES_ROOT\APPID

	dwResult = regKey.Open(HKEY_CLASSES_ROOT, _T("APPID"), DELETE);
	if(dwResult == ERROR_SUCCESS) regKey.DeleteSubKey(strAppId);

	// Remove the service CLSID key from HKEY_CLASSES_ROOT\CLSID

	dwResult = regKey.Open(HKEY_CLASSES_ROOT, _T("CLSID"), DELETE);
	if(dwResult == ERROR_SUCCESS) regKey.DeleteSubKey(strClassId);

	// Remove the service ProgId and VersionIndependentProgId keys as well

	dwResult = regKey.Open(HKEY_CLASSES_ROOT, NULL, DELETE);
	if(dwResult == ERROR_SUCCESS) {

		if(GetComServiceProgId()) 
			regKey.DeleteSubKey(GetComServiceProgId());

		if(GetComServiceProgIdIndependent()) 
			regKey.DeleteSubKey(GetComServiceProgIdIndependent());
	}
}

//---------------------------------------------------------------------------
// ComServiceBase::CreateInstance
//
// Creates an instance of the requested object.  The SVCTL implementation
// doesn't actually create anything, but just QI()'s the service itself.
//
// Arguments :
//
//	punkOuter		- IUnknown pointer to create aggregate object
//	riid			- Requested object interface ID (IID)
//	ppv				- Pointer to receive requested interface pointer

STDMETHODIMP ComServiceBase::CreateInstance(IUnknown *punkOuter, REFIID riid,
											void **ppv)
{
	if(!ppv) return E_POINTER;			// Invalid [out] pointer
	*ppv = NULL;						// Initialize [out] pointer

	// Out-of-process (EXE) components cannot be aggregated, so deny
	// any requests to aggregate the service class object
	
	if(punkOuter) return CLASS_E_NOAGGREGATION;

	// Use the derived service's QueryInterface() implementation to return
	// the requested interface pointer to the client
	
	return ComServiceQueryInterface(riid, ppv);
}

//---------------------------------------------------------------------------
// ComServiceBase::CreateKeyAndValue (private)
//
// Used by the registrar code to create a subkey and set one value
//
// Arguments :
//
//	hkey		- Handle to the parent registry key (requires KEY_WRITE)
//	pszSubKey	- Name of the subkey to be created
//	pszValue	- Name of the value to be set
//	psz			- Pointer to the string data to set for pszValue

DWORD ComServiceBase::CreateKeyAndValue(HKEY hkey, LPCTSTR pszSubKey,
	LPCTSTR pszValue, LPCTSTR psz) const
{
	RegistryKey			regKey;			// The registry key to be created
	DWORD				dwResult;		// Result from function call

	// Attempt to create the target registry key with KEY_WRITE access

	dwResult = regKey.Create(hkey, pszSubKey, KEY_WRITE);
	if(dwResult != ERROR_SUCCESS) return dwResult;

	return regKey.SetString(pszValue, psz);		// Set the named value
}

//---------------------------------------------------------------------------
// ComServiceBase::CreateKeyAndValue (private)
//
// Used by the registrar code to create a subkey and set one value
//
// Arguments :
//
//	hkey		- Handle to the parent registry key (requires KEY_WRITE)
//	pszSubKey	- Name of the subkey to be created
//	pszValue	- Name of the value to be set
//	dw			- DWORD data to be set

DWORD ComServiceBase::CreateKeyAndValue(HKEY hkey, LPCTSTR pszSubKey,
	LPCTSTR pszValue, DWORD dw) const
{
	RegistryKey			regKey;			// The registry key to be created
	DWORD				dwResult;		// Result from function call

	// Attempt to create the target registry key with KEY_WRITE access

	dwResult = regKey.Create(hkey, pszSubKey, KEY_WRITE);
	if(dwResult != ERROR_SUCCESS) return dwResult;

	return regKey.SetDWORD(pszValue, dw);		// Set the named value
}

//---------------------------------------------------------------------------
// ComServiceBase::ReleaseConnection
//
// Decrements the COM service's external reference count
//
// Arguments :
//
//	dwConnType				- Connection type.  Must be EXTCONN_STRONG
//	dwReserved				- Reserved by OLE
//	bFinalReleaseCloses		- Flag to disconnect all stubs on zero reference

STDMETHODIMP_(DWORD) ComServiceBase::ReleaseConnection(DWORD dwConnType, 
									 DWORD dwReserved, BOOL bFinalReleaseCloses)
{
	// If a STRONG connection has been cleared, decrement the external
	// reference counter and disconnect all stubs when it reaches zero

	if((dwConnType & EXTCONN_STRONG) == EXTCONN_STRONG) {

		if(InterlockedDecrement(const_cast<long*>(&m_cExtRefCount)) == 0)
			if(bFinalReleaseCloses) CoDisconnectObject(ComServiceUnknown(), 0);
	}

#ifdef SVCTL_TRACE_COM_REFCOUNT

	TraceReferenceCount(ExternalReference, false);

#endif	// SVCTL_TRACE_COM_REFCOUNT

	return m_cExtRefCount;			// Return current external reference count
}

//---------------------------------------------------------------------------
// ComServiceBase::TraceReferenceCount (private)
//
// Dumps the current object reference count to the system debugger
//
// Arguments :
//
//	nType			- Type of reference that was altered (External / Internal)
//	bIncreased		- Flag to determine count increase or decrease

#ifdef SVCTL_TRACE_COM_REFCOUNT

void ComServiceBase::TraceReferenceCount(COM_REFERENCE_TYPE nType, bool bIncreased) const
{
	String			strOutput;			// String for DBMON output

	strOutput = GetServiceName();		// Start with the service name
	strOutput.ToUpper();				// Convert to all uppercase

	// Finish off the string formatting with a printf-style operation

	strOutput.AppendFormat(_T(": %s %c %d\r\n"), 
		(nType == InternalReference) ? _T("InternalRefs") : _T("ExternalRefs"),
		(bIncreased) ? _T('>') : _T('<'),
		(nType == InternalReference) ? m_cIntRefCount : m_cExtRefCount);

	OutputDebugString(strOutput);		// Send the string to the debugger
};

#endif	// SVCTL_TRACE_COM_REFCOUNT

//---------------------------------------------------------------------------

END_NAMESPACE(SVCTL)

#pragma warning(pop)
