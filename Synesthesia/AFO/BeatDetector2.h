//
//  BeatDetector2.h
//  Synesthesia
//
//  Created by Marc on 24/01/15.
//  Copyright (c) 2015 Marc Junyent Martín. All rights reserved.
//

#ifndef __Synesthesia__BeatDetector2__
#define __Synesthesia__BeatDetector2__


#include "Tobago.h"
#include "AudioInput.h"
#include "ffft/FFTReal.h"

#include <set>
#include <list>
#include <algorithm>
#include <utility>
#include <iterator>

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
};

#endif /* defined(__Synesthesia__BeatDetector2__) */
