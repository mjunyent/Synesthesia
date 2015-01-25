//
//  BeatDetector2.cpp
//  Synesthesia
//
//  Created by Marc on 24/01/15.
//  Copyright (c) 2015 Marc Junyent Martín. All rights reserved.
//

#include "BeatDetector2.h"

BeatDetector2::BeatDetector2(AudioInput* adc) : adc(adc) {
    if(adc->bufferSize != 768) {
        throw 4;
    }

    fft_object = new ffft::FFTReal<float>(1024);

    tempBuffer = (float*)malloc(sizeof(float)*1024);
    remaining = (float*)malloc(sizeof(float)*256);
    Nremaining = 256;
    
    timeSize = 7;
    freqSize = 7;

    fft = std::vector< std::vector<float> >(timeSize, std::vector<float>(1024, 0.0));
    rfft = std::vector< std::vector<float> >(timeSize, std::vector<float>(512, 0.0));
    P = std::vector<float>(512, 0.0);
    H = std::vector<float>(512, 0.0);
    fftPointer = 0;

    output = (float*)malloc(sizeof(float)*1024);
    masked = (float*)malloc(sizeof(float)*1024);
    maskSelector = 0;

    adc->setCallback([&](float* in, float* out, int n, double t) { this->callback(in, out, n, t); });
}

void BeatDetector2::callback(float *in, float* out, int n, double t) {
    memcpy(tempBuffer, remaining, sizeof(float)*Nremaining);
    memcpy(tempBuffer+Nremaining, in, sizeof(float)*n);
    memcpy(remaining, in+(n-Nremaining), sizeof(float)*Nremaining);

    fft_object->do_fft(&fft[fftPointer][0], tempBuffer);

    rfft[fftPointer][0] = std::fabs(fft[fftPointer][0]);
    for(int i=1; i<512; i++) {
        rfft[fftPointer][i] = sqrt( fft[fftPointer][i]*fft[fftPointer][i] +
                                    fft[fftPointer][i+512]*fft[fftPointer][i+512]);
    }
    
    int jumpBack = timeSize/2;
    int cP;
    if(fftPointer < jumpBack) {
        cP = timeSize - (jumpBack-fftPointer);
    } else {
        cP = fftPointer-jumpBack;
    }

    for(int i=0; i<512; i++) {
        std::vector<float> median;
        for(int j=std::max(0, i-freqSize/2); j<=std::min(511, i+freqSize/2); j++) {
            median.push_back(rfft[cP][j]);
        }
        sort(median.begin(), median.end());

        P[i] = median[median.size()/2];
    }
    
    
    std::vector<float> median(timeSize);
    for(int i=0; i<512; i++) {
        for(int j=0; j<timeSize; j++) {
            median[j] = rfft[j][i];
        }
        sort(median.begin(), median.end());
        
        H[i] = median[median.size()/2];
    }


    if(maskSelector == 0) {
        float m = P[0]*P[0] / (H[0]*H[0] + P[0]*P[0]);
        masked[0] = m*fft[cP][0];

        for(int i=1; i<512; i++) {
            float m = P[i]*P[i] / (H[i]*H[i] + P[i]*P[i]);
            masked[i] = m*fft[cP][i];
            masked[i+512] = m*fft[cP][i];
        }
        
        fft_object->do_ifft(masked, output);
        fft_object->rescale(output);


        memcpy(out, output+256, sizeof(float)*768);
    }


    fftPointer++;
    fftPointer %= timeSize;
}

