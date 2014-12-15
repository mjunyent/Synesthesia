//
//  NOSXPlayer.h
//  Synesthesia
//
//  Created by Marc Junyent Martín on 13/12/14.
//  Copyright (c) 2014 Marc Junyent Martín. All rights reserved.
//

#ifndef Synesthesia_NOSXPlayer_h
#define Synesthesia_NOSXPlayer_h

#include <string>


class NOSXPlayer {
public:
    NOSXPlayer();

    bool loadWithURL(std::string url);
    
    bool createAssetReaderWithTimeRange(void* timeRange);

private:
    void* player;
    void* playerItem;
    void* asset;
    void* assetReader;
    void* assetReaderVideoTrackOutput;
    
    void* duration;
};

#endif
