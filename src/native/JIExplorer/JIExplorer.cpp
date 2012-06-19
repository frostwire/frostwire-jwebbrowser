/*
 * Created by Alden Torres (aldenml)
 * Copyright (c) 2011, 2012, FrostWire(TM). All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "stdafx.h" 
#include "JIExplorer.h"
#include "BrMain.h"
#include "BrHolderThread.h"
#include "windowsx.h"


//struct __declspec(uuid("BB1A2AE1-A4F9-11CF-8F20-00805F2CD064")) IActiveScript;
_COM_SMARTPTR_TYPEDEF(IActiveScript, __uuidof(IActiveScript));

//struct __declspec(uuid("BB1A2AE2-A4F9-11CF-8F20-00805F2CD064")) IActiveScriptParse;
_COM_SMARTPTR_TYPEDEF(IActiveScriptParse, __uuidof(IActiveScriptParse));


#if defined(__IHTMLDocument2_INTERFACE_DEFINED__)
_COM_SMARTPTR_TYPEDEF(IHTMLDocument2, __uuidof(IHTMLDocument2));
#endif// #if defined(__IHTMLDocument2_INTERFACE_DEFINED__)
#if defined(__IHTMLDocument3_INTERFACE_DEFINED__)
_COM_SMARTPTR_TYPEDEF(IHTMLDocument3, __uuidof(IHTMLDocument3));
#endif// #if defined(__IHTMLDocument3_INTERFACE_DEFINED__)
#if defined(__IDisplayServices_INTERFACE_DEFINED__)
_COM_SMARTPTR_TYPEDEF(IDisplayServices, __uuidof(IDisplayServices));
#endif// #if defined(__IDisplayServices_INTERFACE_DEFINED__)
#if defined(__IHTMLCaret_INTERFACE_DEFINED__)
_COM_SMARTPTR_TYPEDEF(IHTMLCaret, __uuidof(IHTMLCaret));
#endif// #if defined(__IHTMLCaret_INTERFACE_DEFINED__)
#if defined(__IHTMLWindow2_INTERFACE_DEFINED__)
_COM_SMARTPTR_TYPEDEF(IHTMLWindow2, __uuidof(IHTMLWindow2));
#endif// #if defined(__IHTMLWindow2_INTERFACE_DEFINED__)
#if defined(__IHTMLElement_INTERFACE_DEFINED__)
_COM_SMARTPTR_TYPEDEF(IHTMLElement, __uuidof(IHTMLElement));
#endif// #if defined(__IHTMLElement_INTERFACE_DEFINED__)
/*
#if defined(__IActiveScript_INTERFACE_DEFINED__)
_COM_SMARTPTR_TYPEDEF(IActiveScript, __uuidof(IActiveScript));
#endif// #if defined(__IActiveScript_INTERFACE_DEFINED__)
#if defined(__IActiveScriptError_INTERFACE_DEFINED__)
_COM_SMARTPTR_TYPEDEF(IActiveScriptError, __uuidof(IActiveScriptError));
#endif// #if defined(__IActiveScriptError_INTERFACE_DEFINED__)
*/


#if defined(__IActiveScriptParseProcedure_INTERFACE_DEFINED__)
_COM_SMARTPTR_TYPEDEF(IActiveScriptParseProcedure, __uuidof(IActiveScriptParseProcedure));
#endif// #if defined(__IActiveScriptParseProcedure_INTERFACE_DEFINED__)


jclass    JIExplorer::ms_IExplorerComponent = NULL;

jfieldID  JIExplorer::ms_IExplorerComponent_x = NULL;
jfieldID  JIExplorer::ms_IExplorerComponent_y = NULL;
jfieldID  JIExplorer::ms_IExplorerComponent_width = NULL;
jfieldID  JIExplorer::ms_IExplorerComponent_height = NULL;

jfieldID  JIExplorer::ms_IExplorerComponent_data = NULL;

jmethodID JIExplorer::ms_IExplorerComponent_postEvent = NULL;
jmethodID JIExplorer::ms_IExplorerComponent_callJava = NULL;

void JIExplorer::initIDs(JNIEnv *env, jclass clazz)
{
    ms_IExplorerComponent = getGlobalJavaClazz(
        env,
        "com/frostwire/gui/webbrowser/IExplorerComponent"
    );

    ms_IExplorerComponent_x = env->GetFieldID(ms_IExplorerComponent, "x", "I");
    ms_IExplorerComponent_y = env->GetFieldID(ms_IExplorerComponent, "y", "I");
    ms_IExplorerComponent_width = env->GetFieldID(ms_IExplorerComponent, "width", "I");
    ms_IExplorerComponent_height = env->GetFieldID(ms_IExplorerComponent, "height", "I");
    
	ms_IExplorerComponent_data = env->GetFieldID(clazz, "data", "J");

	ms_IExplorerComponent_postEvent = env->GetMethodID(
        clazz, 
        "postEvent", 
        "(ILjava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
    ms_IExplorerComponent_callJava = env->GetMethodID(
        clazz, 
        "callJava", 
        "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
}


/************************************************************************
 * IExplorer methods
 */

JIExplorer::JIExplorer(
    JNIEnv *env, 
    jobject othis
)
:m_synthetic(false),
 m_bBlockNativeInputHandler(false),
 m_this(makeGlobal(env, othis)),
 m_hChildArea(CreateRectRgn(0,0,0,0)),
 m_pThread(BrowserThread::GetInstance())
{
}

HRESULT JIExplorer::getTargetRect(
    JNIEnv *env, 
    LPRECT prc)
{
    if(NULL!=m_this){
        prc->left = env->GetIntField(m_this, ms_IExplorerComponent_x);
        prc->top = env->GetIntField(m_this, ms_IExplorerComponent_y);
        prc->right = prc->left + env->GetIntField(m_this, ms_IExplorerComponent_width);
        prc->bottom = prc->top + env->GetIntField(m_this, ms_IExplorerComponent_height);
        return S_OK;
    }
    return E_INVALIDARG;
}

HRESULT JIExplorer::create(    
    JNIEnv *env, 
    HWND    hParent,
    int     ePaintAlgorithm)
{
    SEP(_T("create"))
    m_ePaintAlgorithm = ePaintAlgorithm;
    STRACE(_T("paint: %04x"), ePaintAlgorithm);
    OLE_TRY
    RECT rcIE = {0};
    SetWindowLong(
        hParent, 
        GWL_STYLE, 
        GetWindowLong(hParent, GWL_STYLE) & ~(WS_CLIPCHILDREN | WS_CLIPSIBLINGS) );

    OLE_HRT( getTargetRect(
        env, 
        &rcIE))
    OLE_HRT( CreateControl(
        hParent,
        &rcIE,
        NULL))

    HWND hFO = GetFocus();
    if( GetTopWnd()==hFO || GetIEWnd()==hFO ){
        SetFocus(GetParent());
    }
    
	OLE_CATCH
    OLE_RETURN_HR
}

JIExplorer::~JIExplorer()
{
    SEP0(_T("~JIExplorer"));
    if(!bool(m_spIWebBrowser2) || NULL!=m_this){
        STRACE1(_T("alarm!"));
    }
    if(m_pThread){
        m_pThread->Release();
    }
}

void JIExplorer::destroy(JNIEnv *env)
{
    if(m_spIWebBrowser2){
        OLE_TRY
        OLE_HRT( DestroyControl() )
        OLE_CATCH
    }
    releaseGlobal(env, m_this);
    if(m_hChildArea){
        DeleteObject((HGDIOBJ)m_hChildArea);
        m_hChildArea = NULL;
    }
    m_this = NULL;
}

void JIExplorer::RedrawParentRect(LPRECT pRect)
{
}

HRESULT JIExplorer::SendIEEvent(
    int iId,
    LPTSTR lpName, 
    LPTSTR lpValue,
    _bstr_t &bsResult)
{
	OLE_DECL
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    if(NULL==env){
        OLE_HR = E_FAIL;
    } else {   
        jstring jsName = JNU_NewStringPlatform(env, lpName);
        if(NULL==jsName){
            OLE_HR = E_OUTOFMEMORY;
        } else {
            jstring jsValue = JNU_NewStringPlatform(env, lpValue);
            if(NULL==jsValue){
                OLE_HR = E_OUTOFMEMORY;
            } else {
                jstring jsRes = (jstring)env->CallObjectMethod(
                    m_this, 
                    ms_IExplorerComponent_postEvent,
                    iId, 
                    jsName, 
                    jsValue);
                if( NULL!=jsRes ){
                    bsResult = JStringBuffer(env, jsRes);
                    env->DeleteLocalRef(jsRes);
                }
                env->DeleteLocalRef(jsValue);
            }
            env->DeleteLocalRef(jsName);
        }
    }
    OLE_RETURN_HR
}

HRESULT STDMETHODCALLTYPE JIExplorer::GetIDsOfNames(REFIID riid,
	LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId)
{
	HRESULT hr = S_OK;
	
	for (UINT i = 0; i < cNames; i++) {
		if (wmemcmp(rgszNames[i], _T("callJava"), 8) == 0) {
			rgDispId[i] = DISPID_JBROWSER_CALLJAVA;
		} else {
			rgDispId[i] = DISPID_UNKNOWN;
			hr = DISP_E_UNKNOWNNAME;
		}
	}
	
	return hr;
}

HRESULT STDMETHODCALLTYPE JIExplorer::Invoke(
        DISPID dispIdMember,
        REFIID riid,
        LCID lcid,
        WORD wFlags,
        DISPPARAMS  *pDispParams,
        VARIANT  *pVarResult,
        EXCEPINFO  *pExcepInfo,
        UINT  *puArgErr
)
{
	OLE_TRY
    BOOL bNotify = TRUE;
    switch(dispIdMember){
    
	case DISPID_JBROWSER_CALLJAVA:
		CallJava(pDispParams, pVarResult);
		return S_OK;
    default:
        OLE_HR = CBrIELWControl::Invoke(
         dispIdMember,
         riid,
         lcid,
         wFlags,
         pDispParams,
         pVarResult,
         pExcepInfo,
         puArgErr);
        break;
    }
    
    OLE_CATCH
    OLE_RETURN_HR
}

void JIExplorer::NavigateComplete()
{
	this->AddCustomObject(this, L"jbrowser");
}

void JIExplorer::AddCustomObject(IDispatch *custObj, BSTR name)
{
	IHTMLDocument2 *doc = GetDoc();

	if (doc == NULL) {
		return;
	}

	IHTMLWindow2 *win = NULL;
	doc->get_parentWindow(&win);
	doc->Release();

	if (win == NULL) {
		return;
	}

	IDispatchEx *winEx;
	win->QueryInterface(&winEx);
	win->Release();

	if (winEx == NULL) {
		return;
	}

	DISPID dispid; 
	HRESULT hr = winEx->GetDispID(name, fdexNameEnsure, &dispid);

	if (FAILED(hr)) {
		return;
	}

	DISPID namedArgs[] = {DISPID_PROPERTYPUT};
	DISPPARAMS params;
	params.rgvarg = new VARIANT[1];
	params.rgvarg[0].pdispVal = custObj;
	params.rgvarg[0].vt = VT_DISPATCH;
	params.rgdispidNamedArgs = namedArgs;
	params.cArgs = 1;
	params.cNamedArgs = 1;

	hr = winEx->InvokeEx(dispid, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUT, &params, NULL, NULL, NULL); 
	winEx->Release();

	if (FAILED(hr)) {
		return;
	}

	STRACE0(_T("Registered jbrowser object"));
}

void JIExplorer::CallJava(DISPPARAMS* pDispParams, VARIANT* pVarResult)
{
	if (pDispParams->cArgs == 2)
	{
		BSTR bstrArgName = pDispParams->rgvarg[1].bstrVal;
		BSTR bstrArgData = pDispParams->rgvarg[0].bstrVal;

		JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
		if (NULL != env)
		{
			jstring jsName = JNU_NewStringPlatform(env, bstrArgName);
			jstring jsData = JNU_NewStringPlatform(env, bstrArgData);
            
			jstring jsRes = (jstring)env->CallObjectMethod(
                    m_this, 
                    ms_IExplorerComponent_callJava,
                    jsName, 
                    jsData);
			
			if (NULL != jsRes)
			{
				pVarResult->vt = VT_BSTR;
				pVarResult->bstrVal = SysAllocString(JStringBuffer(env, jsRes));
				env->DeleteLocalRef(jsRes);
			}

            env->DeleteLocalRef(jsName);
			env->DeleteLocalRef(jsData);
        }
	}
}

LRESULT JIExplorer::NewIEProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes = 0;

	LONG_PTR pHook = NULL;
    if(WM_NCDESTROY==msg){
        RemoveHook(hWnd);
    } else if(
        (msg >= WM_KEYFIRST 
        && msg <= WM_KEYLAST)
    ){
        MSG _msg = { hWnd, msg, wParam, lParam, 0, { 0, 0 } };
        LPMSG lpMsg = &_msg;
        IOleInPlaceActiveObjectPtr spInPlaceActiveObject(m_spIWebBrowser2);
        if(spInPlaceActiveObject){
            OLE_DECL;
            OLE_HR = spInPlaceActiveObject->TranslateAccelerator(lpMsg);
            if(WM_KEYDOWN == msg && VK_TAB==wParam && !IsCtrlKeyDown() && !IsAltKeyDown()){
                STRACE(_T("WM_KEYDOWN tab %08x %08x"), lParam, OLE_HR);
                if(S_FALSE==OLE_HR){
                    SendIEEvent(
                        -2,
                        _T("FocusMove"),
                        IsShiftKeyDown() ? _T("false") : _T("true")
                    );
                }    
            }    
            if(OLE_HR == S_OK){
                return 0;
            }
        }
        pHook = ::SetWindowLongPtr(hWnd, GWLP_WNDPROC, GetOldIEWndProc());        
    }

	switch(msg){
    case WM_SETFOCUS:
        SendIEEvent(-1, _T("OnFocusMove"), _T("true"));
        STRACE0(_T("WM_SETFOCUS"));
        break;
    case WM_KILLFOCUS:
        SendIEEvent(-1, _T("OnFocusMove"), _T("false"));
        STRACE0(_T("WM_KILLFOCUS"));
        break;
    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_CHAR:
    case WM_SYSCHAR:

    case WM_SETCURSOR:

    case WM_MOUSEACTIVATE:

    case WM_MOUSEMOVE:

    case WM_NCLBUTTONDBLCLK:
    case WM_NCLBUTTONDOWN:
    case WM_NCLBUTTONUP:

    case WM_NCRBUTTONDBLCLK:
    case WM_NCRBUTTONDOWN:
    case WM_NCRBUTTONUP:

    case WM_NCMBUTTONDBLCLK:
    case WM_NCMBUTTONDOWN:
    case WM_NCMBUTTONUP:

    case WM_LBUTTONDBLCLK:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:

    case WM_RBUTTONDBLCLK:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:

    case WM_MBUTTONDBLCLK: 
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP: 

    case WM_MOUSEWHEEL:

    default:
        break;
    }

	lRes = ::CallWindowProc((WNDPROC)GetOldIEWndProc(), hWnd, msg, wParam, lParam);      
    if(pHook && m_hwndIE==hWnd) //window can be disconnected ;)
        ::SetWindowLongPtr(hWnd, GWLP_WNDPROC, pHook);

    return lRes;
}

struct JavaInputStream : public CStubStream
{
    static jclass ms_jcidInputStream;
    static jmethodID ms_jcidInputStream_readBytes; 

    static void initIDs(
        IN JNIEnv *env)
    {
        if(NULL==ms_jcidInputStream){
            ms_jcidInputStream = getGlobalJavaClazz(
                env,
                "java/io/InputStream"
            );
            ms_jcidInputStream_readBytes = env->GetMethodID(ms_jcidInputStream, "read", "([BII)I");
        }
    }

    JavaInputStream(
        IN JNIEnv *env,
        IN jobject jis
    ): CStubStream(),
       m_jis(jis),
       m_env(env)
    {
        initIDs(env);
    }

    virtual  HRESULT STDMETHODCALLTYPE Read(
        OUT void *pv,
        IN  ULONG cb,
        OUT ULONG *pcbRead)
    {
        OLE_DECL
        //java read
        //No more exceptions here!
        jbyteArray jba = m_env->NewByteArray( jsize(cb) );
        if( NULL == jba ){
            OLE_HR = E_OUTOFMEMORY;
        } else {
            jint ret = 0, read;
            while(0 < cb){
                read = m_env->CallIntMethod(
                    m_jis, 
                    ms_jcidInputStream_readBytes,
                    jba,
                    ret,
                    cb);
                if( m_env->ExceptionCheck() ){
                    OLE_HR = E_JAVAEXCEPTION;
                    break;
                }
                if( -1 == read ){
                    break;
                }
                cb -= read;
                ret += read;
            }

            //copy to stream
            if(SUCCEEDED(OLE_HR)){
                jbyte *pSrc = m_env->GetByteArrayElements(jba, NULL);
                if(NULL==pSrc){
                    OLE_HR = E_OUTOFMEMORY;
                } else {
                    memcpy(pv, pSrc, ret);
                    if(pcbRead){
                        *pcbRead = ret;
                    }
                    m_env->ReleaseByteArrayElements(jba, pSrc, JNI_ABORT);
                }
            }
            m_env->DeleteLocalRef(jba);
        }  
        return S_OK;
    }

    jobject m_jis;
    JNIEnv *m_env; 
};

jclass JavaInputStream::ms_jcidInputStream = NULL;
jmethodID JavaInputStream::ms_jcidInputStream_readBytes = NULL;

HRESULT JIExplorer::Connect(
    IN BSTR bsURL, 
    IN JNIEnv *env, 
    IN jobject jis)
{
    OLE_DECL
    if(NULL!=jis){
        OLE_NEXT_TRY
        IHTMLDocument2Ptr doc;     
        OLE_HRT( m_spIWebBrowser2->get_Document((LPDISPATCH *)&doc) )
        OLE_CHECK_NOTNULLSP(doc)
        IPersistStreamInitPtr ipsi(doc);
        JavaInputStream is(env, jis);
	OLE_HRT( ipsi->InitNew() )
	OLE_HRT( ipsi->Load(&is) )
        OLE_CATCH
    }else{
        OLE_HR = CBrIELWControl::Connect(bsURL);
    }
    OLE_RETURN_HR
}

void JIExplorer::Go(IN BSTR bsURL)
{
	OLE_TRY
	IWebBrowser2Ptr br(m_spIWebBrowser2);
	OLE_CHECK_NOTNULLSP(br)
	VARIANT v;
	v.vt = VT_I4;
	v.lVal = 0; // v.lVal = navNoHistory;
	OLE_HRT(br->Navigate(bsURL, &v, _PO, _PO, _PO));
	OLE_CATCH
}

void JIExplorer::GoBack()
{
	OLE_TRY
	IWebBrowser2Ptr br(m_spIWebBrowser2);
	OLE_CHECK_NOTNULLSP(br)
	OLE_HRT(br->GoBack());
	OLE_CATCH
}

void JIExplorer::GoForward()
{
	OLE_TRY
	IWebBrowser2Ptr br(m_spIWebBrowser2);
	OLE_CHECK_NOTNULLSP(br)
	OLE_HRT(br->GoForward());
	OLE_CATCH
}

void JIExplorer::Refresh(BOOL bClearCache)
{
	OLE_TRY
	IWebBrowser2Ptr br(m_spIWebBrowser2);
	OLE_CHECK_NOTNULLSP(br)
	if (bClearCache) {
		VARIANT v;
		v.vt = VT_I4;
		v.lVal = REFRESH_COMPLETELY;
		OLE_HRT(br->Refresh2(&v));
	} else {
		OLE_HRT(br->Refresh());
	}
	OLE_CATCH
}

void JIExplorer::Stop()
{
	OLE_TRY
	IWebBrowser2Ptr br(m_spIWebBrowser2);
	OLE_CHECK_NOTNULLSP(br)
	OLE_HRT(br->Stop());
	OLE_CATCH
}

void JIExplorer::RunJS(IN BSTR bsCode)
{
	OLE_TRY
	IWebBrowser2Ptr br(m_spIWebBrowser2);
	OLE_CHECK_NOTNULLSP(br)
	IHTMLDocument2 *doc = GetDoc();
	if (doc != NULL) {
		IHTMLWindow2 *win = NULL;
		doc->get_parentWindow(&win);
		
		if (win != NULL) {
			_variant_t vtResult;
			OLE_HRT( win->execScript(bsCode, _B(""), &vtResult ));

			vtResult.Clear();
			win->Release();
		}

		doc->Release();
	}
	OLE_CATCH
}

/************************************************************************
 * IExplorerComponent native methods
 */

extern "C" {

/*
 * Class:     com_frostwire_gui_webbrowser_IExplorerComponent
 * Method:    initIDs
  */
JNIEXPORT void JNICALL Java_com_frostwire_gui_webbrowser_IExplorerComponent_initIDs(
    JNIEnv *env, 
    jclass cls)
{
    JIExplorer::initIDs(env, cls);
}

/*
 * Class:     com_frostwire_gui_webbrowser_IExplorerComponent
 * Method:    create
  */
struct CreateAction : public BrowserAction
{
    HWND    m_parent;
    JIExplorer *m_pThis;

    CreateAction(
        JIExplorer *pThis,
        HWND parent)
    : m_pThis(pThis),
      m_parent(parent)
    {}

    virtual HRESULT Do(JNIEnv *env){
        return m_pThis->create(env, m_parent, 0x0004);
    }
};

JNIEXPORT jlong JNICALL Java_com_frostwire_gui_webbrowser_IExplorerComponent_create(
    JNIEnv *env, 
    jobject self,
    jlong parent)
{
	JIExplorer *pThis = new JIExplorer(env, self);
	if(pThis){
        OLE_TRY
        OLE_HRT(pThis->GetThread()->MakeAction(
            env,
            "Browser create error",
            CreateAction(
                pThis,
                (HWND)parent)));
        OLE_CATCH
        if(FAILED(OLE_HR)){
            pThis->destroy(env);
            delete pThis;
            pThis = NULL;
        }
    }
    return jlong(pThis);
}

/*
 * Class:     com_frostwire_gui_webbrowser_IExplorerComponent
 * Method:    destroy
  */
struct DestroyAction : public BrowserAction
{
    JIExplorer *m_pThis;
    DestroyAction(JIExplorer *pThis)
    : m_pThis(pThis)
    {}

    virtual HRESULT Do(JNIEnv *env){
        m_pThis->destroy(env);
        return S_OK;
    }
};
JNIEXPORT void JNICALL Java_com_frostwire_gui_webbrowser_IExplorerComponent_destroy(
    JNIEnv *env, 
    jobject self)
{
    JIExplorer *pThis = (JIExplorer *)env->GetLongField(self, JIExplorer::ms_IExplorerComponent_data);
    if(pThis){
        pThis->GetThread()->MakeAction(
            env,
            "Browser destroy error",
            DestroyAction(pThis));
        delete pThis;
        env->SetLongField(self, JIExplorer::ms_IExplorerComponent_data, 0L);
    }
}

/*
 * Class:     com_frostwire_gui_webbrowser_IExplorerComponent
 * Method:    execJS
 */
struct ExecJSAction : public BrowserAction
{
    JIExplorer *m_pThis;
    JStringBuffer m_jstCode;
    _bstr_t m_bsResult;

    ExecJSAction(
        JIExplorer *pThis,
        JNIEnv *env,
        jstring jsCode
    ): m_pThis(pThis),
       m_jstCode(env, jsCode),
       m_bsResult(_T("error:Null interface"))
    {}

    virtual HRESULT Do(JNIEnv *env)
    {
        SEP(_T("_ExecJS"))
        LPTSTR pCode = (LPTSTR)m_jstCode;
        STRACE(_T("code:%s"), pCode);
        if(NULL==pCode){
            return S_FALSE;
        } else if( 0==_tcsncmp(pCode, _T("##"), 2) ){
            if( 0==_tcscmp(pCode, _T("##stop")) ){
                OLE_TRY
                OLE_HRT( m_pThis->m_spIWebBrowser2->Stop() );
                OLE_CATCH
                OLE_RETURN_HR
            } else if( 0==_tcsncmp(pCode, _T("##setFocus"), 10) ){
                OLE_TRY
                STRACE0(_T("##setFocus"));
                IOleObjectPtr spOleObject(m_pThis->m_spIWebBrowser2);
                OLE_CHECK_NOTNULLSP(spOleObject)
                OLE_HRT( spOleObject->DoVerb(
                    OLEIVERB_UIACTIVATE, 
                    NULL, 
                    m_pThis, 
                    0, 
                    NULL, 
                    NULL))
                if( 0!=_tcscmp(pCode, _T("##setFocus(false)")) ){
                    HWND hwnd = m_pThis->GetIEWnd();
                    if(NULL==hwnd)
                        hwnd = m_pThis->GetTopWnd();
                    SendMessage(hwnd, WM_KEYDOWN, VK_TAB, 0x0000000);
                    SendMessage(hwnd, WM_KEYUP, VK_TAB, 0xc000000);
                }
                //::OLE_CoPump();//Flash hole. Dead circle if is.
                OLE_CATCH
                OLE_RETURN_HR
            } else if( 0==_tcsncmp(pCode, _T("##setNativeDraw("), 16) ) {
                m_pThis->m_ePaintAlgorithm = pCode[17] - _T('0'); 
                //STRACE(_T(">>>>>>>>NativeDraw %d"), m_pThis->m_ePaintAlgorithm);
                return S_OK;
            } else if( 0==_tcsncmp(pCode, _T("##showCaret"), 11) ) {
                OLE_TRY
                IWebBrowser2Ptr br(m_pThis->m_spIWebBrowser2);
                OLE_CHECK_NOTNULLSP(br)

                IHTMLDocument2Ptr doc;     
                OLE_HRT( br->get_Document((LPDISPATCH *)&doc) )
                OLE_CHECK_NOTNULLSP(doc)
                
                IDisplayServicesPtr ds(doc);                     
                OLE_CHECK_NOTNULLSP(ds)

                IHTMLCaretPtr cr;                     
                OLE_HRT( ds->GetCaret(&cr) )
                OLE_CHECK_NOTNULLSP(cr)

                if( 0==_tcscmp(pCode, _T("##showCaret(true)")) ){
                   OLE_HRT( cr->Show(FALSE) )
                   STRACE1(_T("{Show------"));
                } else {
                   OLE_HRT( cr->Hide() )
                   STRACE1(_T("}Hide------"));
                }
                OLE_CATCH
                OLE_RETURN_HR
            }
            return S_FALSE;
        }
        OLE_TRY
        IWebBrowser2Ptr br(m_pThis->m_spIWebBrowser2);
        OLE_CHECK_NOTNULLSP(br)

        //that can be any type of document
        //Acrobat Reader for example
        IDispatchPtr docO;     
        OLE_HRT( br->get_Document(&docO) )

        //we are trying customize it to HTML document
        //empty document is a valid argument
        IHTMLDocument2Ptr doc(docO);     
        OLE_CHECK_NOTNULLSP(doc)

        IHTMLWindow2Ptr wnd;
        OLE_HRT( doc->get_parentWindow(&wnd) )
        OLE_CHECK_NOTNULLSP(wnd)

        _variant_t vtResult;
        STRACE0(_T("makeScript"));
        _bstr_t bsEval( ('#'!=*pCode && ':'!=*pCode)
            ? (_B(L"document.documentElement.setAttribute(\'javaEval\', eval(\'") + pCode + L"\'))")
            : _B(pCode+1)
        );
        
        STRACE0(_T("execScript"));
        OLE_HRT( wnd->execScript( bsEval, _B(""), &vtResult) )
        vtResult.Clear();

        if( ':'!=*pCode ){
            IHTMLDocument3Ptr doc3(doc);     
            OLE_CHECK_NOTNULLSP(doc3)
            
            IHTMLElementPtr el;
            OLE_HRT( doc3->get_documentElement(&el))
            OLE_CHECK_NOTNULLSP(el)

            OLE_HRT( el->getAttribute(_B("javaEval"), 0, &vtResult) )
        }

        if(VT_NULL!=vtResult.vt){
            m_bsResult = vtResult;
        } else {
            m_bsResult = "";
        }
        STRACE0(_T("result:%s"), (LPCTSTR)m_bsResult);
        OLE_CATCH
        if(FAILED(OLE_HR)){
            m_bsResult = _T("error:");
            m_bsResult += _B(_V(OLE_HR));
            m_bsResult += _T("code:");
            m_bsResult += pCode;
        }
        OLE_RETURN_HR
    }
};

JNIEXPORT jstring JNICALL Java_com_frostwire_gui_webbrowser_IExplorerComponent_execJS(
    JNIEnv *env, 
    jobject self,
    jstring jsCode)
{
    JIExplorer *pThis = (JIExplorer *)env->GetLongField(self, JIExplorer::ms_IExplorerComponent_data);
    if(pThis){
        OLE_TRY
        ExecJSAction a(
            pThis,
            env,
            jsCode);
        OLE_HRT( pThis->GetThread()->MakeAction(
            env,
            "Browser JScript execution error",
            a));
        LPCTSTR lpRes = a.m_bsResult;
        return JNU_NewStringPlatform(env, (LPCTSTR) (lpRes ? lpRes : _T("")));
        OLE_CATCH
    }
    return 0;
}

/*
 * Class:     com_frostwire_gui_webbrowser_IExplorerComponent
 * Method:    setURL
 * Signature: (Ljava/lang/String;)V
 */
struct SetURLAction : public BrowserAction{
    JStringBuffer m_jstURL;
    JIExplorer *m_pThis;
    jobject m_jisURL;

    SetURLAction(
        JIExplorer *pThis,
        JNIEnv *env,
        jstring jURL,
        jobject jisURL
    ):m_pThis(pThis),
      m_jstURL(env, jURL),
      m_jisURL(makeGlobal(env, jisURL))
    {}
    virtual HRESULT Do(JNIEnv *env)
    {
        OLE_TRY
        OLE_HRT( m_pThis->Connect(_B(m_jstURL), env, m_jisURL) )
        OLE_CATCH_ALL
        if(!repeatOnError(OLE_HR)){
            releaseGlobal(env, m_jisURL);
        }
        OLE_RETURN_HR
    }
};

JNIEXPORT void JNICALL Java_com_frostwire_gui_webbrowser_IExplorerComponent_setURL(
    JNIEnv *env, 
    jobject self,
    jstring jsURL,
    jobject jisURL)
{
    JIExplorer *pThis = (JIExplorer *)env->GetLongField(self, JIExplorer::ms_IExplorerComponent_data);
    if(pThis){
        pThis->GetThread()->MakeAction(
            env,
            "URL navigation error",
            SetURLAction(
                pThis,
                env,
                jsURL,
                jisURL));
    }
}


/*
 * Class:     com_frostwire_gui_webbrowser_IExplorerComponent
 * Method:    nativeSetVisible
 */
struct ShowAction : public BrowserAction
{
    JIExplorer *m_pThis;
    BOOL bShow;

    ShowAction(
        JIExplorer *pThis,
        BOOL _bShow
    ):m_pThis(pThis),
      bShow(_bShow)
    {}
    virtual HRESULT Do(JNIEnv *env)
    {
        ::ShowWindow(m_pThis->GetTopWnd(), bShow ? SW_SHOW : SW_HIDE );        
        return S_OK; 
    }
};

JNIEXPORT void JNICALL Java_com_frostwire_gui_webbrowser_IExplorerComponent_nativeSetVisible(
    JNIEnv *env, 
    jobject self,
    jboolean aFlag)
{
    JIExplorer *pThis = (JIExplorer *)env->GetLongField(self, JIExplorer::ms_IExplorerComponent_data);
    if(pThis){
        pThis->GetThread()->MakeAction(
            env,
            "ShowWindow error",
            ShowAction(
                pThis,
                aFlag));
    }
}

/*
 * Class:     com_frostwire_gui_webbrowser_IExplorerComponent
 * Method:    nativeSetEnabled
 */
struct EnableAction : public BrowserAction
{
    JIExplorer *m_pThis;
    BOOL bEnable;

    EnableAction(
        JIExplorer *pThis,
        BOOL _bEnable
    ):m_pThis(pThis),
      bEnable(_bEnable)
    {}
    virtual HRESULT Do(JNIEnv *env)
    {
        ::EnableWindow(m_pThis->GetTopWnd(), bEnable);
        return S_OK; 
    }
};

JNIEXPORT void JNICALL Java_com_frostwire_gui_webbrowser_IExplorerComponent_nativeSetEnabled(
    JNIEnv *env, 
    jobject self,
    jboolean enabled)
{
    JIExplorer *pThis = (JIExplorer *)env->GetLongField(self, JIExplorer::ms_IExplorerComponent_data);
    if(pThis){
        pThis->GetThread()->MakeAction(
            env,
            "EnableWindow error",
            EnableAction(
                pThis,
                enabled));
    }
}

/*
 * Class:     com_frostwire_gui_webbrowser_IExplorerComponent
 * Method:    nativeSetBounds
 */
JNIEXPORT void JNICALL Java_com_frostwire_gui_webbrowser_IExplorerComponent_nativeSetBounds(
	JNIEnv *env, 
	jobject self)
{
	JIExplorer *pThis = (JIExplorer *)env->GetLongField(self, JIExplorer::ms_IExplorerComponent_data);
	if(pThis ){
		RECT rc;
		::GetWindowRect(pThis->GetParent(),&rc);
		HWND hwndChild = GetWindow(pThis->GetParent(), GW_CHILD);
		::SetWindowPos(hwndChild,NULL,0,0,rc.right-rc.left,rc.bottom-rc.top,SWP_NOZORDER|SWP_NOACTIVATE|SWP_SHOWWINDOW|SWP_NOMOVE | SWP_NOCOPYBITS);
		::InvalidateRect(hwndChild, &rc, FALSE);
		::UpdateWindow(hwndChild);
	}
}

/*
 * Class:     com_frostwire_gui_webbrowser_IExplorerComponent
 * Method:    nativeRefresh
 */
struct RefreshAction : public BrowserAction
{
    JIExplorer *m_pThis;
    BOOL bClearCache;

    RefreshAction(
        JIExplorer *pThis,
        BOOL _bClearCache
    ):m_pThis(pThis),
      bClearCache(_bClearCache)
    {}
    virtual HRESULT Do(JNIEnv *env)
    {
        m_pThis->Refresh(bClearCache);
        return S_OK; 
    }
};

JNIEXPORT void JNICALL Java_com_frostwire_gui_webbrowser_IExplorerComponent_nativeRefresh(
    JNIEnv *env, 
    jobject self,
    jboolean clearCache)
{
    JIExplorer *pThis = (JIExplorer *)env->GetLongField(self, JIExplorer::ms_IExplorerComponent_data);
    if(pThis){
        pThis->GetThread()->MakeAction(
            env,
            "Refresh error",
            RefreshAction(
                pThis,
                clearCache));
    }
}

/*
 * Class:     com_frostwire_gui_webbrowser_IExplorerComponent
 * Method:    nativeReload
 */
struct ReloadAction : public BrowserAction
{
    JIExplorer *m_pThis;

    ReloadAction(
        JIExplorer *pThis
    ):m_pThis(pThis)
    {}
    virtual HRESULT Do(JNIEnv *env)
    {
        m_pThis->Refresh(FALSE);
        return S_OK; 
    }
};

JNIEXPORT void JNICALL Java_com_frostwire_gui_webbrowser_IExplorerComponent_nativeReload(
    JNIEnv *env, 
    jobject self)
{
    JIExplorer *pThis = (JIExplorer *)env->GetLongField(self, JIExplorer::ms_IExplorerComponent_data);
    if(pThis){
        pThis->GetThread()->MakeAction(
            env,
            "Reload error",
            ReloadAction(
                pThis));
    }
}

/*
 * Class:     com_frostwire_gui_webbrowser_IExplorerComponent
 * Method:    nativeBack
 */
struct BackAction : public BrowserAction
{
    JIExplorer *m_pThis;

    BackAction(
        JIExplorer *pThis
    ):m_pThis(pThis)
    {}
    virtual HRESULT Do(JNIEnv *env)
    {
        m_pThis->GoBack();
        return S_OK; 
    }
};

JNIEXPORT void JNICALL Java_com_frostwire_gui_webbrowser_IExplorerComponent_nativeBack(
    JNIEnv *env, 
    jobject self)
{
    JIExplorer *pThis = (JIExplorer *)env->GetLongField(self, JIExplorer::ms_IExplorerComponent_data);
    if(pThis){
        pThis->GetThread()->MakeAction(
            env,
            "Back error",
            BackAction(
                pThis));
    }
}

/*
 * Class:     com_frostwire_gui_webbrowser_IExplorerComponent
 * Method:    nativeForward
 */
struct ForwardAction : public BrowserAction
{
    JIExplorer *m_pThis;

    ForwardAction(
        JIExplorer *pThis
    ):m_pThis(pThis)
    {}
    virtual HRESULT Do(JNIEnv *env)
    {
        m_pThis->GoForward();
        return S_OK; 
    }
};

JNIEXPORT void JNICALL Java_com_frostwire_gui_webbrowser_IExplorerComponent_nativeForward(
    JNIEnv *env, 
    jobject self)
{
    JIExplorer *pThis = (JIExplorer *)env->GetLongField(self, JIExplorer::ms_IExplorerComponent_data);
    if(pThis){
        pThis->GetThread()->MakeAction(
            env,
            "Forward error",
            ForwardAction(
                pThis));
    }
}

/*
 * Class:     com_frostwire_gui_webbrowser_IExplorerComponent
 * Method:    nativeGo
 * Signature: (Ljava/lang/String;)V
 */
struct GoAction : public BrowserAction{
    JStringBuffer m_jstURL;
    JIExplorer *m_pThis;

    GoAction(
        JIExplorer *pThis,
        JNIEnv *env,
        jstring jURL
    ):m_pThis(pThis),
      m_jstURL(env, jURL)
    {}
    virtual HRESULT Do(JNIEnv *env)
    {
        m_pThis->Go(_B(m_jstURL));
		return S_OK; 
    }
};

JNIEXPORT void JNICALL Java_com_frostwire_gui_webbrowser_IExplorerComponent_nativeGo(
    JNIEnv *env, 
    jobject self,
    jstring jsURL)
{
    JIExplorer *pThis = (JIExplorer *)env->GetLongField(self, JIExplorer::ms_IExplorerComponent_data);
    if(pThis){
        pThis->GetThread()->MakeAction(
            env,
            "Go error",
            GoAction(
                pThis,
                env,
                jsURL));
    }
}

/*
 * Class:     com_frostwire_gui_webbrowser_IExplorerComponent
 * Method:    nativeStop
 */
struct StopAction : public BrowserAction
{
    JIExplorer *m_pThis;

    StopAction(
        JIExplorer *pThis
    ):m_pThis(pThis)
    {}
    virtual HRESULT Do(JNIEnv *env)
    {
        OLE_TRY
        OLE_HRT( m_pThis->m_spIWebBrowser2->Stop() );
        OLE_CATCH
        OLE_RETURN_HR
    }
};

JNIEXPORT void JNICALL Java_com_frostwire_gui_webbrowser_IExplorerComponent_nativeStop(
    JNIEnv *env, 
    jobject self)
{
    JIExplorer *pThis = (JIExplorer *)env->GetLongField(self, JIExplorer::ms_IExplorerComponent_data);
    if(pThis){
        pThis->GetThread()->MakeAction(
            env,
            "Stop error",
            StopAction(
                pThis));
    }
}

/*
 * Class:     com_frostwire_gui_webbrowser_IExplorerComponent
 * Method:    nativeRunJS
 * Signature: (Ljava/lang/String;)V
 */
struct RunJSAction : public BrowserAction{
    JStringBuffer m_jstCode;
    JIExplorer *m_pThis;

    RunJSAction(
        JIExplorer *pThis,
        JNIEnv *env,
        jstring jCode
    ):m_pThis(pThis),
      m_jstCode(env, jCode)
    {}
    virtual HRESULT Do(JNIEnv *env)
    {
        m_pThis->RunJS(_B(m_jstCode));
		return S_OK; 
    }
};

JNIEXPORT void JNICALL Java_com_frostwire_gui_webbrowser_IExplorerComponent_nativeRunJS(
    JNIEnv *env, 
    jobject self,
    jstring jsCode)
{
    JIExplorer *pThis = (JIExplorer *)env->GetLongField(self, JIExplorer::ms_IExplorerComponent_data);
    if(pThis){
        pThis->GetThread()->MakeAction(
            env,
            "RunJS error",
            RunJSAction(
                pThis,
                env,
                jsCode));
    }
}

} /* extern "C" */
