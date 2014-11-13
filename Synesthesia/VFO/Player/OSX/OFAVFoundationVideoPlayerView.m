//
//  OFAVFoundationVideoPlayerView.m
//  Created by Lukasz Karluk on 6/07/2014.
//  http://julapy.com
//

#import "OFAVFoundationVideoPlayerView.h"

//---------------------------------------------------------- video player view.
@implementation OFAVFoundationVideoPlayerView

+ (Class)layerClass {
	return [AVPlayerLayer class];
}


- (AVPlayer*)player {
	return [(AVPlayerLayer*)[self layer] player];
}


- (void)setPlayer:(AVPlayer*)player {
	[(AVPlayerLayer*)[self layer] setPlayer:player];
}

//----------------------------------------------------------
- (id)initVideoPlayerView {
    /**
     *  initialise video player view to full screen by default.
     *  later the view frame can be changed if need be.
     */
    NSRect playerViewFrame = [NSScreen mainScreen].visibleFrame;
    self = [super initWithFrame:playerViewFrame];
    if(self != NULL) {
        //
    }
    return self;
}

//----------------------------------------------------------
- (void)dealloc {
    self.player = nil;
    [super dealloc];
}

//----------------------------------------------------------
- (void)setVideoPosition:(CGPoint)position {
    NSRect playerViewFrame = self.frame;
    playerViewFrame.origin.x = position.x;
    playerViewFrame.origin.y = position.y;
    self.frame = playerViewFrame;
}

- (void)setVideoSize:(CGSize)size {
    NSRect playerViewFrame = self.frame;
    playerViewFrame.size.width = size.width;
    playerViewFrame.size.height = size.height;
    self.frame = playerViewFrame;    
}

@end
