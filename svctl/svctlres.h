//---------------------------------------------------------------------------
// svctlres.h
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

#ifndef __SVCTLRES_H_
#define __SVCTLRES_H_
#pragma once

//---------------------------------------------------------------------------
// Base SVCTL Resource ID = 0xD800
//---------------------------------------------------------------------------

// 0xD800 = SVCTL::MinimalUI

#define SVCTL_IDDMUI_PROGRESS				0xD800
#define SVCTL_IDCMUI_PROGRESS_TEXT			0xD801

// 0xD880 = SVCTL::GraphicalUI

#define SVCTL_IDSGUI_WNDCLASS				0xD880
#define SVCTL_IDSGUI_DEFTITLE				0xD881

// 0xD900 = SVCTL::ConsoleUI

#define SVCTL_IDSCUI_XXXXX

// 0xD980 = SVCTL::ServiceManager

#define	SVCTL_IDSMGR_INSTALL_SUCCESS		0xD980
#define SVCTL_IDSMGR_REMOVE_SUCCESS			0xD981
#define SVCTL_IDSMGR_REMOVE_PROMPT			0xD982
#define SVCTL_IDSMGR_INSTALL_RESTART		0xD983
#define SVCTL_IDSMGR_REMOVE_RESTART			0xD984
#define SVCTL_IDSMGR_RESTART_FAILED			0xD985

// 0xDA00 = SVCTL::ServiceInstall

#define SVCTL_IDSINST_INSTALLERR_FMT		0xDA00
#define SVCTL_IDSINST_REMOVEERR_FMT			0xDA01
#define SVCTL_IDSINST_RECONFIGURE_FMT		0xDA02
#define SVCTL_IDSINST_STOPSERVICE_FMT		0xDA03
#define SVCTL_IDSINST_STOPDEPENDENTS_FMT	0xDA04
#define SVCTL_IDSINST_STOPDEPENDENTS_FOOTER	0xDA05

// 0xDA80 = 

// 0xDB00 = 

// 0xDB80 = 

// 0xDC00 = 

// 0xDC80 = 

// 0xDD00 = 

// 0xDD80 = 

// 0xDE00 = 

// 0xDE80 = 

// 0xDF00 = 

// 0xDF80 = 

//---------------------------------------------------------------------------

#endif	// __SVCTLRES_H_
