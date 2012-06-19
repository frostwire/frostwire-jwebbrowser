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
#include "BrHolderThread.h"

DWORD OLE_CoGetThreadingModel()
{
    return COINIT_APARTMENTTHREADED;
}


HRESULT OLE_CoWaitForMultipleHandles(
    IN DWORD dwFlags,
    IN DWORD dwMilliseconds,
    IN ULONG nCount,
    IN LPHANDLE lpHandles,
    OUT LPDWORD lpdwIndex
){
    HRESULT hr = S_OK;
    DWORD dwStartTime = ::GetTickCount();
    DWORD dwRes = ::WaitForMultipleObjects(
        nCount,         // number of handles in the handle array
        lpHandles,      // pointer to the object-handle array
        (dwFlags&COWAIT_WAITALL) == COWAIT_WAITALL,       // wait for all or wait for one
        0);
    if( WAIT_FAILED == dwRes ) {
        STRACE1(TEXT("WaitForMultipleObjects[%08x]"), hr = HRESULT_FROM_WIN32(::GetLastError()) );
    } else if( WAIT_TIMEOUT == dwRes ) {
        while( SUCCEEDED(hr) ) {
            dwRes = ::MsgWaitForMultipleObjects(
                nCount,         // number of handles in the handle array
                lpHandles,      // pointer to the object-handle array
                (dwFlags&COWAIT_WAITALL) == COWAIT_WAITALL,       // wait for all or wait for one
                dwMilliseconds, // time-out interval in milliseconds
                /*QS_POSTMESSAGE|QS_SENDMESSAGE|QS_PAINT*/
                QS_SENDMESSAGE|QS_ALLINPUT);// type of input events to wait for
            if( WAIT_FAILED == dwRes) {
                STRACE1(TEXT("MsgWaitForMultipleObjects[%08x]"), hr = HRESULT_FROM_WIN32(::GetLastError()) );
            } else if( WAIT_TIMEOUT == dwRes) {
                hr = RPC_S_CALLPENDING;
            } else if( (WAIT_OBJECT_0 + nCount) == dwRes ) {
                DWORD dwCurTime = ::GetTickCount();
                DWORD dwDeltaTime = dwCurTime - dwStartTime;
                MSG msg;
                while( ::PeekMessage(&msg,NULL,0,0,PM_REMOVE) ) {
                    ::TranslateMessage(&msg);
                    ::DispatchMessage(&msg);
                    dwCurTime = ::GetTickCount();
                    dwDeltaTime = dwCurTime - dwStartTime;
                    if(dwDeltaTime>dwMilliseconds){
                        hr = RPC_S_CALLPENDING;
                        break;
                    }
                }
                if( SUCCEEDED(hr) ) {
                    if(dwDeltaTime>dwMilliseconds)
                        hr = RPC_S_CALLPENDING;
                    else{
                        if(INFINITE!=dwMilliseconds){
                            dwMilliseconds -= dwDeltaTime;
                        }
                        dwStartTime = dwCurTime;
                    }
                }
            } else {
                *lpdwIndex = dwRes - WAIT_OBJECT_0;
                break;
            }
        }
    }else{
        *lpdwIndex = dwRes - WAIT_OBJECT_0;
    }
    return hr;
}
DWORD OLE_CoWaitForSingleObject(
    IN HANDLE hHandle,
    IN DWORD dwMilliseconds)
{
    return OLE_CoWaitForMultipleObjects(
        1,
        &hHandle,
        FALSE,
        dwMilliseconds);
}

DWORD OLE_CoWaitForMultipleObjects(
    IN DWORD nCount,
    IN LPHANDLE lpHandles,
    IN BOOL bWaitAll,
    IN DWORD dwMilliseconds)
{
    DWORD dwRes;
    if( COINIT_APARTMENTTHREADED==OLE_CoGetThreadingModel() ){
        HRESULT hr;
        DWORD dwIndex = 0;
        hr = OLE_CoWaitForMultipleHandles(
            bWaitAll?COWAIT_WAITALL:0,
            dwMilliseconds,
            nCount,
            (LPHANDLE)lpHandles,
            &dwIndex
            );
        if(RPC_S_CALLPENDING==hr || RPC_E_TIMEOUT==hr){
            dwRes = WAIT_TIMEOUT;
        }else if(S_OK==hr){
            dwRes = WAIT_OBJECT_0 + dwIndex;
        }else{
            ::SetLastError(hr);
            dwRes = WAIT_FAILED;
        }
    }else{
        dwRes = ::WaitForMultipleObjects(
            nCount,
            lpHandles,
            bWaitAll,
            dwMilliseconds);
    }
    return dwRes;
}


void OLE_CoSleep(IN DWORD dwMilliseconds)
{
    if( COINIT_APARTMENTTHREADED==OLE_CoGetThreadingModel() ){
        HANDLE hEvent = ::CreateEvent(NULL,FALSE,FALSE,NULL);
        if(!hEvent){
            STRACE1(TEXT("CreateEvent[%08x]"), HRESULT_FROM_WIN32(::GetLastError()));
        }else{
            ::OLE_CoWaitForSingleObject(hEvent, dwMilliseconds);
            ::CloseHandle(hEvent);
        }
    }else{
        ::Sleep(dwMilliseconds);
    }
}


void OLE_CoPump()
{
    MSG msg;
    while( ::PeekMessage(&msg,NULL,0,0,PM_REMOVE) ) {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }
}


LPCTSTR         BrowserThread::ms_lpStaffWndClass = _T("StaffWndClass");
HINSTANCE       BrowserThread::ms_hInstance = NULL;


void BrowserThread::RegisterStaffWndClass(HINSTANCE hModule)
{
    WNDCLASSEX wcex;
    memset(&wcex,0, sizeof(wcex));
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.hInstance = ms_hInstance = hModule;
    wcex.lpfnWndProc = StaffWnd;
    wcex.lpszClassName = ms_lpStaffWndClass; 
    ::RegisterClassEx(&wcex);
}

LRESULT BrowserThread::StaffWnd(
    HWND hWnd, 
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam)
{
    if(WM_CREATE==uMsg){
        ::SetWindowLong(hWnd, GWL_USERDATA, (LONG)((LPCREATESTRUCT)lParam)->lpCreateParams);
    } else { 
        BrowserThread *pThis = (BrowserThread *)::GetWindowLong(hWnd, GWL_USERDATA);
        if( pThis )
            return pThis->DefWindowProc(uMsg, wParam, lParam);
    }
    return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT BrowserThread::DefWindowProc(
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam)
{
    LRESULT ret = 0;
    if(WM_USER<=uMsg && uMsg<=(WM_USER + 0x100)){
        BrowserAction *pAction = (BrowserAction *)wParam;
        ret = pAction->Do(m_env_nt);
        if(lParam)
            delete pAction;
        return ret;
    }
    return ::DefWindowProc(m_hQueueWnd, uMsg, wParam, lParam);
}

BrowserThread::BrowserThread():
    m_hStart(::CreateEvent(NULL, FALSE, FALSE, NULL)),
    m_hTerminate(::CreateEvent(NULL, FALSE, FALSE, NULL)),
    m_hAtomMutex(CreateMutex(NULL, FALSE, NULL)),
    m_hThread(NULL),
    m_dwThreadId(0),
    m_hQueueWnd(NULL),
    m_env_nt(NULL),
    m_bBusy(FALSE),
    m_cRef(1)
{}

BrowserThread::~BrowserThread()
{
    Terminate();
}

ULONG BrowserThread::AddRef(){
    return (ULONG)::InterlockedIncrement( (LPLONG)&m_cRef );
}

ULONG BrowserThread::Release(){
    ULONG cRef = (ULONG)::InterlockedDecrement( (LPLONG)&m_cRef );
    if(cRef == 0) {
        delete this;
    }
    return cRef;
}

BrowserThread *BrowserThread::GetInstance()
{
    return new BrowserThread();
}

void BrowserThread::Terminate()
{
    _HP_
    if(NULL!=m_hThread){
        ::SetEvent(m_hTerminate);
        //That can be an alarm termination
        //in this case the thread already disappear.
        //Long but limited wait is the best solution here.
        ::WaitForSingleObject(m_hThread, 15000);
        ::CloseHandle(m_hThread);
        m_hThread = NULL;
        m_dwThreadId = 0;
    }
}

void BrowserThread::SetBusy(BOOL bBusy)
{

    if(m_bBusy != bBusy){
        m_bBusy = bBusy;
        if(bBusy && ::GetCurrentThreadId() == m_dwThreadId  ){
            //clean the message stack
            MSG msg;
            while( ::PeekMessage(&msg, m_hQueueWnd, 0, 0, PM_REMOVE) ) {
                ::DispatchMessage(&msg);
            }
        }
    }
}

void BrowserThread::MakeActionAsync(BrowserAction *pAction)
{
    RestartIfNeed();
    ::PostMessage(
        m_hQueueWnd, 
        WM_USER, 
        (WPARAM)pAction,
        TRUE);
}


HRESULT BrowserThread::MakeAction(
    JNIEnv *env, 
    const char *msg, 
    BrowserAction &Action)
{
    if(m_bBusy)
        return RPC_E_RETRY;
    OLE_DECL
    RestartIfNeed();    
    while(true){
        if( 0==::SendMessageTimeout(
            m_hQueueWnd, 
            WM_USER, 
            (WPARAM)&Action,
            FALSE,
            SMTO_ABORTIFHUNG,
            500,
            (PDWORD_PTR)&OLE_HR
        )){
            //Flash is blocking message queue 
            OLE_HR = 0x800700AA;
        }
        if( Action.repeatOnError(OLE_HR) ){
            OLE_CoSleep(500);
            continue;
        }
        break;
    }
    if(FAILED(OLE_HR)){
        STRACE(_T("MakeAction %08x"), OLE_HR);
        Action.throwExeption(env, msg, OLE_HR);
    }
    OLE_RETURN_HR
}

unsigned BrowserThread::RunEx(){
    SEP(_T("Processor"));
    unsigned int ret = (unsigned int)jvm->AttachCurrentThread(
        (void**)&m_env_nt,
        NULL
    );
    if(m_env_nt){
        OLE_TRY
        COLEHolder oh;
        OLE_HRW32_BOOL( (m_hQueueWnd = ::CreateWindow(
            ms_lpStaffWndClass,
            _T("StaffWnd"),
            WS_OVERLAPPED,
            0, 0,
            0, 0,
            NULL,
            NULL,
            ms_hInstance,
            this
        )))

        ::SetEvent(m_hStart);

        OLE_CoWaitForSingleObject(
            m_hTerminate,
            INFINITE
        );
        OLE_CATCH
        if(m_hQueueWnd)
            ::DestroyWindow(m_hQueueWnd);
        OLE_CoSleep(100);//IE message pumping
        jvm->DetachCurrentThread();
    }
    return 0;
}

unsigned __stdcall BrowserThread::RunMe(LPVOID pThis){
    return ((BrowserThread *)pThis)->RunEx();
}
void BrowserThread::RestartIfNeed()
{
    _HP_
    if(NULL!=m_hThread)
        return;
    m_hThread = (HANDLE)_beginthreadex(
        NULL,
        0,
        RunMe,
        this,
        0,
        &m_dwThreadId);
    if(INVALID_HANDLE_VALUE==m_hThread){
        m_hThread = NULL;
        return;
    }
    ::WaitForSingleObject(m_hStart, INFINITE);
}