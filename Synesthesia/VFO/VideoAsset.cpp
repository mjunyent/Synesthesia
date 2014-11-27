//
//  VideoAsset.cpp
//  Synesthesia
//
//  Created by Marc Junyent Martín on 15/11/14.
//  Copyright (c) 2014 Marc Junyent Martín. All rights reserved.
//

#include "VideoAsset.h"

VideoAsset::VideoAsset(std::string url) {
    this->url = url;

    
    unsigned long bar = url.find_last_of("/\\");
    std::string fullname;

    if(bar == string::npos) {
        path = "";
        fullname = url;
    } else {
        path = url.substr(0,bar);
        fullname = url.substr(bar+1);
    }

    unsigned long dot = fullname.find_last_of(".");
    if(dot == string::npos) {
        name = fullname;
        extension = "";
    } else {
        name = fullname.substr(0, dot);
        extension = fullname.substr(dot+1);
    }
    
    std::cout << url << "\n" << path << "\n" << fullname << "\n" << name << "\n" << extension << "\n" << std::endl;
    
    
#ifdef __APPLE__
    player = new OSXPlayer();
#endif

    player->load(this->url);

    if(!player->isError() && player->isLoaded()) valid = false;
    else valid = true;
}


/*
 #include <sys/param.h>
 #include <CoreFoundation/CoreFoundation.h>

static void changeToResourcesDirectory(void)
{
    char resourcesPath[MAXPATHLEN];
    
    CFBundleRef bundle = CFBundleGetMainBundle();
    if (!bundle)
        return;
    
    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(bundle);
    
    CFStringRef last = CFURLCopyLastPathComponent(resourcesURL);
    if (CFStringCompare(CFSTR("Resources"), last, 0) != kCFCompareEqualTo)
    {
        CFRelease(last);
        CFRelease(resourcesURL);
        return;
    }
    
    CFRelease(last);
    
    if (!CFURLGetFileSystemRepresentation(resourcesURL,
                                          true,
                                          (UInt8*) resourcesPath,
                                          MAXPATHLEN))
    {
        CFRelease(resourcesURL);
        return;
    }
    
    CFRelease(resourcesURL);
    
    chdir(resourcesPath);
}*/
