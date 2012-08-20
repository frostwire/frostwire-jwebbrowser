/*
 * Created by Alden Torres (aldenml)
 * Copyright (c) 2011, 2012, FrostWire(R). All rights reserved.
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

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;

import javax.swing.GroupLayout;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.JTextField;
import javax.swing.JToolBar;
import javax.swing.SwingConstants;
import javax.swing.ToolTipManager;
import javax.swing.UIManager;
import javax.swing.WindowConstants;

import com.frostwire.gui.webbrowser.BrowserFactory;
import com.frostwire.gui.webbrowser.BrowserFunction;
import com.frostwire.gui.webbrowser.WebBrowser;
import com.frostwire.gui.webbrowser.WebBrowserListener;

/**
 * Sample browser implementation.
 * 
 * @author aldenml
 *
 */
public class BrowserFrame extends JFrame implements WebBrowserListener {

    private static final long serialVersionUID = -7065448910990348234L;

    private JButton buttonBack;
    private JButton buttonForward;
    private JButton buttonGo;
    private JButton buttonReload;
    private JButton buttonStop;

    private JTextField textAddress;
    private JMenu menuFile;
    private JToolBar toolBarMain;
    private JPanel panelMain;
    private JLabel labelUrl;
    private JMenuBar menuBarMain;
    private JMenuItem menuExit;
    private JMenu menuTools;
    private JMenuItem menuRunJS;
    private JMenuItem menuRunJS2;

    private WebBrowser browser;

    public BrowserFrame() {
        initComponents();
    }

    private void initComponents() {
        panelMain = new JPanel();
        toolBarMain = new JToolBar();
        buttonBack = new JButton();
        buttonForward = new JButton();
        buttonReload = new JButton();
        buttonStop = new JButton();
        labelUrl = new JLabel();
        textAddress = new JTextField();
        buttonGo = new JButton();
        menuBarMain = new JMenuBar();
        menuFile = new JMenu();
        menuExit = new JMenuItem();
        menuTools = new JMenu();

        addWindowListener(new WindowAdapter() {
            public void windowClosing(WindowEvent e) {
                System.exit(0);
            }
        });
        setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
        setIconImage(new ImageIcon(getClass().getResource("/images/reload.png")).getImage());

        panelMain.setLayout(new BorderLayout());

        browser = BrowserFactory.instance().createBrowser();
        browser.setListener(this);
        browser.go("http://www.google.com");

        panelMain.add(browser.getComponent(), BorderLayout.CENTER);

        toolBarMain.setRollover(true);

        buttonBack.setIcon(new ImageIcon(getClass().getResource("/images/back.png")));
        buttonBack.setToolTipText("Go back one page");
        buttonBack.setFocusable(false);
        buttonBack.setHorizontalTextPosition(SwingConstants.CENTER);
        buttonBack.setVerticalTextPosition(SwingConstants.BOTTOM);

        buttonBack.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                buttonBack_actionPerformed(evt);
            }
        });
        toolBarMain.add(buttonBack);

        buttonForward.setIcon(new ImageIcon(getClass().getResource("/images/forward.png")));
        buttonForward.setToolTipText("Go forward one page");
        buttonForward.setFocusable(false);
        buttonForward.setHorizontalTextPosition(SwingConstants.CENTER);
        buttonForward.setVerticalTextPosition(SwingConstants.BOTTOM);

        buttonForward.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                buttonForward_actionPerformed(evt);
            }
        });
        toolBarMain.add(buttonForward);

        buttonReload.setIcon(new ImageIcon(getClass().getResource("/images/reload.png")));
        buttonReload.setToolTipText("Reload current page");
        buttonReload.setFocusable(false);
        buttonReload.setHorizontalTextPosition(SwingConstants.CENTER);
        buttonReload.setVerticalTextPosition(SwingConstants.BOTTOM);
        buttonReload.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                buttonReload_actionPerformed(evt);
            }
        });
        toolBarMain.add(buttonReload);

        buttonStop.setIcon(new ImageIcon(getClass().getResource("/images/stop.png")));
        buttonStop.setToolTipText("Stop current page");
        buttonStop.setFocusable(false);
        buttonStop.setHorizontalTextPosition(SwingConstants.CENTER);
        buttonStop.setVerticalTextPosition(SwingConstants.BOTTOM);
        buttonStop.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                buttonStop_actionPerformed(evt);
            }
        });
        toolBarMain.add(buttonStop);

        labelUrl.setText(" URL:");
        toolBarMain.add(labelUrl);

        textAddress.setText("http://");
        textAddress.setToolTipText("URL for navigation");
        textAddress.setPreferredSize(new Dimension(400, 20));
        textAddress.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                textAddress_actionPerformed(evt);
            }
        });
        toolBarMain.add(textAddress);

        buttonGo.setText("Go");
        buttonGo.setToolTipText("Go to entered URL");
        buttonGo.setFocusable(false);
        buttonGo.setHorizontalTextPosition(SwingConstants.CENTER);
        buttonGo.setVerticalTextPosition(SwingConstants.BOTTOM);
        buttonGo.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                textAddress_actionPerformed(evt);
            }
        });
        toolBarMain.add(buttonGo);

        panelMain.add(toolBarMain, BorderLayout.PAGE_START);

        menuFile.setText("File");
        menuFile.setToolTipText("File Operations");

        menuExit.setText("Exit");
        menuExit.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                menuExit_actionPerformed(evt);
            }
        });
        menuFile.add(menuExit);

        menuBarMain.add(menuFile);

        menuRunJS = new JMenuItem();
        menuRunJS.setText("Run JS");
        menuRunJS.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                menuRunJS_actionPerformed(e);
            }
        });

        menuRunJS2 = new JMenuItem();
        menuRunJS2.setText("Run JS 2");
        menuRunJS2.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                menuRunJS2_actionPerformed(e);
            }
        });

        menuTools.setText("Tools");
        menuTools.add(menuRunJS);
        menuTools.add(menuRunJS2);

        menuBarMain.add(menuTools);

        setJMenuBar(menuBarMain);

        GroupLayout layout = new GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(layout.createParallelGroup(GroupLayout.Alignment.LEADING).addComponent(panelMain, GroupLayout.Alignment.TRAILING, GroupLayout.DEFAULT_SIZE, 573, Short.MAX_VALUE));
        layout.setVerticalGroup(layout.createParallelGroup(GroupLayout.Alignment.LEADING).addComponent(panelMain, GroupLayout.Alignment.TRAILING, GroupLayout.DEFAULT_SIZE, 556, Short.MAX_VALUE));

        pack();
    }

    protected void menuRunJS_actionPerformed(ActionEvent e) {
        browser.runJS("alert('test alert');");
    }

    protected void menuRunJS2_actionPerformed(ActionEvent e) {
        browser.runJS("alert(testFn('hello'))");
    }

    protected void menuExit_actionPerformed(ActionEvent evt) {
        System.exit(0);
    }

    protected void textAddress_actionPerformed(ActionEvent evt) {
        browser.go(textAddress.getText());
    }

    protected void buttonBack_actionPerformed(ActionEvent evt) {
        browser.back();
    }

    protected void buttonForward_actionPerformed(ActionEvent evt) {
        browser.forward();
    }

    protected void buttonReload_actionPerformed(ActionEvent evt) {
        browser.reload();
    }

    protected void buttonStop_actionPerformed(ActionEvent evt) {
        browser.stop();
    }

    @Override
    public void onComplete(WebBrowser browser, String url) {
        browser.function(new BrowserFunction("testFn") {
            @Override
            public String run(String data) {
                return "Callback: " + data;
            }
        });
    }

    public static void main(String args[]) {
        System.setProperty("com.apple.eawt.CocoaComponent.CompatibilityMode", "false");
        System.setProperty("apple.laf.useScreenMenuBar", "true");
        System.setProperty("sun.awt.noerasebackground", "true");

        ToolTipManager.sharedInstance().setLightWeightPopupEnabled(false);
        JPopupMenu.setDefaultLightWeightPopupEnabled(false);

        try {
            UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
        } catch (Exception e) {
        }

        new BrowserFrame().setVisible(true);
    }
}
