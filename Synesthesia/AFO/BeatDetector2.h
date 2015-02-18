//
//  BeatDetector2.h
//  Synesthesia
//
//  Created by Marc on 24/01/15.
//  Copyright (c) 2015 Marc Junyent Martín. All rights reserved.
//

#ifndef __Synesthesia__BeatDetector2__
#define __Synesthesia__BeatDetector2__


#include <iomanip>

#include "Tobago.h"
#include "AudioInput.h"
#include "ffft/FFTReal.h"

#include <set>
#include <list>
#include <algorithm>
#include <utility>
#include <iterator>
#include <fstream>

class BeatDetector2 {
public:
    BeatDetector2(AudioInput* adc);

    void callback(float* in, float* out, int n, double t);

    AudioInput* adc;
    ffft::FFTReal<float>* fft_object;

    int timeSize;
    int freqSize;
    
    int Nremaining;
    float* remaining;
    float* tempBuffer;

    std::vector< std::vector<float> > fft;
    std::vector< std::vector<float> > rfft;
    std::vector<float> P; //median filter applied to frequencies.
    std::vector<float> H; //median filter applied to time.
    int fftPointer;

    int maskSelector;
    float* masked;
    float* output;
    
    int cP;

    
    int numOctaves;
    std::vector<int> bandsPerOctave;
    int numBands;
    int currentReg;
    int numRegs;
    
    std::vector< std::vector<float> > maskedrfft;
    std::vector< std::vector<float> > rfftBands;
    float* maskedrfftMean;
    float* maskedVariances;
    float* rfftMean;
    float* Variances;
    float* cValue;

    
    std::vector<float> fBeats;
//    std::ofstream passthrough;
//    std::ofstream percussion;
    
    
    
    
    void setupDraw();
    void draw();
    Shader fftBandsShad;
    VAO *rfftVAO;
    VBO *rfftVBO;
};

#endif /* defined(__Synesthesia__BeatDetector2__) */
