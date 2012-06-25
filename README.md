frostwire-jwebbrowser
==================

The ultimate (GPL) Web browser component for Java. Light, Simple.

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