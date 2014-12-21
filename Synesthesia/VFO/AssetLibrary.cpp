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
            if(!bfs::create_directory(video_path)) {
                (*Tobago.log)(Log::ERROR) << "Could not create video folder " << video_path.string();
                exit(EXIT_FAILURE);
            }
        } else {
            if(!bfs::is_directory(video_path)) {
                (*Tobago.log)(Log::ERROR) << "Specified path is not a directory: " << video_path.string();
                exit(EXIT_FAILURE);
            }
        }
    } catch (const bfs::filesystem_error& ex) {
        (*Tobago.log)(Log::ERROR) << "Exception trying to open " << path.string() << ": " << ex.what();
        exit(EXIT_FAILURE);
    }
}

void AssetLibrary::loadAssets() {
    scanVideoFolder();
    
    try {
        for(bfs::path& p : vvideo_paths) {
            if(bfs::is_directory(p)) { //we ignore files that are not a directory.
                loadAsset(p);
            }
        }
    } catch (const bfs::filesystem_error& ex) {
        (*Tobago.log)(Log::ERROR) << "Exception trying to open something inside " << video_path.string() << ": " << ex.what();
        exit(EXIT_FAILURE);
    }
}

void AssetLibrary::loadAsset(bfs::path p) {
    try {
        VideoAsset v(p);
        va.push_back(v);
    } catch (exception& e) {
        Tobago.log->write(Log::WARNING) << "Exception loading asset " << p << ": " << e.what() << " - The asset has been ignored.";
    }
}

bool AssetLibrary::addAsset(bfs::path videoFile, bool copy) {
    bfs::path assetPath = video_path;
    assetPath /= videoFile.stem().string();

    int i=1;
    while(bfs::exists(assetPath)) {
        assetPath = video_path;
        assetPath /= videoFile.stem().string() + std::to_string(i);
        i++;
    }

    if(!bfs::create_directory(assetPath)) {
        (*Tobago.log)(Log::ERROR) << "Could not create asset folder " << assetPath.string();
        return false;
    }

    try {
        VideoAsset v(videoFile, assetPath, copy);
        va.push_back(v);
    } catch(exception& e) {
        Tobago.log->write(Log::WARNING) << "Exception creating asset " << videoFile << ": " << e.what() << " - The asset has not been created.";
        bfs::remove_all(assetPath);
        return false;
    }
    return true;
}

void AssetLibrary::removeAsset(std::string name) {
    std::vector<VideoAsset> cva;

    for(VideoAsset& v : va) {
        if(v.name == name) {
            bfs::remove_all(v.path);
        } else {
            cva.push_back(v);
        }
    }
    
    va = cva;
}

void AssetLibrary::scanVideoFolder() {
    try {
        vvideo_paths = std::vector<bfs::path>();
        std::copy(bfs::directory_iterator(video_path), bfs::directory_iterator(), std::back_inserter(vvideo_paths));
        std::sort(vvideo_paths.begin(), vvideo_paths.end());
    } catch (const bfs::filesystem_error& ex) {
        (*Tobago.log)(Log::ERROR) << "Exception trying to open " << video_path.string() << ": " << ex.what();
        exit(EXIT_FAILURE);
    }
}


void AssetLibrary::process() {
    for(VideoAsset& v : va) {
        std::cout << v.name << std::endl;
        if(!v.isReady) v.process();
    }
}


void AssetLibrary::listAssets() {
    for(VideoAsset& v : va) {
        std::cout << v.name << std::endl;
    }
}






