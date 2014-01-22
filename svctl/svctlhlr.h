//---------------------------------------------------------------------------
// svctlhlr.h
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

#ifndef __SVCTLHLR_H_
#define __SVCTLHLR_H_
#pragma once

#pragma warning(push, 4)				// Enable maximum compiler warnings
#pragma warning(disable:4100)			// "unreferenced formal parameter"
#pragma warning(disable:4127)			// "conditional expression is constant"

//---------------------------------------------------------------------------
// Classes declared in this unit:
//
//	SVCTL::ControlHandler			(abstract base class)
//	SVCTL::AsyncControlHandler		(template)
//	SVCTL::EventControlHandler		(template)
//	SVCTL::InlineControlHandler		(template)
//---------------------------------------------------------------------------

BEGIN_NAMESPACE(SVCTL)

class ControlHandler;					// Forward class declaration

//---------------------------------------------------------------------------
// Type Declarations

// SERVICE_CONTROL_MAP - Structure used to pass control handler data between
// the template-based service classes and the ServiceBase HandlerEx() function

typedef struct {

	DWORD				dwControl;		// Control that the service handles
	ControlHandler*		pHandler;		// Pointer to the handler function
	bool				bSetPending;	// Flag to automatically launch pending

} SERVICE_CONTROL_ENTRY, *PSERVICE_CONTROL_ENTRY, *PSERVICE_CONTROL_MAP;

//---------------------------------------------------------------------------
// Class SVCTL::ControlHandler
//
// ControlHandler is an abstract base class that is used in conjunction with
// the various xxxxControlHandler<> templates to invoke service class member 
// functions in response to service control codes
//---------------------------------------------------------------------------

class __declspec(novtable) ControlHandler
{
public:

	//-----------------------------------------------------------------------
	// Constructor / Destructor
	
	explicit ControlHandler(bool bAsync) : m_bAsync(bAsync) {}
	virtual ~ControlHandler() {}

	//-----------------------------------------------------------------------
	// Member Functions

	DWORD Invoke(void *pvClassThis, DWORD dwEventType, void *pvEventData);

private:

	ControlHandler(const ControlHandler& rhs);				// Disable copy
	ControlHandler& operator=(const ControlHandler& rhs);	// Disable assignment

	//-----------------------------------------------------------------------
	// Private Type Declarations

	typedef struct AsyncInvokeContext {

		ControlHandler*		pThis;			// Pointer to this class object
		void*				pvClassThis;	// Pointer to derived class object
	};

	//-----------------------------------------------------------------------
	// Private Member Functions

	static DWORD WINAPI AsyncInvokeProc(void *pvArg);

	virtual DWORD InvokeMember(void *pvClassThis, DWORD dwEventType,
		void *pvEventData) const = 0;
	
	//-----------------------------------------------------------------------
	// Member Variables

	const bool		m_bAsync;			// Flag for asynchronous invocation
};

//---------------------------------------------------------------------------
// Template SVCTL::AsyncControlHandler
//
// AsyncControlHandler provides the specialization necessary to implement an
// asynchronous control handler function in the service class object
//---------------------------------------------------------------------------

template<class _derived>
class AsyncControlHandler : public ControlHandler
{
public:

	//-----------------------------------------------------------------------
	// Type Declarations
	
	typedef void(_derived::*PMFN_HANDLER)(void);
	
	//-----------------------------------------------------------------------
	// Constructor

	explicit AsyncControlHandler(PMFN_HANDLER pmfn) : ControlHandler(true), 
		m_pmfn(pmfn) {}

private:

	AsyncControlHandler(const AsyncControlHandler &rhs);
	AsyncControlHandler& operator=(const AsyncControlHandler& rhs);

	//-----------------------------------------------------------------------
	// Private Member Functions

	DWORD InvokeMember(void *pvClassThis, DWORD dwEventType, 
		void *pvEventData) const;
		
	//-----------------------------------------------------------------------
	// Member Variables

	const PMFN_HANDLER		m_pmfn;			// Pointer to the member function
};

//---------------------------------------------------------------------------
// AsyncControlHandler::InvokeMember
//
// Calls the member function pointed to by this class object
//
// Arguments :
//
//	pvClassThis			- Pointer to the derived class object instance
//	dwEventType			- Event type code argument from HandlerEx()
//	pvEventData			- Event data pointer argument from HandlerEx()

template<class _derived>
DWORD AsyncControlHandler<_derived>::InvokeMember(void *pvClassThis,
	DWORD dwEventType, void *pvEventData) const
{
	_ASSERTE(pvClassThis != NULL);			// Should never be NULL
	_ASSERTE(m_pmfn != NULL);				// Should never be NULL
	
	// Call into the member function pointed to by the m_pmfn variable
	
	(reinterpret_cast<_derived*>(pvClassThis)->*m_pmfn)();

	return ERROR_SUCCESS;					// Always return NO_ERROR for async
}

//---------------------------------------------------------------------------
// Template SVCTL::EventControlHandler
//
// EventControlHandler provides the specialization necessary to implement a 
// control handler that signals a kernel event object
//---------------------------------------------------------------------------

template<class _derived>
class EventControlHandler : public ControlHandler
{
public:

	//-----------------------------------------------------------------------
	// Type Declarations
	
	typedef HANDLE	_derived::*PHEVT_HANDLER;
	
	//-----------------------------------------------------------------------
	// Constructor

	explicit EventControlHandler(PHEVT_HANDLER phevt) : ControlHandler(false),
		m_phevt(phevt) {}

private:

	EventControlHandler(const EventControlHandler &rhs);
	EventControlHandler& operator=(const EventControlHandler& rhs);

	//-----------------------------------------------------------------------
	// Private Member Functions

	DWORD InvokeMember(void *pvClassThis, DWORD dwEventType, 
		void *pvEventData) const;
		
	//-----------------------------------------------------------------------
	// Member Variables

	const PHEVT_HANDLER		m_phevt;		// Pointer to the event object
};

//---------------------------------------------------------------------------
// EventControlHandler::InvokeMember
//
// Signals the kernel event object pointed to by this class object
//
// Arguments :
//
//	pvClassThis			- Pointer to the derived class object instance
//	dwEventType			- Event type code argument from HandlerEx()
//	pvEventData			- Event data pointer argument from HandlerEx()

template<class _derived>
DWORD EventControlHandler<_derived>::InvokeMember(void *pvClassThis,
	DWORD dwEventType, void *pvEventData) const
{
	_ASSERTE(pvClassThis != NULL);			// Should never be NULL
	_ASSERTE(m_phevt != NULL);				// Should never be NULL
	
	// Cast a pointer to the derived class object for clarity below

	_derived* pDerived = reinterpret_cast<_derived*>(pvClassThis);

	// Signal the kernel event object pointer to by the m_phevt variable

	return (SetEvent(pDerived->*m_phevt)) ? ERROR_SUCCESS : GetLastError();
}

//---------------------------------------------------------------------------
// Template SVCTL::InlineControlHandler
//
// InlineControlHandler provides the specialization necessary to implement a
// control handler function that is directly called by HandlerEx() 
//---------------------------------------------------------------------------

template<class _derived>
class InlineControlHandler : public ControlHandler
{
public:

	//-----------------------------------------------------------------------
	// Type Declarations
	
	typedef DWORD(_derived::*PMFN_HANDLER)(void);
	typedef DWORD(_derived::*PMFN_HANDLEREX)(DWORD, void*);
	
	//-----------------------------------------------------------------------
	// Constructors

	explicit InlineControlHandler(PMFN_HANDLER pmfn) : ControlHandler(false), 
		m_pmfn(pmfn), m_pmfnEx(NULL) {}

	explicit InlineControlHandler(PMFN_HANDLEREX pmfn) : ControlHandler(false),
		m_pmfn(NULL), m_pmfnEx(pmfn) {}

private:

	InlineControlHandler(const InlineControlHandler &rhs);
	InlineControlHandler& operator=(const InlineControlHandler& rhs);

	//-----------------------------------------------------------------------
	// Private Member Functions

	DWORD InvokeMember(void *pvClassThis, DWORD dwEventType, 
		void *pvEventData) const;
		
	//-----------------------------------------------------------------------
	// Member Variables

	// 08.26.2005 MGB
	//
	// Added m_reserved variable to prevent the misalignment in here that the VC.NET 2003
	// compiler is yelling at me about.  Funny how VC6 couldn't have cared less about this

	const PMFN_HANDLER		m_pmfn;			// Pointer to the member function
	__int32					m_reserved;		// RESERVED - Used to correct alignment
	const PMFN_HANDLEREX	m_pmfnEx;		// Pointer to the member function
};

//---------------------------------------------------------------------------
// InlineControlHandler::InvokeMember
//
// Calls the member function pointed to by this class object
//
// Arguments :
//
//	pvClassThis			- Pointer to the derived class object instance
//	dwEventType			- Event type code argument from HandlerEx()
//	pvEventData			- Event data pointer argument from HandlerEx()

template<class _derived>
DWORD InlineControlHandler<_derived>::InvokeMember(void *pvClassThis, 
	DWORD dwEventType, void *pvEventData) const
{
	_ASSERTE(pvClassThis != NULL);						// Should never be NULL
	_ASSERTE((m_pmfn != NULL) || (m_pmfnEx != NULL));	// One must be non-NULL
	
	// Cast a pointer to the derived class object for clarity below

	_derived *pDerived = reinterpret_cast<_derived*>(pvClassThis);

	// Call into either the standard or extended handler function, depending
	// on which kind was set when this object was constructed
	
	if(m_pmfn) return (pDerived->*m_pmfn)();
	else return (pDerived->*m_pmfnEx)(dwEventType, pvEventData);
}

//---------------------------------------------------------------------------

END_NAMESPACE(SVCTL)

#pragma warning(pop)

#endif	// _SVCTLSVC_H_