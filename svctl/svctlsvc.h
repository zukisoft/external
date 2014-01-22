//---------------------------------------------------------------------------
// svctlsvc.h
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

#ifndef __SVCTLSVC_H_
#define __SVCTLSVC_H_
#pragma once

#pragma warning(push, 4)				// Enable maximum compiler warnings
#pragma warning(disable:4100)			// "unreferenced formal parameter"
#pragma warning(disable:4127)			// "conditional expression is constant"

//---------------------------------------------------------------------------
// Classes declared in this unit:
//
//	SVCTL::ServiceBase				(abstract base class)
//	SVCTL::AuxServiceBase			(template)
//	SVCTL::Service					(template)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// DECLARE_SERVICE Macros
//
// Used to define the properties of an SVCTL::ServiceBase derived class object

#define DECLARE_SERVICE_NAME(x)	\
LPCTSTR GetServiceName(void) const { return _GetServiceName(); } \
static LPCTSTR WINAPI _GetServiceName(void) { return _T(x); }

#define DECLARE_SERVICE_NAME_ID(x) \
LPCTSTR GetServiceName(void) const { return _GetServiceName(); } \
static LPCTSTR WINAPI _GetServiceName(void) \
{ static SVCTL::ResString rstr(x); return rstr; }

#define DECLARE_SERVICE_DISPLAYNAME(x)	\
LPCTSTR GetServiceDisplayName(void) const { return _GetDisplayName(); } \
static LPCTSTR WINAPI _GetDisplayName(void) { return _T(x); }

#define DECLARE_SERVICE_DISPLAYNAME_ID(x) \
LPCTSTR GetServiceDisplayName(void) const { return _GetDisplayName(); } \
static LPCTSTR WINAPI _GetDisplayName(void) \
{ static SVCTL::ResString rstr(x); return rstr; }

#define DECLARE_SERVICE_DISPLAY_NAME(x)		DECLARE_SERVICE_DISPLAYNAME(x)
#define DECLARE_SERVICE_DISPLAY_NAME_ID(x)	DECLARE_SERVICE_DISPLAYNAME_ID(x)

#define DECLARE_SERVICE_TYPE(x) \
DWORD GetServiceType(void) const { return x; } \
static DWORD WINAPI _GetServiceType(void) { return x; }

#define DECLARE_SERVICE_DESCRIPTION(x) \
LPCTSTR GetServiceDescription(void) const { return _T(x); }

#define DECLARE_SERVICE_DESCRIPTION_ID(x) \
LPCTSTR GetServiceDescription(void) const \
{ static SVCTL::ResString rstr(x); return rstr; }

#define DECLARE_SERVICE_GROUP(x) \
LPCTSTR GetServiceGroup(void) const { return _T(x); }

#define DECLARE_SERVICE_GROUP_ID(x) \
LPCTSTR GetServiceGroup(void) const \
{ static SVCTL::ResString rstr(x); return rstr; }

#define DECLARE_SERVICE_START_TYPE(x) \
DWORD GetServiceStartType(void) const { return x; }

#define DECLARE_SERVICE_ERROR_CONTROL(x) \
DWORD GetServiceErrorControl(void) const { return x; }

//---------------------------------------------------------------------------
// DEPENDENCY_MAP Macros
//
// Used to set up optional service dependencies in the derived class

#pragma message("TODO -- Implement a DEPENDENCY_MAP that uses resource strings")

#define BEGIN_DEPENDENCY_MAP() \
LPCTSTR GetServiceDependencies(void) const { static TCHAR pszDependencies[] = 

#define SERVICE_DEPENDENCY(x) _T(x)_T("\0")

#define GROUP_DEPENDENCY(x) _T("+")_T(x)_T("\0")

#define END_DEPENDENCY_MAP() _T("\0"); return pszDependencies;}

//---------------------------------------------------------------------------
// CONTROL_MAP Macros
//
// Used to set up the service control handler callback functions in the derived
// service class.  
//
// These are somewhat difficult to follow, so below is an example CONTROL_MAP
// and the code that it would represent in a class declaration:
//
//	BEGIN_CONTROL_MAP(MyService)
//		HANDLER_EVENT(SERVICE_CONTROL_STOP, m_hevtStop)
//		HANDLER_FUNC(SERVICE_CONTROL_PAUSE, OnPause)
//		HANDLER_FUNC(SERVICE_CONTROL_CONTINUE, OnContinue)
//		INLINE_HANDLER_FUNC(SERVICE_CONTROL_PARAMCHANGE, OnParamChange)
//	END_CONTROL_MAP()
//
//	--------------------------------------------------
//
//	typedef SVCTL::AsyncControlHandler<MyService>	_asynchandler;
//	typedef SVCTL::EventControlHandler<MyService>	_eventhandler;
//	typedef SVCTL::InlineControlHandler<MyService>	_inlinehandler;
//
//	static SVCTL::PSERVICE_CONTROL_MAP WINAPI _GetControlMap(void)
//	{
//		// Construct the static SERVICE_CONTROL_MAP structure for this service
//
//		static SVCTL::SERVICE_CONTROL_ENTRY pControlMap[] = {
//
//			{ SERVICE_CONTROL_STOP,			new _eventhandler(&m_hevtStop),		true},
//			{ SERVICE_CONTROL_PAUSE,		new _asynchandler(&OnPause),		true},
//			{ SERVICE_CONTROL_CONTINUE,		new _asynchandler(&OnContinue),		true},
//			{ SERVICE_CONTROL_PARAMCHANGE,  new _inlinehandler(&OnParamChange), false },
//			{ SERVICE_CONTROL_INTERROGATE, 
//				new _inlinehandler(&SVCTL::ServiceBase::OnInterrogate), false },
//			{ 0, NULL },
//		};
//
//	#if defined(_DEBUG) || defined(SVCTL_DONT_LEAK_HANDLERS)
//
//		static bool	bDoOnExit = true;		// Flag to register with _onexit()
//
//		// If we haven't already registered with _onexit to release the
//		// dynamically allocated ControlHandler objects, do so now
//
//		if(bDoOnExit) {
//
//			_onexit(_ReleaseControlMap);	// Register callback with _onexit
//			bDoOnExit = false;				// Don't do this more than once
//		}
//
//	#endif	// _DEBUG || SVCTL_DONT_LEAK_HANDLERS
//	
//		return pControlMap;					// Return pointer to the control map
//	}

// 08.26.2005 MGB
//
// Changed this a little so that it works right with Visual C++ .NET 2003. Basically,
// just added the "typedef _class __base" and qualified the arguments to each of the
// HANDLER_FUNC/HANDLER_EVENT/INLINE_HANDLER_FUNC to specify the class name before the
// mvar/function name.  Made the compiler quite a bit happier!

#define BEGIN_CONTROL_MAP(_class) \
typedef SVCTL::AsyncControlHandler<_class> _asynchandler; \
typedef SVCTL::EventControlHandler<_class> _eventhandler; \
typedef SVCTL::InlineControlHandler<_class> _inlinehandler; \
static SVCTL::PSERVICE_CONTROL_MAP WINAPI _GetControlMap(void) { \
typedef _class __base; \
static SVCTL::SERVICE_CONTROL_ENTRY pControlMap[] = {

#define HANDLER_FUNC(control, func) { control, new _asynchandler(&__base::func), true },

#define HANDLER_EVENT(control, event) { control, new _eventhandler(&__base::event), true },

#define INLINE_HANDLER_FUNC(control, func) { control, new _inlinehandler(&__base::func), false },

// In _DEBUG and SVCTL_DONT_LEAK_HANDLERS builds, END_CONTROL_MAP() adds the code
// necessary to register the _ReleaseControlMap callback with _onexit() as well

#if defined(_DEBUG) || defined(SVCTL_DONT_LEAK_HANDLERS)

#define END_CONTROL_MAP() \
{ SERVICE_CONTROL_INTERROGATE, new _inlinehandler(&SVCTL::ServiceBase::OnInterrogate), false }, \
{ 0, NULL }}; static bool bDoOnExit = true; \
if(bDoOnExit) { _onexit(_ReleaseControlMap); bDoOnExit = false; } return pControlMap; }

#else	// OK to leak the control handler objects

#define END_CONTROL_MAP() \
{ SERVICE_CONTROL_INTERROGATE, new _inlinehandler(&SVCTL::ServiceBase::OnInterrogate), false }, \
{ 0, NULL }}; return pControlMap; }

#endif	// _DEBUG || SVCTL_DONT_LEAK_HANDLERS

//---------------------------------------------------------------------------
// BLOCK_PENDING Macros
//
// Used to inform the ServiceStatus class object that certain control codes 
// should be rejected when the service is in a pending state

// DECLARE_BLOCK_WHILE_PENDING -- User-defined accepted control mask to block out

#define DECLARE_BLOCK_WHILE_PENDING(x) \
DWORD GetBlockedPendingControls(void) const { return x; }

// DECLARE_BLOCK_MULTIPLE_PENDING -- Prevents mutliple pending controls

#define DECLARE_BLOCK_MULTIPLE_PENDING() \
DWORD GetBlockedPendingControls(void) const { return SERVICE_ACCEPT_STOP | \
SERVICE_ACCEPT_PAUSE_CONTINUE; }

// DECLARE_BLOCK_ALL_WHILE_PENDING -- Prevents anything from coming in when pending

#define DECLARE_BLOCK_ALL_WHILE_PENDING() \
DWORD GetBlockedPendingControls(void) const { return 0xFFFFFFFF; }

//---------------------------------------------------------------------------

BEGIN_NAMESPACE(SVCTL)

//---------------------------------------------------------------------------
// Class SVCTL::ServiceBase
//
// ServiceBase implements all of the base functionality that gets inherited
// by the template-based service classes.
//---------------------------------------------------------------------------

class __declspec(novtable) ServiceBase
{

friend class ServiceMapEntry;		// ServiceMapEntry calls Base Execution funcs

protected:

	//-----------------------------------------------------------------------
	// Constructor / Destructor (protected)

	ServiceBase() {}
	virtual ~ServiceBase() {}

public:

	//-----------------------------------------------------------------------
	// Service Properties
	
	virtual DWORD GetBlockedPendingControls(void) const { return 0; }
	
	virtual LPCTSTR GetServiceDependencies(void) const { return NULL; }

	virtual LPCTSTR GetServiceDescription(void) const { return NULL; }

	virtual LPCTSTR GetServiceDisplayName(void) const = 0;

	virtual DWORD GetServiceErrorControl(void) const { return SERVICE_ERROR_NORMAL; }

	virtual LPCTSTR GetServiceGroup(void) const { return NULL; }

	virtual LPCTSTR GetServiceName(void) const = 0;

	virtual DWORD GetServiceStartType(void) const { return SERVICE_DEMAND_START; }

	virtual DWORD GetServiceType(void) const = 0;

	__declspec(property(get=GetServiceDependencies)) LPCTSTR	Dependencies;
	__declspec(property(get=GetServiceDescription))	 LPCTSTR	Description;
	__declspec(property(get=GetServiceDisplayName))	 LPCTSTR	DisplayName;
	__declspec(property(get=GetServiceErrorControl)) DWORD		ErrorControl;
	__declspec(property(get=GetServiceGroup))		 LPCTSTR	Group;
	__declspec(property(get=GetServiceName))		 LPCTSTR	Name;
	__declspec(property(get=GetServiceStartType))	 DWORD		StartType;
	__declspec(property(get=GetServiceType))		 DWORD		Type;

	// SERVICE_FAILURE_ACTIONS are special in that the derived service is
	// responsible for allocating and releasing the underlying structure
	
	virtual LPSERVICE_FAILURE_ACTIONS GetServiceFailureActions(void) const
		{ return NULL; }

	virtual void FreeServiceFailureActions(LPSERVICE_FAILURE_ACTIONS pFailureActions) 
		const {}

	//-----------------------------------------------------------------------
	// Service Execution (optionally implemented in derived class)

	virtual DWORD ClassInit(void) { return ERROR_SUCCESS; }

	virtual DWORD Init(DWORD dwArgc, LPTSTR *rgszArgv) { return ERROR_SUCCESS; }

	virtual DWORD Install(void) { return ERROR_SUCCESS; }

	virtual DWORD Run(void) = 0;
	
	virtual DWORD Remove(void) { return ERROR_SUCCESS; }

	virtual void Term(void) { return; }

	virtual void ClassTerm(void) { return; }

	//-----------------------------------------------------------------------
	// Service Control
	
	DWORD AddControls(DWORD dwControls) 
		{ return m_status.AddControls(dwControls); }

	DWORD ChangeControls(DWORD dwControls)
		{ return m_status.ChangeControls(dwControls); }
	
	DWORD ControlSelf(DWORD dwControl) { return HandlerEx(dwControl, 0, NULL); }

	DWORD ControlSelf(DWORD dwControl, DWORD dwEventType, void *pvEventData)
		{ return HandlerEx(dwControl, dwEventType, pvEventData); }
	
	DWORD ChangeStatus(DWORD dwStatus) { return m_status.Change(dwStatus); }

	DWORD QueryStatus(void) const { return m_status.Query(); }

	DWORD RemoveControls(DWORD dwControls)
		{ return m_status.RemoveControls(dwControls); }
	
	DWORD ReportStatus(void) const { return m_status.Report(); }

protected:

	//-----------------------------------------------------------------------
	// Protected Member Functions

	DWORD OnInterrogate(DWORD dwEventType, void *pvEventData)
		{ return m_status.Report(); }

	void ServiceMain(DWORD dwArgc, LPTSTR *rgszArgv);

private:

	ServiceBase(const ServiceBase &rhs);				// Disable copy
	ServiceBase& operator=(const ServiceBase &rhs);		// Disable assignment

	//-----------------------------------------------------------------------
	// Internal Service Control
		
	static DWORD WINAPI _HandlerEx(DWORD dwControl, DWORD dwEventType, 
		void* pvEventData, void *pvContext);

	virtual const PSERVICE_CONTROL_MAP GetControlMap(void) const = 0;
	
	DWORD HandlerEx(DWORD dwControl, DWORD dwEventType, void *pvEventData);
	
	virtual DWORD InvokeControlHandler(ControlHandler *pControlHandler, 
		DWORD dwEventType, void *pvEventData) = 0;

	//-----------------------------------------------------------------------
	// Internal Service Execution (called by ServiceMapEntry)

	DWORD BaseClassInit(void);

	void BaseClassTerm(void) { ClassTerm(); AuxiliaryClassTerm(); }
	
	DWORD BaseInstall(LPCTSTR, LPCTSTR);

	DWORD BaseRemove(void);
	
	//-----------------------------------------------------------------------
	// Auxiliary Base Class Execution (implemented in Service<>)

	virtual DWORD AuxiliaryClassInit(void) = 0;

	virtual void AuxiliaryClassTerm(void) = 0;

	virtual DWORD AuxiliaryInit(void) = 0;

	virtual DWORD AuxiliaryInstall(void) = 0;

	virtual void AuxiliaryRemove(void) = 0;

	virtual DWORD AuxiliaryStart(void) = 0;

	virtual void AuxiliaryStop(void) = 0;

	virtual void AuxiliaryTerm(void) = 0;

	//-----------------------------------------------------------------------
	// Private Member Functions

	DWORD DeriveAcceptedControls(PSERVICE_CONTROL_MAP pControlMap) const;
	
	//-----------------------------------------------------------------------
	// Member Variables

	ServiceStatus				m_status;		// Service status control class
};

//---------------------------------------------------------------------------
// Template SVCTL::AuxServiceBase
//
// AuxServiceBase is a vritual base class used by all of the "auxiliary"
// service class objects (such as registry and event logging) to automatically
// hook into various portions of the service initialization and installation
//---------------------------------------------------------------------------

template <class _derived>
class __declspec(novtable) AuxServiceBase
{
public:

	//-----------------------------------------------------------------------
	// Constructor / Destructor

	AuxServiceBase() : m_bAllocFailure(false) {}
	virtual ~AuxServiceBase() {}

	//-----------------------------------------------------------------------
	// Type Declarations

	typedef DWORD (_derived::*PMFN_INITFUNC)(void);
	typedef void		(_derived::*PMFN_TERMFUNC)(void);

protected:

	//-----------------------------------------------------------------------
	// Member Function Registration

	const bool RegisterAuxClassInit(PMFN_INITFUNC pmfnInit, PMFN_TERMFUNC pmfnTerm)
		{ return HookInitTerm(pmfnInit, pmfnTerm, m_hookClassInit); }
	
	const bool RegisterAuxInit(PMFN_INITFUNC pmfnInit, PMFN_TERMFUNC pmfnTerm)
		{ return HookInitTerm(pmfnInit, pmfnTerm, m_hookServiceInit); }

	const bool RegisterAuxInstall(PMFN_INITFUNC pmfnInstall, PMFN_TERMFUNC pmfnRemove)
		{ return HookInitTerm(pmfnInstall, pmfnRemove, m_hookInstall, false); }

	const bool RegisterAuxRun(PMFN_INITFUNC pmfnStart, PMFN_TERMFUNC pmfnStop)
		{ return HookInitTerm(pmfnStart, pmfnStop, m_hookRun); }

	//-----------------------------------------------------------------------
	// Member Function Invocation

	DWORD ClassInitAuxiliaries(_derived *pDerived)
		{ return HookedInit(pDerived, m_hookClassInit); }

	void ClassTermAuxiliaries(_derived *pDerived) 
		{ HookedTerm(pDerived, m_hookClassInit); }

	DWORD InitAuxiliaries(_derived *pDerived)
		{ return HookedInit(pDerived, m_hookServiceInit); }

	DWORD InstallAuxiliaries(_derived *pDerived) 
		{ return HookedInit(pDerived, m_hookInstall); }

	void RemoveAuxiliaries(_derived *pDerived)
		{ HookedTerm(pDerived, m_hookInstall); }

	DWORD StartAuxiliaries(_derived *pDerived)
		{ return HookedInit(pDerived, m_hookRun); }

	void StopAuxiliaries(_derived *pDerived)
		{ HookedTerm(pDerived, m_hookRun); }

	void TermAuxiliaries(_derived *pDerived) 
		{ HookedTerm(pDerived, m_hookServiceInit); }

private:

	AuxServiceBase(const AuxServiceBase &rhs);				// Disable copy
	AuxServiceBase& operator=(const AuxServiceBase &rhs);	// Disable assignment

	//-----------------------------------------------------------------------
	// Private Type Declarations

	typedef struct HookFuncEntry {

		// 08.26.2005 MGB
		//
		// Added reserved variable to prevent the misalignment in here that the VC.NET 2003
		// compiler is yelling at me about.  Funny how VC6 couldn't have cared less about this

		PMFN_INITFUNC		pmfnInit;			// Init / Install function ptr
		__int32				reserved;			// RESERVED: Used to correct alignment
		PMFN_TERMFUNC		pmfnTerm;			// Term / Remove function ptr
		bool				bInitialized;		// Flag if pmfnInit() succeeded
	};

	typedef Buffer<HookFuncEntry>	HookTable;	// A table of hook functions

	//-----------------------------------------------------------------------
	// Private Member Functions

	DWORD HookedInit(_derived *pDerived, HookTable &hookTable);

	void HookedTerm(_derived *pDerived, HookTable &hookTable);

	const bool HookInitTerm(PMFN_INITFUNC pmfnInit, PMFN_TERMFUNC pmfnTerm,
		HookTable &hookTable, bool bInitRequired = true);

	//-----------------------------------------------------------------------
	// Member Variables

	bool			m_bAllocFailure;		// Flag indicating allocation failure
	HookTable		m_hookClassInit;		// ClassInit hook table
	HookTable		m_hookInstall;			// Installation hook table
	HookTable		m_hookRun;				// Start/Stop hook table
	HookTable		m_hookServiceInit;		// ServiceInit hook table
};

//---------------------------------------------------------------------------
// AuxServiceBase::HookedInit (private)
//
// Calls into each hooked pmfnInit() function stored in a HookTable object
//
// Arguments :
//
//	pDerived		- Pointer to class object instance
//	hookTable		- Hook table to be initialized via cascade

template<class _derived>
DWORD AuxServiceBase<_derived>::HookedInit(_derived *pDerived, HookTable &hookTable)
{
	size_t			dwIndex;			// Loop index variable
	DWORD			dwResult;			// Result from function call

	if(m_bAllocFailure) return ERROR_NOT_ENOUGH_MEMORY;		// Verify alloc

	// Walk through the HookTable to call each of the pmfnInit() functions

	for(dwIndex = 0; dwIndex < hookTable.Length(); dwIndex++) {

		// If a function pointer is present, call through it
		
		if(hookTable[dwIndex].pmfnInit) {

			dwResult = (pDerived->*hookTable[dwIndex].pmfnInit)();
			
			if(dwResult != ERROR_SUCCESS) {

				HookedTerm(pDerived, hookTable);	// HookedTerm() everything
				return dwResult;					// Return the error code
			}
		}

		hookTable[dwIndex].bInitialized = true;		// Hook has been called
	}

	return ERROR_SUCCESS;
}

//---------------------------------------------------------------------------
// AuxServiceBase::HookedTerm (private)
//
// Calls into each hooked pmfnTerm() function stored in a HookTable object
//
// Arguments :
//
//	pDerived		- Pointer to class object instance
//	hookTable		- Hook table to be terminated via cascade

template<class _derived>
void AuxServiceBase<_derived>::HookedTerm(_derived *pDerived, HookTable &hookTable)
{
	int			lIndex;				// Loop index variable (signed)

	// Walk through the HookTable backwards to call each of the pmfnTerm() 
	// functions for any entry that has had pmfnInit() called successfully

	for(lIndex = static_cast<int>(hookTable.Length() - 1); lIndex >= 0; lIndex--) {

		// If pmfnInit() has been called, call through the pmfnTerm() pointer
		
		if((hookTable[lIndex].bInitialized) && (hookTable[lIndex].pmfnTerm))
			(pDerived->*hookTable[lIndex].pmfnTerm)();

		hookTable[lIndex].bInitialized = false;		// pmfnTerm() has been called
	}
}

//---------------------------------------------------------------------------
// AuxServiceBase::HookInitTerm (private)
//
// Adds a HookTableEntry to the specified HookTable object
//
// Arguments :
//
//	pmfnInit		- Pointer to PMFN_INITFUNC member.  Can be NULL
//	pmfnTerm		- Pointer to PMFN_TERMFUNC member.  Can be NULL
//	hookTable		- Hook table object to add the function pointers to
//	bInitRequired	- Flag indicating if Init() is required before Term()

template<class _derived>
const bool AuxServiceBase<_derived>::HookInitTerm(PMFN_INITFUNC pmfnInit,
								PMFN_TERMFUNC pmfnTerm, HookTable &hookTable,
								bool bInitRequired)
{
	size_t			dwEntries;			// Number of entries in the hook table

	if(m_bAllocFailure) return false;	// Previous allocation failure
	
	dwEntries = hookTable.Length();		// Get current number of table entries

	// Attempt to increase the size of the hook table by one element

	if(!hookTable.ReAllocate(dwEntries + 1)) {

		_RPTF0(_CRT_ASSERT, "Insufficient memory to hook init/term function");
		
		m_bAllocFailure = true;			// Set the allocation failure flag
		return false;					// Failed to hook the function pointers
	}

	hookTable[dwEntries].pmfnInit = pmfnInit;		// Store the function ptr
	hookTable[dwEntries].pmfnTerm = pmfnTerm;		// Store the function ptr

	// If Init() must be called before Term(), set bInitialized to false,
	// otherwise automatically set it to true (install/remove needs this)

	hookTable[dwEntries].bInitialized = !bInitRequired;

	return true;
}

//---------------------------------------------------------------------------
// Template SVCTL::Service
//
// Service is the template that specializes the ServiceBase class for each
// service class object in the module.  This also includes the necessary
// static entry points for ServiceMain() and the SERVICE_MAP entries
//---------------------------------------------------------------------------

template<class _derived>
class __declspec(novtable) Service : protected ServiceBase, 
	virtual private AuxServiceBase<_derived>
{
protected:

	//-----------------------------------------------------------------------
	// Constructor / Destructor (protected)
	
	Service() {}
	virtual ~Service() { s_pThis = NULL; }

public:

	//-----------------------------------------------------------------------
	// Static Entry Points
	//
	// Not illustrated:
	//
	//	_GetControlMap			- Implemented by BEGIN_CONTROL_MAP
	//	_GetServiceName			- Implemented by DECLARE_SERVICE_NAME
	//	_GetServiceDisplayName	- Implemented by DECLARE_SERVICE_DISPLAYNAME
	//	_GetServiceType			- Implemented by DECLARE_SERVICE_TYPE
	
	static ServiceBase* WINAPI _Construct(void);

	static int __cdecl _ReleaseControlMap(void);

	static void WINAPI _ServiceMain(DWORD dwArgc, LPTSTR *rgszArgv);

	//-----------------------------------------------------------------------
	// Default Static Entry Implementations

	// NOTE : Every derived class object must use the DECLARE_SERVICE_NAME
	// and DECLARE_SERVICE_DISPLAYNAME macros in their declaration

	DECLARE_SERVICE_TYPE(SERVICE_WIN32_SHARE_PROCESS)

	BEGIN_CONTROL_MAP(_derived)		// The default CONTROL_MAP will only respond
	END_CONTROL_MAP()				// to SERVICE_CONTROL_INTERROGATE

private:

	Service(const Service &rhs);				// Disable copy
	Service& operator=(const Service &rhs);		// Disable assignment

	//-----------------------------------------------------------------------
	// Internal Service Control 

	const PSERVICE_CONTROL_MAP GetControlMap(void) const
		{ return _derived::_GetControlMap(); }

	DWORD InvokeControlHandler(ControlHandler *pControlHandler, 
		DWORD dwEventType, void *pvEventData);

protected:

	//-----------------------------------------------------------------------
	// Auxiliary Base Class Execution Hooks
	
	DWORD AuxiliaryClassInit(void)
		{ _ASSERTE(s_pThis != NULL); return ClassInitAuxiliaries(s_pThis); }

	void AuxiliaryClassTerm(void) 
		{ _ASSERTE(s_pThis != NULL); ClassTermAuxiliaries(s_pThis); }
	
	DWORD AuxiliaryInit(void) 
		{ _ASSERTE(s_pThis != NULL); return InitAuxiliaries(s_pThis); }

	DWORD AuxiliaryInstall(void)
		{ _ASSERTE(s_pThis != NULL); return InstallAuxiliaries(s_pThis); }

	void AuxiliaryRemove(void)
		{ _ASSERTE(s_pThis != NULL); RemoveAuxiliaries(s_pThis); }

	DWORD AuxiliaryStart(void)
		{ _ASSERTE(s_pThis != NULL); return StartAuxiliaries(s_pThis); }

	void AuxiliaryStop(void)
		{ _ASSERTE(s_pThis != NULL); StopAuxiliaries(s_pThis); }

	void AuxiliaryTerm(void) 
		{ _ASSERTE(s_pThis != NULL); TermAuxiliaries(s_pThis); }

	//-----------------------------------------------------------------------
	// Member Variables

	static _derived*	s_pThis;			// Derived class instance pointer
};

//---------------------------------------------------------------------------
// Service static member variable initialization

template<class _derived> _derived* Service<_derived>::s_pThis = NULL;

//---------------------------------------------------------------------------
// Service::_Construct (static)
//
// Used to construct a new derived service class object instance
//
// Arguments :
//
//	NONE

template<class _derived>
ServiceBase* WINAPI Service<_derived>::_Construct(void)
{
	_ASSERTE(s_pThis == NULL);			// Should always be NULL here
	
	// Construct a new derived service class object and return a pointer
	// to it's ServiceBase implementation to the caller
	
	s_pThis = new _derived;
	return static_cast<ServiceBase*>(s_pThis);
}

//---------------------------------------------------------------------------
// Service::_ReleaseControlMap (static)
//
// Registered with _onexit() to ensure the destruction of the dynamically
// allocated ControlHandler objects from the _GetControlMap() function
//
// Arguments :
//
//	NONE

#if defined(_DEBUG) || defined(SVCTL_DONT_LEAK_HANDLERS)

template<class _derived>
int __cdecl Service<_derived>::_ReleaseControlMap(void)
{
	PSERVICE_CONTROL_MAP	pControlMap;		// Pointer to the control map

	pControlMap = _derived::_GetControlMap();	// Get the control map pointer
	_ASSERTE(pControlMap != NULL);				// Should never be NULL

	// Walk the control map in order to release all of the ControlHandler objects

	while(pControlMap->dwControl) { delete pControlMap->pHandler; pControlMap++; }

	return 0;
}

#endif	// _DEBUG || SVCTL_DONT_LEAK_HANDLERS

//---------------------------------------------------------------------------
// Service::_ServiceMain (static)
//
// Main service entry point for the derived service class object
//
// Arguments :
//
//	dwArgc			- Number of command line arguments passed in rgszArgv
//	rgszArgv		- Array of command line argument strings

template<class _derived> inline
void WINAPI Service<_derived>::_ServiceMain(DWORD dwArgc, LPTSTR *rgszArgv)
{
	_ASSERTE(s_pThis != NULL);			// _Construct() has not been called

	// Call into ServiceBase::ServiceMain to do all the actual work here

	s_pThis->ServiceMain(dwArgc, rgszArgv);
}

//---------------------------------------------------------------------------
// Service::InvokeControlHandler (private)
//
// Invokes a service control handler member function in the derived class
//
// Arguments :
//
//	pControlHandler		- Pointer to the control handler object to invoke
//	dwEventType			- Event type code argument from HandlerEx()
//	pvEventData			- Event data pointer argument from HandlerEx()

template<class _derived> inline
DWORD Service<_derived>::InvokeControlHandler(ControlHandler *pControlHandler,
												DWORD dwEventType, void *pvEventData)
{
	_ASSERTE(s_pThis != NULL);			// _Construct() has not been called

	// Invoke the member function by providing s_pThis as the instance pointer

	return pControlHandler->Invoke(s_pThis, dwEventType, pvEventData);
}

//---------------------------------------------------------------------------

END_NAMESPACE(SVCTL)

#pragma warning(pop)

#endif	// _SVCTLSVC_H_