//
//  ShotDetector.h
//  Synesthesia
//
//  Created by Marc Junyent Martín on 26/11/14.
//  Copyright (c) 2014 Marc Junyent Martín. All rights reserved.
//

#ifndef __Synesthesia__ShotDetector__
#define __Synesthesia__ShotDetector__

#include "HistogramHSV.h"
#include <vector>
#include <set>


class ShotDetector {
public:
    ShotDetector(HistogramHSV* histograms);

    void process();

    std::vector<int> boundaries;
    std::set<int> flashes;

private:
    HistogramHSV* histograms;
    int currentFrame;
    
    void findFlashes();
    int FLASH_MAX_LENGTH = 9;
    double FLASH_THRESHOLD_MIN = 0.04;
    double FLASH_THRESHOLD_MAX = 0.15;
    int FLASH_MAX_FRAMES_UP = 2;
    
    void findShots();
    int WINDOW = 12;
    double THRESHOLD = 0.15;
    
    bool start_transition = false;
    int previous_rank = 0;
    int rank = 0;
    int possibility = 0;
};

#endif /* defined(__Synesthesia__ShotDetector__) */
