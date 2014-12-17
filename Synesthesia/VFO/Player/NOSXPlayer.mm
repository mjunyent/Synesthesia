//
//  NOSXPlayer.m
//  Synesthesia
//
//  Created by Marc Junyent Martín on 13/12/14.
//  Copyright (c) 2014 Marc Junyent Martín. All rights reserved.
//

#include "NOSXPlayer.h"

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>

CVOpenGLTextureCacheRef _videoTextureCache = NULL;
CVOpenGLTextureRef _videoTextureRef = NULL;

NOSXPlayer::NOSXPlayer() {
    videoSampleBuffer = nil;

    duration = kCMTimeZero;
    videoSampleTime = kCMTimeZero;
    videoSampleTimePrev = kCMTimeZero;
    
    isReady = false;
    isFinished = false;
    isNewFrame = false;

    width = 0;
    height = 0;
    framerate = 0;
    currentFrame = 0;

    assetReader = nil;
    assetReaderVideoTrackOutput = nil;
    asset = nil;
    
    textureCacheEnabled = false;
    pixels = NULL;
    texture = NULL;
    updatePixels = false;
    updateTexture = false;
}

void NOSXPlayer::dealloc() {
    [(AVAssetReader*)assetReader cancelReading];
    assetReader = nil;

    assetReaderVideoTrackOutput = nil;
    asset = nil;

    if(videoSampleBuffer) {
        CFRelease((CMSampleBufferRef*)videoSampleBuffer);
        videoSampleBuffer = nil;
    }
    
    if(pixels != NULL) free(pixels);
    pixels = NULL;
    if(texture != NULL) delete texture;
    texture = NULL;
    
    updatePixels = false;
    updateTexture = false;
}

//TODO try THROWS
bool NOSXPlayer::load(std::string url) {
    unload();
    
    NSString * nsURL = [NSString stringWithCString:url.c_str() encoding:[NSString defaultCStringEncoding]];
    
    NSURL * fileURL = [NSURL fileURLWithPath:nsURL];

    asset = [AVURLAsset URLAssetWithURL:fileURL options:nil];
    
    if(!asset) throw new NOSXPlayerException("NOSXPlyer: asset could not be created");
    
    duration = [((AVURLAsset *) asset) duration];

    if(CMTimeCompare(duration, kCMTimeZero) == 0)
        throw new NOSXPlayerException("NOSXPlyer: video loaded with time 0");
    
    if(!isfinite(CMTimeGetSeconds(duration)))
        throw new NOSXPlayerException("NOSXPlyer: video loaded with no finite time");

    createAssetReaderWithTimeRange(CMTimeRangeMake(kCMTimeZero, duration));

    videoSampleBuffer = [((AVAssetReaderTrackOutput*)assetReaderVideoTrackOutput) copyNextSampleBuffer];
    CVImageBufferRef imageBuffer = CMSampleBufferGetImageBuffer((CMSampleBufferRef)videoSampleBuffer);
    CVPixelBufferLockBaseAddress(imageBuffer, 0);
    width = CVPixelBufferGetWidth(imageBuffer);
    height = CVPixelBufferGetHeight(imageBuffer);
    CVPixelBufferUnlockBaseAddress(imageBuffer, 0);
    
    NSArray * videoTracks = [((AVAsset*)asset) tracksWithMediaType:AVMediaTypeVideo];
    if([videoTracks count] > 0) {
        AVAssetTrack* track = [videoTracks objectAtIndex:0];
        framerate = track.nominalFrameRate;
    }

    isReady = true;
    
    
    if(pixels != NULL) free(pixels);
    pixels = NULL;
    if(texture != NULL) delete texture;
    texture = NULL;

    updatePixels = true;
    updateTexture = true;

    return true;
}

void NOSXPlayer::unload() {
    isReady = false;
    isFinished = false;
    isNewFrame = false;
    
    duration = kCMTimeZero;
    videoSampleTime = kCMTimeZero;
    videoSampleTimePrev = kCMTimeZero;

    width = 0;
    height = 0;
    framerate = 0;
    currentFrame = 0;
    
    [((AVAssetReader*)assetReader) cancelReading];
    assetReader = nil;
    assetReaderVideoTrackOutput = nil;
    asset = nil;
    
    if(videoSampleBuffer) {
        CFRelease((CMSampleBufferRef)videoSampleBuffer);
    }

    textureCacheEnabled = false;
    updatePixels = false;
    updateTexture = false;
}

bool NOSXPlayer::createAssetReaderWithTimeRange(CMTimeRange timeRange) {
    videoSampleTime = videoSampleTimePrev = timeRange.start;

    NSError *error = nil;
    assetReader = [AVAssetReader assetReaderWithAsset:(AVAsset*)asset error:&error];

    if(error)
        throw new NOSXPlayerException(std::string("NOSXPlyer: error during initialisation of asset reader") + std::string([[error localizedDescription] UTF8String]));
    
    ((AVAssetReader*)assetReader).timeRange = timeRange;
    
    
    NSMutableDictionary * videoOutputSettings = [[[NSMutableDictionary alloc] init] autorelease];
    [videoOutputSettings setObject:[NSNumber numberWithInt:kCVPixelFormatType_32BGRA] forKey:(NSString*)kCVPixelBufferPixelFormatTypeKey];
    NSArray * videoTracks = [((AVAsset*)asset) tracksWithMediaType:AVMediaTypeVideo];
    if([videoTracks count] > 0) {
        AVAssetTrack * videoTrack = [videoTracks objectAtIndex:0];
        assetReaderVideoTrackOutput = [AVAssetReaderTrackOutput assetReaderTrackOutputWithTrack:videoTrack outputSettings:videoOutputSettings];

        if(assetReaderVideoTrackOutput == nil)
            throw new NOSXPlayerException("NOSXPlyer: null track output on asset reader");
        
        if([((AVAssetReader*)assetReader) canAddOutput:((AVAssetReaderTrackOutput*)assetReaderVideoTrackOutput)]) {
            [((AVAssetReader*)assetReader) addOutput:((AVAssetReaderTrackOutput*)assetReaderVideoTrackOutput)];
        } else
            throw new NOSXPlayerException("NOSXPlyer: output can not be added to asset reader");
    } else
        throw new NOSXPlayerException("NOSXPlyer: no video tracks");
    
    
    BOOL bOk = [((AVAssetReader*)assetReader) startReading];
    if(!bOk) {
        throw new NOSXPlayerException("NOSXPlyer: could not start reading");
    }

    return true;
}

void NOSXPlayer::updateToNextFrame() {
    //Check that its ready and not finished, else ERROR. TODO.
    if(isFinished || !isReady) return;
    
    if(assetReader == nil) {
        //TODO createAssetReader with time range or ERROR
        return;
    }
    
    if (((AVAssetReader*)assetReader).status != AVAssetReaderStatusReading) {
        //ERROR TODO
        return;
    }
    
    bool copiedNewSamples = false;
    if(assetReaderVideoTrackOutput != nil &&
       ((AVAssetReader*)assetReader).status == AVAssetReaderStatusReading) {
        CMSampleBufferRef videoBufferTemp;
        @try {
            videoBufferTemp = [((AVAssetReaderTrackOutput*)assetReaderVideoTrackOutput) copyNextSampleBuffer];
        } @catch (NSException *e) {
            //TODO somethig.
        }
        
        if(videoBufferTemp) {
            if(videoSampleBuffer) {
                CFRelease(((CMSampleBufferRef)videoSampleBuffer));
                videoSampleBuffer = nil;
            }
            videoSampleBuffer = videoBufferTemp;
            
            videoSampleTime = CMSampleBufferGetPresentationTimeStamp(((CMSampleBufferRef)videoSampleBuffer));
            
            copiedNewSamples = true;
        } else {
            isFinished = true;
        }
    }
    
    if(copiedNewSamples) {
        isNewFrame = CMTimeCompare(videoSampleTime, videoSampleTimePrev) == 1;

        if(isNewFrame) {
            videoSampleTimePrev = videoSampleTime;
            currentFrame++;
            updatePixels = true;
            updateTexture = true;
        }
    }
}

double NOSXPlayer::getPosition() {
    return CMTimeGetSeconds(videoSampleTime);
}

unsigned char* NOSXPlayer::getPixels() {
    if(!isReady) return NULL;
    
    if(!updatePixels) return pixels;

    CGImageRef currentFrameRef;
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    CVImageBufferRef imageBuffer = CMSampleBufferGetImageBuffer((CMSampleBufferRef)videoSampleBuffer);

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

    if(pixels == NULL) pixels = (GLubyte *) malloc(width * height * 4);

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


Texture* NOSXPlayer::getTexture() {
    if(!isReady) return NULL;
    
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
            
            if(width > maxTextureSize ||
               height > maxTextureSize) {
                Tobago.log->write(Log::WARNING) << "NOSXPlayer::getTexture: video image is bigger than max supported texture size " << maxTextureSize;
                return NULL;
            }
            
            texture = new Texture(GL_TEXTURE_RECTANGLE_ARB);
        }
        texture->setData(int(width), int(height), getPixels());
    }
    
    updateTexture = false;
    return texture;
}


bool NOSXPlayer::enableTextureCache() {
    textureCacheSupported = false;
    textureCacheSupported = (CVOpenGLTextureCacheCreate != NULL);
    textureCacheEnabled = true;
    
    if(textureCacheSupported && _videoTextureCache == NULL) {
        CVReturn err;
        
        err = CVOpenGLTextureCacheCreate(kCFAllocatorDefault,
                                         NULL,
                                         CGLGetCurrentContext(),
                                         CGLGetPixelFormat(CGLGetCurrentContext()),
                                         NULL,
                                         &_videoTextureCache);

        if(err) {
            Tobago.log->write(Log::ERROR) << "NOSXPlayer::enableTextureCache: error at CVOpenGLESTextureCacheCreate: " << err;
        }
    }
    
    return textureCacheSupported;
}


void NOSXPlayer::initTextureCache() {
    CVImageBufferRef imageBuffer = CMSampleBufferGetImageBuffer((CMSampleBufferRef)videoSampleBuffer);

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
                                                     _videoTextureCache,
                                                     imageBuffer,
                                                     NULL,
                                                     &_videoTextureRef);
    
    textureCacheID = CVOpenGLTextureGetName(_videoTextureRef);
    
    if(texture == NULL) {
        texture = new Texture(GL_TEXTURE_RECTANGLE_ARB, int(width), int(height), 0, textureCacheID);
    } else texture->id = textureCacheID;

    if(err) {
        Tobago.log->write(Log::ERROR) << "NOSXPlayer::initTextureCache: error at CVOpenGLTextureCacheCreateTextureFromImage: " << err;
    }

    CVPixelBufferUnlockBaseAddress(imageBuffer, 0);

    CVOpenGLTextureCacheFlush(_videoTextureCache, 0);
    if(_videoTextureRef) {
        CVOpenGLTextureRelease(_videoTextureRef);
        _videoTextureRef = NULL;
    }
}








