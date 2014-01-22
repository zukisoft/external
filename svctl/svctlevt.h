//---------------------------------------------------------------------------
// svctlevt.h
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

#ifndef __SVCTLEVT_H_
#define __SVCTLEVT_H_
#pragma once

#pragma warning(push, 4)				// Enable maximum compiler warnings

//---------------------------------------------------------------------------
// Classes declared in this unit:
//
//	SVCTL::EventStrings
//	SVCTL::Event
//	SVCTL::EventsBase		(abstract base class)
//	SVCTL::ServiceEvents	(template)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// DECLARE_EVENTLOG Macros

#define DECLARE_EVENTLOG_CATEGORIES(x) \
DWORD GetEventLogCategoryCount(void) const { return x; }

#define DECLARE_EVENTLOG_CATEGORYFILE(x) \
LPCTSTR GetEventLogCategoryFile(void) const { return _T(x); }

#define DECLARE_EVENTLOG_CATEGORYFILE_ID(x) \
LPCTSTR GetEventLogCategoryFile(void) const { static SVCTL::ResString rstr(x); return rstr; }

#define DECLARE_EVENTLOG_MESSAGEFILE(x) \
LPCTSTR GetEventLogMessageFile(void) const { return _T(x); }

#define DECLARE_EVENTLOG_MESSAGEFILE_ID(x) \
LPCTSTR GetEventLogMessageFile(void) const { static SVCTL::ResString rstr(x); return rstr; }

#define DECLARE_EVENTLOG_PARAMETERFILE(x) \
LPCTSTR GetEventLogParameterFile(void) const { return _T(x); }

#define DECLARE_EVENTLOG_PARAMETERFILE_ID(x) \
LPCTSTR GetEventLogParameterFile(void) const { static SVCTL::ResString rstr(x); return rstr; }

#define DECLARE_EVENTLOG_SUPPORTED_TYPES(x) \
DWORD GetEventLogTypesSupported(void)(void) const { return x; }

#define DECLARE_EVENTLOG_TARGET(x) \
LPCTSTR GetEventLogTarget(void) const { return _T(x); }

#define DECLARE_EVENTLOG_TARGET_ID(x) \
LPCTSTR GetEventLogTarget(void) const { static SVCTL::ResString rstr(x); return rstr; }

BEGIN_NAMESPACE(SVCTL)

//---------------------------------------------------------------------------
// Constants

const WORD EVENTLOG_AUTO_CATEGORY	= 0xFFFF;		// Automatic Category code
const WORD EVENTLOG_AUTO_SEVERITY	= 0xFFFF;		// Automatic Severity code

const TCHAR EVENTLOG_REG_PREFIX[] = _T("System\\CurrentControlSet\\Services\\EventLog\\");

//---------------------------------------------------------------------------
// Class SVCTL::EventStrings
//
// EventStrings is used to manage insertion strings for the Event class
//---------------------------------------------------------------------------

class EventStrings
{
public:

	//-----------------------------------------------------------------------
	// Constructor / Destructor

	EventStrings() {}
	~EventStrings();

	//-----------------------------------------------------------------------
	// Overloaded Operators

	LPCTSTR operator[] (int index) const
		{ return operator[](static_cast<size_t>(index)); }
	
	LPCTSTR operator[] (size_t index) const;

	//-----------------------------------------------------------------------
	// Member Functions

	size_t GetCount(void) const { return m_rgstrBuffer.Length(); }

	const bool Add(LPCTSTR pszString);
	
	const bool Add(size_t dwIndex, LPCTSTR pszString);

	void Clear(void) { m_rgstrBuffer.Free(); }

	//-----------------------------------------------------------------------
	// Properties

	__declspec(property(get=GetCount))	size_t		Count;

private:

	EventStrings(const EventStrings& rhs);
	EventStrings& operator=(const EventStrings& rhs);

	//-----------------------------------------------------------------------
	// Member Variables

	Buffer<String*>			m_rgstrBuffer;			// Buffer to hold pointers
};

//---------------------------------------------------------------------------
// Class SVCTL::Event
//
// Event is used to manage all of the information concerning a Win32 event
//---------------------------------------------------------------------------

class Event
{
public:

	//-----------------------------------------------------------------------
	// Constructor / Destructor

	Event(DWORD dwEventId = 0, WORD wSeverity = EVENTLOG_AUTO_SEVERITY,
		WORD wCategory = EVENTLOG_AUTO_CATEGORY) : m_dwEventId(dwEventId), 
		m_wSeverity(wSeverity), m_wCategory(wCategory) {}

	~Event() {}

	//-----------------------------------------------------------------------
	// Member Functions

	const void* GetBinaryData(void) const { return m_binaryData; }

	size_t GetBinaryDataSize(void) const { return m_binaryData.Length(); }
	
	const WORD GetCategory(void) const { return m_wCategory; }

	DWORD GetEventId(void) const { return m_dwEventId; }

	EventStrings& GetInsertionStrings(void) { return m_strings; }

	const EventStrings& GetInsertionStrings(void) const { return m_strings; }

	const WORD GetSeverity(void) const { return m_wSeverity; }

	const PSID GetUserSid(void) const { return m_sidUser; }

	void SetBinaryData(const void* pvData, DWORD cbData);
	
	void SetCategory(WORD wCategory) { m_wCategory = wCategory; }

	void SetEventId(DWORD dwEventId) { m_dwEventId = dwEventId; }

	void SetSeverity(WORD wSeverity) { m_wSeverity = wSeverity; }
	
	void SetUserSid(PSID psidUser);

	void SetUserSidFromThread(bool bOpenAsSelf = false);

	//-----------------------------------------------------------------------
	// Properties

	__declspec(property(get=GetBinaryData))					void*	BinaryData;
	__declspec(property(get=GetBinaryDataSize))				size_t	BinaryDataSize;
	__declspec(property(get=GetCategory,put=SetCategory))	WORD	Category;
	__declspec(property(get=GetEventId,put=SetEventId))		DWORD	EventId;
	__declspec(property(get=GetSeverity,put=SetSeverity))	WORD	Severity;
	__declspec(property(get=GetUserSid,put=SetUserSid))		PSID	UserSid;

	__declspec(property(get=GetInsertionStrings))	EventStrings&	InsertionStrings;

private:

	Event(const Event& rhs);
	Event& operator=(const Event& rhs);

	//-----------------------------------------------------------------------
	// Member Variables

	DWORD					m_dwEventId;		// Event ID code
	WORD					m_wSeverity;		// Event severity code
	WORD					m_wCategory;		// Event category code
	EventStrings			m_strings;			// Event insertion strings
	Buffer<SID, BYTE>		m_sidUser;			// Event User SID data buffer
	Buffer<BYTE>			m_binaryData;		// Event Binary data buffer
};

//---------------------------------------------------------------------------
// Class SVCTL::Win32Event
//
// Wrapper around the Event class object to aid in logging Win32 error codes
// Note: The %1 insertion string will contain the Win32 error code
//---------------------------------------------------------------------------

class Win32Event : public Event
{
public:

	//-----------------------------------------------------------------------
	// Constructor / Destructor

	Win32Event(DWORD dwEventId = 0, DWORD dwStatus = GetLastError(), 
		WORD wSeverity = EVENTLOG_AUTO_SEVERITY, WORD wCategory = EVENTLOG_AUTO_CATEGORY);

	~Win32Event() {}

private:

	Win32Event(const Win32Event &rhs);				// Disable copy constructor
	Win32Event& operator=(const Win32Event &rhs);	// Disable assignment operator
};

//---------------------------------------------------------------------------
// Class SVCTL::ServiceEventsBase
//
// ServiceEventsBase is the shared base implementation behind the template
// class ServiceEvents<>
//---------------------------------------------------------------------------

class __declspec(novtable) ServiceEventsBase
{
public:

	//-----------------------------------------------------------------------
	// Constructor / Destructor

	ServiceEventsBase() : m_hEventLog(NULL) {}
	virtual ~ServiceEventsBase() {}

	//------------------------------------------------------------------------
	// Public Member Functions

	// SVCTL::Event Object

	DWORD LogEvent(const Event& event) const;

	// No insertion strings

	DWORD LogEvent(DWORD dwEventId, WORD wSeverity = EVENTLOG_AUTO_SEVERITY,
		WORD wCategory = EVENTLOG_AUTO_CATEGORY, PSID psidUser = NULL) const;

	// Single insertion string

	DWORD LogEvent(DWORD dwEventId, LPCTSTR pszInsert, 
		WORD wSeverity = EVENTLOG_AUTO_SEVERITY, WORD wCategory = EVENTLOG_AUTO_CATEGORY,
		PSID psidUser = NULL) const;
	
	// Multiple insertion strings

	DWORD LogEvent(DWORD dwEventId, LPCTSTR *rgszInserts, size_t cInserts,
		WORD wSeverity = EVENTLOG_AUTO_SEVERITY, WORD wCategory = EVENTLOG_AUTO_CATEGORY,
		PSID psidUser = NULL) const;

	// Binary Data

	DWORD LogEvent(DWORD dwEventId, const void *pvBinaryData, DWORD cbBinaryData,
		WORD wSeverity = EVENTLOG_AUTO_SEVERITY, WORD wCategory = EVENTLOG_AUTO_CATEGORY,
		PSID psidUser = NULL) const;

	// Anything and everything

	DWORD LogEvent(DWORD dwEventId, LPCTSTR *rgszInserts, size_t cInserts,
		const void *pvBinaryData, DWORD cbBinaryData, WORD wSeverity = EVENTLOG_AUTO_SEVERITY,
		WORD wCategory = EVENTLOG_AUTO_CATEGORY, PSID psidUser = NULL) const;

protected:

	//-----------------------------------------------------------------------
	// Protected Member Functions

	// 10.23.2009 - Changed this to a pure virtual, since it really doesn't make
	// sense to allow zero categories.  Anything that derives from ServiceEvents
	// has to have a DECLARE_EVENTLOG_CATEGORIES(x) in it now
	virtual DWORD GetEventLogCategoryCount(void) const = 0;  /// { return 0; }
	
	virtual LPCTSTR GetEventLogCategoryFile(void) const { return NULL; }
	
	virtual LPCTSTR GetEventLogMessageFile(void) const { return NULL; }

	virtual LPCTSTR GetEventLogParameterFile(void) const { return NULL; }

	virtual LPCTSTR GetEventLogTarget(void) const { return _T("Application"); }

	virtual DWORD GetEventLogTypesSupported(void) const
		{ return (EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE); }

	virtual LPCTSTR GetServiceName(void) const = 0;

	//-----------------------------------------------------------------------
	// Auxiliary Service Class Member Functions

	DWORD ServiceEventsInit(void);

	DWORD ServiceEventsInstall(void);

	void ServiceEventsRemove(void);

	void ServiceEventsTerm(void);

private:

	ServiceEventsBase(const ServiceEventsBase &rhs);			// Disable copy ctor
	ServiceEventsBase& operator=(const ServiceEventsBase &rhs);	// Disable assignment

	//-----------------------------------------------------------------------
	// Private Member Functions

	const WORD CategoryFromEventId(DWORD dwEventId) const;
	
	void FindMessageModule(LPCTSTR pszModule, String &strModule) const;
	
	const WORD SeverityFromEventId(DWORD dwEventId) const;

	//-----------------------------------------------------------------------
	// Member Variables

	HANDLE				m_hEventLog;		// Event source object handle
};

//---------------------------------------------------------------------------
// Class SVCTL::ServiceEvents
//
// ServiceEvents provides the specializations necessary to use the EventsBase
// class in conjunction with a derived service class object
//---------------------------------------------------------------------------

template<class _derived>
class __declspec(novtable) ServiceEvents : public ServiceEventsBase, 
	virtual private AuxServiceBase<_derived>
{
public:

	//-----------------------------------------------------------------------
	// Constructor / Destructor

	ServiceEvents();
	virtual ~ServiceEvents() {}

private:

	ServiceEvents(const ServiceEvents &rhs);				// Disable copy
	ServiceEvents& operator=(const ServiceEvents &rhs);		// Disable assignment
};

//---------------------------------------------------------------------------
// ServiceEvents Constructor
//
// Arguments :
//
//	NONE

template<class _derived>
ServiceEvents<_derived>::ServiceEvents()
{
	RegisterAuxInit(&ServiceEvents::ServiceEventsInit, &ServiceEvents::ServiceEventsTerm);
	RegisterAuxInstall(&ServiceEvents::ServiceEventsInstall, &ServiceEvents::ServiceEventsRemove);
}

//---------------------------------------------------------------------------

END_NAMESPACE(SVCTL)

#pragma warning(pop)

#endif	// __SVCTLEVT_H_

