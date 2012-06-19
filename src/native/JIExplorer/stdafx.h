//////////////////////////////////
// Copyright (C) 2008 Sun Microsystems, Inc. All rights reserved. Use is
// subject to license terms.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the Lesser GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.

//////////////////////////////////
//  ATTENTION!!!
//================================  
//  To build the project the 
//  following environment variables 
//  have to be set like this:
//
//  set ALT_OUTPUTDIR=D:\java1.7.0\j2se\build
//  set ALT_J2SE_SRC=D:\java1.7.0\j2se\src

// Windows Header Files:
#define UNICODE
#define _UNICODE
#define _WIN32_DCOM
#define _WIN32_WINNT 0x0501


#include <windows.h>
// standard Windows and C headers
#define VC_EXTRALEAN	/* speeds compilation */
    #ifndef STRICT
    #define STRICT /* forces explicit typedef's for windows.h */
#endif
#include <windows.h>
#include <process.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <tchar.h>
#include <io.h>
#include <errno.h>
#include <fdi.h>
#include <fci.h>


#include <jni.h>
#include <jni_util.h>
#include <jvm.h>
extern JavaVM *jvm;
extern RECT g_emptyRect;

#define CAB_NewStringPlatform(env, x) env->NewStringUTF(x) 

//WIN32 standard MACRO and logger implementation
#define _AWT_OLE_EX_
#include "awt_ole.h"

#if defined(_WIN64)
    #define ptoa i64toa
    #define atop atoi64
#else
    #define ptoa ltoa
    #define atop atol
#endif

bool IsShiftKeyDown();
bool IsAltKeyDown();
bool IsCtrlKeyDown();

