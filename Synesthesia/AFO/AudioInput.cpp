//
//  AudioInput.cpp
//  Synesthesia
//
//  Created by Marc Junyent Martín on 03/01/15.
//  Copyright (c) 2015 Marc Junyent Martín. All rights reserved.
//

#include "AudioInput.h"

RtAudio AudioInput::adc;

unsigned int AudioInput::getNumDevices() {
    return adc.getDeviceCount();
}

RtAudio::DeviceInfo AudioInput::getDeviceInfo(unsigned int i) {
    return adc.getDeviceInfo(i);
}

void AudioInput::printDevicesInfo() {
    RtAudio::DeviceInfo info;
    
    for(int i=0; i<getNumDevices(); i++) {
        info = getDeviceInfo(i);

        std::cout << "\nDevice Name = " << info.name << '\n';
        if ( info.probed == false )
            std::cout << "Probe Status = UNsuccessful\n";
        else {
            std::cout << "Probe Status = Successful\n";
            std::cout << "Output Channels = " << info.outputChannels << '\n';
            std::cout << "Input Channels = " << info.inputChannels << '\n';
            std::cout << "Duplex Channels = " << info.duplexChannels << '\n';
            if ( info.isDefaultOutput ) std::cout << "This is the default output device.\n";
            else std::cout << "This is NOT the default output device.\n";
            if ( info.isDefaultInput ) std::cout << "This is the default input device.\n";
            else std::cout << "This is NOT the default input device.\n";
            if ( info.nativeFormats == 0 )
                std::cout << "No natively supported data formats(?)!";
            else {
                std::cout << "Natively supported data formats:\n";
                if ( info.nativeFormats & RTAUDIO_SINT8 )
                    std::cout << "  8-bit int\n";
                if ( info.nativeFormats & RTAUDIO_SINT16 )
                    std::cout << "  16-bit int\n";
                if ( info.nativeFormats & RTAUDIO_SINT24 )
                    std::cout << "  24-bit int\n";
                if ( info.nativeFormats & RTAUDIO_SINT32 )
                    std::cout << "  32-bit int\n";
                if ( info.nativeFormats & RTAUDIO_FLOAT32 )
                    std::cout << "  32-bit float\n";
                if ( info.nativeFormats & RTAUDIO_FLOAT64 )
                    std::cout << "  64-bit float\n";
            }
            if ( info.sampleRates.size() < 1 )
                std::cout << "No supported sample rates found!";
            else {
                std::cout << "Supported sample rates = ";
                for (unsigned int j=0; j<info.sampleRates.size(); j++)
                    std::cout << info.sampleRates[j] << " ";
            }
            std::cout << std::endl;
        }
    }
}

