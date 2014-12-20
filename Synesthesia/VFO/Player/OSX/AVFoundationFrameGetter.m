//
//  FrameGetter.m
//  Synesthesia
//
//  Created by Marc on 19/12/14.
//  Copyright (c) 2014 Marc Junyent Martín. All rights reserved.
//

#import "AVFoundationFrameGetter.h"


@implementation AVFoundationFrameGetter

@synthesize asset = _asset;
@synthesize assetReader = _assetReader;
@synthesize assetReaderVideoTrackOutput = _assetReaderVideoTrackOutput;

- (id) init {
    self = [super init];
    
    if(self) {
        videoSampleBuffer = nil;
        
        duration = kCMTimeZero;
        videoSampleTime = kCMTimeZero;
        videoSampleTimePrev = kCMTimeZero;
        
        bReady    = NO;
        bFinished = NO;
        bNewFrame = NO;
        
        videoWidth = 0;
        videoHeight = 0;
        framerate = 0;
        currentFrame = 0;
    }

    return self;
}

- (void)dealloc {
    [self.assetReader cancelReading];
    self.assetReader = nil;
    self.assetReaderVideoTrackOutput = nil;
    self.asset = nil;
    
    if(videoSampleBuffer) {
        CFRelease(videoSampleBuffer);
        videoSampleBuffer = nil;
    }
    
    [super dealloc];
}

- (BOOL)load:(NSString*)path {
    [self unload];
    
    NSURL * fileURL = [NSURL fileURLWithPath:path];

    self.asset = [AVURLAsset URLAssetWithURL:fileURL options:nil];
    if(!self.asset) {
        return NO;
    }

    duration = [self.asset duration];
    
    if(CMTimeCompare(duration, kCMTimeZero) == 0) {
        return NO;
    }
    
    if(!isfinite(CMTimeGetSeconds(duration))) {
        return NO;
    }
    
    BOOL bOk = [self createAssetReaderWithTimeRange:CMTimeRangeMake(kCMTimeZero, duration)];
    if(!bOk) {
        return NO;
    }
    
    videoSampleBuffer = [self.assetReaderVideoTrackOutput copyNextSampleBuffer];
    CVImageBufferRef imageBuffer = CMSampleBufferGetImageBuffer(videoSampleBuffer);
    CVPixelBufferLockBaseAddress(imageBuffer, 0);
    videoWidth = CVPixelBufferGetWidth(imageBuffer);
    videoHeight = CVPixelBufferGetHeight(imageBuffer);
    CVPixelBufferUnlockBaseAddress(imageBuffer, 0);
    
    NSArray * videoTracks = [self.asset tracksWithMediaType:AVMediaTypeVideo];
    if([videoTracks count] > 0) {
        AVAssetTrack * track = [videoTracks objectAtIndex:0];
        framerate = track.nominalFrameRate;
    }

    bReady = true;

    return YES;
}

- (BOOL)createAssetReaderWithTimeRange:(CMTimeRange)timeRange {
    videoSampleTime = videoSampleTimePrev = timeRange.start;

    NSError * error = nil;
    self.assetReader = [AVAssetReader assetReaderWithAsset:self.asset error:&error];
    
    if(error) {
        NSLog(@"assetReader: error during initialisation: %@", [error localizedDescription]);
        return NO;
    }
    self.assetReader.timeRange = timeRange;
    

    NSMutableDictionary * videoOutputSettings = [[[NSMutableDictionary alloc] init] autorelease];
    [videoOutputSettings setObject:[NSNumber numberWithInt:kCVPixelFormatType_32BGRA] forKey:(NSString*)kCVPixelBufferPixelFormatTypeKey];
    
    NSArray * videoTracks = [self.asset tracksWithMediaType:AVMediaTypeVideo];
    if([videoTracks count] > 0) {
        AVAssetTrack * videoTrack = [videoTracks objectAtIndex:0];
        self.assetReaderVideoTrackOutput = [AVAssetReaderTrackOutput assetReaderTrackOutputWithTrack:videoTrack outputSettings:videoOutputSettings];
        
        if(self.assetReaderVideoTrackOutput == nil) {
            return NO;
        }
        
        if([self.assetReader canAddOutput:self.assetReaderVideoTrackOutput]) {
            [self.assetReader addOutput:self.assetReaderVideoTrackOutput];
        } else {
            return NO;
        }
    } else {
        return NO;
    }
    
    BOOL bOk = [self.assetReader startReading];
    if(!bOk) {
        NSLog(@"assetReader couldn't startReading: %@", [self.assetReader error]);
        return NO;
    }
    
    return YES;
}

- (void)unload {
    bReady = NO;
    bFinished = NO;
    bNewFrame = NO;

    duration = kCMTimeZero;
    videoSampleTime = kCMTimeZero;
    videoSampleTimePrev = kCMTimeZero;

    videoWidth = 0;
    videoHeight = 0;
    framerate = 0;
    currentFrame = 0;
    
    [self.assetReader cancelReading];
    self.assetReader = nil;
    self.assetReaderVideoTrackOutput = nil;
    self.asset = nil;
    
    if(videoSampleBuffer) {
        CFRelease(videoSampleBuffer);
        videoSampleBuffer = nil;
    }
}

- (BOOL)update {
    if(bFinished ||!bReady) {
        return NO;
    }
    
    if(self.assetReader == nil) {
        return NO;
    }
    
    if(self.assetReader.status != AVAssetReaderStatusReading) {
        return NO;
    }
    
    if(self.assetReaderVideoTrackOutput == nil) {
        return NO;
    }

    BOOL bCopiedNewSamples = false;
    CMSampleBufferRef videoBufferTemp;
    @try {
        videoBufferTemp = [self.assetReaderVideoTrackOutput copyNextSampleBuffer];
    } @catch (NSException * e) {
        //???
    }
    
    if(videoBufferTemp) {
        if(videoSampleBuffer) {
            CFRelease(videoSampleBuffer);
            videoSampleBuffer = nil;
        }
        videoSampleBuffer = videoBufferTemp;
        
        videoSampleTime = CMSampleBufferGetPresentationTimeStamp(videoSampleBuffer);
        
        bCopiedNewSamples = YES;
    } else {
        bFinished = YES;
    }
    
    if(bCopiedNewSamples) {
        bNewFrame = CMTimeCompare(videoSampleTime, videoSampleTimePrev) == 1;
        
        if(bNewFrame) {
            videoSampleTimePrev = videoSampleTime;
            currentFrame++;
        }
    }
    
    return YES;
}

- (NSInteger)getWidth {
    return videoWidth;
}

- (NSInteger)getHeight {
    return videoHeight;
}

- (double)getCurrentTime {
    return CMTimeGetSeconds(videoSampleTime);
}

- (float)getFrameRate {
    return framerate;
}

- (NSUInteger)getFrameNum {
    return currentFrame;
}

- (BOOL)isReady {
    return bReady;
}

- (BOOL)isFinished {
    return bFinished;
}

- (BOOL)isNewFrame {
    return bNewFrame;
}

- (CVImageBufferRef)getCurrentFrame {
    return CMSampleBufferGetImageBuffer(videoSampleBuffer);
}

@end