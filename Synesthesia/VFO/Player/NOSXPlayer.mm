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
        }
    }
}

double NOSXPlayer::getPosition() {
    return CMTimeGetSeconds(videoSampleTime);
}




















