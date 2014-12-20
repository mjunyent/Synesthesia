//
//  OSXFrameGetter.h
//  Synesthesia
//
//  Created by Marc on 20/12/14.
//  Copyright (c) 2014 Marc Junyent Martín. All rights reserved.
//

#ifndef __Synesthesia__OSXFrameGetter__
#define __Synesthesia__OSXFrameGetter__

#include "Tobago.h"
#include <string>

class OSXFrameGetterException : public exception {
public:
    OSXFrameGetterException(std::string s) : s(s) { };
    std::string s;
    virtual const char* what() const throw() {
        return s.c_str();
    };
};

class OSXFrameGetter {
public:
    OSXFrameGetter();
    ~OSXFrameGetter();

    void load(std::string url);
    void unload();

    void update();
    
    bool enableTextureCache();

    double getCurrentTime();
    long getWidth();
    long getHeight();
    float getFrameRate();
    unsigned long getFrameNum();
    
    bool isReady();
    bool isNewFrame();
    bool isFinished();
    
    unsigned char* getPixels();
    Texture* getTexture();

    
private:
    void * frameGetter;

    GLubyte* pixels;
    Texture* texture;
    
    bool textureCacheEnabled;
    bool textureCacheSupported;
    bool updatePixels;
    bool updateTexture;

    void initTextureCache();
};

#endif /* defined(__Synesthesia__OSXFrameGetter__) */
