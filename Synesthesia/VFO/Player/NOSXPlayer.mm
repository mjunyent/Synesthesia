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
    this->player = [[[AVPlayer alloc] init] autorelease];

    duration = kCMTimeZero;
    videoSampleTime = kCMTimeZero;
    videoSampleTimePrev = kCMTimeZero;
    
    isLoaded = false;
    isFinished = false;
}

bool NOSXPlayer::loadWithURL(std::string url) {
    //TODO unload video.
    
    NSString * nsURL = [NSString stringWithCString:url.c_str() encoding:[NSString defaultCStringEncoding]];
    
    NSURL * fileURL = [NSURL fileURLWithPath:nsURL];
    
    asset = [AVURLAsset URLAssetWithURL:fileURL options:nil];
    
    if(!this->asset) return false;
    
    duration = [((AVURLAsset *) asset) duration];

    NSLog(@"seconds = %f", CMTimeGetSeconds(duration));

    if(CMTimeCompare(duration, kCMTimeZero) == 0) {
        //TODO ERROR
    }
    
//    if(!isfinite(getDurationInSec())) {
        //TODO ERROR
//    }

    //TODO call createassetreaderwithtimerange.
    
    //.......

    
    
    return true;
}

bool NOSXPlayer::createAssetReaderWithTimeRange(CMTimeRange timeRange) {

    
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
        
    } else {
        //TODO ERROR
    }
    
    
    BOOL bOk = [((AVAssetReader*)assetReader) startReading];
    if(!bOk) {
        //TODO ERROR
    }

    return true;
}























