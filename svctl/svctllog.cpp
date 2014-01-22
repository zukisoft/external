//---------------------------------------------------------------------------
// svctllog.cpp
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
// SVCTL::ServiceActivityLogBase Implementation
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// ServiceActivityLogBase::CreateLogDirectory (private)
//
// Checks access to, or creates if necessary, the logging directory
//
// Arguments :

#pragma warning(disable:4100)
DWORD ServiceActivityLogBase::CreateLogDirectory(LPCTSTR pszDirectory) const
{
	return 0;
}

//---------------------------------------------------------------------------
// ServiceActivityLogBase::ActivityLogInit (private)
//
// Initializes the activity log class by opening the log file
//
// Arguments :
//
//	NONE

DWORD ServiceActivityLogBase::ActivityLogInit(void)
{
	String		strDirectory;		// Directory name to place the log file
	String		strLogFile;			// Activity log base file name

	// Retrieve and expand the directory name where the log will be stored

	strDirectory = GetActivityLogDirectoryName();
	strDirectory.ExpandVariables();

	// Attempt to create the target directory if it does not already exist

	if(!CreateDirectory(strDirectory, NULL))
		if(GetLastError() != ERROR_ALREADY_EXISTS) return GetLastError();

	// Attempt to 

	strLogFile = GetActivityLogFileName();


	CreateDirectory(_T("C:\\Winnt\\System32\\MikeB"), NULL);
	// ERROR_ALREADY_EXISTS is OK

	return 0;
}

void ServiceActivityLogBase::ActivityLogTerm(void)
{
}

//---------------------------------------------------------------------------

END_NAMESPACE(SVCTL)

#pragma warning(pop)
