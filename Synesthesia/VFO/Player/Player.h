//
//  Player.h
//  Synesthesia
//
//  Created by Marc Junyent Martín on 12/11/14.
//  Copyright (c) 2014 Marc Junyent Martín. All rights reserved.
//

#ifndef __Synesthesia__Player__
#define __Synesthesia__Player__

#include <string>
#include "Tobago/Tobago.h"

class Player {
public:
    //Player() = 0;
    
    virtual void load(std::string url) = 0;
    virtual void loadAsync(std::string url) = 0;
    virtual void setAudioSampling(bool onoff) = 0;
    virtual void close() = 0;
    virtual void update() = 0;

    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;

    virtual bool isError() = 0;
    virtual bool isLoaded() = 0;
    virtual bool isPaused() = 0;
    virtual bool isPlaying() = 0;
    virtual bool isMovieDone() = 0;
    virtual bool isFrameNew() = 0;
    
    virtual float getPosition() = 0;
    virtual int getFrameNum() = 0;
    virtual float getDuration() = 0;
    virtual int getTotalNumFrames() = 0;
    virtual float getSpeed() = 0;

    virtual unsigned char* getPixels() = 0;
    virtual Texture* getTexture() = 0;
    
    virtual void setPosition(float pct) = 0;
    virtual void setFrame(int frame) = 0;
    virtual void setSpeed(float speed) = 0;
    virtual void setVolume(float volume) = 0;
    
    virtual void firstFrame() = 0;
    virtual void nextFrame() = 0;
    virtual void previousFrame() = 0;
    
    virtual void syncNextFrame() = 0;
};


#endif /* defined(__Synesthesia__Player__) */
