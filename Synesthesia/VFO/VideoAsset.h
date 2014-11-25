//
//  VideoAsset.h
//  Synesthesia
//
//  Created by Marc Junyent Martín on 15/11/14.
//  Copyright (c) 2014 Marc Junyent Martín. All rights reserved.
//

#ifndef __Synesthesia__VideoAsset__
#define __Synesthesia__VideoAsset__

#include <string>
#include "Player.h"

#ifdef TARGET_OS_MAC
#include "OSXPlayer.h"
#endif

class VideoAsset {
public:
    VideoAsset(std::string url);

    std::string name;
    std::string url;
    
    Player* player;
    //vector<whatever> shotboundaries;
    //whatever histograms;
    //features...
};

#endif /* defined(__Synesthesia__VideoAsset__) */
