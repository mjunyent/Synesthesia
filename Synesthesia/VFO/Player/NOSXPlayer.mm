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
}

bool NOSXPlayer::loadWithURL(std::string url) {
    //TODO unload video.
    
    NSString * nsURL = [NSString stringWithCString:url.c_str() encoding:[NSString defaultCStringEncoding]];
    
    NSURL * fileURL = [NSURL fileURLWithPath:nsURL];
    
    asset = [AVURLAsset URLAssetWithURL:fileURL options:nil];
    
    if(!this->asset) return false;
    
    duration = new CMTime;
    CMTime tduration = [((AVURLAsset *) asset) duration];
    ((CMTime*)duration)->epoch = tduration.epoch;
    ((CMTime*)duration)->flags = tduration.flags;
    ((CMTime*)duration)->timescale = tduration.timescale;
    ((CMTime*)duration)->value = tduration.value;
    
    //TODO perform time checks (nonzero not infinite).
    
    //TODO call createassetreaderwithtimerange.
    
    //.......

    
    
    return true;
}

bool NOSXPlayer::createAssetReaderWithTimeRange(void *VtimeRange) {
    CMTimeRange *timeRange = (CMTimeRange*)VtimeRange;

    
    NSError *error = nil;
    assetReader = [AVAssetReader assetReaderWithAsset:(AVAsset*)asset error:&error];

    if(
    
    
    return true;
}























