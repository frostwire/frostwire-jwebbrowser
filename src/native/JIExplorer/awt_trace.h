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
#ifndef	AWT_TRACE_H
#define AWT_TRACE_H

#include <time.h>
#include <tchar.h>
#include <sys/timeb.h>
#include <assert.h>

/////////////////////////////////
// Security sybsystem
/////////////////////////////////
#ifndef ZZ
  #ifdef _LIB
    #define ZZ SUN_dbg_lib
  #else
    #define ZZ SUN_dbg_glb
  #endif //_LIB
#endif //ZZ 


#define IMPLEMENT_SUN_LOG( szLogName ) 

#ifndef  TRACE_SUFFIX 
  #define TRACE_SUFFIX 
#endif// TRACE_SUFFIX 

namespace ZZ{
  extern SECURITY_ATTRIBUTES __sa__;
  extern SECURITY_DESCRIPTOR __sd__;
  extern LPSECURITY_ATTRIBUTES __psd__;
  inline int InitSecurity()
  {
    // Initialize the new security descriptor.
    OSVERSIONINFO vi;
    vi.dwOSVersionInfoSize = sizeof(vi);
    if( !::GetVersionEx(&vi) || VER_PLATFORM_WIN32_NT != vi.dwPlatformId)
      __psd__ = NULL; //WIN95
    else{
      InitializeSecurityDescriptor (&__sd__, SECURITY_DESCRIPTOR_REVISION);
      // Add a NULL descriptor ACL to the security descriptor.
      SetSecurityDescriptorDacl (&__sd__, TRUE, (PACL)NULL, FALSE);
      __sa__.nLength = sizeof(__sa__);
      __sa__.lpSecurityDescriptor = &__sd__;
      __sa__.bInheritHandle = FALSE;
      __psd__ =  &__sa__;
    }
    return 0;
  }
  #define SUN_SECURITY_ATTRIBUTES         NULL
  #define SUN_KERNAL_SECURITY_ATTRIBUTES  ZZ::__psd__

  class CHandlerSup{
  public:
    HANDLE m_Handle;
    CHandlerSup():m_Handle( ::CreateMutex(SUN_KERNAL_SECURITY_ATTRIBUTES, FALSE, _T("$DEBUG$"))){}
    CHandlerSup(HANDLE Handle):m_Handle(Handle){}
    ~CHandlerSup(){ if(m_Handle) ::CloseHandle(m_Handle); }
    operator HANDLE() { return m_Handle; }
  };

  #ifdef _TO_MAPFILE
    #pragma comment(lib, "Advapi32.lib")
    //use customized output memory pipe
    //IMPLEMENT_SECURITY() macro call is mandatory 
    extern CHandlerSup g_hDataReady;
    extern CHandlerSup g_hSharedDbgFile;
    void DbgOut(LPCTSTR lpStr);
    #define IMPLEMENT_SECURITY()\
      SECURITY_ATTRIBUTES ZZ::__sa__;\
      SECURITY_DESCRIPTOR ZZ::__sd__;\
      LPSECURITY_ATTRIBUTES ZZ::__psd__;\
      static __security_init_result = ZZ::InitSecurity();\
      \
      ZZ::CHandlerSup ZZ::g_hDataReady(\
        ::CreateEvent(\
          SUN_KERNAL_SECURITY_ATTRIBUTES,\
          FALSE,\
          FALSE,\
          _T("SUN_DBWIN_DATA_READY")\
        )\
      );\
      ZZ::CHandlerSup ZZ::g_hSharedDbgFile(\
        ::CreateFileMapping(\
          (HANDLE)-1,\
          SUN_KERNAL_SECURITY_ATTRIBUTES,\
          PAGE_READWRITE, \
          0,\
          4096,\
          _T("SUN_DBWIN_BUFFER")\
        )\
      );\
      void ZZ::DbgOut(LPCTSTR lpStr)\
      {\
        HANDLE   hBufferReady = ::OpenEvent(\
          SYNCHRONIZE,\
          FALSE, \
          _T("SUN_DBWIN_BUFFER_READY")\
        );\
        if( hBufferReady ){\
          if( (HANDLE)ZZ::g_hSharedDbgFile && ::WaitForSingleObject(hBufferReady, 100)==WAIT_OBJECT_0 ){\
            DWORD dwSize = (_tcslen(lpStr)+1)*sizeof(TCHAR) + sizeof(DWORD);\
            LPVOID pSharedMem = ::MapViewOfFile(\
              ZZ::g_hSharedDbgFile,\
              FILE_MAP_WRITE,\
              0,\
              0,\
              dwSize\
            );\
            if(pSharedMem){\
              LPSTR pString = (LPSTR)pSharedMem + sizeof(DWORD);\
              *(LPDWORD)pSharedMem = ::GetCurrentProcessId();\
              _tcscpy((LPTSTR)pString, lpStr);\
              ::UnmapViewOfFile(pSharedMem);\
              ::SetEvent(ZZ::g_hDataReady);\
            }\
          }\
          ::CloseHandle(hBufferReady);\
        }\
      }
  #else
    //use standard Windows output stream
    //IMPLEMENT_SECURITY() macro call is optional
    inline void DbgOut(LPCTSTR lpStr) { ::OutputDebugString(lpStr); }
    #define IMPLEMENT_SECURITY()
    //\
    //  SECURITY_ATTRIBUTES ZZ::__sa__;\
    //  SECURITY_DESCRIPTOR ZZ::__sd__;\
    //  LPSECURITY_ATTRIBUTES ZZ::__psd__;\
    //  static __security_init_result = ZZ::InitSecurity()
  #endif


  inline LPCTSTR CreateTimeStamp(LPTSTR lpBuffer, int iBufferSize)
  {
    struct _timeb tb;
    _ftime(&tb);
    int	len = _tcsftime(lpBuffer, iBufferSize, _T("%b %d %H:%M:%S"), localtime(&tb.time));
    if(len && len+4<iBufferSize) 
      if( _sntprintf(lpBuffer+len, iBufferSize-len-1, _T(".%03d"), tb.millitm)<0 )
        lpBuffer[iBufferSize-len-1] = 0;
    return lpBuffer;
  }
  #define DTRACE_BUF_LEN 10240
  inline void snTraceEmp(LPCTSTR, ...) { }
  inline void snvTrace(LPCTSTR lpszFormat, va_list argList)
  {
    TCHAR szBuffer[DTRACE_BUF_LEN];//not under Mutex!!!!
    if( _vsntprintf( szBuffer, DTRACE_BUF_LEN, lpszFormat, argList )<0 ) 
      szBuffer[DTRACE_BUF_LEN-1] = 0;
    TCHAR szTime[32];
    CreateTimeStamp(szTime, sizeof(szTime));   
    _tcscat(szTime, _T(" "));
    TCHAR szBuffer1[DTRACE_BUF_LEN];
    int iFormatLen = _tcslen(lpszFormat);
    BOOL bErrorReport = iFormatLen>6 && _tcscmp(lpszFormat + iFormatLen - 6, _T("[%08x]"))==0;
    int iTimeLen = _tcslen(szTime);
    if( _sntprintf( 
      szBuffer1 + iTimeLen, 
      DTRACE_BUF_LEN - iTimeLen - 1, //reserver for \n
      _T("P:%04d T:%04d ") TRACE_SUFFIX _T("%s%s"), 
      ::GetCurrentProcessId(), 
      ::GetCurrentThreadId(), 
      bErrorReport?_T("Error:"):_T(""),
      szBuffer
    )<0)
      _tcscpy(szBuffer1 + DTRACE_BUF_LEN - 5, _T("...")); //reserver for \n

    memcpy(szBuffer1, szTime, iTimeLen*sizeof(TCHAR));
    _tcscat(szBuffer1, _T("\n"));
    //_ftprintf(stderr, szBuffer1 ); 
  }
  inline void snTrace(LPCTSTR lpszFormat, ... ) 
  {
    va_list argList;
    va_start(argList, lpszFormat);
    snvTrace(lpszFormat, argList);
    va_end(argList);
  }

  #ifndef _MDEBUG 
    inline int  snCheckExeption(int) {return EXCEPTION_EXECUTE_HANDLER;}  
    inline void snAssertEx(BOOL bCheck) { 
      if(!bCheck) 
        snTrace(_T("Assert")); 
    }  
  #else //_MDEBUG
    inline int snCheckExeption(int code){
      //TCHAR szBuffer[DTRACE_BUF_LEN];
      //_stprintf( szBuffer, _T("Exception[%08x]! P:%04d T:%04d\nPress \'Yes\' after debugger attach!"), code, ::GetCurrentProcessId(), ::GetCurrentThreadId() );
      //return (::MessageBox(NULL, szBuffer, _T(""), MB_YESNO)==IDYES)?/*EXCEPTION_CONTINUE_SEARCH*/EXCEPTION_CONTINUE_EXECUTION:EXCEPTION_EXECUTE_HANDLER;    
      BOOL  bContinue = TRUE, bNotBreak = TRUE;
      while(bNotBreak){
        ::MessageBeep((UINT)-1);
        ::Sleep(1000);
      }
      return  bContinue?EXCEPTION_CONTINUE_EXECUTION:EXCEPTION_EXECUTE_HANDLER;
    }
    inline void snAssertEx(BOOL bCheck) { 
      if(!bCheck) 
        _asm int 3; 
    }  
  #endif//_MDEBUG
  #define snAssert(exp) ZZ::snAssertEx((BOOL)(exp))
}//ZZ namespace end

//inline BOOL _istspace(TCHAR ch) {return ch && _tcschr(_T(" \t\n\r"), ch); }  

#define STRACE1       ZZ::snTrace
#define SCHKE()       ZZ::snCheckExeption(GetExceptionCode())
#ifndef  _MDEBUG
  #define STRACE      ZZ::snTrace
#else // _MDEBUG
  #define STRACE      ZZ::snTraceEmp
#endif// _MDEBUG
#define STRACE0       ZZ::snTrace
#define SASSERT       snAssert
struct CLogEntryPoint1 {
    LPCTSTR m_lpTitle;
    CLogEntryPoint1(LPCTSTR lpTitle):m_lpTitle(lpTitle) { STRACE1(_T("{%s"), m_lpTitle); }
    ~CLogEntryPoint1(){ STRACE1(_T("}%s"), m_lpTitle); }
};
struct CLogEntryPoint0 {
    LPCTSTR m_lpTitle;
    CLogEntryPoint0(LPCTSTR lpTitle):m_lpTitle(lpTitle) { STRACE0(_T("{%s"), m_lpTitle); }
    ~CLogEntryPoint0(){ STRACE0(_T("}%s"), m_lpTitle); }
};
#define SEP1(msg)    CLogEntryPoint1 _ep1_(msg);
#define SEP0(msg)    CLogEntryPoint0 _ep0_(msg);
#ifndef  _MDEBUG
  #define SEP(msg)   CLogEntryPoint1 _ep1_(msg);
#else // _MDEBUG
  #define SEP(msg)   CLogEntryPoint0 _ep0_(msg);
#endif// _MDEBUG

#endif
