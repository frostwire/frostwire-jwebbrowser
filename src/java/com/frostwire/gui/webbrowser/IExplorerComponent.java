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

package com.frostwire.gui.webbrowser;

import java.awt.Canvas;
import java.awt.Component;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.KeyboardFocusManager;
import java.awt.peer.ComponentPeer;
import java.util.HashMap;
import java.util.Map;

import javax.swing.FocusManager;
import javax.swing.SwingUtilities;

import sun.awt.windows.WComponentPeer;

/**
 * @author aldenml
 *
 */
public class IExplorerComponent extends Canvas implements WebBrowser {

    private static final long serialVersionUID = -7081910735920998419L;

    /**
     * DISPID_XXXX constats from MSDN
     */
    public static final int DISPID_BEFORENAVIGATE = 100; // this is sent before navigation to give a chance to abort
    public static final int DISPID_NAVIGATECOMPLETE = 101; // in async, this is sent when we have enough to show
    public static final int DISPID_STATUSTEXTCHANGE = 102;
    public static final int DISPID_QUIT = 103;
    public static final int DISPID_DOWNLOADCOMPLETE = 104;
    public static final int DISPID_COMMANDSTATECHANGE = 105;
    public static final int DISPID_DOWNLOADBEGIN = 106;
    public static final int DISPID_NEWWINDOW = 107; // sent when a new window should be created
    public static final int DISPID_PROGRESSCHANGE = 108; // sent when download progress is updated
    public static final int DISPID_WINDOWMOVE = 109; // sent when main window has been moved
    public static final int DISPID_WINDOWRESIZE = 110; // sent when main window has been sized
    public static final int DISPID_WINDOWACTIVATE = 111; // sent when main window has been activated
    public static final int DISPID_PROPERTYCHANGE = 112; // sent when the PutProperty method is called
    public static final int DISPID_TITLECHANGE = 113; // sent when the document title changes
    public static final int DISPID_TITLEICONCHANGE = 114; // sent when the top level window icon may have changed.
    public static final int DISPID_FRAMEBEFORENAVIGATE = 200;//frameBeforeNavigate
    public static final int DISPID_FRAMENAVIGATECOMPLETE = 201;//frameNavigateComplete
    public static final int DISPID_FRAMENEWWINDOW = 204;//frameNewWindow

    public static final int DISPID_BEFORENAVIGATE2 = 250;//beforeNavigate2")},// hyperlink clicked on
    public static final int DISPID_NEWWINDOW2 = 251;//newWindow2")},
    public static final int DISPID_NAVIGATECOMPLETE2 = 252;//navigateComplete2")},// UIActivate new document
    public static final int DISPID_ONQUIT = 253;//onQuit")},
    public static final int DISPID_ONVISIBLE = 254;//onVisible")},// sent when the window goes visible/hidden
    public static final int DISPID_ONTOOLBAR = 255;//onToolbar")},// sent when the toolbar should be shown/hidden
    public static final int DISPID_ONMENUBAR = 256;//ONMENUBAR")},// sent when the menubar should be shown/hidden
    public static final int DISPID_ONSTATUSBAR = 257;//ONSTATUSBAR")},// sent when the statusbar should be shown/hidden
    public static final int DISPID_ONFULLSCREEN = 258;//ONFULLSCREEN")},// sent when kiosk mode should be on/off
    public static final int DISPID_DOCUMENTCOMPLETE = 259;//DOCUMENTCOMPLETE")},// new document goes ReadyState_Complete
    public static final int DISPID_ONTHEATERMODE = 260;//ONTHEATERMODE")},// sent when theater mode should be on/off
    public static final int DISPID_ONADDRESSBAR = 261;//ONADDRESSBAR")},// sent when the address bar should be shown/hidden
    public static final int DISPID_WINDOWSETRESIZABLE = 262;//WINDOWSETRESIZABLE")},// sent to set the style of the host window frame
    public static final int DISPID_WINDOWCLOSING = 263;//WINDOWCLOSING")},// sent before script window.close closes the window
    public static final int DISPID_WINDOWSETLEFT = 264;//WINDOWSETLEFT")},// sent when the put_left method is called on the WebOC
    public static final int DISPID_WINDOWSETTOP = 264;//WINDOWSETTOP")},// sent when the put_top method is called on the WebOC
    public static final int DISPID_WINDOWSETWIDTH = 266;//WINDOWSETWIDTH")},// sent when the put_width method is called on the WebOC
    public static final int DISPID_WINDOWSETHEIGHT = 267;//WINDOWSETHEIGHT")},// sent when the put_height method is called on the WebOC
    public static final int DISPID_CLIENTTOHOSTWINDOW = 268;//CLIENTTOHOSTWINDOW")},// sent during window.open to request conversion of dimensions
    public static final int DISPID_SETSECURELOCKICON = 269;//SETSECURELOCKICON")},// sent to suggest the appropriate security icon to show
    public static final int DISPID_FILEDOWNLOAD = 270;//FILEDOWNLOAD")},// Fired to indicate the File Download dialog is opening
    public static final int DISPID_NAVIGATEERROR = 271;//NAVIGATEERROR")},// Fired to indicate the a binding error has occured
    public static final int DISPID_PRIVACYIMPACTEDSTATECHANGE = 272;//PRIVACYIMPACTEDSTATECHANGE")},// Fired when the user's browsing experience is impacted
    // Printing events
    public static final int DISPID_PRINTTEMPLATEINSTANTIATION = 225;//PRINTTEMPLATEINSTANTIATION")},// Fired to indicate that a print template is instantiated
    public static final int DISPID_PRINTTEMPLATETEARDOWN = 226;//PRINTTEMPLATETEARDOWN")},// Fired to indicate that a print templete is completely gone
    public static final int DISPID_UPDATEPAGESTATUS = 227;//UPDATEPAGESTATUS")},// Fired to indicate that the spooling status has changed
    //staff
    public static final int DISPID_ONFOCUCHANGE = -1;
    public static final int DISPID_ONFOCUSMOVE = -2;
    public static final int DISPID_REFRESH = -3;

    private final Map<String, BrowserFunction> functions;

    long data = 0;

    private int notifyCounter = 0;
    private boolean isFocusOwner = false;

    private String url;
    private WebBrowserListener listener;

    public IExplorerComponent() {
        functions = new HashMap<String, BrowserFunction>();
        setMinimumSize(new Dimension(25, 25));
    }

    /**
     * Makes this Component displayable by connecting it to a
     * native screen resource.
     * This method is called internally by the toolkit and should
     * not be called directly by programs.
     * @see       #removeNotify
     */
    @Override
    public void addNotify() {
        super.addNotify();
        onAddNotify();
        if (url != null && data != 0) {
            nativeGo(url);
        }
    }

    /**
     * Removes the <code>BrComponent</code>'s peer.
     * The peer allows us to modify the appearance of the
     * <code>BrComponent</code> without changing its
     * functionality.
     */
    @Override
    public void removeNotify() {
        onRemoveNotify();
        super.removeNotify();
    }

    /**
     * Makes the component visible or invisible.  
     * @param aFlag  <code>true</code> to make the component visible; 
     * <code>false</code> to make it invisible
     */
    @Override
    public void setVisible(boolean aFlag) {
        super.setVisible(aFlag);
        if (data != 0) {
            nativeSetVisible(isVisible());
        }
    }

    /**
     * Sets whether or not this component is enabled. A component that is 
     * enabled may respond to user input, while a component that is not enabled 
     * cannot respond to user input. Some components may alter their visual 
     * representation when they are disabled in order to provide feedback to 
     * the user that they cannot take input. <br/>
     * Note: Disabling a component does not disable its children.<br/>
     * @param enabled <code>true</code> if this component should be enabled, 
     * <code>false</code> otherwise
     */
    @Override
    public void setEnabled(boolean enabled) {
        super.setVisible(enabled);
        if (data != 0) {
            nativeSetEnabled(isEnabled());
        }
    }

    @Override
    public void setBounds(int x, int y, int width, int height) {
        super.setBounds(x, y, width, height);

        if (data != 0) {
            nativeSetBounds();
        }
    }

    @Override
    public void paint(Graphics g) {
    }

    @Override
    public boolean hasFocus() {
        return isFocusOwner;
    }

    @Override
    public Component getComponent() {
        return this;
    }

    @Override
    public WebBrowserListener getListener() {
        return listener;
    }

    @Override
    public void setListener(WebBrowserListener listener) {
        this.listener = listener;
    }

    @Override
    public void go(String url) {
        this.url = url;
        if (data != 0) {
            if (url != null)
                nativeGo(url);
            else {
                nativeGo("about:blank");
            }
        }
    }

    @Override
    public void back() {
        if (data != 0) {
            nativeBack();
        }
    }

    @Override
    public void forward() {
        if (data != 0) {
            nativeForward();
        }
    }

    @Override
    public void reload() {
        if (data != 0) {
            nativeReload();
        }
    }

    @Override
    public void stop() {
        if (data != 0) {
            nativeStop();
        }
    }

    @Override
    public void runJS(String code) {
        if (data != 0) {
            nativeRunJS(code);
        }
    }

    @Override
    public void function(BrowserFunction fn) {
        functions.put(fn.getName(), fn);
        runJS(fn.createJS());
    }

    public String callJava(String function, String data) {
        if (functions.containsKey(function)) {
            return functions.get(function).run(data);
        } else {
            return null;
        }
    }

    private void onRemoveNotify() {
        --notifyCounter;
        if (0 == notifyCounter) {
            if (0 != data) {
                destroy();
                data = 0;
            }
        }
    }

    private void onAddNotify() {
        if (0 == notifyCounter) {
            initIDs();

            for (Container c = getParent(); null != c; c = c.getParent()) {
                @SuppressWarnings("deprecation")
                ComponentPeer cp = getPeer();
                if ((cp instanceof WComponentPeer)) {
                    data = create(((WComponentPeer) cp).getHWnd());
                    break;
                }
            }
        }
        ++notifyCounter;
    }

    /**
     * Synchronous callback for notifications from native code.
     * @param iId the event identifier - <code>BrComponentEvent.DISPID_XXXX</code> const
     * @param stName the event name of (optional)
     * @param stValue the event paramenter(s) (optional)
     * @return the application respont
     */
    private String postEvent(int iId, String stName, String stValue) {
        switch (iId) {
        case DISPID_REFRESH:
            break;
        case DISPID_DOCUMENTCOMPLETE:
            //            if(isEditable()!=editable){
            //                //System.out.println("setEditable(" + editable + ");");
            //                enableEditing(editable);
            //            } else if(editable) {
            //                //System.out.println("refreshHard");
            //                refreshHard();
            //            }
            //            documentReady = true;
            if (listener != null) {
                listener.onComplete(this, url);
            }
            break;
        case DISPID_NAVIGATECOMPLETE2:
            break;
        case DISPID_ONFOCUCHANGE:
            isFocusOwner = Boolean.parseBoolean(stValue);
            break;
        case DISPID_ONFOCUSMOVE:
            focusMove(Boolean.parseBoolean(stValue));
            break;
        case DISPID_PROGRESSCHANGE:
            break;
        }
        //return processBrComponentEvent(new BrComponentEvent(this, iId, stName, stValue));

        return "";
    }

    private void focusMove(final boolean bNext) {
        SwingUtilities.invokeLater(new Runnable() {
            public void run() {
                KeyboardFocusManager fm = FocusManager.getCurrentKeyboardFocusManager();
                if (bNext) {
                    fm.focusNextComponent(IExplorerComponent.this);
                } else {
                    fm.focusPreviousComponent(IExplorerComponent.this);
                }
            }
        });
    }

    private static native void initIDs();

    private native long create(long hwnd);

    private native void destroy();

    private native void nativeSetEnabled(boolean enabled);

    private native void nativeSetVisible(boolean visible);

    private native void nativeGo(String url);

    private native void nativeBack();

    private native void nativeForward();

    private native void nativeReload();

    private native void nativeStop();

    private native void nativeRunJS(String code);

    private native void nativeSetBounds();

    /**
     * Internal browser event processor. Converts some events to 
     * <code>BrComponentListener</code> inteface callbacks and property-changed 
     * notifications.
     * @param e the happened browser event
     */
    /*
    public String processBrComponentEvent(final BrComponentEvent e) {
        //System.out.println( "IEE:" + e.getID() + " Name:" + e.getName() + " Value:"+ e.getValue() ); 
        String res = null;
        BrComponentListener listener = null;//ieListener;
        if (listener != null) {
            res = listener.sync(e);
        }
        SwingUtilities.invokeLater(new Runnable() {
            public void run() {
                //System.out.println(e);
                String stValue = e.getValue();
                stValue = (null == stValue) ? "" : stValue;
                switch (e.getID()) {
                case BrComponentEvent.DISPID_STATUSTEXTCHANGE:
                    //setStatusText(stValue);
                    break;
                case BrComponentEvent.DISPID_BEFORENAVIGATE2:
                    break;
                case BrComponentEvent.DISPID_NAVIGATECOMPLETE2:
                    //setNavigatedURL(stValue);
                    break;
                case BrComponentEvent.DISPID_DOWNLOADCOMPLETE:
                    //setDocumentReady(true);
                    break;
                case BrComponentEvent.DISPID_PROGRESSCHANGE:
                    //setProgressBar(stValue);
                    break;
                case BrComponentEvent.DISPID_SETSECURELOCKICON:
                    //setSecurityIcon(stValue);
                    break;
                case BrComponentEvent.DISPID_TITLECHANGE:
                    //setWindowTitle(stValue);
                    break;
                case BrComponentEvent.DISPID_COMMANDSTATECHANGE: {
                    String st[] = stValue.split(",");
                    boolean enable = (0 != Integer.valueOf(st[0]));
                    switch (Integer.valueOf(st[1])) {
                    case -1:
                        ///setToolbarChanged(enable);                            
                        break;
                    case 1:
                        //setGoForwardEnable(enable);                            
                        break;
                    case 2:
                        //setGoBackEnable(enable);                                                        
                        break;
                    }
                }
                    break;
                }
            }
        });//end posponed operation   
        return res;
    }*/
}
