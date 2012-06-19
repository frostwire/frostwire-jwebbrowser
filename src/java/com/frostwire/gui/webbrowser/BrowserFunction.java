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

/**
 * @author aldenml
 *
 */
public class BrowserFunction {

    private final String name;

    public BrowserFunction(String name) {
        this.name = name;
    }

    public String getName() {
        return name;
    }

    public String run(String data) {
        return null;
    }

    public String createJS() {
        StringBuilder sb = new StringBuilder("window.");
        sb.append(name);
        sb.append(" = function ");
        sb.append("(data) {return window.jbrowser.callJava('" + name + "', data);}");

        return sb.toString();
    }
}
