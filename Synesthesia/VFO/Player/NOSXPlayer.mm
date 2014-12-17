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


bool NOSXPlayer::loadWithURL(std::string url) {
    unload();
    
    NSString * nsURL = [NSString stringWithCString:url.c_str() encoding:[NSString defaultCStringEncoding]];
    
    NSURL * fileURL = [NSURL fileURLWithPath:nsURL];
    
    asset = [AVURLAsset URLAssetWithURL:fileURL options:nil];
    
    if(!this->asset) return false;
    
    duration = [((AVURLAsset *) asset) duration];

    if(CMTimeCompare(duration, kCMTimeZero) == 0) {
        //TODO ERROR
    }
    
    if(!isfinite(CMTimeGetSeconds(duration))) {
        //TODO ERROR
    }

    bool 
    //TODO call createassetreaderwithtimerange.
    
    //.......

    
    
    return true;
}

bool NOSXPlayer::createAssetReaderWithTimeRange(CMTimeRange timeRange) {
    videoSampleTime = videoSampleTimePrev = timeRange.start;

    NSError *error = nil;
    assetReader = [AVAssetReader assetReaderWithAsset:(AVAsset*)asset error:&error];

    if(error) {
//        NSLog(@"assetReader: error during initialisation: %@", [error localizedDescription]);
        //TODO ERROR
    }
    
    ((AVAssetReader*)assetReader).timeRange = timeRange;
    
    
    NSMutableDictionary * videoOutputSettings = [[[NSMutableDictionary alloc] init] autorelease];
    [videoOutputSettings setObject:[NSNumber numberWithInt:kCVPixelFormatType_32BGRA] forKey:(NSString*)kCVPixelBufferPixelFormatTypeKey];
    NSArray * videoTracks = [((AVAsset*)asset) tracksWithMediaType:AVMediaTypeVideo];
    if([videoTracks count] > 0) {
        AVAssetTrack * videoTrack = [videoTracks objectAtIndex:0];
        assetReaderVideoTrackOutput = [AVAssetReaderTrackOutput assetReaderTrackOutputWithTrack:videoTrack outputSettings:videoOutputSettings];

        if(assetReaderVideoTrackOutput == nil) {
            //TODO ERROR
        }
        
        if([((AVAssetReader*)assetReader) canAddOutput:((AVAssetReaderTrackOutput*)assetReaderVideoTrackOutput)]) {
            [((AVAssetReader*)assetReader) addOutput:((AVAssetReaderTrackOutput*)assetReaderVideoTrackOutput)];
        } else {
            //TODO ERROR
        }        
    } else {
        //TODO ERROR
    }
    
    
    BOOL bOk = [((AVAssetReader*)assetReader) startReading];
    if(!bOk) {
        //TODO ERROR
    }

    return true;
}



void NOSXPlayer::updateToNextFrame() {
    //Check that its ready and not finished, else ERROR. TODO.
    
    if(assetReader == nil) {
        //TODO createAssetReader with time range or ERROR
    }
    
    if (((AVAssetReader*)assetReader).status != AVAssetReaderStatusReading) {
        //ERROR TODO
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
                CFRelease(((CMSampleBufferRef*)videoSampleBuffer));
                videoSampleBuffer = nil;
            }
            videoSampleBuffer = videoBufferTemp;
            
            videoSampleTime = CMSampleBufferGetPresentationTimeStamp(*((CMSampleBufferRef*)videoSampleBuffer));
            
            copiedNewSamples = true;
        } else {
            isFinished = true;
        }
    }
    
    if(copiedNewSamples) {
        isNewFrame = CMTimeCompare(videoSampleTime, videoSampleTimePrev) == 1;

        if(isNewFrame)
            videoSampleTimePrev = videoSampleTime;
    }
}



















