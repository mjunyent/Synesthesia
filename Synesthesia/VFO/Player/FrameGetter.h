//
//  FrameGetter.h
//  Synesthesia
//
//  Created by Marc Junyent Martín on 20/12/14.
//  Copyright (c) 2014 Marc Junyent Martín. All rights reserved.
//

#ifndef Synesthesia_FrameGetter_h
#define Synesthesia_FrameGetter_h

#include "Tobago/Tobago.h"
#include <string>

class FrameGetter {
public:
    virtual void load(std::string url) = 0;
    virtual void unload() = 0;

    virtual void update() = 0;

    virtual double getCurrentTime() = 0;
    virtual long getWidth() = 0;
    virtual long getHeight() = 0;
    virtual float getFrameRate() = 0;
    virtual unsigned long getFrameNum() = 0;

    virtual bool isReady() = 0;
    virtual bool isNewFrame() = 0;
    virtual bool isFinished() = 0;

    virtual unsigned char* getPixels() = 0;
    virtual Texture* getTexture() = 0;
};

#endif
