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

//
//  JWebKit.h
//  JWebKit
//

#import <Foundation/Foundation.h>
#import <JavaVM/jni.h>
#import <JavaVM/AWTCocoaComponent.h>
#import <AppKit/AppKit.h>
#import <WebKit/WebView.h>
#import <WebKit/WebFrame.h>
#import <WebKit/WebPreferences.h>
#import <WebKit/WebDocument.h>
#import <WebKit/WebPolicyDelegate.h>

#ifdef __cplusplus
extern "C" {
#endif
    
    JNIEXPORT jlong JNICALL Java_com_frostwire_gui_webbrowser_WebKitComponent_createNSView1
    (JNIEnv *, jobject);
    
#ifdef __cplusplus
}
#endif

enum {
	JWebKit_loadURL         = 1,
    JWebKit_loadHTMLString  = 2,
    JWebKit_stopLoading     = 3,
    JWebKit_reload          = 4,
    JWebKit_goBack          = 5,
    JWebKit_goForward       = 6,
    JWebKit_runJS           = 7,
    JWebKit_dispose         = 8
};

@interface JWebKit : WebView<AWTCocoaComponent> {
@private
    jobject jowner;
    NSView* html;
}

- (id) initWithFrame: (jobject) owner frame:(NSRect) frame;

-(void)startLoading : (NSString *)url;
-(void)finishLoading;

-(NSString*) callJava: (NSString *)function data:(NSString *)data;

@end
