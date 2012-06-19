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

import java.awt.Toolkit;

/**
 * @author aldenml
 *
 */
public final class BrowserFactory {

    private static final String IEXPLORER_LIBRARY = "JIExplorer";
    private static final String WEBKIT_LIBRARY = "JWebKit";

    private static final String OS_NAME = System.getProperty("os.name");

    private static final boolean IS_OS_WINDOWS = isCurrentOS("Windows");

    private static final boolean IS_OS_MAC = isCurrentOS("Mac");

    private static boolean nativeLibLoaded = false;

    private static BrowserFactory instance;

    public static synchronized BrowserFactory instance() {
        if (instance == null) {
            instance = new BrowserFactory();
        }
        return instance;
    }

    private BrowserFactory() {
    }

    public WebBrowser createBrowser() {
        if (loadLibrary()) {
            if (IS_OS_WINDOWS) {
                return new IExplorerComponent();
            } else if (IS_OS_MAC) {
                return new WebKitComponent();
            }
        }

        return null;
    }

    private boolean loadLibrary() {
        if (!nativeLibLoaded) {
            try {
                //force loading of libjawt.so/jawt.dll
                Toolkit.getDefaultToolkit();

                if (IS_OS_WINDOWS) {
                    System.loadLibrary(IEXPLORER_LIBRARY);
                } else if (IS_OS_MAC) {
                    System.loadLibrary(WEBKIT_LIBRARY);
                }

                nativeLibLoaded = true;
            } catch (Throwable e) {
                e.printStackTrace();
            }
        }

        return nativeLibLoaded;
    }

    /**
     * Used to check whether the current operating system matches a operating
     * system name (osname).
     * 
     * @param osname
     *            Name of an operating system we are looking for as being part
     *            of the Sytem property os.name
     * @return true, if osname matches the current operating system,false if not
     *         and if osname is null
     */
    private static boolean isCurrentOS(String osname) {
        if (osname == null) {
            return false;
        } else {
            return (OS_NAME.indexOf(osname) >= 0);
        }
    }
}
