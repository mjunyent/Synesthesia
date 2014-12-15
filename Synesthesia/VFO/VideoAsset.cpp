//
//  VideoAsset.cpp
//  Synesthesia
//
//  Created by Marc Junyent Martín on 15/11/14.
//  Copyright (c) 2014 Marc Junyent Martín. All rights reserved.
//

#include "VideoAsset.h"

#include "HistogramHSV.h"
#include "ShotDetector.h"

VideoAsset::VideoAsset(bfs::path path) : path(path) {
    try {
        readInfoFile();
        readTimetable();
        readShotBoundaries();
        readHistograms();
        readMeans();

        loadPlayerSync();
    } catch (const bfs::filesystem_error& ex) {
        (*Tobago.log)(Log::ERROR) << "Error loading video asset " << path.string() << ": " << ex.what();
        throw VideoAssetException(ex.what());
    }
}

VideoAsset::VideoAsset(bfs::path videoFile, bfs::path path, bool copy, bool process) : path(path) {
    try {
        if(copy) {
            video_path = path;
            video_path /= videoFile.filename();
            bfs::copy_file(videoFile, video_path, bfs::copy_option::overwrite_if_exists);
        } else video_path = videoFile;
        
        isExternal = !copy;

        loadPlayerSync();

        writeInfoFile();
        
        hasTimetable = false;
        hasShotBoundaries = false;
        hasHistograms = false;
        hasMeans = false;

        
    } catch (const bfs::filesystem_error& ex) {
        (*Tobago.log)(Log::ERROR) << "Error creating video asset " << path.string() << ": " << ex.what();
        throw VideoAssetException(ex.what());
    }
}

//TODO: function to rewing syncronously the videos.
//TODO: correct frame number getter!
void VideoAsset::process() {
    HistogramHSV histograms(player);
    
    while(!player->isMovieDone()) {
        histograms.iterate();
        frame2Timestamp.push_back(std::make_pair(player->getFrameNum(),player->getPosition()));
        player->syncNextFrame();
        
        std::cout << player->getFrameNum() << " " << player->getPosition() << std::endl;
    }
    
    this->histograms = histograms.histograms;

    histograms.save();
    writeHistograms();
    readHistograms();
    histograms.histograms = this->histograms;
    int ab; std::cin >> ab;
    histograms.save();
    
}

void VideoAsset::loadPlayerSync() {
    #ifdef __APPLE__
        player = new OSXPlayer();
    #endif
    
    player->load(video_path.string());

    if(player->isError() || !player->isLoaded())
        throw VideoAssetException("Could not load video file!");
}

void VideoAsset::readInfoFile() {
    bfs::path info_path = path;
    info_path /= "videoInfo.txt";

    if(!bfs::exists(info_path))
        throw VideoAssetException("Info file doesn't exist");
    
    std::ifstream infoFile(info_path.string());
    if(!infoFile.is_open())
        throw VideoAssetException("Info file can't be opened");
    
    int t;
    infoFile >> t;
    if(t == 0) isExternal = false;
    else if(t == 1) isExternal = true;
    else throw VideoAssetException("Info file has wrong format");

    infoFile.ignore();

    std::string str_video_path;
    getline(infoFile, str_video_path);
    
    if(isExternal) video_path = bfs::path(str_video_path);
    else {
        video_path = path;
        video_path /= str_video_path;
    }
    
    infoFile.close();
}
void VideoAsset::writeInfoFile() {
    bfs::path info_path = path;
    info_path /= "videoInfo.txt";
    
    std::ofstream infoFile(info_path.string());
    if(!infoFile.is_open())
        throw VideoAssetException("Couldn't create info file!");
    
    time_t t = time(0);  // current time: http://cplusplus.com/reference/clibrary/ctime/time/
    struct tm * now = localtime(&t);  // http://cplusplus.com/reference/clibrary/ctime/localtime/
    // struct tm: http://cplusplus.com/reference/clibrary/ctime/tm/
    
    infoFile << (isExternal?'1':'0') << std::endl <<
    (isExternal?video_path.string():video_path.filename().string()) << std::endl <<
    now->tm_hour << ":" << now->tm_min << ":" << now->tm_sec << " " <<
    now->tm_mday << "/" << (now->tm_mon+1) << "/" << (now->tm_year+1900) << std::endl;
    
    infoFile.close();
}

void VideoAsset::readTimetable() {
    bfs::path timetable_path = path;
    timetable_path /= "timeTable";

    
}

void VideoAsset::readShotBoundaries() {
    
}

void VideoAsset::readHistograms() {
    bfs::path histograms_path = path;
    histograms_path /= "histograms";

    if(!bfs::exists(histograms_path)) {
        hasHistograms = false;
        return;
    }

    std::ifstream histogramFile(histograms_path.string(), std::ifstream::binary);
    if(!histogramFile.is_open()) throw VideoAssetException("Histograms file exists but couldn't be oppened");


    unsigned long s;
    histogramFile.read(reinterpret_cast<char*>(&s), sizeof(unsigned long));

    if(!histogramFile.good() || s <= 0) throw VideoAssetException("Wrong histograms file format!");

    histograms = std::vector< std::vector<double> >(s, std::vector<double>(24));

    for(unsigned long i=0; i<s; i++) {
        histogramFile.read(reinterpret_cast<char*>(&histograms[i][0]), 24*sizeof(double));
        if(!histogramFile.good()) throw VideoAssetException("Wrong histograms file format!");
    }

    hasHistograms = true;
}

void VideoAsset::writeHistograms() {
    bfs::path histograms_path = path;
    histograms_path /= "histograms";
    
    std::ofstream histogramFile(histograms_path.string(), std::ofstream::binary);
    if(!histogramFile.is_open()) throw VideoAssetException("Could not open or create histogram file to write");

    unsigned long s = histograms.size();
    histogramFile.write(reinterpret_cast<const char*>(&s), sizeof(unsigned long));
    for(std::vector<double>& vd : histograms)
        histogramFile.write(reinterpret_cast<const char*>(&vd[0]), sizeof(double)*vd.size());

    histogramFile.close();
}

void VideoAsset::readMeans() {
    
}






VideoAssetException::VideoAssetException(std::string s) : s(s) {
}

const char* VideoAssetException::what() const throw() {
    return s.c_str();
}


