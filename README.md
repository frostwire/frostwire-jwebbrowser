frostwire-jwebbrowser
==================

The ultimate (GPL) Web browser component for Java. Light, Simple.

Motivation:
 * No announcement from Sun/Oracle in years in respect with a Swing Web Browser component.
 * The only thing they have going is JavaFX, but you don't want to include JavaFX in your project.
 * The abandonment of JDIC.
 * HTML5 everywhere.

We dived into the JDIC code and rescued it to create a Web browser component for Java,
one that is light (less than 30Kb on MacOSX and 185Kb on Windows), best of all simple to use.


    WebBrowser browser = BrowserFactory.instance().createBrowser();
    browser.setListener(this); //See the WebBrowser interface for the available events.
    browser.go("http://www.google.com");

    someJPanel.add(browser.getComponent(), BorderLayout.CENTER);
    
    /** 
     * Register a Java function to invoke it from the JavaScript world.
     *
     * (Javascript functions must only receive a String parameter for now,
     * what we do is pass A json formatted string here)
     * /
    browser.function(new BrowserFunction("testFn") {
        @Override
        public String run(String data) {
            return "Callback: " + data;
        }
    });
    
    /** 
     * This is how you invoke the function from JavaScript:
     *   testFn(data);
     *  
     *  An alternate form of invoking this function would be:
     *    window.jbrowser.callJava("testFn",data);
     */
    
    //And this is how you invoke JavaScript from the Java World.
    browser.runJS("alert('test alert');");
    
