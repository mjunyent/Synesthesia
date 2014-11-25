//
//  OSXPlayer.cpp
//  Synesthesia
//
//  Created by Marc Junyent Martín on 12/11/14.
//  Copyright (c) 2014 Marc Junyent Martín. All rights reserved.
//

#include "OSXPlayer.h"
#import "OSX/OFAVFoundationVideoPlayer.h"

CVOpenGLTextureCacheRef _videoTextureCachea = NULL;
CVOpenGLTextureRef _videoTextureRefa = NULL;

OSXPlayer::OSXPlayer() {
    videoPlayer = NULL;
    newFrame = false;
    updatePixels = false;
    updateTexture = false;
    textureCacheEnabled = false;
    pixels = NULL;
    texture = NULL;
}

OSXPlayer::~OSXPlayer() {
    close();
}




void OSXPlayer::load(std::string url) {
    if(!videoPlayer) {
        videoPlayer = [[OFAVFoundationVideoPlayer alloc] init];
        [(OFAVFoundationVideoPlayer *)videoPlayer setWillBeUpdatedExternally:YES];
    }
    
    NSString* videoPath = [NSString stringWithUTF8String:url.c_str()];
    [(OFAVFoundationVideoPlayer*)videoPlayer loadWithPath:videoPath];
    
    if(pixels != NULL) free(pixels);
    pixels = NULL;
    if(texture != NULL) delete texture;
    texture = NULL;

    updatePixels = true;
    updateTexture = true;
    
    while(!isLoaded() && !isError()) {
        pollEvents();
    }
    
    setAudioSampling(false);
    
    if(isError()) Tobago.log->write(Log::INFO) << "OSXPlayer::load could not load video: " << url;
}

void OSXPlayer::pollEvents() {
    NSEvent * event;

    do {
        event = [NSApp nextEventMatchingMask:NSAnyEventMask untilDate:[NSDate distantPast] inMode:NSDefaultRunLoopMode dequeue:YES];
        [NSApp sendEvent: event];
    }
    while(event != nil);
}

void OSXPlayer::loadAsync(std::string url) {
    if(!videoPlayer) {
        videoPlayer = [[OFAVFoundationVideoPlayer alloc] init];
        [(OFAVFoundationVideoPlayer *)videoPlayer setWillBeUpdatedExternally:YES];
    }
    
    NSString* videoPath = [NSString stringWithUTF8String:url.c_str()];
    [(OFAVFoundationVideoPlayer*)videoPlayer loadWithPath:videoPath];
    
    if(pixels != NULL) free(pixels);
    pixels = NULL;
    if(texture != NULL) delete texture;
    texture = NULL;
    
    updatePixels = true;
    updateTexture = true;
    
    setAudioSampling(false);
}

void OSXPlayer::setAudioSampling(bool onoff) {
    if(onoff)
        [((OFAVFoundationVideoPlayer *)videoPlayer) setEnableAudioSampling:YES];
    else
        [((OFAVFoundationVideoPlayer *)videoPlayer) setEnableAudioSampling:NO];
}

void OSXPlayer::close() {
    if(videoPlayer != NULL) {
        ((OFAVFoundationVideoPlayer *)videoPlayer).delegate = nil;
        [(OFAVFoundationVideoPlayer *)videoPlayer release];
    }

    videoPlayer = NULL;
    newFrame = false;
    updatePixels = false;
    updateTexture = false;
    textureCacheEnabled = false;
    if(pixels != NULL) free(pixels);
    pixels = NULL;
    if(texture != NULL) delete texture;
    texture = NULL;
}

void OSXPlayer::update() {
    if(!isLoaded()) return;
    
    newFrame = false;
    [(OFAVFoundationVideoPlayer *)videoPlayer update];
    newFrame = [(OFAVFoundationVideoPlayer *)videoPlayer isNewFrame];
    
    if(newFrame) {
        updatePixels = true;
        updateTexture = true;
    }
}




void OSXPlayer::play() {
    if(videoPlayer == NULL) {
        Tobago.log->write(Log::WARNING) << "OSXPlayer::play(): video not loaded!";
        return;
    }
    
    [(OFAVFoundationVideoPlayer *)videoPlayer play];
}

void OSXPlayer::pause() {
    if(videoPlayer == NULL) {
        Tobago.log->write(Log::WARNING) << "OSXPlayer::pause(): video not loaded!";
        return;
    }
    
    [(OFAVFoundationVideoPlayer *)videoPlayer pause];
}

void OSXPlayer::stop() {
    if(videoPlayer == NULL) {
        Tobago.log->write(Log::WARNING) << "OSXPlayer::stop(): video not loaded!";
        return;
    }
    
    [(OFAVFoundationVideoPlayer *)videoPlayer pause];
    [(OFAVFoundationVideoPlayer *)videoPlayer setPosition:0];
}




bool OSXPlayer::isError() {
    if(videoPlayer == NULL) return false;
    return [((OFAVFoundationVideoPlayer *) videoPlayer) isError];
}

bool OSXPlayer::isLoaded() {
    if(videoPlayer == NULL) return false;
    return [((OFAVFoundationVideoPlayer *) videoPlayer) isReady];
}

bool OSXPlayer::isPaused() {
    if(videoPlayer == NULL) return false;
    return ![((OFAVFoundationVideoPlayer *)videoPlayer) isPlaying];
}

bool OSXPlayer::isPlaying() {
    if(videoPlayer == NULL) return false;
    return [((OFAVFoundationVideoPlayer *)videoPlayer) isPlaying];
}

bool OSXPlayer::isMovieDone() {
    if(videoPlayer == NULL) return false;
    return [((OFAVFoundationVideoPlayer *)videoPlayer) isFinished];
}

bool OSXPlayer::isFrameNew() {
    if(videoPlayer == NULL) return newFrame;
    return false;
}




float OSXPlayer::getPosition() {
    if(videoPlayer == NULL) return -1;
    return [((OFAVFoundationVideoPlayer *)videoPlayer) getPosition];
}

int OSXPlayer::getFrameNum() {
    if(videoPlayer == NULL) return -1;
    return [((OFAVFoundationVideoPlayer *)videoPlayer) getCurrentFrameNum];
}

float OSXPlayer::getDuration() {
    if(videoPlayer == NULL) return -1;
    return [((OFAVFoundationVideoPlayer *)videoPlayer) getDurationInSec];
}

int OSXPlayer::getTotalNumFrames() {
    if(videoPlayer == NULL) return -1;
    return [((OFAVFoundationVideoPlayer *)videoPlayer) getDurationInFrames];
}

float OSXPlayer::getSpeed() {
    if(videoPlayer == NULL) return -1;
    return [((OFAVFoundationVideoPlayer *)videoPlayer) getSpeed];
}




unsigned char* OSXPlayer::getPixels() {
    if(!isLoaded()) return NULL;

    if(!updatePixels) { //pixels have not changed, return already calculated pixels.
        return pixels;
    }

    CGImageRef currentFrameRef;
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    CVImageBufferRef imageBuffer = [(OFAVFoundationVideoPlayer *)videoPlayer getCurrentFrame];

    /*Lock the image buffer*/
    CVPixelBufferLockBaseAddress(imageBuffer, 0);

    /*Get information about the image*/
    uint8_t *baseAddress	= (uint8_t *)CVPixelBufferGetBaseAddress(imageBuffer);
    size_t bytesPerRow		= CVPixelBufferGetBytesPerRow(imageBuffer);
    size_t width			= CVPixelBufferGetWidth(imageBuffer);
    size_t height			= CVPixelBufferGetHeight(imageBuffer);

    /*Create a CGImageRef from the CVImageBufferRef*/
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef newContext = CGBitmapContextCreate(baseAddress,
                                                    width,
                                                    height,
                                                    8,
                                                    bytesPerRow,
                                                    colorSpace,
                                                    kCGBitmapByteOrder32Little | kCGImageAlphaPremultipliedFirst);
    CGImageRef newImage	= CGBitmapContextCreateImage(newContext);
    currentFrameRef = CGImageCreateCopy(newImage);

    /*We release some components*/
    CGContextRelease(newContext);
    CGColorSpaceRelease(colorSpace);
    
    /*We relase the CGImageRef*/
    CGImageRelease(newImage);
    
    /*We unlock the  image buffer*/
    CVPixelBufferUnlockBaseAddress(imageBuffer,0);
    
    
    //THINGS//
    if(pixels == NULL) {
        pixels = (GLubyte *) malloc(width * height * 4);
    }
    
    
    [pool drain];

    CGContextRef spriteContext;
    spriteContext = CGBitmapContextCreate(pixels,
                                          width,
                                          height,
                                          CGImageGetBitsPerComponent(currentFrameRef),
                                          width * 4,
                                          CGImageGetColorSpace(currentFrameRef),
                                          kCGImageAlphaPremultipliedLast);
    
    CGContextDrawImage(spriteContext,
                       CGRectMake(0.0, 0.0, (CGFloat)width, (CGFloat)height),
                       currentFrameRef);
    
    CGContextRelease(spriteContext);
    CGImageRelease(currentFrameRef);

    updatePixels = false;
    
    return pixels;
}

Texture* OSXPlayer::getTexture() {
    if(!isLoaded()) {
        (*Tobago.log)(Log::WARNING) << "OSXPlayer::getTexture(): video not loaded.";
        return texture;
    }
    
    if(!updateTexture) return texture;
    
    if(textureCacheSupported && textureCacheEnabled) {
        initTextureCache();
    } else {
        /**
         *  no video texture cache.
         *  load texture from pixels.
         *  this method is the slower alternative.
         */
        
        if(texture == NULL) {
            int maxTextureSize = 0;
            glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
            
            if([(OFAVFoundationVideoPlayer *)videoPlayer getWidth] > maxTextureSize ||
               [(OFAVFoundationVideoPlayer *)videoPlayer getHeight] > maxTextureSize) {
                Tobago.log->write(Log::WARNING) << "OSXPlayer::getTexture: video image is bigger than max supported texture size " << maxTextureSize;
                return NULL;
            }
            
            texture = new Texture(GL_TEXTURE_RECTANGLE_ARB);
        }
        texture->setData([(OFAVFoundationVideoPlayer *)videoPlayer getWidth],
                         [(OFAVFoundationVideoPlayer *)videoPlayer getHeight],
                         getPixels());
    }

    updateTexture = false;
    return texture;
}




void OSXPlayer::setPosition(float pct) {
    if(videoPlayer == NULL) return;
    [((OFAVFoundationVideoPlayer *)videoPlayer) setPosition:pct];
}

void OSXPlayer::setFrame(int frame) {
    if(videoPlayer == NULL) return;
    [((OFAVFoundationVideoPlayer *)videoPlayer) setFrame:frame];
}

void OSXPlayer::setSpeed(float speed) {
    if(videoPlayer == NULL) return;
    [((OFAVFoundationVideoPlayer *)videoPlayer) setSpeed:speed];
}

void OSXPlayer::setVolume(float volume) {
    if(videoPlayer == NULL) return;

    if ( volume > 1.0f ){
        Tobago.log->write(Log::WARNING) << "OSXPlayer::setVolume: expected range is 0-1, limiting requested volume " << volume << " to 1.0";
        volume = 1.0f;
    }
    [((OFAVFoundationVideoPlayer *)videoPlayer) setVolume:volume];
}




void OSXPlayer::firstFrame() {
    if(videoPlayer == NULL) return;
    [((OFAVFoundationVideoPlayer *)videoPlayer) seekToStart];
}

void OSXPlayer::nextFrame() {
    int nextFrameNum = clamp(getFrameNum() + 1, 0, getTotalNumFrames());
    setFrame(nextFrameNum);
}

void OSXPlayer::previousFrame() {
    int nextFrameNum = clamp(getFrameNum() - 1, 0, getTotalNumFrames());
    setFrame(nextFrameNum);
}




void OSXPlayer::syncNextFrame() {
    double sampleTime = [((OFAVFoundationVideoPlayer *)videoPlayer) getCurrentTimeInSec];
//    double sampleTime2 = [((OFAVFoundationVideoPlayer *)videoPlayer) getVideoSampleTimeInSec];
    
    double fps = [((OFAVFoundationVideoPlayer *)videoPlayer) getFrameRate];
    [((OFAVFoundationVideoPlayer *)videoPlayer) setSynchSampleTimeInSec:sampleTime+1.0/fps];
}




bool OSXPlayer::enableTextureCache() {
    textureCacheSupported = false;
    textureCacheSupported = (CVOpenGLTextureCacheCreate != NULL);
    textureCacheEnabled = true;

    if(textureCacheSupported && _videoTextureCachea == NULL) {
        CVReturn err;
        
        err = CVOpenGLTextureCacheCreate(kCFAllocatorDefault,
                                         NULL,
                                         CGLGetCurrentContext(),
                                         CGLGetPixelFormat(CGLGetCurrentContext()),
                                         NULL,
                                         &_videoTextureCachea);
        
        if(err) {
            Tobago.log->write(Log::ERROR) << "OSXPlayer::enableTextureCache: error at CVOpenGLESTextureCacheCreate: " << err;
        }
    }
    
    return textureCacheSupported;
}

void OSXPlayer::disableTextureCache() {
    textureCacheEnabled = false;
    
    if(_videoTextureRefa != NULL) {
        CVOpenGLTextureRelease(_videoTextureRefa);
        _videoTextureRefa = NULL;
    }
    
    if(_videoTextureCachea != NULL) {
        CVOpenGLTextureCacheRelease(_videoTextureCachea);
        _videoTextureCachea = NULL;
    }
    if(texture != NULL) {
        delete texture;
        texture = NULL;
    }
}

void OSXPlayer::initTextureCache() {
    CVImageBufferRef imageBuffer = [(OFAVFoundationVideoPlayer *)videoPlayer getCurrentFrame];
    
    if(imageBuffer == nil) return;
    
    CVPixelBufferLockBaseAddress(imageBuffer, 0);
    
    /**
     *  video texture cache is available.
     *  this means we don't have to copy any pixels,
     *  and we can reuse the already existing video texture.
     *  this is very fast! :)
     */
    
    /**
     *  CVOpenGLESTextureCache does this operation for us.
     *  it automatically returns a texture reference which means we don't have to create the texture ourselves.
     *  this creates a slight problem because when we create an ofTexture objects, it also creates a opengl texture for us,
     *  which is unecessary in this case because the texture already exists.
     *  so... we can use ofTexture::setUseExternalTextureID() to get around this.
     */
    
    long videoTextureW = [(OFAVFoundationVideoPlayer *)videoPlayer getWidth];
    long videoTextureH = [(OFAVFoundationVideoPlayer *)videoPlayer getHeight];
    
    CVReturn err;
    unsigned int textureCacheID;
    
    err = CVOpenGLTextureCacheCreateTextureFromImage(NULL,
                                                     _videoTextureCachea,
                                                     imageBuffer,
                                                     NULL,
                                                     &_videoTextureRefa);
    
    textureCacheID = CVOpenGLTextureGetName(_videoTextureRefa);
    
    if(texture == NULL) {
        texture = new Texture(GL_TEXTURE_RECTANGLE_ARB, videoTextureW, videoTextureH, 0, textureCacheID);
    } else texture->id = textureCacheID;

    if(err) {
        Tobago.log->write(Log::ERROR) << "OSXPlayer::initTextureCache: error at CVOpenGLTextureCacheCreateTextureFromImage: " << err;
    }
    
    CVPixelBufferUnlockBaseAddress(imageBuffer, 0);
    
    CVOpenGLTextureCacheFlush(_videoTextureCachea, 0);
    if(_videoTextureRefa) {
        CVOpenGLTextureRelease(_videoTextureRefa);
        _videoTextureRefa = NULL;
    }
}













