//
//  OSXFrameGetter.cpp
//  Synesthesia
//
//  Created by Marc on 20/12/14.
//  Copyright (c) 2014 Marc Junyent Martín. All rights reserved.
//

#include "OSXFrameGetter.h"
#import "OSX/AVFoundationFrameGetter.h"

CVOpenGLTextureCacheRef _videoTextureCacheb = NULL;
CVOpenGLTextureRef _videoTextureRefb = NULL;

OSXFrameGetter::OSXFrameGetter() {
    textureCacheEnabled = false;
    pixels = NULL;
    texture = NULL;
    updatePixels = false;
    updateTexture = false;
    
    frameGetter = NULL;
}

OSXFrameGetter::~OSXFrameGetter() {
    unload();
}

void OSXFrameGetter::load(std::string url) {
    if(!frameGetter)
        frameGetter = [[AVFoundationFrameGetter alloc] init];

    NSString* videoPath = [NSString stringWithUTF8String:url.c_str()];
    BOOL bOk = [((AVFoundationFrameGetter*)frameGetter) load:videoPath];
    if(!bOk || !isReady()) {
        throw FrameGetterException("Could not load video " + url);
    }

    if(pixels != NULL) free(pixels);
    pixels = NULL;
    if(texture != NULL) delete texture;
    texture = NULL;

    updatePixels = true;
    updateTexture = true;
}

void OSXFrameGetter::unload() {
    if(frameGetter) {
        [((AVFoundationFrameGetter*)frameGetter) release];
    }
    frameGetter = NULL;
    
    if(pixels != NULL) free(pixels);
    pixels = NULL;
    if(texture != NULL) delete texture;
    texture = NULL;

    updatePixels = false;
    updateTexture = false;
    textureCacheEnabled = false;
}

void OSXFrameGetter::update() {
    if(!frameGetter) return;

    BOOL bOk = [((AVFoundationFrameGetter*)frameGetter) update];
    
    if(!bOk && !isFinished())
        throw FrameGetterException("Could not update the video");
    
    if(isNewFrame()) {
        updatePixels = true;
        updateTexture = true;
    }
}

bool OSXFrameGetter::enableTextureCache() {
    textureCacheSupported = false;
    textureCacheSupported = (CVOpenGLTextureCacheCreate != NULL);
    textureCacheEnabled = true;
    
    if(textureCacheSupported && _videoTextureCacheb == NULL) {
        CVReturn err;
        
        err = CVOpenGLTextureCacheCreate(kCFAllocatorDefault,
                                         NULL,
                                         CGLGetCurrentContext(),
                                         CGLGetPixelFormat(CGLGetCurrentContext()),
                                         NULL,
                                         &_videoTextureCacheb);
        
        if(err) {
            Tobago.log->write(Log::ERROR) << "OSXFrameGetter::enableTextureCache: error at CVOpenGLESTextureCacheCreate: " << err;
        }
    }
    
    return textureCacheSupported;
}

double OSXFrameGetter::getCurrentTime() {
    return [((AVFoundationFrameGetter*)frameGetter) getCurrentTime];
}

long OSXFrameGetter::getWidth() {
    return [((AVFoundationFrameGetter*)frameGetter) getWidth];
}

long OSXFrameGetter::getHeight() {
    return [((AVFoundationFrameGetter*)frameGetter) getHeight];
}

float OSXFrameGetter::getFrameRate() {
    return [((AVFoundationFrameGetter*)frameGetter) getFrameRate];
}

unsigned long OSXFrameGetter::getFrameNum() {
    return [((AVFoundationFrameGetter*)frameGetter) getFrameNum];
}

bool OSXFrameGetter::isReady() {
    if(!frameGetter) return false;
    return [((AVFoundationFrameGetter*)frameGetter) isReady];
}

bool OSXFrameGetter::isNewFrame() {
    if(!frameGetter) return false;
    return [((AVFoundationFrameGetter*)frameGetter) isNewFrame];
}

bool OSXFrameGetter::isFinished() {
    if(!frameGetter) return false;
    return [((AVFoundationFrameGetter*)frameGetter) isFinished];
}

unsigned char* OSXFrameGetter::getPixels() {
    if(!frameGetter) return NULL;
    if(!updatePixels) { //pixels have not changed, return already calculated pixels.
        return pixels;
    }
    
    CGImageRef currentFrameRef;
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    CVImageBufferRef imageBuffer = [(AVFoundationFrameGetter *)frameGetter getCurrentFrame];

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

Texture* OSXFrameGetter::getTexture() {
    if(!frameGetter) return NULL;

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
            
            if(getWidth() > maxTextureSize ||
               getHeight() > maxTextureSize) {
                Tobago.log->write(Log::WARNING) << "NOSXPlayer::getTexture: video image is bigger than max supported texture size " << maxTextureSize;
                return NULL;
            }
            
            texture = new Texture(GL_TEXTURE_RECTANGLE_ARB);
        }
        texture->setData(int(getWidth()), int(getHeight()), getPixels());
    }
    
    updateTexture = false;
    return texture;
}

void OSXFrameGetter::initTextureCache() {
    CVImageBufferRef imageBuffer = [((AVFoundationFrameGetter*)frameGetter) getCurrentFrame];

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
    
    CVReturn err;
    unsigned int textureCacheID;
    
    err = CVOpenGLTextureCacheCreateTextureFromImage(NULL,
                                                     _videoTextureCacheb,
                                                     imageBuffer,
                                                     NULL,
                                                     &_videoTextureRefb);
    
    textureCacheID = CVOpenGLTextureGetName(_videoTextureRefb);
    
    if(texture == NULL) {
        texture = new Texture(GL_TEXTURE_RECTANGLE_ARB, int(getWidth()), int(getHeight()), 0, textureCacheID);
    } else texture->id = textureCacheID;
    
    if(err) {
        Tobago.log->write(Log::ERROR) << "NOSXPlayer::initTextureCache: error at CVOpenGLTextureCacheCreateTextureFromImage: " << err;
    }
    
    CVPixelBufferUnlockBaseAddress(imageBuffer, 0);
    
    CVOpenGLTextureCacheFlush(_videoTextureCacheb, 0);
    if(_videoTextureRefb) {
        CVOpenGLTextureRelease(_videoTextureRefb);
        _videoTextureRefb = NULL;
    }
}
