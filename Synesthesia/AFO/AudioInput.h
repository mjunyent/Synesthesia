//
//  AudioInput.h
//  Synesthesia
//
//  Created by Marc Junyent Martín on 03/01/15.
//  Copyright (c) 2015 Marc Junyent Martín. All rights reserved.
//

#ifndef __Synesthesia__AudioInput__
#define __Synesthesia__AudioInput__

#include <cstdio>
#include <RtAudio.h>
#include <functional>
#include "Tobago.h"

#define REWIRE_OUTPUT 1

class AudioInput {
public:
    static RtAudio adc;
    static unsigned int getNumDevices();
    static RtAudio::DeviceInfo getDeviceInfo(unsigned int i);
    static void printDevicesInfo();

    static int record(void *outputBuffer,
                      void *inputBuffer,
                      unsigned int nBufferFrames,
                      double streamTime,
                      RtAudioStreamStatus status,
                      void *userData);

    AudioInput(unsigned int device=-1,
               unsigned int channel=0,
               unsigned int bufferSize=1024);

    void start(); //can throw exception.

    void record(float* in, float* out, int nbf, double st);

    void setCallback(std::function<void(float*, float*, int, double)> f);

    unsigned int device;
    unsigned int channel;
    unsigned int sampleRate;
    unsigned int bufferSize;
    
    std::function<void(float*, float*, int, double)> f;
};


#endif /* defined(__Synesthesia__AudioInput__) */
