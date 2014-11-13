//
//  OSXPlayer.h
//  Synesthesia
//
//  Created by Marc Junyent Martín on 12/11/14.
//  Copyright (c) 2014 Marc Junyent Martín. All rights reserved.
//

#ifndef __Synesthesia__OSXPlayer__
#define __Synesthesia__OSXPlayer__

#include "Tobago/Tobago.h"
#include "Player.h"

class OSXPlayer : public Player {
public:
    OSXPlayer();
    ~OSXPlayer();
    
    void load(std::string url);
    void close();
    void update();
    
    void play();
    void pause();
    void stop();

    bool isLoaded();
    bool isPaused();
    bool isPlaying();
    bool isMovieDone();
    bool isFrameNew();

    float getPosition();
    int getFrameNum();
    float getDuration();
    int getTotalNumFrames();
    float getSpeed();

    unsigned char* getPixels();
    Texture* getTexture();
    
    void setPosition(float pct);
    void setFrame(int frame);
    void setSpeed(float speed);
    void setVolume(float volume);
    
    void firstFrame();
    void nextFrame();
    void previousFrame();

    
    bool enableTextureCache();
    void disableTextureCache();
    void initTextureCache();
//    long framecount();
    
protected:
    void* videoPlayer;
    bool newFrame;
    bool updatePixels;
    bool updateTexture;
    
    bool textureCacheSupported;
    bool textureCacheEnabled;

    GLubyte* pixels;
    Texture* texture;
};


#endif /* defined(__Synesthesia__OSXPlayer__) */
