#!/bin/bash
MACOS_FRAMEWORKS="-framework CoreFoundation -framework Foundation -framework AppKit -framework WebKit"
FLAGS="-dynamiclib -fPIC -mmacosx-version-min=10.11 -fno-strict-aliasing -fvisibility=default -D_DARWIN_C_SOURCE -Os -arch x86_64"

g++ ${FLAGS} ${MACOS_FRAMEWORKS} JWebKit.mm -o libJWebKit.dylib
