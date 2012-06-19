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

// FrostWire
// extended
#include "stdafx.h" 
#include <vector>
#include "BrIELWControl.h"       // main symbols
#include <Shlwapi.h>
IMPLEMENT_SECURITY();

bool IsShiftKeyDown() { return GetKeyState(VK_SHIFT) < 0; }
bool IsAltKeyDown() { return GetKeyState(VK_MENU) < 0; }
bool IsCtrlKeyDown() { return GetKeyState(VK_CONTROL) < 0; }

RECT g_emptyRect = {0};

LPCTSTR CBrIELWControl::ms_lpPN = _T("awt_hook");
#define TID_LAIZYUPDATE 1234
CBrIELWControl::CBrIELWControl()
{
    m_cRef = 1L;
    m_cSkipDraw = 0L;
    m_hwndParent = NULL;
    m_hwndIE = NULL;
    m_hwndFrame = NULL;
    m_hwndShell = NULL;

    m_pOldIEWndProc = NULL;

    m_rcIE2.left = 0;
    m_rcIE2.top = 0;
    m_rcIE2.right = 100;
    m_rcIE2.top = 100;

    m_dwAdvised = OLE_BAD_COOKIE;
    m_dwWebCPCookie = OLE_BAD_COOKIE;

    m_tidUpdate = 0;
    m_ePaintAlgorithm = PAINT_JAVA;
    m_bTransparent = false;
}

void CBrIELWControl::UpdateWindowRect()                       
{
    HWND hRel = m_hwndIE 
        ? m_hwndIE 
        : m_hwndShell;
    if(hRel && m_hwndParent){
        ::GetClientRect(hRel, &m_rcIE2);
        MapWindowPoints(
            hRel,
            m_hwndParent,
            (LPPOINT)&m_rcIE2,
            2);
        STRACE0(_T("UpdateWindowRect x:%d y:%d w:%d h:%d"),
            m_rcIE2.left,
            m_rcIE2.top,
            m_rcIE2.right - m_rcIE2.left,
            m_rcIE2.bottom - m_rcIE2.top
        );
    }
}

ULONG CBrIELWControl::Terminate() { 
    if(m_spIWebBrowser2){
        OLE_TRY
        OLE_HRT( DestroyControl() )
        OLE_CATCH
    }
    return Release();
}   

void CBrIELWControl::LaizyUpdate(DWORD dwDelayInMs){
    if(m_hwndIE){
        if(TID_LAIZYUPDATE == m_tidUpdate)
            ::KillTimer(m_hwndIE, m_tidUpdate);
        m_tidUpdate = ::SetTimer(m_hwndIE, TID_LAIZYUPDATE, dwDelayInMs, NULL);
    }
} 

HRESULT CBrIELWControl::InplaceActivate(BOOL bActivate)
{
    OLE_TRY                                    
    IOleObjectPtr spOleObject(m_spIWebBrowser2);
    OLE_CHECK_NOTNULLSP(spOleObject)

    if(!bActivate){
        OLE_HRT( spOleObject->Close(OLECLOSE_NOSAVE) )
    }
    OLE_HRT( spOleObject->SetClientSite( bActivate ? (IOleClientSite *)this : NULL ) )
    if( bActivate ){
        OLE_HRT( spOleObject->Advise((IAdviseSink *)this, &m_dwAdvised) )
    } else if(OLE_BAD_COOKIE != m_dwAdvised){
        OLE_HRT( spOleObject->Unadvise(m_dwAdvised) )
        m_dwAdvised = OLE_BAD_COOKIE;
    }

    IViewObjectPtr spViewObject(m_spIWebBrowser2);
    OLE_CHECK_NOTNULLSP(spViewObject)

    OLE_HRT( spViewObject->SetAdvise(
        DVASPECT_CONTENT,
        0,
        bActivate ? (IAdviseSink *)this : NULL))
    
    if( bActivate ){
        UpdateWindowRect();
        MSG msg; //fake param
        OLE_HRT( spOleObject->DoVerb(
            OLEIVERB_INPLACEACTIVATE, 
            &msg, 
            this, 
            0, 
            m_hwndParent, 
            &m_rcIE2))
/*
        OLE_HRT( spOleObject->DoVerb(
            OLEIVERB_HIDE, 
            &msg, 
            this, 
            0, 
            m_hwndParent, 
            &m_rcIE2))
*/
    }
    OLE_CATCH
    OLE_RETURN_HR
}

HRESULT CBrIELWControl::AdviseBrowser(IN BOOL bAdvise)
{
    OLE_TRY 
    IConnectionPointContainerPtr spConnectionPointContainer(m_spIWebBrowser2);
    OLE_CHECK_NOTNULLSP(spConnectionPointContainer)

    IConnectionPointPtr spConnectionPoint;
    OLE_HRT( spConnectionPointContainer->FindConnectionPoint(
	DIID_DWebBrowserEvents2,
	&spConnectionPoint))
    OLE_CHECK_NOTNULLSP(spConnectionPoint)

    if(bAdvise){
	OLE_HRT( spConnectionPoint->Advise((IDispatch *)this, &m_dwWebCPCookie) )
    }else{
        OLE_HRT( spConnectionPoint->Unadvise(m_dwWebCPCookie) )
        m_dwWebCPCookie = OLE_BAD_COOKIE;
    }
    OLE_CATCH
    OLE_RETURN_HR
}

void OLE_CoPump();
HRESULT CBrIELWControl::DestroyControl()
{
    if(m_hwndIE){
        RemoveHook(m_hwndIE);
    }
    m_hwndShell = NULL;

    OLE_TRY 
    OLE_HRT( InplaceActivate(FALSE) )       
    OLE_HRT( AdviseBrowser(FALSE) )
    OLE_CATCH
    m_dwAdvised = OLE_BAD_COOKIE;
    m_dwWebCPCookie = OLE_BAD_COOKIE;
    m_spIWebBrowser2 = NULL;
    m_ccIWebBrowser2 = NULL;

    OLE_CoPump();
    OLE_RETURN_HR
}

HRESULT CBrIELWControl::Connect(IN BSTR bsURL)
{
    OLE_TRY 
    static _bstr_t bsAboutBlank(L"about:blank");
    OLE_HRT( m_spIWebBrowser2->Navigate(
        bsURL ? bsURL : ((BSTR)bsAboutBlank),
        _PO, _PO, _PO, _PO))
    OLE_CATCH
    OLE_RETURN_HR
}

HRESULT CBrIELWControl::CreateControl(
    IN HWND     hParent,
    IN LPCRECT  prcIE,
    IN BSTR     bsURL)
{
    OLE_TRY 
    OLE_CHECK_NOTNULL(hParent)
    OLE_CHECK_NOTNULL(prcIE)
    m_hwndParent = hParent;
    m_rcIE2 = *prcIE;

    if(m_spIWebBrowser2){
        OLE_HRT( DestroyControl() )
    }

	SetBrowserEmulation();

    OLE_HRT( CoCreateInstance(
        CLSID_WebBrowser, 
        NULL, 
        CLSCTX_INPROC, 
        IID_IWebBrowser2, 
        (void**)&m_spIWebBrowser2))
    OLE_CHECK_NOTNULLSP(m_spIWebBrowser2)        

    m_ccIWebBrowser2 = m_spIWebBrowser2;
    OLE_HRT( AdviseBrowser(TRUE) )
    OLE_HRT( InplaceActivate(TRUE) )       

    OLE_HRT( Connect(bsURL) )   

    IOleWindowPtr pOleWindow(m_spIWebBrowser2);
    OLE_CHECK_NOTNULLSP(pOleWindow)        
    OLE_HRT(pOleWindow->GetWindow(&m_hwndShell));

    OLE_CATCH
    OLE_RETURN_HR
}


HRESULT CBrIELWControl::OnPaint(HDC hdc, LPCRECT prcClip/*InParent*/)
{
    OLE_TRY
    const RECT &rc = *prcClip;
    IViewObjectPtr spViewObject(m_spIWebBrowser2);
    OLE_CHECK_NOTNULLSP(spViewObject)

    SEP(_T("Draw"));
    RECTL rcIE = {
        0, 0, 
        m_rcIE2.right - m_rcIE2.left, m_rcIE2.bottom - m_rcIE2.top
    };
    RECTL rcIEClip = {
        rc.left - m_rcIE2.left, rc.top - m_rcIE2.top, 
        rc.right - m_rcIE2.left, rc.bottom - m_rcIE2.top
    };

    CDCHolder shCopy;
    shCopy.Create(hdc, rcIE.right, rcIE.bottom, FALSE);
    //STRACE1(_T("shCopy.Create done"));
    ++m_cSkipDraw;
    OLE_HRT( spViewObject->Draw(
        DVASPECT_CONTENT,
        -1, 
        NULL, 
        NULL,
        NULL, 
        shCopy, 
        &rcIE, 
        &rcIEClip,
        NULL, 
        0
    ));
    --m_cSkipDraw;
    OLE_HRW32_BOOL( ::BitBlt(
        hdc, 
        rc.left,
        rc.top,
        rc.right - rc.left,
        rc.bottom - rc.top,
        shCopy,
        rcIEClip.left,
        rcIEClip.top,
        SRCCOPY ));                      
    OLE_CATCH
    OLE_RETURN_HR
}


BOOL CBrIELWControl::IsSmoothScroll()
{
    HWND hScroll = ::GetCapture();
    if(hScroll){
#define _MAX_CLASS 64
        TCHAR szClassName[_MAX_CLASS] = {0};
        hScroll = (
            _MAX_CLASS > GetClassName(
                hScroll,
                szClassName,
                _MAX_CLASS - 1
            ) 
            && 0==_tcscmp(szClassName, _T("Internet Explorer_Hidden"))
        )
            ? hScroll
            : NULL;
    }
    return (BOOL)hScroll; 
}

LRESULT CBrIELWControl::NewIEProcStub(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CBrIELWControl *pThis = (CBrIELWControl *)::GetProp(hWnd, ms_lpPN);
    return pThis
        ? pThis->NewIEProc(hWnd, msg, wParam, lParam)
        : 0;
}                                         

void CBrIELWControl::RedrawParentRect(LPRECT pRect)
{
    InvalidateRect(m_hwndParent, pRect, FALSE);
}

void CBrIELWControl::RedrawInParent()
{
    OLE_TRY
    UpdateWindowRect();
    MSG msg; //fake param
    OLE_HRT( IOleObjectPtr(m_spIWebBrowser2)->DoVerb(
        OLEIVERB_INPLACEACTIVATE, 
        &msg, 
        this, 
        0, 
        m_hwndParent, 
        &m_rcIE2))
    OLE_CATCH
}

void CBrIELWControl::updateTransparentMask(LPRECT prc)
{
}

void CBrIELWControl::PaintScreenShort()
{
    SEP(_T("PaintScreenShort"));
    int iWidth = m_rcIE2.right - m_rcIE2.left;//rc.right - rc.left;
    int iHeight = m_rcIE2.bottom - m_rcIE2.top;//rc.bottom - rc.top;
    if( (iWidth > m_shScreen.m_iWidth) ||  (iHeight > m_shScreen.m_iHeight) ){
        HDC hdc = ::GetDC(m_hwndIE);
        if(hdc){
            OLE_TRY
            m_shScreen.Create(
                hdc, 
                max(iWidth, m_shScreen.m_iWidth),
                max(iHeight, m_shScreen.m_iHeight),
                FALSE);
            OLE_CATCH
            ::ReleaseDC(m_hwndIE, hdc);
        }
    }

    ++m_cSkipDraw;
    OLE_TRY
    //lock the COM server any way!!!
    IViewObjectPtr spViewObject(m_spIWebBrowser2);
    OLE_CHECK_NOTNULLSP(spViewObject)
    if(PAINT_NATIVE == m_ePaintAlgorithm){
//          ::SendMessage(
//              m_hwndShell, 
//              WM_PRINT, 
//              (WPARAM)(HDC)m_shScreen,
//              PRF_CLIENT | PRF_NONCLIENT | PRF_OWNED | PRF_CHILDREN | PRF_ERASEBKGND);
        //PrintWindow(m_hwndIE?m_hwndIE:m_hwndShell, (HDC)m_shScreen, 0);
        PrintWindow(m_hwndShell, (HDC)m_shScreen, 0);
    } else {
        RECTL rcIE = {0, 0, iWidth, iHeight};
        SEP(_T("DrawJava"));
        OLE_HRT( spViewObject->Draw(
            DVASPECT_CONTENT,
            -1, 
            NULL, 
            NULL,
            NULL, 
            m_shScreen, 
            &rcIE, 
            NULL,
            NULL, 
            0
        ));
//         HDC hdc = ::GetDC(m_hwndParent); //TODO: pParent->GetDCFromComponent();
//         if(hdc) {
//             SEP1(_T("DrawNative"));
//             OLE_NEXT_TRY
//             OLE_HRW32_BOOL( ::SetViewportOrgEx(
//                 hdc, 
//                 m_rcIE2.left, 
//                 m_rcIE2.top, 
//                 NULL ))
//             OLE_HRT( spViewObject->Draw(
//                 DVASPECT_CONTENT,
//                 -1, 
//                 NULL, 
//                 NULL,
//                 NULL, 
//                 hdc, 
//                 &rcIE, 
//                 NULL,
//                 NULL, 
//                 0
//             ));
//             OLE_HRW32_BOOL( ::SetViewportOrgEx(
//                 hdc, 
//                 -m_rcIE2.left, 
//                 -m_rcIE2.top, 
//                 NULL ))
//             OLE_CATCH
//             ReleaseDC(m_hwndParent, hdc);
//         }
    }
    OLE_CATCH        
    --m_cSkipDraw;
}

void CBrIELWControl::RemoveHook(HWND hWnd)
{
    if(NULL==m_hwndIE || m_hwndIE!=hWnd){
        STRACE(_T("}hook-alarm! w:%08x"), hWnd);
        return;
    }
    ::SetWindowLongPtr(m_hwndIE, GWLP_WNDPROC, m_pOldIEWndProc);        
    ::RemoveProp(m_hwndIE, ms_lpPN);
    m_pOldIEWndProc = NULL;
    STRACE(_T("}hook ie:%08x"), m_hwndIE);
    m_hwndIE = NULL;
    m_hwndFrame = NULL;
}

void CBrIELWControl::AddHook()
{
    if(NULL!=m_hwndIE)
        RemoveHook(m_hwndIE);

    UpdateWindowRect();
    STRACE0(_T("hwndShell:%08x"), m_hwndShell);
    if(m_hwndShell){
        m_hwndFrame = ::GetWindow(m_hwndShell, GW_CHILD); 
        if(m_hwndFrame){
            m_hwndIE = ::GetWindow(m_hwndFrame, GW_CHILD); 
            if(m_hwndIE){
                WNDPROC pOldIEWndProc = (WNDPROC)::GetWindowLongPtr(m_hwndIE, GWLP_WNDPROC);
                if(NewIEProcStub!=pOldIEWndProc){
                    STRACE(_T("{hook %08x"), m_hwndIE);
                    ::SetProp(m_hwndIE, ms_lpPN, this);
                    m_pOldIEWndProc = ::SetWindowLongPtr(m_hwndIE, GWLP_WNDPROC, (LONG_PTR)NewIEProcStub);
                    SendIEEvent(
                        -3, 
                        _T(""), 
                        _T("")
                    );
                }
            }
        }
    }            
    if(PAINT_NATIVE!=m_ePaintAlgorithm){
        RedrawInParent();
    } 
}

LRESULT CBrIELWControl::NewIEProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    STRACE0(_T("msg:%08x hWnd:%08x"), msg, hWnd);
    if(PAINT_NATIVE!=m_ePaintAlgorithm && (WM_PAINT==msg || WM_NCPAINT==msg || WM_SYNCPAINT==msg) ){
        if( IsSmoothScroll() ){
            STRACE(_T("_SPAINT %08x"), msg);
            LaizyUpdate();
        } else {
            if( 0==m_cSkipDraw){
                RECT rc;            
                if( GetUpdateRect(hWnd, &rc, FALSE) ){
                    RECT clip = rc; 
                    UpdateWindowRect();       

                    STRACE(_T("_PAINT %08x %08x"), msg, hWnd);

                    if(0!=m_tidUpdate){
                        ::KillTimer(m_hwndIE, m_tidUpdate);
                        m_tidUpdate = 0;
                        rc = m_rcIE2;
                    } else {
                        rc.top += m_rcIE2.top;
                        rc.bottom += m_rcIE2.top;                                  
                        rc.left += m_rcIE2.left;
                        rc.right += m_rcIE2.left;
                    }

                    PaintScreenShort();

                    if(rc.top != rc.bottom && rc.left!=rc.right){
                        SEP(_T("Draw"));
                        if(PAINT_JAVA==m_ePaintAlgorithm){
                            ValidateRect(hWnd, &clip);
                        }
                        RedrawParentRect(&rc);
                    } 
                }
            } 
            //seems empty redraw is a signal...
            STRACE(_T("_NPAINT %08x"), msg);
            return ::CallWindowProc((WNDPROC)m_pOldIEWndProc, hWnd, msg, wParam, lParam);      
        } 
        //end paint processing
    } else if(WM_GETDLGCODE == msg){
        return DLGC_WANTALLKEYS 
            | DLGC_WANTARROWS
            | DLGC_HASSETSEL
            | DLGC_WANTCHARS
            | DLGC_WANTTAB;
    } else if(WM_TIMER == msg ){
        if(wParam == m_tidUpdate){
            if( IsSmoothScroll() ){
                LaizyUpdate();
            } else {
                ::KillTimer(m_hwndIE, m_tidUpdate);
                m_tidUpdate = 0;
                InvalidateRect(hWnd, NULL, FALSE);
                UpdateWindow(hWnd);
            }
            return 0;
        }
    } else if(WM_PARENTNOTIFY == msg && WM_CREATE==wParam){
        STRACE(_T("Created Child hWnd:%08x"), hWnd);
    }

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
        pHook = ::SetWindowLongPtr(hWnd, GWLP_WNDPROC, m_pOldIEWndProc);        
    }

    LRESULT res = ::CallWindowProc((WNDPROC)m_pOldIEWndProc, hWnd, msg, wParam, lParam);      
    if(pHook && m_hwndIE==hWnd) //window can be disconnected ;)
        ::SetWindowLongPtr(hWnd, GWLP_WNDPROC, pHook);

    return res;
}

HRESULT CBrIELWControl::SendIEEvent(
    int iId,
    LPTSTR lpName, 
    LPTSTR lpValue,
    _bstr_t &bsResult)
{
    return S_OK;
}

// IDispatch
_bstr_t CreateParamString(DISPPARAMS  *pDispParams)
{
    _bstr_t ret(_T(""));
    for(UINT i = 0; i < pDispParams->cArgs; ++i )
    {
        if(0!=i)
            ret += _T(",");
        OLE_TRY
        ret += _bstr_t( _variant_t(pDispParams->rgvarg[i], true) );
        OLE_CATCH
        if(FAILED(OLE_HR)){
            ret += _T("<unknown>");
        }
    }
    return ret;
}

struct IE_EVENT {
   DISPID   dispid;
   LPCTSTR  lpEventName;
   static int __cdecl compare(const IE_EVENT *p1, const IE_EVENT *p2){
       return p1->dispid - p2->dispid;
   }
   static int sort(IE_EVENT *base, size_t count){
        qsort( 
            (void *)base, 
            count, 
            sizeof(IE_EVENT), 
            (int (*)(const void*, const void*))compare 
        );
        return 0;
   }
   static IE_EVENT *find(DISPID dispid, IE_EVENT *base, size_t count){
        IE_EVENT ev = {dispid, NULL};
        return (IE_EVENT *)bsearch( 
            (void *)&ev, 
            (void *)base, 
            count, 
            sizeof(IE_EVENT), 
            (int (*)(const void*, const void*))compare 
        );
   }
};

IE_EVENT g_aevSupported[] = {
    {DISPID_BEFORENAVIGATE                , _T("beforeNavigate")}, // this is sent before navigation to give a chance to abort
    {DISPID_NAVIGATECOMPLETE              , _T("navigateComplete")}, // in async, this is sent when we have enough to show
    {DISPID_STATUSTEXTCHANGE              , _T("statusTextChange")}, 
    {DISPID_QUIT                          , _T("quit")},
    {DISPID_DOWNLOADCOMPLETE              , _T("downloadComplete")},
    {DISPID_COMMANDSTATECHANGE            , _T("commandStateChange")},
    {DISPID_DOWNLOADBEGIN                 , _T("downloadBegin")},
    {DISPID_NEWWINDOW                     , _T("newWindow")},// sent when a new window should be created
    {DISPID_PROGRESSCHANGE                , _T("progressChange")},// sent when download progress is updated
    {DISPID_WINDOWMOVE                    , _T("windowMove")},// sent when main window has been moved
    {DISPID_WINDOWRESIZE                  , _T("windowResize")},// sent when main window has been sized
    {DISPID_WINDOWACTIVATE                , _T("windowActivate")},// sent when main window has been activated
    {DISPID_PROPERTYCHANGE                , _T("propertyChange")},// sent when the PutProperty method is called
    {DISPID_TITLECHANGE                   , _T("titleChange")},// sent when the document title changes
    {DISPID_TITLEICONCHANGE               , _T("titleIconChange")},// sent when the top level window icon may have changed.
                                          
    {DISPID_FRAMEBEFORENAVIGATE           , _T("frameBeforeNavigate")},  
    {DISPID_FRAMENAVIGATECOMPLETE         , _T("frameNavigateComplete")},
    {DISPID_FRAMENEWWINDOW                , _T("frameNewWindow")},
                                         
    {DISPID_BEFORENAVIGATE2               , _T("beforeNavigate2")},// hyperlink clicked on
    {DISPID_NEWWINDOW2                    , _T("newWindow2")},
    {DISPID_NAVIGATECOMPLETE2             , _T("navigateComplete2")},// UIActivate new document
    {DISPID_ONQUIT                        , _T("onQuit")},
    {DISPID_ONVISIBLE                     , _T("onVisible")},// sent when the window goes visible/hidden
    {DISPID_ONTOOLBAR                     , _T("onToolbar")},// sent when the toolbar should be shown/hidden
    {DISPID_ONMENUBAR                     , _T("ONMENUBAR")},// sent when the menubar should be shown/hidden
    {DISPID_ONSTATUSBAR                   , _T("ONSTATUSBAR")},// sent when the statusbar should be shown/hidden
    {DISPID_ONFULLSCREEN                  , _T("ONFULLSCREEN")},// sent when kiosk mode should be on/off
    {DISPID_DOCUMENTCOMPLETE              , _T("DOCUMENTCOMPLETE")},// new document goes ReadyState_Complete
    {DISPID_ONTHEATERMODE                 , _T("ONTHEATERMODE")},// sent when theater mode should be on/off
    {DISPID_ONADDRESSBAR                  , _T("ONADDRESSBAR")},// sent when the address bar should be shown/hidden
    {DISPID_WINDOWSETRESIZABLE            , _T("WINDOWSETRESIZABLE")},// sent to set the style of the host window frame
    {DISPID_WINDOWCLOSING                 , _T("WINDOWCLOSING")},// sent before script window.close closes the window 
    {DISPID_WINDOWSETLEFT                 , _T("WINDOWSETLEFT")},// sent when the put_left method is called on the WebOC
    {DISPID_WINDOWSETTOP                  , _T("WINDOWSETTOP")},// sent when the put_top method is called on the WebOC
    {DISPID_WINDOWSETWIDTH                , _T("WINDOWSETWIDTH")},// sent when the put_width method is called on the WebOC
    {DISPID_WINDOWSETHEIGHT               , _T("WINDOWSETHEIGHT")},// sent when the put_height method is called on the WebOC 
    {DISPID_CLIENTTOHOSTWINDOW            , _T("CLIENTTOHOSTWINDOW")},// sent during window.open to request conversion of dimensions
    {DISPID_SETSECURELOCKICON             , _T("SETSECURELOCKICON")},// sent to suggest the appropriate security icon to show
    {DISPID_FILEDOWNLOAD                  , _T("FILEDOWNLOAD")},// Fired to indicate the File Download dialog is opening
    {DISPID_NAVIGATEERROR                 , _T("NAVIGATEERROR")},// Fired to indicate the a binding error has occured
    {DISPID_PRIVACYIMPACTEDSTATECHANGE    , _T("PRIVACYIMPACTEDSTATECHANGE")},// Fired when the user's browsing experience is impacted
// Printing events
    {DISPID_PRINTTEMPLATEINSTANTIATION    , _T("PRINTTEMPLATEINSTANTIATION")},// Fired to indicate that a print template is instantiated
    {DISPID_PRINTTEMPLATETEARDOWN         , _T("PRINTTEMPLATETEARDOWN")},// Fired to indicate that a print templete is completely gone 
    {DISPID_UPDATEPAGESTATUS              , _T("UPDATEPAGESTATUS")},// Fired to indicate that the spooling status has changed
};

std::vector<_bstr_t> split(
    LPCOLESTR pstr, 
    LPCOLESTR delim = L",",
    bool empties = true)
{
    std::vector<_bstr_t> results;
    LPOLESTR r = NULL;
    r = (LPOLESTR) wcsstr(pstr, delim);
    int dlen = wcslen(delim);
    while( NULL != r ){
        LPOLESTR cp = new OLECHAR[(r-pstr)+1];
        memcpy( cp, pstr, (r-pstr)*sizeof(OLECHAR) );
        cp[(r-pstr)] = L'\0';
        if( 0 < wcslen(cp) || empties ){
            results.push_back(cp);
        }
        delete[] cp;
        pstr = r + dlen;
        r = (LPOLESTR) wcsstr(pstr, delim);
    }
    if( 0 < wcslen(pstr) || empties ){
        results.push_back(pstr);
    }
    return results;
}

HRESULT CBrIELWControl::Invoke(
        DISPID dispIdMember,
        REFIID riid,
        LCID lcid,
        WORD wFlags,
        DISPPARAMS  *pDispParams,
        VARIANT  *pVarResult,
        EXCEPINFO  *pExcepInfo,
        UINT  *puArgErr
){
    STRACE0(_T("DISPID_: %08x %d"), dispIdMember, dispIdMember);

    OLE_TRY
    BOOL bNotify = TRUE;
    switch(dispIdMember){
    case DISPID_HTMLWINDOWEVENTS_ONERROR:
        //STRACE0(_T("DISPID_NAVIGATECOMPLETE2"));
        //::MessageBoxA(NULL, "Error", "Error", MB_OK);
        break;
    case DISPID_NAVIGATECOMPLETE2:
        STRACE0(_T("DISPID_NAVIGATECOMPLETE2"));
        //SendIEEvent(dispIdMember, _T("navigateComplete2"), _T("")/* CreateParamString(pDispParams)*/);
        AddHook();
		NavigateComplete();
        break;
    case DISPID_PROGRESSCHANGE:
        break;
    case DISPID_HTMLDOCUMENTEVENTS_ONREADYSTATECHANGE:
        STRACE(_T("document ready!"));
	break;
    case DISPID_WINDOWCLOSING:

#define INDEX_WINDOWCLOSING_bIsChildWindow 1
#define INDEX_WINDOWCLOSING_pbCancel       0
        
        *(pDispParams->rgvarg[INDEX_WINDOWCLOSING_pbCancel].pboolVal) =    
            VARIANT_TRUE==pDispParams->rgvarg[INDEX_WINDOWCLOSING_bIsChildWindow].boolVal
                ? VARIANT_FALSE
                : VARIANT_TRUE;

        OLE_HR = S_OK; 
        break;
    case DISPID_AMBIENT_DLCONTROL:
        // respond to this ambient to indicate that we only want to
        // download the page, but we don't want to run scripts,
        // Java applets, or ActiveX controls
        if(FALSE){
            V_VT(pVarResult) = VT_I4;
            V_I4(pVarResult) =
                DLCTL_DOWNLOADONLY |
                DLCTL_NO_JAVA |
                DLCTL_NO_SCRIPTS |
                DLCTL_NO_DLACTIVEXCTLS |
                DLCTL_NO_RUNACTIVEXCTLS;
            return S_OK;
        }
		//pVarResult->lVal = DLCTL_DLIMAGES | DLCTL_VIDEOS | DLCTL_BGSOUNDS | DLCTL_SILENT;
        return DISP_E_MEMBERNOTFOUND;
	case DISPID_DOCUMENTCOMPLETE:
		STRACE0(_T("DOCUMENT_COMPLETE"));
		break;
	default:
        OLE_HR = DISP_E_MEMBERNOTFOUND;
        break;
    }

    if(bNotify){
        //just ones and only if it needs
        static int __sortEvents__ = IE_EVENT::sort( 
            g_aevSupported, 
            sizeof(g_aevSupported)/sizeof(*g_aevSupported) 
        );

        IE_EVENT *pEvent = IE_EVENT::find( 
            dispIdMember,
            g_aevSupported, 
            sizeof(g_aevSupported)/sizeof(*g_aevSupported) 
        );

        if(NULL!=pEvent){
            SEP(_T("SendIEEvent"));
            _bstr_t bsRes;
            SendIEEvent(
                pEvent->dispid, 
                _bstr_t(pEvent->lpEventName), 
                CreateParamString(pDispParams),
                bsRes
            );
            
            switch(dispIdMember){
            case DISPID_NEWWINDOW2:
#define INDEX_NEWWINDOW2_ppDisp         1
#define INDEX_NEWWINDOW2_pbCancel       0
                if( 0 < bsRes.length() ){
                    SEP(_T("new window"));
                    std::vector<_bstr_t> par = split(bsRes);

                    if( INDEX_NEWWINDOW2_pbCancel < par.size() )
                        *(pDispParams->rgvarg[INDEX_NEWWINDOW2_pbCancel].pboolVal) = 
                            _V(par[INDEX_NEWWINDOW2_pbCancel]);

                    if( INDEX_NEWWINDOW2_ppDisp < par.size() ){
                        CBrIELWControl *pThis = (CBrIELWControl *)((LONG)_V(par[INDEX_NEWWINDOW2_ppDisp]));
                        if(NULL!=pThis && bool(pThis->m_spIWebBrowser2)){
                            *(pDispParams->rgvarg[INDEX_NEWWINDOW2_ppDisp].ppdispVal) = 
                                pThis->m_ccIWebBrowser2.GetThreadInstance();
                                //pThis->m_spIWebBrowser2;
                                //pThis->m_spIWebBrowser2->AddRef();
                        } else {
                            STRACE1(_T("bad new window"));
                        }
                    }

                    OLE_HR = S_OK; 
                }
                break;
            }
        }
    }
    OLE_CATCH
    OLE_RETURN_HR
}

// IDocHostShowUI
HRESULT STDMETHODCALLTYPE CBrIELWControl::ShowMessage(HWND hwnd,
	LPOLESTR lpstrText, LPOLESTR lpstrCaption, DWORD dwType,
	LPOLESTR lpstrHelpFile, DWORD dwHelpContext, LRESULT *plResult)
{
	// Called by MSHTML to display a message box.
	// It is used for Microsoft JScript alerts among other things
	// S_OK: Host displayed its UI. MSHTML does not display its message box.
	// S_FALSE: Host did not display its UI. MSHTML displays its message box.

	//*plResult = IDCANCEL;
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CBrIELWControl::ShowHelp(HWND hwnd,
	LPOLESTR pszHelpFile, UINT uCommand, DWORD dwData, POINT ptMouse,
	IDispatch *pDispatchObjectHit)
{
	return E_NOTIMPL;
}

// IDocHostUIHandler
HRESULT STDMETHODCALLTYPE CBrIELWControl::ShowContextMenu(DWORD dwID,
	POINT *ppt, IUnknown *pcmdtReserved, IDispatch *pdispReserved)
{
	// Do not show context menus by default with S_OK.
    HRESULT hr(S_OK);

    // If text select or control menu, then return S_FALSE to show menu.
    if( dwID == CONTEXT_MENU_TEXTSELECT || 
        dwID == CONTEXT_MENU_CONTROL) {
        hr = S_FALSE;
	}

    return hr;
}

HRESULT STDMETHODCALLTYPE CBrIELWControl::GetHostInfo(DOCHOSTUIINFO *pInfo)
{
	// Called at initialization of the browser object UI. We can set various
	// features of the browser object here.
	// We can do disable the 3D border (DOCHOSTUIFLAG_NO3DOUTERBORDER) and
	// other things like hide the scroll bar (DOCHOSTUIFLAG_SCROLL_NO), display
	// picture display (DOCHOSTUIFLAG_NOPICS), disable any script running when
	// the page is loaded (DOCHOSTUIFLAG_DISABLE_SCRIPT_INACTIVE), open a site
	// in a new browser window when the user clicks on some link
	// (DOCHOSTUIFLAG_OPENNEWWIN), and lots of other things. See the MSDN docs
	// on the DOCHOSTUIINFO struct passed to us.

	//pInfo->dwFlags = (hasScrollbars ? 0 : DOCHOSTUIFLAG_SCROLL_NO) | DOCHOSTUIFLAG_NO3DOUTERBORDER;
	pInfo->dwFlags = DOCHOSTUIFLAG_NO3DOUTERBORDER | DOCHOSTUIFLAG_DISABLE_HELP_MENU | DOCHOSTUIFLAG_SCROLL_NO;

	return S_OK;
}

HRESULT STDMETHODCALLTYPE CBrIELWControl::ShowUI(DWORD dwID,
	IOleInPlaceActiveObject *pActiveObject, IOleCommandTarget *pCommandTarget,
	IOleInPlaceFrame *pFrame, IOleInPlaceUIWindow *pDoc)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CBrIELWControl::HideUI()
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CBrIELWControl::UpdateUI()
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CBrIELWControl::EnableModeless(BOOL fEnable)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CBrIELWControl::OnDocWindowActivate(BOOL fActivate)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CBrIELWControl::OnFrameWindowActivate(
	BOOL fActivate)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CBrIELWControl::ResizeBorder(LPCRECT prcBorder,
	IOleInPlaceUIWindow *pUIWindow, BOOL fRameWindow)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CBrIELWControl::TranslateAccelerator(LPMSG lpMsg,
	const GUID *pguidCmdGroup, DWORD nCmdID)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CBrIELWControl::GetOptionKeyPath(LPOLESTR *pchKey,
	DWORD dw)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CBrIELWControl::GetDropTarget(
	IDropTarget *pDropTarget, IDropTarget **ppDropTarget)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CBrIELWControl::GetExternal(IDispatch **ppDispatch)
{
	// Gets the host's IDispatch interface.
	// If the host exposes an automation interface, it can provide a reference
	// to MSHTML through ppDispatch.
	// If the method implementation does not supply an IDispatch, set
	// ppDispatch to NULL, even if the method fails or returns S_FALSE.

	*ppDispatch = NULL;
	return S_FALSE;
}

HRESULT STDMETHODCALLTYPE CBrIELWControl::TranslateUrl(DWORD dwTranslate,
	OLECHAR *pchURLIn, OLECHAR **ppchURLOut)
{
	*ppchURLOut = 0;

	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CBrIELWControl::FilterDataObject(IDataObject *pDO,
	IDataObject **ppDORet)
{
	*ppDORet = 0;

	return E_NOTIMPL;
}

// IOleInPlaceUIWindow
HRESULT STDMETHODCALLTYPE CBrIELWControl::GetBorder(LPRECT lprectBorder)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CBrIELWControl::RequestBorderSpace(
	LPCBORDERWIDTHS pborderwidths)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CBrIELWControl::SetBorderSpace(
	LPCBORDERWIDTHS pborderwidths)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CBrIELWControl::SetActiveObject(
	IOleInPlaceActiveObject *pActiveObject, LPCOLESTR pszObjName)
{
	return E_NOTIMPL;
}

// IOleInPlaceFrame
HRESULT STDMETHODCALLTYPE CBrIELWControl::InsertMenus(HMENU hmenuShared,
	LPOLEMENUGROUPWIDTHS lpMenuWidths)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CBrIELWControl::SetMenu(HMENU hmenuShared,
	HOLEMENU holemenu, HWND hwndActiveObject)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CBrIELWControl::RemoveMenus(HMENU hmenuShared)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CBrIELWControl::SetStatusText(
	LPCOLESTR pszStatusText)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CBrIELWControl::TranslateAccelerator(LPMSG lpmsg,
	WORD wID)
{
	return E_NOTIMPL;
}

IHTMLDocument2 *CBrIELWControl::GetDoc()
{
	IDispatch *dispatch = 0;
	m_spIWebBrowser2->get_Document(&dispatch);

	if (dispatch == NULL) {
		return NULL;
	}

	IHTMLDocument2 *doc;
	dispatch->QueryInterface(IID_IHTMLDocument2, (void**)&doc);
	dispatch->Release();

	return doc;
}

void CBrIELWControl::NavigateComplete()
{
}

void CBrIELWControl::SetBrowserEmulation()
{
	int version = 9000;
	HKEY key = NULL;
	TCHAR* subkey = _T("Software\\Microsoft\\Internet Explorer\\Main\\FeatureControl\\FEATURE_BROWSER_EMULATION");
	if (RegCreateKeyEx(HKEY_CURRENT_USER, subkey, 0, NULL, REG_OPTION_VOLATILE, KEY_WRITE | KEY_QUERY_VALUE, 0, &key, NULL) == 0) {
		TCHAR lpszPath[MAX_PATH];
		GetModuleFileName(0, lpszPath, MAX_PATH);
		PathStripPath(lpszPath);
		if (RegQueryValueEx(key, lpszPath, 0, NULL, NULL, NULL) == ERROR_FILE_NOT_FOUND) {
			RegSetValueEx(key, lpszPath, 0, REG_DWORD, (BYTE*)&version, 4);
		}
		RegCloseKey(key);
	}
}

////////////////////////
// struct CDCHolder

CDCHolder::CDCHolder()
: m_hMemoryDC(NULL),
    m_iWidth(0),
    m_iHeight(0),
    m_bForImage(FALSE),
    m_hBitmap(NULL),
    m_hOldBitmap(NULL),
    m_hAlphaBitmap(NULL),
    m_pPoints(NULL)
{}

void CDCHolder::Create(
    HDC hRelDC,
    int iWidth,
    int iHeght,
    BOOL bForImage
){
    OLE_DECL
/*
    if( bForImage && (iWidth & 0x03) ){
        iWidth &= ~0x03;
        iWidth += 0x04;
    }
*/
    m_iWidth = iWidth;
    m_iHeight = iHeght;
    m_bForImage = bForImage;
    if(NULL==m_hMemoryDC){
        m_hMemoryDC = ::CreateCompatibleDC(hRelDC);
    } else {
        if(m_hOldBitmap)
            ::SelectObject(m_hMemoryDC, m_hOldBitmap);
        if(m_hBitmap)
            ::DeleteObject(m_hBitmap);
        if(m_hAlphaBitmap)
            ::DeleteObject(m_hAlphaBitmap);
    }
    //NB: can not throw an error in non-safe stack!!! Just conversion and logging!
    //OLE_WINERROR2HR just set OLE_HR without any throw! 
    if( !m_hMemoryDC ){
        OLE_THROW_LASTERROR(_T("CreateCompatibleDC"))
    } 
    m_hBitmap = m_bForImage
        ? CreateJavaContextBitmap(hRelDC, m_iWidth, m_iHeight, &m_pPoints)
        : ::CreateCompatibleBitmap(hRelDC, m_iWidth, m_iHeight);
    if( !m_hBitmap ){
        OLE_THROW_LASTERROR(_T("CreateCompatibleBitmap"))
    }
    m_hOldBitmap = (HBITMAP)::SelectObject(m_hMemoryDC, m_hBitmap);
    if( !m_hOldBitmap ){
        OLE_THROW_LASTERROR(_T("SelectBMObject"))
    } 
}

CDCHolder::~CDCHolder(){
    if(m_hOldBitmap)
        ::SelectObject(m_hMemoryDC, m_hOldBitmap);
    if(m_hBitmap)
        ::DeleteObject(m_hBitmap);
    if(m_hAlphaBitmap)
        ::DeleteObject(m_hAlphaBitmap);
    if(m_hMemoryDC)
        ::DeleteDC(m_hMemoryDC);
};

//BITMAPINFO extended with 
typedef struct tagBITMAPINFOEX  {
    union{
        BITMAPV4HEADER    bmiV4Header;
        BITMAPINFOHEADER  bmiHeader;
    };
    DWORD bmiColors[4];
}   BITMAPINFOEX, *LPBITMAPINFOEX;

BOOL IsOSAlphaSupport(){
    DWORD dwVer = ::GetVersion();
    DWORD isNT = !(dwVer & 0x80000000);
    return !(!isNT && HIBYTE(LOWORD(dwVer)) < 10) //not before WIN 98
        && !(isNT && LOBYTE(LOWORD(dwVer)) < 5);  //not before WIN 2000
}

static BOOL g_bOSAlphaSupport = IsOSAlphaSupport();

void CDCHolder::CreateAlphaImageIfCan()
{
    if(NULL==m_pPoints || !g_bOSAlphaSupport)
        return;

    if( NULL!=m_hOldBitmap ){
        m_hBitmap = (HBITMAP)::SelectObject(m_hMemoryDC, m_hOldBitmap);
        m_hOldBitmap = NULL;
    } 

    BITMAPINFOEX    bmi = {0};
    bmi.bmiV4Header.bV4Width = m_iWidth;
    bmi.bmiV4Header.bV4Height = -m_iHeight;
    bmi.bmiV4Header.bV4Planes = 1;
    bmi.bmiV4Header.bV4BitCount = 32;

    bmi.bmiV4Header.bV4Size = sizeof(BITMAPV4HEADER);
    bmi.bmiV4Header.bV4V4Compression = BI_BITFIELDS;
    bmi.bmiV4Header.bV4XPelsPerMeter = 72;
    bmi.bmiV4Header.bV4YPelsPerMeter = 72;
    bmi.bmiV4Header.bV4RedMask   = 0x00FF0000;
    bmi.bmiV4Header.bV4GreenMask = 0x0000FF00;
    bmi.bmiV4Header.bV4BlueMask  = 0x000000FF;
    bmi.bmiV4Header.bV4AlphaMask = 0xFF000000;

    OLE_TRY
    m_hAlphaBitmap = CreateDIBitmap(
        m_hMemoryDC, 
        (BITMAPINFOHEADER*)&bmi,
        CBM_INIT,
        (void *)m_pPoints,
        (BITMAPINFO*)&bmi,
        DIB_RGB_COLORS);
    if( !m_hAlphaBitmap ){
        OLE_THROW_LASTERROR(_T("CreateDIBitmap"))
    } 
    OLE_CATCH
}

HBITMAP CDCHolder::CreateJavaContextBitmap(
    HDC hdc,
    int iWidth,
    int iHeight,
    void **ppPoints)
{
    BITMAPINFO    bitmapInfo = {0};
    bitmapInfo.bmiHeader.biWidth = iWidth;
    bitmapInfo.bmiHeader.biHeight = -iHeight;
    bitmapInfo.bmiHeader.biPlanes = 1;
    bitmapInfo.bmiHeader.biBitCount = 32;
    bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmapInfo.bmiHeader.biCompression = BI_RGB;

    return ::CreateDIBSection(
        hdc,
        (BITMAPINFO *)&bitmapInfo,
        DIB_RGB_COLORS,
        (void **)ppPoints,
        NULL,
        0
    );
}
