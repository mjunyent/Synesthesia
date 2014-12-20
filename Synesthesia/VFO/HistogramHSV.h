//
//  HistogramHSV.h
//  Synesthesia
//
//  Created by Marc Junyent Martín on 26/11/14.
//  Copyright (c) 2014 Marc Junyent Martín. All rights reserved.
//

#ifndef __Synesthesia__HistogramHSV__
#define __Synesthesia__HistogramHSV__

#include "FrameGetter.h"
#include <vector>
#include <fstream>
#include <sstream>

class HistogramHSV {
public:
    HistogramHSV(FrameGetter* p);

    void iterate();
    void iterateGPU();
    void iterateCPU();

    void save();
    void load();

    std::vector< std::vector<double> > histograms;
    std::vector<rgb> rgbMeans;



    FrameGetter* player;
    
    int binsH, binsS, binsV;
    int numBins;
    
    long framesize;
    bool useOCL;
private:

    cl_program program;
    cl_kernel histogram_kernel;
    cl_kernel sum_partials_kernel;
    cl_mem histogram_buffer;
    cl_mem partial_histogram_buffer;
    
    size_t workgroup_size_hk;
    size_t workgroup_size_spk;
    size_t global_work_size[2];
    size_t local_work_size[2];
    size_t partial_global_work_size[2];
    size_t partial_local_work_size[2];
    size_t num_groups;
};

#endif /* defined(__Synesthesia__HistogramHSV__) */
