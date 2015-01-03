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

class AudioInput {
public:
    static RtAudio adc;
    static unsigned int getNumDevices();
    static RtAudio::DeviceInfo getDeviceInfo(unsigned int i);
    static void printDevicesInfo();

    AudioInput(int device=-1);

    
};


#endif /* defined(__Synesthesia__AudioInput__) */
