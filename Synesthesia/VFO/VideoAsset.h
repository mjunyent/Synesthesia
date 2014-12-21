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
#include <boost/filesystem.hpp>
#include <fstream>
#include <ctime>
#include <vector>
#include <utility>
#include <set>

#include "FrameGetter.h"

#ifdef __APPLE__
#include "OSXFrameGetter.h"
#endif

namespace bfs = boost::filesystem;
/*******************
 *        |videoN/
 *               |videoFile.mp4  <- optional.
 *               |videoInfo.txt  <- info such as name, path, date,
 *               |timeTable      <- table with frame to second relation
 *               |shotBoundaries <- list of shot boundaries
 *               |histograms     <- list of frame histograms
 *               |means          <- list of means
 *
 *
 * videoInfo.txt format
 * 1 internal(0)/external(1)
 * 2 fileurl
 * 3 date
 */

class VideoAssetException: public exception {
public:
    VideoAssetException(std::string s);
    std::string s;
    virtual const char* what() const throw();
};

class VideoAsset {
public:
    VideoAsset(bfs::path path);
    VideoAsset(bfs::path videoFile,
               bfs::path path,
               bool copy);
    
    
    void process();

    bool isExternal;
    bool hasTimetable;
    bool hasShotBoundaries;
    bool hasHistograms;
    bool hasMeans;
    bool isReady; //true if has everything.

    std::string name;
    bfs::path path;
    bfs::path video_path;

    FrameGetter * player;

    std::vector< std::vector<double> > histograms;
    std::vector< std::pair<int, double> > frame2Timestamp;
    std::vector<int> boundaries;
    std::set<int> flashes;
    std::vector<rgb> rgbMeans;

private:
    void readInfoFile();
    void writeInfoFile();

    void readTimetable();
    void writeTimetable();

    void readShotBoundaries();
    void writeShotBoundaries();

    void readHistograms();
    void writeHistograms();

    void readMeans();
    void writeMeans();
    
    void loadPlayerSync();
};

#endif /* defined(__Synesthesia__VideoAsset__) */
