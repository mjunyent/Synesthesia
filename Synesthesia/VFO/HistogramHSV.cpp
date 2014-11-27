//
//  HistogramHSV.cpp
//  Synesthesia
//
//  Created by Marc Junyent Martín on 26/11/14.
//  Copyright (c) 2014 Marc Junyent Martín. All rights reserved.
//

#include "HistogramHSV.h"

#include <OpenCL/cl.h>
#include <OpenCL/cl_gl_ext.h>
#include <OpenGL/OpenGL.h>

HistogramHSV::HistogramHSV(Player* p) {
    this->player = p;
    
    binsH = 16;
    binsS = 4;
    binsV = 4;
    numBins = binsH + binsS + binsV;
    
    framesize = player->getWidth()*player->getHeight();
    /*
    // Get current CGL Context and CGL Share group
    CGLContextObj kCGLContext = CGLGetCurrentContext();
    CGLShareGroupObj kCGLShareGroup = CGLGetShareGroup(kCGLContext);
    // Create CL context properties, add handle & share-group enum !
    cl_context_properties properties[] = {
        CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE,
        (cl_context_properties)kCGLShareGroup, 0
    };
    // Create a context with device in the CGL share group
    cl_context context = clCreateContext(properties, 0, 0, NULL, 0, 0);
    
    std::cout << context << std::endl;*/
}

void HistogramHSV::iterate() {
    unsigned char* image = player->getPixels();
    
    std::vector<double> hist(numBins, 0.0);
    rgb mean = {0.0, 0.0, 0.0};

    for(int i=0; i<player->getNumChannels()*framesize; i+=player->getNumChannels()) {
        hsv chsv = rgb2hsv((rgb){image[i]/255.0, image[i+1]/255.0, image[i+2]/255.0});

        if(!std::isnan(chsv.h)) hist[std::min(binsH-1, (int)(chsv.h/360.0*binsH))]++;
        hist[binsH+std::min(binsS-1, (int)(chsv.s*binsS))]++;
        hist[binsH+binsS+std::min(binsV-1, (int)(chsv.v*binsV))]++;
        
        mean.r += image[i]/255.0;
        mean.g += image[i+1]/255.0;
        mean.b += image[i+1]/255.0;
    }

    for(double& d : hist) d /= framesize;
    
    mean.r /= framesize;
    mean.g /= framesize;
    mean.b /= framesize;

    histograms.push_back(hist);
    rgbMeans.push_back(mean);
}

void HistogramHSV::save() {
    std::ofstream output("tempHist.txt");

    for(std::vector<double>& h : histograms) {
        for(double& v : h) output << v << " ";
        output << std::endl;
    }

    output.close();
    
    std::ofstream output2("tempMeans.txt");
    
    for(rgb& c : rgbMeans) {
        output2 << c.r << " " << c.g << " " << c.b << std::endl;
    }
    
    output2.close();
}

void HistogramHSV::load() {
    std::ifstream input1("tempHist.txt");
    std::string line;
    
    while(getline(input1, line)) {
        if(line=="-1") break;
        if(line=="") continue;

        std::stringstream in(line);
        
        std::vector<double> tmpHist(numBins);

        for(int i=0; i<tmpHist.size(); i++) {
            in >> tmpHist[i];
        }
        
        histograms.push_back(tmpHist);
    }
    input1.close();

    std::ifstream input2("tempMeans.txt");
    rgb t;
    while(input2 >> t.r) {
        input2 >> t.g >> t.b;
        rgbMeans.push_back(t);
    }
    input2.close();
    
    std::cout << histograms.size() << " " << rgbMeans.size() << std::endl;
}