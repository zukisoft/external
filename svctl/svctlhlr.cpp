//---------------------------------------------------------------------------
// svctlhlr.cpp
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

BEGIN_NAMESPACE(SVCTL)

//---------------------------------------------------------------------------
// SVCTL::ControlHandler Implementation
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// ControlHandler::AsyncInvokeProc (private, static)
//
// Invokes the member function pointed to by this class object on a pooled
// worker thread
//
// Arguments :
//
//	pvArg			- Argument passed into QueueUserWorkItem()

DWORD WINAPI ControlHandler::AsyncInvokeProc(void *pvArg)
{
	AsyncInvokeContext*		pContext;		// Context information structure
	DWORD					dwResult;		// Result from function call

	// Cast the pointer to the AsyncInvokeContext data structure

	pContext = reinterpret_cast<AsyncInvokeContext*>(pvArg);
	_ASSERTE(pContext != NULL);

	// Invoke the actual control handler member function on this thread,
	// using dummy arguments for the event type and event data

	dwResult = pContext->pThis->InvokeMember(pContext->pvClassThis, 0, NULL);

	delete pContext;						// Release the context data
	return dwResult;						// Return result from handler
}

//---------------------------------------------------------------------------
// ControlHandler::Invoke
//
// Invokes the member function pointed to by this class object
//
// Arguments :
//
//	pvClassThis			- Pointer to derived class object instance
//	dwEventType			- Event type code parameter from HandlerEx()
//	pvEventData			- Event data pointer parameter from HandlerEx()

DWORD ControlHandler::Invoke(void *pvClassThis, DWORD dwEventType, 
								   void *pvEventData)
{
	// For synchronous (inline) handler functions, just call through our
	// virtual function call operator
	
	if(!m_bAsync) return InvokeMember(pvClassThis, dwEventType, pvEventData);

	// Asynchronous handler functions require a new AsyncInvokeContext
	// structure to be passed into the AsyncInvokeProc static callback
	
	AsyncInvokeContext* pAsyncContext = new AsyncInvokeContext;
	if(!pAsyncContext) return ERROR_NOT_ENOUGH_MEMORY;

	pAsyncContext->pThis = this;				// Copy our (this) pointer
	pAsyncContext->pvClassThis = pvClassThis;	// Copy the class object pointer

	// Attempt to invoke the control handler on a pooled worker thread
	
	if(!QueueUserWorkItem(AsyncInvokeProc, pAsyncContext, WT_EXECUTELONGFUNCTION)) {

		delete pAsyncContext;			// Release the context structure on failure
		return GetLastError();			// Return error from QueueUserWorkItem()
	}

	return ERROR_SUCCESS;
}

//---------------------------------------------------------------------------

END_NAMESPACE(SVCTL)