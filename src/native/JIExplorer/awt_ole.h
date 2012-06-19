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
#ifndef AWT_OLE_H
#define AWT_OLE_H

#include <windows.h>
#include <ole2.h>
#include <tchar.h>
#include <comdef.h>
#include <comutil.h>
#include "awt_trace.h"

#define OLE_BAD_COOKIE ((DWORD)-1)

#define OLE_TRACENOTIMPL(msg)\
        STRACE(_T("Warning:%s"), msg);\
        return E_NOTIMPL;

#define OLE_TRACEOK(msg)\
        STRACE0(_T("Info:%s"), msg);\
        return S_OK;


#define OLE_DECL\
        HRESULT _hr_ = S_OK;

#define OLE_NEXT_TRY\
        try{

#define OLE_TRY\
        OLE_DECL\
        try{

#define OLE_HRT(fnc)\
        _hr_ = fnc;\
        if(FAILED(_hr_)){\
            STRACE1(_T("Error:%08x in ") _T(#fnc),  _hr_);\
            _com_raise_error(_hr_);\
        }

#define OLE_WINERROR2HR(msg, erCode)\
        _hr_ = erCode;\
        STRACE1(_T("OSError:%d in ") msg,  _hr_);\
        _hr_ = HRESULT_FROM_WIN32(_hr_);

#define OLE_THROW_LASTERROR(msg)\
        OLE_WINERROR2HR(msg, ::GetLastError())\
        _com_raise_error(_hr_);

#define OLE_CHECK_NOTNULL(x)\
        if(!(x)){\
            STRACE1(_T("Null pointer:") _T(#x));\
            _com_raise_error(_hr_ = E_POINTER);\
        }

#define OLE_CHECK_NOTNULLSP(x)\
        if(!bool(x)){\
            STRACE1(_T("Null pointer:") _T(#x));\
            _com_raise_error(_hr_ = E_POINTER);\
        }

#define OLE_HRW32(fnc)\
        _hr_ = fnc;\
        if(ERROR_SUCCESS!=_hr_){\
            STRACE1(_T("OSError:%d in ") _T(#fnc),  _hr_);\
            _com_raise_error(_hr_ = HRESULT_FROM_WIN32(_hr_));\
        }

#define OLE_HRW32_BOOL(fnc)\
        if(!fnc){\
            OLE_THROW_LASTERROR(_T(#fnc))\
        }

#define OLE_CATCH\
        }catch(_com_error &e){\
            _hr_ = e.Error();\
            STRACE1(_T("COM Error:%08x %s"), _hr_, e.ErrorMessage());\
        }

#define OLE_CATCH_BAD_ALLOC\
        }catch(_com_error &e){\
            _hr_ = e.Error();\
            STRACE1(_T("COM Error:%08x %s"), _hr_, e.ErrorMessage());\
        }catch(std::bad_alloc&){\
            _hr_ = E_OUTOFMEMORY;\
            STRACE1(_T("Error: Out of Memory"));\
        }

#define OLE_CATCH_ALL\
        }catch(_com_error &e){\
            _hr_ = e.Error();\
            STRACE1(_T("COM Error:%08x %s"), _hr_, e.ErrorMessage());\
        }catch(...){\
            _hr_ = E_FAIL;\
            STRACE1(_T("Error: General Pritection Failor"));\
        }

#define OLE_RETURN_SUCCESS return SUCCEEDED(_hr_);
#define OLE_RETURN_HR      return _hr_;
#define OLE_HR             _hr_


#define E_JAVAEXCEPTION  MAKE_HRESULT(SEVERITY_ERROR, 0xDE, 1)

//reserved for future IE integration - extended COM diagnostic
#ifdef JNI_UTIL_H

// Now the platform encoding is Unicode (UTF-16), re-define JNU_ functions
// to proper JNI functions.
#ifdef UNICODE
    #define JNU_NewStringPlatform(env, x) env->NewString((jchar*) x, static_cast<jsize>(_tcslen(x)))
    #define _W2T(x) x
#else
    #define _W2T(x) _B(x)
#endif
//WM_MOUSEACTIVATE HTCLIENT MA_NOACTIVATEANDEAT 
class JStringBuffer
{
protected:
    LPWSTR m_pStr;
    jsize  m_dwSize;

public:
    JStringBuffer(jsize cbTCharCount)
    {
        m_dwSize = cbTCharCount;
        m_pStr = (LPWSTR)malloc( (m_dwSize+1)*sizeof(WCHAR) );
    }

    JStringBuffer(JNIEnv *env, jstring text)
    {
        m_dwSize = env->GetStringLength(text);
        m_pStr = (LPWSTR)malloc( (m_dwSize+1)*sizeof(WCHAR) );
        env->GetStringRegion(text, 0, m_dwSize, (jchar*) m_pStr);
        m_pStr[m_dwSize] = 0;
    }

    ~JStringBuffer(){
        free(m_pStr);
    }

    void Resize(jsize cbTCharCount){
        m_dwSize = cbTCharCount;
        m_pStr = (LPWSTR)realloc(m_pStr, (m_dwSize+1)*sizeof(TCHAR) );
    }
    operator LPWSTR() { return m_pStr; }
    void *GetData() { return (void *)m_pStr; }
    jsize  GetSize() { return m_dwSize; }
};


inline void ThrowJNIErrorOnOleError(JNIEnv *env, HRESULT hr, const char *msg)
{
    if(SUCCEEDED(hr))
        return;

    switch(hr){
    case E_NOINTERFACE:
    case E_POINTER:
        JNU_ThrowNullPointerException(env, msg);
        break;
    case TYPE_E_OUTOFBOUNDS:
        JNU_ThrowArrayIndexOutOfBoundsException(env, msg);
        break;
    case E_OUTOFMEMORY:
        JNU_ThrowOutOfMemoryError(env, msg);
        break;
    case E_INVALIDARG:
        JNU_ThrowIllegalArgumentException(env, msg);
        break;
    case E_ACCESSDENIED:
        JNU_ThrowIllegalAccessException(env, msg);
        break;
    case E_FAIL:
    case E_UNEXPECTED:
    case E_JAVAEXCEPTION:
        JNU_ThrowInternalError(env, msg);
        break;
    case TYPE_E_FIELDNOTFOUND:
        JNU_ThrowNoSuchFieldException(env, msg);
        break;
    default:{
        WORD fs = HRESULT_FACILITY(hr);
        WORD sc = SCODE_CODE(hr);
        if(FACILITY_SECURITY == fs) {
            JNU_ThrowIllegalAccessError(env, msg);
        } else if(
            FACILITY_WINDOWS == fs ||
            FACILITY_STORAGE == fs ||
            FACILITY_RPC == fs ||
            FACILITY_WIN32 == fs)
        {
            if(ERROR_ACCESS_DENIED == sc){
                JNU_ThrowIllegalAccessException(env, msg);
            } else {
                JNU_ThrowIOException(env, msg);
            }
        } else {
            JNU_ThrowInternalError(env, msg);
        }
        break;
    }}
}
#endif //JNI_UTIL_H

//reserved for future IE integration
#ifdef _AWT_OLE_EX_
inline BOOL IsEmptyValue(const _variant_t &vt)
{
        return
            VT_EMPTY == vt.vt
            || VT_NULL == vt.vt
            || (VT_BSTR == vt.vt &&  (NULL==vt.bstrVal || 0 == *vt.bstrVal) );
}

inline _bstr_t  _SV(VARIANT &vt)
{
        _variant_t _vt(vt, false);
        if(IsEmptyValue(_vt))
            return _T("");
        return _vt;
}

inline long _LV(VARIANT &vt)
{
        _variant_t _vt(vt, false);
        if(IsEmptyValue(_vt))
            return 0L;
        return _vt;
}

#define _B(x)    _bstr_t(x)
#define _V(x)    _variant_t(x)
#define _VV(vrt) _variant_t(vrt, false)
#define _VE      _variant_t()
#define _VB(b)   _variant_t(bool(b))
#define _VTrue   ZZ::g__Vtrue
#define _VFalse  ZZ::g__Vfalse
#define _O       ZZ::g__Vopt
#define _PO     &ZZ::g__Vopt
#define _O5      ZZ::g__Vopt,ZZ::g__Vopt,ZZ::g__Vopt,ZZ::g__Vopt,ZZ::g__Vopt
#define _O10     _O5,_O5
#define _O15     _O5,_O5,_O5,
#define _O20     _O5,_O5,_O5,_O5


#define IMPLEMENT_OLEDISPATCH_HELPERS()\
    _variant_t ZZ::g__Vopt(DISP_E_PARAMNOTFOUND, VT_ERROR), ZZ::g__Vtrue(true), ZZ::g__Vfalse(false);\
    IGlobalInterfaceTablePtr ZZ::g_spGIT;

namespace ZZ{
extern __declspec(selectany) _variant_t
    g__Vopt(DISP_E_PARAMNOTFOUND, VT_ERROR),
    g__Vtrue(true),
    g__Vfalse(false);
extern __declspec(selectany) IGlobalInterfaceTablePtr g_spGIT;
}



inline HRESULT CreateGlobalInterfaceTable()
{
  if( bool(ZZ::g_spGIT) ){
    return S_OK;
  }
  return ZZ::g_spGIT.CreateInstance(CLSID_StdGlobalInterfaceTable);
}

template <class T, const IID* piid = &__uuidof(T)>
class COLECrossMarshal{
public:
    DWORD m_dwCookie;

    COLECrossMarshal(T *pInterface)
    :m_dwCookie(0)
    {
        attach(pInterface);
    }

    COLECrossMarshal()
    :m_dwCookie(0)
    {}

    ~COLECrossMarshal(){
        clean();
    }

    COLECrossMarshal& operator=(T *pInterface) 
    {
        attach(pInterface);
        return *this;
    }

    HRESULT attach(T *pInterface)
    {
        OLE_TRY
        OLE_HRT(clean())
        if(NULL!=pInterface){
            OLE_NEXT_TRY
            OLE_HRT(CreateGlobalInterfaceTable())
            OLE_HRT(ZZ::g_spGIT->RegisterInterfaceInGlobal(
                (T *)pInterface,
                *piid,
                &m_dwCookie))
            OLE_CATCH
        }
        OLE_CATCH
        OLE_RETURN_HR
    }

    HRESULT clean()
    {
        if(0==m_dwCookie)
            return S_OK;
        OLE_TRY
        OLE_HRT(ZZ::g_spGIT->RevokeInterfaceFromGlobal(m_dwCookie))
        OLE_CATCH
        m_dwCookie = 0;
        OLE_RETURN_HR
    }

    T *GetThreadInstance()
    {
        if(0!=m_dwCookie){
            OLE_TRY
            T *pInterfaceLoc = NULL;
            OLE_HRT(ZZ::g_spGIT->GetInterfaceFromGlobal(
                m_dwCookie,
                *piid,
                (void **)&pInterfaceLoc))
            return pInterfaceLoc;
            OLE_CATCH
        }
        return NULL;
    }
};

#endif _AWT_OLE_EX_

class OLESyncro{
public:
    HANDLE m_hEvent;
    BOOL   m_bOwner;
    int    m_iTimeout;

    OLESyncro(HANDLE hEvent, int iTimeout = INFINITE)
    : m_hEvent(hEvent),
      m_bOwner(FALSE),
      m_iTimeout(iTimeout)
    {
        On();
    }

    ~OLESyncro(){
        Off();
    }

    void On(){
        if( !m_bOwner && 
            WAIT_TIMEOUT!=::WaitForSingleObject(m_hEvent,m_iTimeout) )
        {
            m_bOwner = TRUE;
        }    
    }

    void Off(){
        if( m_bOwner ){
            ::ReleaseMutex(m_hEvent);
            m_bOwner = FALSE;
        }
    }
};

class CStubStream: public IStream
{
public:
    //IUnknown
    virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
            IN REFIID riid,
            OUT void **ppvObject)
    {
        if( ::IsEqualIID(IID_IUnknown,riid) || ::IsEqualIID(IID_IStream,riid) ){
            *ppvObject = (void *)this;
            AddRef();
            return S_OK;
        }
        return E_NOINTERFACE;
    }
    virtual ULONG STDMETHODCALLTYPE AddRef(){
        return (ULONG)::InterlockedIncrement( (LPLONG)&m_cRef );
    }
    virtual ULONG STDMETHODCALLTYPE Release(){
        ULONG cRef = (ULONG)::InterlockedDecrement( (LPLONG)&m_cRef );
        if(cRef == 0)
            delete this;
        return cRef;
    }

public:
    //ISequentialStream
    virtual  HRESULT STDMETHODCALLTYPE Read(
        OUT void *pv,
        IN  ULONG cb,
        OUT ULONG *pcbRead)
    {
       return E_NOTIMPL;
    }

    virtual  HRESULT STDMETHODCALLTYPE Write(
        IN const void *pv,
        IN ULONG cb,
        OUT ULONG *pcbWritten)
    {
       return E_NOTIMPL;
    }


    //IStream
    virtual  HRESULT STDMETHODCALLTYPE Seek(
        IN LARGE_INTEGER dlibMove,
        IN DWORD dwOrigin,
        OUT ULARGE_INTEGER *plibNewPosition)
    {
        return S_OK;
    }

    virtual HRESULT STDMETHODCALLTYPE SetSize(
        IN ULARGE_INTEGER libNewSize)
    {
        return E_NOTIMPL;
    }

    virtual  HRESULT STDMETHODCALLTYPE CopyTo(
        IN  IStream* pstm,
        IN  ULARGE_INTEGER cb,
        OUT ULARGE_INTEGER* pcbRead,
        OUT ULARGE_INTEGER* pcbWritten)
    {
        return E_NOTIMPL;
    }

    virtual HRESULT STDMETHODCALLTYPE Commit(IN DWORD grfCommitFlags)
    {
        return E_NOTIMPL;
    }
    virtual HRESULT STDMETHODCALLTYPE Revert()
    {
        return E_NOTIMPL;
    }

    virtual HRESULT STDMETHODCALLTYPE LockRegion(
        IN ULARGE_INTEGER libOffset, 
        IN ULARGE_INTEGER cb, 
        IN DWORD dwLockType)
    {
        return E_NOTIMPL;
    }

    virtual HRESULT STDMETHODCALLTYPE UnlockRegion(
        IN ULARGE_INTEGER libOffset, 
        IN ULARGE_INTEGER cb, 
        IN DWORD dwLockType)
    {
        return E_NOTIMPL;
    }


    virtual HRESULT STDMETHODCALLTYPE Stat(
        OUT STATSTG *pstatstg, 
        IN DWORD grfStatFlag)  
    {
        return E_NOTIMPL;
    }

    virtual HRESULT STDMETHODCALLTYPE Clone(OUT IStream**)
    {
        return E_NOTIMPL;
    }

    CStubStream()
    : m_cRef(1)
    {}

    virtual ~CStubStream(){};
protected:
    ULONG  m_cRef;
};

struct COLEHolder
{
    COLEHolder()
    : m_hr(::OleInitialize(NULL))
    {}

    ~COLEHolder(){
        if(SUCCEEDED(m_hr))
            ::OleUninitialize();
    }
    HRESULT m_hr;
};




#endif//AWT_OLE_H