//
//  HistogramHSV.h
//  Synesthesia
//
//  Created by Marc Junyent Martín on 26/11/14.
//  Copyright (c) 2014 Marc Junyent Martín. All rights reserved.
//

#ifndef __Synesthesia__HistogramHSV__
#define __Synesthesia__HistogramHSV__

#include "Player.h"
#include <vector>
#include <fstream>
#include <sstream>

class HistogramHSV {
public:
    HistogramHSV(Player* p);

    void iterate();

    void save();
    void load();

    std::vector< std::vector<double> > histograms;
    std::vector<rgb> rgbMeans;



    Player* player;
    
    int binsH, binsS, binsV;
    int numBins;
    
    long framesize;
};

#endif /* defined(__Synesthesia__HistogramHSV__) */