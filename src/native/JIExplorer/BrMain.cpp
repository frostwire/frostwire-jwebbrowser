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

#include "stdafx.h"
#include "BrMain.h"
#include "BrHolderThread.h"

JavaVM *jvm;

BOOL APIENTRY DllMain( 
    HANDLE hModule, 
    DWORD  ul_reason_for_call, 
    LPVOID lpReserved
){
    switch( ul_reason_for_call ){
    case DLL_PROCESS_ATTACH:
        BrowserThread::RegisterStaffWndClass((HINSTANCE)hModule);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}


jobject makeGlobal(
    JNIEnv* env,     
    jobject l_obj
){
    if(!JNU_IsNull(env, l_obj)) {
        jobject g_obj = env->NewGlobalRef(l_obj);
        env->DeleteLocalRef(l_obj);
        if (JNU_IsNull(env, g_obj)) {
            JNU_ThrowOutOfMemoryError(env, "");
        } else {
            return g_obj;
        }
    }                                                               
    return NULL;
}

void releaseGlobal(
    JNIEnv* env,     
    jobject g_obj
){
    if(!JNU_IsNull(env, g_obj)) {
        env->DeleteGlobalRef(g_obj);
    }
}

jclass getGlobalJavaClazz(
    JNIEnv* env, 
    const char *name
){               
    return (jclass)makeGlobal(env, env->FindClass(name) );
}


extern "C" {

/* Initialize the Java VM instance variable when the library is 
   first loaded */
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
{
    jvm = vm;
    return JNI_VERSION_1_2;
}

}; /* extern "C" */
