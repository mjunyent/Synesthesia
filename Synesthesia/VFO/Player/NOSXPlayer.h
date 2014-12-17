//
//  NOSXPlayer.h
//  Synesthesia
//
//  Created by Marc Junyent Martín on 13/12/14.
//  Copyright (c) 2014 Marc Junyent Martín. All rights reserved.
//

#ifndef Synesthesia_NOSXPlayer_h
#define Synesthesia_NOSXPlayer_h

#include <string>
#include <CoreMedia/CMTime.h>
#include <CoreMedia/CMTimeRange.h>
#include "Tobago.h"
#include <string>

class NOSXPlayerException : public exception {
public:
    NOSXPlayerException(std::string s) : s(s) { };
    std::string s;
    virtual const char* what() const throw() {
        return s.c_str();
    };
};

class NOSXPlayer {
public:
    NOSXPlayer();

    bool load(std::string url);
    void dealloc();
    void unload();

    bool createAssetReaderWithTimeRange(CMTimeRange timeRange);

    void updateToNextFrame();


    double getPosition();

    bool isReady;
    bool isFinished;
    bool isNewFrame;

    long currentFrame;

    size_t width;
    size_t height;
    float framerate;

private:
    void* asset;
    void* assetReader;
    void* assetReaderVideoTrackOutput;
    void* videoSampleBuffer;
    
    CMTime duration;
    CMTime videoSampleTime;
    CMTime videoSampleTimePrev;//maybe can be local variable in updatetonextframe.
    
    
};

#endif
