//
//  AssetLibrary.cpp
//  Synesthesia
//
//  Created by Marc Junyent Martín on 27/11/14.
//  Copyright (c) 2014 Marc Junyent Martín. All rights reserved.
//

#include "AssetLibrary.h"
#include <algorithm>

AssetLibrary::AssetLibrary(std::string url) : path(url) {
    try {
        if(!bfs::exists(path))
            (*Tobago.log)(Log::ERROR) << "Specified path doesn't exist: " << path.string();
        if(!bfs::is_directory(path))
            (*Tobago.log)(Log::ERROR) << "Specified path is not a directory: " << path.string();

        std::vector<bfs::path> v;

        std::copy(bfs::directory_iterator(path), bfs::directory_iterator(),
                  std::back_inserter(v));
        
        std::sort(v.begin(), v.end());

        bool found = false;

        for(bfs::path& p : v) {
            if(p.filename() == "video") {
                found = true;
                video_path = p;
            }
        }
        
        if(!found) {
            video_path = path;
            video_path /= "video";
            if(!bfs::create_directory(video_path))
                (*Tobago.log)(Log::ERROR) << "Could not create video folder " << video_path.string();
        } else {
            if(!bfs::is_directory(video_path))
                (*Tobago.log)(Log::ERROR) << "Specified path is not a directory: " << video_path.string();
        }
    } catch (const bfs::filesystem_error& ex) {
        (*Tobago.log)(Log::ERROR) << "Exception trying to open " << path.string() << ": " << ex.what();
    }
}

void AssetLibrary::loadAssets() {
    scanVideoFolder();
    
    try {
        for(bfs::path& p : vvideo_paths) {
            if(bfs::is_directory(p)) { //we ignore files that are not a directory.
                std::cout << p << std::endl;
                try {
                    VideoAsset v(p);
                } catch (exception& e) {
                    std::cout << p << std::endl;
                    std::cout << e.what() << std::endl;
                }
//                if(v.isValid()) {}
            }
        }
    } catch (const bfs::filesystem_error& ex) {
        (*Tobago.log)(Log::ERROR) << "Exception trying to open something inside " << video_path.string() << ": " << ex.what();
    }
}

void AssetLibrary::scanVideoFolder() {
    try {
        vvideo_paths = std::vector<bfs::path>();
        std::copy(bfs::directory_iterator(video_path), bfs::directory_iterator(), std::back_inserter(vvideo_paths));
        std::sort(vvideo_paths.begin(), vvideo_paths.end());
    } catch (const bfs::filesystem_error& ex) {
        (*Tobago.log)(Log::ERROR) << "Exception trying to open " << video_path.string() << ": " << ex.what();
    }
}



























