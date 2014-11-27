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

#ifdef __APPLE__
#include "OSXPlayer.h"
#endif


class VideoAsset {
public:
    VideoAsset(std::string url);

    bool valid;
    bool parsed;

    std::string url;
    std::string path;
    std::string name;
    std::string extension;

    Player* player;

    //class to read info from files and those things.
    //vector<whatever> shotboundaries;
    //whatever histograms;
    //features...
};

#endif /* defined(__Synesthesia__VideoAsset__) */
