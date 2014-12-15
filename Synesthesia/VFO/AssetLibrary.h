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
#include <boost/filesystem.hpp>

namespace bfs = boost::filesystem;

/* Folder structure
 * /                        <- path
 * |
 * |video/                 <- video asset information stored here
 *        |video1/
 *        |video2/
 *        |video3/
 *        |....
 *        |videoN/
 *               |videoFile.mp4  <- optional.
 *               |videoInfo.txt  <- info such as name, path, date,
 *               |timeTable      <- table with frame to second relation
 *               |shotBoundaries <- list of shot boundaries
 *               |histograms     <- list of frame histograms
 *               |means          <- list of means
 *
 */

class AssetLibrary {
public:
    AssetLibrary(std::string path);

    void loadAssets();
//    void loadAsset(std::string name);

//    bool exists(std::string name);

//    void addAsset(VideoAsset* va);
//    void removeAsset(VideoAsset* va);
    

//    std::vector<VideoAsset*> va;
    
private:
    bfs::path path;
    bfs::path video_path;
    
    void scanVideoFolder();
    std::vector<bfs::path> vvideo_paths;
};

#endif /* defined(__Synesthesia__AssetLibrary__) */
