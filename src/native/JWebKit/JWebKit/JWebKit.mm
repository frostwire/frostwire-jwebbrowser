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
//  JWebKit.mm
//  JWebKit
//

#import "JWebKit.h"

#define JRISigArray(T)		"[" T
#define JRISigByte			"B"
#define JRISigChar			"C"
#define JRISigClass(name)	"L" name ";"
#define JRISigFloat			"F"
#define JRISigDouble		"D"
#define JRISigMethod(args)	"(" args ")"
#define JRISigNoArgs		""
#define JRISigInt			"I"
#define JRISigLong			"J"
#define JRISigShort			"S"
#define JRISigVoid			"V"
#define JRISigBoolean		"Z"

JavaVM* jvm = NULL;

jint GetJNIEnv(JNIEnv **env, bool *mustDetach);

@implementation JWebKit

- (id) initWithFrame: (jobject) owner frame:(NSRect) frame
{
	self = [super initWithFrame:frame frameName:nil groupName:nil];
    jowner = owner;
    html = nil;
    
    [self setFrameLoadDelegate:self];
    [self setUIDelegate:self];
    [self setDownloadDelegate:self];
    //[self setPolicyDelegate:self];
    //[self setResourceLoadDelegate:self];
	
    return self;
}

-(void)awtMessage:(jint)messageID message:(jobject)message env:(JNIEnv*)env
{
    WebFrame* webFrame = NULL;
    
    switch (messageID) {
        case JWebKit_loadURL:
            if(message != nil && env != nil) {
                const jchar *chars = env->GetStringChars((jstring)message, NULL);
                NSString *str = [NSString stringWithCharacters:(UniChar *)chars length: env->GetStringLength((jstring)message)];
                NSURL *url = [NSURL URLWithString : str];
                NSURLRequest *request = [NSURLRequest requestWithURL : url];
                WebFrame *frame = [self mainFrame];
                [frame loadRequest:request];
                env->ReleaseStringChars((jstring)message, chars);
            }
    		break;
        case JWebKit_loadHTMLString:
    	    if(message != nil && env != nil){
                if (!env->IsInstanceOf(message,env->FindClass(JRISigArray(JRISigClass("java/lang/String"))))) {
                    break;
                }
                int objs_length = env->GetArrayLength((jobjectArray)message);
                if(objs_length != 2) {
                    break;
                }
                jstring jHTML = (jstring)env->GetObjectArrayElement((jobjectArray)message, 0);
                if(jHTML == NULL) {
                    return;
                }
                jstring jBaseURL = (jstring)env->GetObjectArrayElement((jobjectArray)message, 1);
                const char *cHTML = env->GetStringUTFChars(jHTML,0);
                const char *cBaseURL = (jBaseURL != NULL)?env->GetStringUTFChars(jBaseURL,0) : NULL;
                NSString *nsHTML = [NSString stringWithUTF8String : cHTML];
                NSURL *nsBaseURL = NULL;
                if(cBaseURL != NULL){
                    nsBaseURL = [NSURL URLWithString : [NSString stringWithUTF8String : cBaseURL]];
                }
                WebFrame *webFrame = [self mainFrame];
                [webFrame loadHTMLString : nsHTML baseURL : nsBaseURL];
                env->ReleaseStringUTFChars(jHTML,cHTML);
                if(jBaseURL != NULL) env->ReleaseStringUTFChars(jBaseURL,cBaseURL);
            }
            break;
        case JWebKit_stopLoading:
            webFrame = [self mainFrame];
            [webFrame stopLoading];
            break;
        case JWebKit_reload:
            webFrame = [self mainFrame];
            [webFrame reload];
            break;
        case JWebKit_goBack:
            webFrame = [self mainFrame];
            [[webFrame webView] goBack];
            break;
        case JWebKit_goForward:
            webFrame = [self mainFrame];
            [[webFrame webView] goForward];
            break;
        case JWebKit_runJS:
        {
            if (!env->IsInstanceOf(message, env->FindClass(JRISigClass("java/lang/String")))){
                break;
            }
            const jchar *chars = env->GetStringChars((jstring)message, NULL);
            NSString *jsString = [NSString stringWithCharacters:(UniChar *)chars length: env->GetStringLength((jstring)message)];
            WebView *webView = [[self mainFrame] webView];
            [webView stringByEvaluatingJavaScriptFromString:jsString];
            env->ReleaseStringChars((jstring)message, chars);
            break;
        }
        case JWebKit_dispose:
            if(jowner != NULL){
                env->DeleteGlobalRef(jowner);
                jowner = NULL;
            }
            break;
        default:
            fprintf(stderr, "JWebKit Error : Unknown message received (%d)\n", (int) messageID);
            break;
    }
}

-(void)startLoading : (NSString *)url
{
	if (jowner == NULL || jvm == NULL) {
        return;
    }
    
    JNIEnv *env = NULL;
	bool shouldDetach = false;
	
	if (GetJNIEnv(&env, &shouldDetach) != 0) {
		NSLog(@"JWebKit: could not attach to JVM");
		return;
	}
    
    if (env == NULL) {
        return;
    }
    
	jclass clazz = env->GetObjectClass(jowner);
	if (clazz != NULL) {
	    jmethodID mID = env->GetMethodID(clazz, "startLoading", JRISigMethod(JRISigClass("java/lang/String")) JRISigVoid); 
	    if (mID != NULL) {
	        jstring jstr = env->NewStringUTF([url UTF8String]);
	        env->CallVoidMethod(jowner, mID, jstr);
            env->DeleteLocalRef(jstr);
	    }
	}
    
	if (shouldDetach) {
        jvm->DetachCurrentThread();
    }
}

-(void)finishLoading
{
    if (jowner == NULL || jvm == NULL) {
        return;
    }
    
    JNIEnv *env = NULL;
	bool shouldDetach = false;
	
	if (GetJNIEnv(&env, &shouldDetach) != 0) {
		NSLog(@"JWebKit: could not attach to JVM");
		return;
	}
    
    if (env == NULL) {
        return;
    }
    
    jclass clazz = env->GetObjectClass(jowner);
    if (clazz != NULL) {
        jmethodID mID = env->GetMethodID(clazz, "finishLoading", JRISigMethod() JRISigVoid); 
        if (mID != NULL){
            env->CallVoidMethod(jowner, mID);
        }
    }
    
    if (shouldDetach) {
        jvm->DetachCurrentThread();
    }
    
    [self setNeedsDisplay:TRUE];
}

- (NSString*) callJava: (NSString *)function data:(NSString *)data
{
    if (jowner == NULL || jvm == NULL) {
        return nil;
    }
    
    JNIEnv *env = NULL;
	bool shouldDetach = false;
	
	if (GetJNIEnv(&env, &shouldDetach) != 0) {
		NSLog(@"JWebKit: could not attach to JVM");
		return nil;
	}
    
    if (env == NULL) {
        return nil;
    }
    
    NSString *result = nil;
    
	jclass clazz = env->GetObjectClass(jowner);
	if (clazz != NULL) {
	    jmethodID mID = env->GetMethodID(clazz, "callJava", JRISigMethod(JRISigClass("java/lang/String") JRISigClass("java/lang/String")) JRISigClass("java/lang/String")); 
	    if (mID != NULL) {
	        jstring jstrFunction = env->NewStringUTF([function UTF8String]);
	        jstring jstrData = env->NewStringUTF([data UTF8String]);
            jstring jstrResult = (jstring) env->CallObjectMethod(jowner, mID, jstrFunction, jstrData);
            if (jstrResult != nil) {
                const jchar *chars = env->GetStringChars((jstring)jstrResult, NULL);
                result = [NSString stringWithCharacters:(UniChar *)chars length: env->GetStringLength((jstring)jstrResult)];
                env->ReleaseStringChars((jstring)jstrResult, chars);
            }
            env->DeleteLocalRef(jstrFunction);
            env->DeleteLocalRef(jstrData);
	    }
	}
    
	if (shouldDetach) {
        jvm->DetachCurrentThread();
    }
    
    return result;
}

- (void) mouseMoved: (NSEvent *) theEvent
{
	[super mouseMoved:theEvent];
    
    WebView* webView = [[self mainFrame] webView];
    if (webView != nil){
        [[webView window] mouseMoved:theEvent];
    }
}

//WebFrameLoadDelegate
- (void)webView:(WebView *)sender didStartProvisionalLoadForFrame:(WebFrame *)frame
{
    if (jowner == NULL || jvm == NULL) {
        return;
    }
    
    if (frame == [sender mainFrame]){
        [self startLoading : [[[[frame provisionalDataSource] request] URL] absoluteString]];
    }
}

- (void)webView:(WebView *)sender didFinishLoadForFrame:(WebFrame *)frame
{
    if (jowner == NULL || jvm == NULL) {
        return;
    }
    
    if (frame == [sender mainFrame]){
        [self finishLoading];
    }
}

- (void)webView:(WebView *)webView windowScriptObjectAvailable:(WebScriptObject *)windowScriptObject
{
    [windowScriptObject setValue:self forKey:@"jbrowser"];
}

//WebUIDelegate
- (void)webView:(WebView *)sender runJavaScriptAlertPanelWithMessage:(NSString *)message initiatedByFrame:(WebFrame *)frame
{
    if (jowner == NULL || jvm == NULL) {
        return;
    }
    
    JNIEnv *env = NULL;
	bool shouldDetach = false;
	
	if (GetJNIEnv(&env, &shouldDetach) != 0) {
		NSLog(@"JWebKit: could not attach to JVM");
		return;
	}
    
    if (env == NULL) {
        return;
    }
    
    BOOL doAlert = TRUE;
    jclass ownerClazz = env->GetObjectClass(jowner);
    if(ownerClazz == NULL){
        if (shouldDetach) {
            jvm->DetachCurrentThread();
        }
        return;
    }
    
    jmethodID mID = env->GetMethodID(ownerClazz, "runJavaScriptAlertPanelWithMessage", JRISigMethod(JRISigClass("java/lang/String")) JRISigBoolean); 
    if(mID == NULL){
        if (shouldDetach) {
            jvm->DetachCurrentThread();
        }
        return;
    }
    jstring jstr1 = (message == NULL) ? NULL : env->NewStringUTF([message UTF8String]);
    doAlert = env->CallBooleanMethod(jowner, mID, jstr1);
    env->DeleteLocalRef(jstr1);
    
    if (shouldDetach) {
        jvm->DetachCurrentThread();
    }
    
    if(doAlert){
        NSRunInformationalAlertPanel(@"JavaScript", message, @"OK", nil, nil);
    }
}
- (BOOL)webView:(WebView *)sender runJavaScriptConfirmPanelWithMessage:(NSString *)message initiatedByFrame:(WebFrame *)frame
{
    return FALSE;
}

- (NSString *)webView:(WebView *)sender runJavaScriptTextInputPanelWithPrompt:(NSString *)prompt defaultText:(NSString *)defaultText initiatedByFrame:(WebFrame *)frame
{
    return NULL;
}

- (NSArray *)webView:(WebView *)sender contextMenuItemsForElement:(NSDictionary *)element 
    defaultMenuItems:(NSArray *)defaultMenuItems
{
    // disable right-click context menu
    return nil;
}

//WebDownload
- (void)downloadDidBegin:(NSURLDownload *)download
{
}


- (void)download:(NSURLDownload *)download decideDestinationWithSuggestedFilename:(NSString *)filename
{
    NSString *destinationFilename;
    NSString *homeDirectory = NSHomeDirectory();
    
    destinationFilename = [[homeDirectory stringByAppendingPathComponent:@"Desktop"]
                           stringByAppendingPathComponent:filename];
    [download setDestination:destinationFilename allowOverwrite:NO];
}


- (void)download:(NSURLDownload *)download didCreateDestination:(NSString *)path
{
}

- (void)download:(NSURLDownload *)download didReceiveResponse:(NSURLResponse *)response
{
}

- (void)download:(NSURLDownload *)download didReceiveDataOfLength:(unsigned)length
{
}

- (BOOL)download:(NSURLDownload *)download shouldDecodeSourceDataOfMIMEType:(NSString *)encodingType;
{
    return FALSE;
}

- (void)downloadDidFinish:(NSURLDownload *)download
{
    // Release the download.
    [download release];
    
    // Do something with the data.
    NSLog(@"%@",@"downloadDidFinish");
}


- (void)download:(NSURLDownload *)download didFailWithError:(NSError *)error
{
    // Release the download.
    [download release];
    
    // Inform the user.
    NSLog(@"Download failed! Error - %@ %@",
          [error localizedDescription],
          [[error userInfo] objectForKey:NSURLErrorFailingURLStringErrorKey]);
}

// JS interface
+ (BOOL)isSelectorExcludedFromWebScript:(SEL)selector {
    if (selector == @selector(callJava:data:)) {
        return NO;
    }
    
    return YES;
}

+ (BOOL)isKeyExcludedFromWebScript:(const char *)property {
    return YES;
}

+ (NSString *) webScriptNameForSelector:(SEL)sel {
    if (sel == @selector(callJava:data:)) {
        return @"callJava";
    } else {
        return nil;
    }
}

@end

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    jvm = vm;
    return JNI_VERSION_1_4;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *reserved) {
}

JNIEXPORT jlong JNICALL Java_com_frostwire_gui_webbrowser_WebKitComponent_createNSView1
(JNIEnv *env, jobject obj) {
    JWebKit* view = nil;
    NS_DURING;
    int width = 200;
    int height = 200;
    
    jobject jowner = env->NewGlobalRef(obj);
    view = [[JWebKit alloc] initWithFrame : jowner frame:NSMakeRect(0, 0, width, height)];
    
    NS_HANDLER;
    fprintf(stderr, "ERROR : Failed to create WebKit view\n");
    NS_VALUERETURN(0, jlong);
    NS_ENDHANDLER;
    
    return (jlong) view;
}

jint GetJNIEnv(JNIEnv **env, bool *mustDetach) {
	jint getEnvErr = JNI_OK;
	*mustDetach = false;
	if (jvm) {
		getEnvErr = jvm->GetEnv((void **)env, JNI_VERSION_1_4);
		if (getEnvErr == JNI_EDETACHED) {
			getEnvErr = jvm->AttachCurrentThread((void **)env, NULL);
			if (getEnvErr == JNI_OK) {
				*mustDetach = true;
			}
		}
	}
	return getEnvErr;
}