//---------------------------------------------------------------------------
// svctllog.h
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

#ifndef __SVCTLLOG_H_
#define __SVCTLLOG_H_
#pragma once

#pragma warning(push, 4)				// Enable maximum compiler warnings
#pragma warning(disable:4127)			// "conditional expression is constant"

//---------------------------------------------------------------------------
// Classes declared in this unit:
//
//	SVCTL::ServiceActivityLogBase		(abstract base class)
//	SVCTL::ServiceActivityLog
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Macros

// DECLARE_ACTIVITY_LOG_DIRECTORY

// DECLARE_ACTIVITY_LOG_FILENAME

BEGIN_NAMESPACE(SVCTL)

//---------------------------------------------------------------------------
// Constants

const TCHAR DEFAULT_LOGDIR_NAME[] = _T("%SYSTEMROOT%\\System32\\LogFiles");
const TCHAR DEFAULT_LOGFILE_EXTENSION[] = _T(".log");

//---------------------------------------------------------------------------
// Class ServiceActivityLogBase
//
// ServiceActivityLogBase is the abstract base class from which all 
// ServiceActivityLog<> template classes inherit their common implementation
//---------------------------------------------------------------------------

class __declspec(novtable) ServiceActivityLogBase
{
public:

	//-----------------------------------------------------------------------
	// Constructor / Destructor

	ServiceActivityLogBase() {}
	virtual ~ServiceActivityLogBase() {}

	//-----------------------------------------------------------------------
	// Public Member Functions

	DWORD __cdecl LogActivity(LPCTSTR pszFormat, ...) const;

protected:

	//-----------------------------------------------------------------------
	// Protected Member Functions

	virtual LPCTSTR GetActivityLogDirectoryName(void) const
		{ return DEFAULT_LOGDIR_NAME; }

	virtual LPCTSTR GetActivityLogFileName(void) const = 0;

	virtual LPCTSTR GetServiceName(void) const = 0;

	//-----------------------------------------------------------------------
	// Auxiliary Service Class Member Functions

	DWORD ActivityLogInit(void);

	void ActivityLogTerm(void);

private:

	ServiceActivityLogBase(const ServiceActivityLogBase &rhs);
	ServiceActivityLogBase& operator=(const ServiceActivityLogBase &rhs);

	//-----------------------------------------------------------------------
	// Private Member Functions

	DWORD CreateLogDirectory(LPCTSTR pszDirectory) const;

	//-----------------------------------------------------------------------
	// Member Variables
};

//---------------------------------------------------------------------------
// Class SVCTL::ServiceActivityLog
//
// ServiceActivityLog provides the specialization necessary to use the 
// ServiceActivityLogBase class in conjunction with a derived service class
//---------------------------------------------------------------------------

template<class _derived>
class __declspec(novtable) ServiceActivityLog : public ServiceActivityLogBase, 
	virtual private AuxServiceBase<_derived>
{
public:

	//-----------------------------------------------------------------------
	// Constructor / Destructor

	ServiceActivityLog();
	virtual ~ServiceActivityLog() {}

protected:

	//-----------------------------------------------------------------------
	// Protected Member Functions

	LPCTSTR GetActivityLogFileName(void) const;

private:

	ServiceActivityLog(const ServiceActivityLog &rhs);
	ServiceActivityLog& operator=(const ServiceActivityLog &rhs);
};

//---------------------------------------------------------------------------
// ServiceActivityLog Constructor
//
// Arguments :
//
//	NONE

template<class _derived>
ServiceActivityLog<_derived>::ServiceActivityLog()
{
	// Hook into the service class initialization and termination code

	RegisterAuxInit(ServiceActivityLogBase::ActivityLogInit, 
		ServiceActivityLogBase::ActivityLogTerm);
}

//---------------------------------------------------------------------------
// ServiceActivityLog::GetActivityLogFileName
//
// Default implementation will use the service key name as the log file name
//
// Arguments :
//
//	NONE

template<class _derived>
LPCTSTR ServiceActivityLog<_derived>::GetActivityLogFileName(void) const
{
	static String strName(GetServiceName(), DEFAULT_LOGFILE_EXTENSION);

	return strName;				// Return a pointer to the static string
}

//---------------------------------------------------------------------------

END_NAMESPACE(SVCTL)

#pragma warning(pop)

#endif		// __SVCTLLOG_H_

