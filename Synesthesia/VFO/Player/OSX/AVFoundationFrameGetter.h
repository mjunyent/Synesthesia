//
//  FrameGetter.h
//  Synesthesia
//
//  Created by Marc on 19/12/14.
//  Copyright (c) 2014 Marc Junyent Martín. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>

@class AVAsset;
@class AVAssetReader;
@class AVAssetReaderOutput;

@interface AVFoundationFrameGetter : NSObject {
    AVAsset * _asset;
    AVAssetReader * _assetReader;
    AVAssetReaderTrackOutput * _assetReaderVideoTrackOutput;

    CMSampleBufferRef videoSampleBuffer;

    CMTime duration;
    CMTime videoSampleTime;
    CMTime videoSampleTimePrev;

    NSInteger videoWidth;
    NSInteger videoHeight;
    
    BOOL bReady;
    BOOL bFinished;
    BOOL bNewFrame;
    
    NSUInteger currentFrame;
    
    float framerate;
}

@property (nonatomic, retain) AVAsset * asset;
@property (nonatomic, retain) AVAssetReader * assetReader;
@property (nonatomic, retain) AVAssetReaderTrackOutput * assetReaderVideoTrackOutput;

-(BOOL) load: (NSString*)path;
-(void) unload;

-(BOOL) update;

- (NSInteger)getWidth;
- (NSInteger)getHeight;
- (double)getCurrentTime;
- (float)getFrameRate;
- (NSUInteger)getFrameNum;

- (BOOL)isReady;
- (BOOL)isFinished;
- (BOOL)isNewFrame;

- (CVImageBufferRef)getCurrentFrame;

@end