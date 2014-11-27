//
//  AssetLibrary.h
//  Synesthesia
//
//  Created by Marc Junyent Martín on 27/11/14.
//  Copyright (c) 2014 Marc Junyent Martín. All rights reserved.
//

#ifndef __Synesthesia__AssetLibrary__
#define __Synesthesia__AssetLibrary__

#include <string>
#include <VideoAsset.h>

class AssetLibrary {
public:
    AssetLibrary(std::string path);
    
    void loadAssets();
    void loadAsset(std::string name);

    bool exists(std::string name);

    void addAsset(VideoAsset* va);
    void removeAsset(VideoAsset* va);
    

    std::vector<VideoAsset*> va;
};

#endif /* defined(__Synesthesia__AssetLibrary__) */
