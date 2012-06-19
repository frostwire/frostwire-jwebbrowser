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

import java.awt.Component;

/**
 * @author aldenml
 *
 */
public interface WebBrowser {

    public Component getComponent();

    public WebBrowserListener getListener();

    public void setListener(WebBrowserListener listener);

    public void go(String url);

    public void back();

    public void forward();

    public void reload();

    public void stop();

    public void runJS(String code);

    public void function(BrowserFunction fn);
}
