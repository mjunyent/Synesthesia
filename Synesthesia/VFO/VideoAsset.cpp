//
//  VideoAsset.cpp
//  Synesthesia
//
//  Created by Marc Junyent Martín on 15/11/14.
//  Copyright (c) 2014 Marc Junyent Martín. All rights reserved.
//

#include "VideoAsset.h"

VideoAsset::VideoAsset(std::string url) {
    this->url = url;
    
#ifdef TARGET_OS_MAC
    player = new OSXPlayer();
#endif
    
    player->load(this->url);
}