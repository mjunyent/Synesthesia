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

#define BEAT_DEBUG

class BeatDetector {
public:
    BeatDetector(AudioInput* adc);
    
    AudioInput* adc;
    ffft::FFTReal<float>* fft_object;

    float *rfft;
    float *fft;
    
    int numBands; //we group the fft in numBands bands
    int numElementsInBand;
    int currentReg;
    int numRegs; //we save numRegs values of each band
    std::vector< std::vector<float> > fftBands;
    std::vector<float> fftBandMeans;

#ifdef BEAT_DEBUG
    float *wave;
    void renderWAVE(float v);
    void renderFFT(float rmax = 10.0f);
    bool renderBands(float rmax = 1.0f);
    
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
