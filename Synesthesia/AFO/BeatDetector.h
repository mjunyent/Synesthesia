//
//  BeatDetector.h
//  Synesthesia
//
//  Created by Marc Junyent Martín on 05/01/15.
//  Copyright (c) 2015 Marc Junyent Martín. All rights reserved.
//

#ifndef __Synesthesia__BeatDetector__
#define __Synesthesia__BeatDetector__

#include "Tobago.h"
#include "AudioInput.h"
#include "ffft/FFTReal.h"

#include <set>
#include <list>
#include <algorithm>
#include <utility>
#include <iterator>

#define BEAT_DEBUG


class BeatDetector {
public:
    BeatDetector(AudioInput* adc, int desiredBandsPerOctave=4); //barsPerOctave should be a power of 2.
    
    AudioInput* adc;
    ffft::FFTReal<float>* fft_object;
    
    const std::vector<bool> getBeats();

    float *rfft;
    float *fft;

    float *medrfft;
    int medSize;
    
    int numOctaves;
    std::vector<int> bandsPerOctave;
    int numBands; //we group the fft in numBands bands
    int currentReg;
    int numRegs; //we save numRegs values of each band
    std::vector< std::vector<float> > fftBands;
    std::vector<float> fftBandMeans;
    std::vector<bool> beats;
    std::vector<float> fBeats;

#ifdef BEAT_DEBUG
    float *wave;
    void renderWAVE(float v);
    void renderFFT(float rmax = 10.0f);
    bool renderBands(float rmax = 50.0f);
    
    Shader waveShad;
    VAO *waveVAO;
    VBO *waveVBO;

    Shader fftShad;
    VAO *fftVAO;
    VBO *fftVBO;
    
    Shader fftBandsShad;
    VAO *fftBandsVAO;
    VBO *fftBandsVBO;
    
private:
    void initDebug();
#endif
};

#endif /* defined(__Synesthesia__BeatDetector__) */
