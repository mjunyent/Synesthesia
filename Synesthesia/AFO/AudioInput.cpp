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


AudioInput::AudioInput(unsigned int device, unsigned int channel, unsigned int bufferSize) : device(device), channel(channel), bufferSize(bufferSize) {
    RtAudio::StreamParameters parameters;

    if(device == -1)
        parameters.deviceId = adc.getDefaultInputDevice();
    else
        parameters.deviceId = device;
    
    std::cerr << "Device selected: " <<  getDeviceInfo(parameters.deviceId).name << std::endl;

    parameters.nChannels = 1;
    parameters.firstChannel = channel;

    bool found = false;
    for(unsigned int j=0; j<getDeviceInfo(parameters.deviceId).sampleRates.size(); j++) {
        if(getDeviceInfo(parameters.deviceId	).sampleRates[j] == 44100) found = true;
    }
    if(!found) {
        for(unsigned int j=0; j<getDeviceInfo(parameters.deviceId).sampleRates.size(); j++) {
            if(getDeviceInfo(parameters.deviceId	).sampleRates[j] == 48000) found = true;
        }
        if(!found) std::cerr << "No suitable sampling rate found for audio device!";
        else sampleRate = 48000;
    } else sampleRate = 44100;

    adc.openStream(NULL, &parameters, RTAUDIO_FLOAT32, sampleRate, &bufferSize, &AudioInput::record, this);
}

void AudioInput::start() {
    adc.startStream();
}

int AudioInput::record(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData) {
    if(status) {
        (Tobago.log)->write(Log::WARNING) << "Stream overflow detected!";
        std::cerr << "Stream overflow detected" << std::endl;
    }
    else {
        AudioInput* ai = (AudioInput*) userData;
        float* r = (float*) inputBuffer;
        ai->record(r, nBufferFrames, streamTime);
    }

    return 0;
}

void AudioInput::record(float *r, int nbf, double st) {
    this->f(r, nbf, st);
}

void AudioInput::setCallback(std::function<void (float *, int, double)> f) {
    this->f = f;
}









