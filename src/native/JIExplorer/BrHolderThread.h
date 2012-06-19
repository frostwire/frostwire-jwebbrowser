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
#ifndef _BrHolderThread_h_
#define _BrHolderThread_h_

DWORD  OLE_CoGetThreadingModel();

HRESULT  OLE_CoWaitForMultipleHandles(
  IN DWORD dwFlags,
  IN DWORD dwMilliseconds,
  IN ULONG nCount,
  IN LPHANDLE lpHandles,
  OUT LPDWORD lpdwIndex
);

DWORD  OLE_CoWaitForSingleObject(
    IN HANDLE hHandle,
    IN DWORD dwMilliseconds
);

DWORD  OLE_CoWaitForMultipleObjects(
  IN DWORD nCount,
  IN LPHANDLE lpHandles,
  IN BOOL bWaitAll,
  IN DWORD dwMilliseconds
);

void  OLE_CoSleep(IN DWORD dwMilliseconds);

void  OLE_CoPump();


struct BrowserAction
{
    BrowserAction():m_iRetryCount(0){}; 
    virtual ~BrowserAction() {}
    virtual HRESULT Do(JNIEnv *env) = 0;
    virtual void throwExeption(JNIEnv *env, const char *msg, HRESULT hr) {
        ThrowJNIErrorOnOleError(env, hr, msg);
    };
    virtual boolean repeatOnError(HRESULT hr){
        //RPC wait-retry request
        return hr==0x800700aa && 0<(m_iRetryCount--);
    }
private:
    int m_iRetryCount;
};


#define _HP_ OLESyncro __hpsync__(m_hAtomMutex);

class BrowserThread {
private:
    ULONG m_cRef;
    HANDLE m_hThread;
    unsigned int  m_dwThreadId;
    ZZ::CHandlerSup m_hStart;
    ZZ::CHandlerSup m_hTerminate;
    ZZ::CHandlerSup m_hAtomMutex;
    HWND m_hQueueWnd;
    BOOL m_bBusy;

    static LPCTSTR         ms_lpStaffWndClass;
    static HINSTANCE       ms_hInstance;

    //params
    JNIEnv        *m_env_nt;
public:
    BrowserThread();
    ~BrowserThread();


    ULONG AddRef();
    ULONG Release();
    static BrowserThread *GetInstance();
    void RestartIfNeed();


    void MakeActionAsync(
        BrowserAction *pAction);
    HRESULT MakeAction(
        JNIEnv *env, 
        const char *msg, 
        BrowserAction &Action);

    void SetBusy(BOOL bBusy);
    static void RegisterStaffWndClass(HINSTANCE hModule);

private:
    void Terminate();
    unsigned RunEx();
    static unsigned __stdcall RunMe(LPVOID pThis);

    static LRESULT CALLBACK StaffWnd(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT DefWindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif //_BrHolderThread_h_