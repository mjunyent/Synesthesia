//
//  OFAVFoundationVideoPlayerView.h
//  Created by Lukasz Karluk on 6/07/2014.
//  http://julapy.com
//

//----------------------------------------------------------
#include <TargetConditionals.h>

//----------------------------------------------------------
#import <AppKit/AppKit.h>
#define BaseView NSView

#import <AVFoundation/AVFoundation.h>

//---------------------------------------------------------- video player view.
@interface OFAVFoundationVideoPlayerView : BaseView {
    //
}
@property (nonatomic, retain) AVPlayer * player;

- (id)initVideoPlayerView;
- (void)setVideoPosition:(CGPoint)position;
- (void)setVideoSize:(CGSize)size;

@end
