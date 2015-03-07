//
//  BeatDetector2.cpp
//  Synesthesia
//
//  Created by Marc on 24/01/15.
//  Copyright (c) 2015 Marc Junyent Martín. All rights reserved.
//

#include "BeatDetector2.h"


template <typename T>
void write(std::ofstream& stream, const T& t) {
    stream.write((const char*)&t, sizeof(T));
}

template <typename T>
void writeFormat(std::ofstream& stream) {
    write<short>(stream, 1);
}

template <>
void writeFormat<float>(std::ofstream& stream) {
    write<short>(stream, 3);
}

template <typename SampleType>
void writeWAVData(char const* outFile, SampleType* buf, size_t bufSize, int sampleRate, short channels) {
    std::ofstream stream(outFile, std::ios::binary);
    stream.write("RIFF", 4);
    write<int>(stream, 36 + bufSize);
    stream.write("WAVE", 4);
    stream.write("fmt ", 4);
    write<int>(stream, 16);
    writeFormat<SampleType>(stream);                                // Format
    write<short>(stream, channels);                                 // Channels
    write<int>(stream, sampleRate);                                 // Sample Rate
    write<int>(stream, sampleRate * channels * sizeof(SampleType)); // Byterate
    write<short>(stream, channels * sizeof(SampleType));            // Frame size
    write<short>(stream, 8 * sizeof(SampleType));                   // Bits per sample
    stream.write("data", 4);
    stream.write((const char*)&bufSize, 4);
    stream.write((const char*)buf, bufSize);
}


BeatDetector2::BeatDetector2(AudioInput* adc) : adc(adc) {
    if(adc->bufferSize != 768) {
        throw 4;
    }


    fft_object = new ffft::FFTReal<float>(1024);

    tempBuffer = (float*)malloc(sizeof(float)*1024);
    remaining = (float*)malloc(sizeof(float)*256);
    Nremaining = 256;

    writeWAVData("mySound.wav", tempBuffer, 1024, 44100, 1);

    timeSize = 7;
    freqSize = 7;

    fft = std::vector< std::vector<float> >(timeSize, std::vector<float>(1024, 0.0));
    rfft = std::vector< std::vector<float> >(timeSize, std::vector<float>(512, 0.0));

    energies = std::vector<float>(timeSize, 0.0);
    displayEnergies = std::vector<float>(1024, 0.0);
    displayMaskedEnergies = std::vector<float>(1024, 0.0);
    displayKickEnergy = std::vector<float>(1024, 0.0);
    displaySnareEnergy = std::vector<float>(1024, 0.0);
    displayCymbalEnergy = std::vector<float>(1024, 0.0);
    displayKickEnergyComp = std::vector<float>(1024, 0.0);
    displaySnareEnergyComp = std::vector<float>(1024, 0.0);
    displayCymbalEnergyComp = std::vector<float>(1024, 0.0);
    displayKickCValue = std::vector<float>(1024, 0.0);
    displaySnareCValue = std::vector<float>(1024, 0.0);
    displayCymbalCValue = std::vector<float>(1024, 0.0);
    displayEnergiesP = 0;

    P = std::vector<float>(512, 0.0);
    H = std::vector<float>(512, 0.0);
    fftPointer = 0;

    output = (float*)malloc(sizeof(float)*1024);
    masked = (float*)malloc(sizeof(float)*1024);
    maskSelector = 0;


    int desiredBandsPerOctave=4;

    numOctaves = std::log2(adc->bufferSize/2);
    bandsPerOctave = std::vector<int>(numOctaves);
    int numSamples = 1;
    numBands = 0;
    for (int i=0; i<numOctaves; i++) {
        if(numSamples < desiredBandsPerOctave) {
            bandsPerOctave[i] = numSamples;
            numBands += numSamples;
        }
        else {
            bandsPerOctave[i] = desiredBandsPerOctave;
            numBands += desiredBandsPerOctave;
        }

        numSamples <<= 1;
    }

    //HARDCODED
    numBands = 3;

    numRegs = 70;
    currentReg = -1;
    maskedrfft = std::vector< std::vector<float> > (numRegs, std::vector<float>(numBands, 0.0f));
    rfftBands = std::vector< std::vector<float> > (numRegs, std::vector<float>(numBands, 0.0f));
    maskedrfftMean = (float*)malloc(sizeof(float)*numBands);
    maskedVariances = (float*)malloc(sizeof(float)*numBands);
    rfftMean = (float*)malloc(sizeof(float)*numBands);
    Variances = (float*)malloc(sizeof(float)*numBands);
    showmaskedrfft = (float*)malloc(sizeof(float)*512);

    cValue = (float*)malloc(sizeof(float)*numBands);

    fBeats = std::vector<float>(numBands, 0.0f);

    cP = 0;


//    passthrough = std::ofstream("pass.raw", std::ios::binary);
//    percussion  = std::ofstream("perc.raw", std::ios::binary);

    adc->setCallback([&](float* in, float* out, int n, double t) { this->callback(in, out, n, t); });
}

void BeatDetector2::callback(float *in, float* out, int n, double t) {
    //Copy to tempBuffer last 256 samples
    memcpy(tempBuffer, remaining, sizeof(float)*Nremaining);
    //Copy to tempBuffer current 768 samples
    memcpy(tempBuffer+Nremaining, in, sizeof(float)*n);
    //Copy to remaining last current 256 samples
    memcpy(remaining, in+(n-Nremaining), sizeof(float)*Nremaining);




    //Calculate energies from tempBuffer
    energies[fftPointer] = 0.0;
    for(int i=0; i<1024; i++) {
        energies[fftPointer] += tempBuffer[i]*tempBuffer[i];
    }
    energies[fftPointer] /= 1024.0f;
    displayEnergies[displayEnergiesP] = energies[fftPointer];




    //Do FFT on tempbuffer
    fft_object->do_fft(&fft[fftPointer][0], tempBuffer);

    //save on rfft real fft values.
    rfft[fftPointer][0] = std::fabs(fft[fftPointer][0]);
    for(int i=1; i<512; i++) {
        rfft[fftPointer][i] = sqrt( fft[fftPointer][i]*fft[fftPointer][i] +
                                    fft[fftPointer][i+512]*fft[fftPointer][i+512]);
    }


    //Calculate median on freq axis (P).
    int jumpBack = timeSize/2;
    jumpBack = 3;

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

    
    //Calculate median on time axis (H).
    std::vector<float> median(timeSize);
    for(int i=0; i<512; i++) {
        for(int j=0; j<timeSize; j++) {
            median[j] = rfft[j][i];
        }
        sort(median.begin(), median.end());
        H[i] = median[(median.size())/2];
    }


    //For each band calculate it's last seconds average.
    for(int i=0; i<numBands; i++) {
        maskedrfftMean[i] = 0.0f;
        rfftMean[i] = 0.0f;
        for(int j=0; j<numRegs; j++) {
            maskedrfftMean[i] += maskedrfft[j][i];
            rfftMean[i] += rfftBands[j][i];
        }
        maskedrfftMean[i] /= float(numRegs);
        rfftMean[i] /= float(numRegs);
    }

    //Calculate variances.
    for(int i=0; i<numBands; i++) {
        maskedVariances[i] = 0.0f;
        Variances[i] = 0.0f;
        for(int j=0; j<numRegs; j++) {
            maskedVariances[i] += std::pow(maskedrfft[j][i]-maskedrfftMean[i], 2.0);
            Variances[i] += std::pow(rfftBands[j][i]-rfftMean[i], 2.0);
        }
        maskedVariances[i] /= float(numRegs);
        Variances[i] /= float(numRegs);
    }

    currentReg++;
    currentReg %= numRegs;
    
    //Parse results on the masked rfft.
    if(maskSelector == 0) {
        //Calculate masked rfft for percussive sounds.
        for(int i=0; i<512; i++) {
            float m = (P[i]*P[i]) / (H[i]*H[i] + P[i]*P[i]);
            masked[i] = m*fft[cP][i];
            masked[i+512] = m*fft[cP][i+512];
            showmaskedrfft[i] = m*rfft[cP][i];
        }

        //Display energies.
        displayEnergies[displayEnergiesP] = 0.0;
        displayMaskedEnergies[displayEnergiesP] = 0.0;
        for(int i=0; i<512; i++) {
            displayEnergies[displayEnergiesP] += rfft[cP][i];
            displayMaskedEnergies[displayEnergiesP] += showmaskedrfft[i];
        }
        displayEnergies[displayEnergiesP] /= 512.0;
        displayMaskedEnergies[displayEnergiesP] /= 512.0;

        //Group rfft in 3 bands for masked and unmasked.
        //BAND 1 :: [0,3)
        maskedrfft[currentReg][0] = 0.0;
        rfftBands[currentReg][0] = 0.0;
        for(int i=0; i<3; i++) {
            float m = (P[i]*P[i]) / (H[i]*H[i] + P[i]*P[i]);
            maskedrfft[currentReg][0] += m*rfft[cP][i];
            rfftBands[currentReg][0] += rfft[cP][i];
        }
        maskedrfft[currentReg][0] /= 3.0;
        rfftBands[currentReg][0] /= 3.0;
        displayKickEnergy[displayEnergiesP] = maskedrfft[currentReg][0];

        //BAND 2 :: [4,11)
        maskedrfft[currentReg][1] = 0.0;
        rfftBands[currentReg][1] = 0.0;
        for(int i=4; i<11; i++) {
            float m = (P[i]*P[i]) / (H[i]*H[i] + P[i]*P[i]);
            maskedrfft[currentReg][1] += m*rfft[cP][i];
            rfftBands[currentReg][1] += rfft[cP][i];
        }
        maskedrfft[currentReg][1] /= 7.0;
        rfftBands[currentReg][1] /= 7.0;
        displaySnareEnergy[displayEnergiesP] = maskedrfft[currentReg][1];

        //BAND 2 :: [100,200)
        maskedrfft[currentReg][2] = 0.0;
        rfftBands[currentReg][2] = 0.0;
        for(int i=100; i<200; i++) {
            float m = (P[i]*P[i]) / (H[i]*H[i] + P[i]*P[i]);
            maskedrfft[currentReg][2] += m*rfft[cP][i];
            rfftBands[currentReg][2] += rfft[cP][i];
        }
        maskedrfft[currentReg][2] /= 100.0;
        rfftBands[currentReg][2] /= 100.0;
        displayCymbalEnergy[displayEnergiesP] = maskedrfft[currentReg][2];


        //For each band look if there is a beat.
        for(int i=0; i<numBands; i++) {
            float c = (-1.0/300.0)*maskedVariances[i] + 2.6;
            cValue[i] = c*maskedrfftMean[i]; //Save c*mrfft to display.

            if(i==0) {
                displayKickCValue[displayEnergiesP] = c-2.5;
                displayKickEnergyComp[displayEnergiesP] = c*maskedrfftMean[i];
            } else if(i==1) {
                displaySnareCValue[displayEnergiesP] = c-2.5;
                displaySnareEnergyComp[displayEnergiesP] = c*maskedrfftMean[i];
            } else if (i==2) {
                displayCymbalCValue[displayEnergiesP] = c-2.5;
                displayCymbalEnergyComp[displayEnergiesP] = c*maskedrfftMean[i];
            }
            fBeats[i] = std::max(0.0, fBeats[i]-10.0*n/44100.0); //Decrease fbeats.
            if(maskedrfft[currentReg][i] >= c*maskedrfftMean[i]) fBeats[i] = 1.2;
        }


        //Inverse masked fft and copy to the output.
        fft_object->do_ifft(masked, output);
        fft_object->rescale(output);
        memcpy(out, output+256, sizeof(float)*768);

        //Save to a file.
        //percussion.write((const char*)(output+256), sizeof(float)*768);
    }

    //Increase counters.
    displayEnergiesP++;
    displayEnergiesP %= 1024;
    fftPointer++;
    fftPointer %= timeSize;
}

void BeatDetector2::setupDraw() {
    waveShad.loadFromString(GL_VERTEX_SHADER,
                            "#version 330 core\
                            layout(location = 0) in float vertexPosition;\
                            uniform int npoints;\
                            uniform float yOffset;\
                            uniform float K;\
                            void main(){\
                            gl_Position =  vec4((2.0*gl_VertexID)/float(npoints-1.0) - 1.0, vertexPosition*K+yOffset, 0, 1);\
                            }");
    waveShad.loadFromString(GL_FRAGMENT_SHADER,
                            "#version 330 core\
                            layout(location = 0) out vec4 color;\
                            uniform vec3 bb;\
                            void main(){\
                            color = vec4(bb, 1.0);\
                            }");
    waveShad.link();
    waveShad.addUniform("yOffset");
    waveShad.addUniform("npoints");
    waveShad.addUniform("bb");
    waveShad.addUniform("K");
    
    waveVAO = new VAO(GL_LINE_STRIP);
    waveVBO = new VBO(displayEnergies);
    waveVAO->addAttribute(0, 1, waveVBO);

    
    
    rfftVAO = new VAO(GL_POINTS);
    rfftVBO = new VBO(&rfft[0][0], 512);
    rfftVAO->addAttribute(0, 1, rfftVBO);
    
    fftBandsShad.loadFromString(GL_VERTEX_SHADER,
                                "#version 330 core\
                                layout(location = 0) in float vertexPosition;\
                                uniform int npoints;\
                                uniform float rmax;\
                                uniform float offset;\
                                void main(){\
                                gl_Position =  vec4((2.0*gl_VertexID)/float(npoints) - 1.0 + offset/float(npoints+1), vertexPosition/rmax-1.0, 0, 1);\
                                }");
    fftBandsShad.loadFromString(GL_GEOMETRY_SHADER,
                                "#version 150 core\
                                layout (points) in;\
                                layout (triangle_strip, max_vertices=4) out;\
                                uniform int npoints;\
                                void main(void){\
                                gl_Position = vec4(gl_in[0].gl_Position.x, -1.0, 0.0, 1.0);\
                                EmitVertex();\
                                gl_Position = vec4(gl_in[0].gl_Position.x+1.0/float(npoints), -1.0, 0.0, 1.0);\
                                EmitVertex();\
                                gl_Position = gl_in[0].gl_Position;\
                                EmitVertex();\
                                gl_Position = vec4(gl_in[0].gl_Position.x+1.0/float(npoints), gl_in[0].gl_Position.y, 0.0, 1.0);\
                                EmitVertex();\
                                }");
    
    fftBandsShad.loadFromString(GL_FRAGMENT_SHADER,
                                "#version 330 core\
                                layout(location = 0) out vec4 color;\
                                uniform vec3 cl;\
                                void main(){\
                                color = vec4(cl, 1.0);\
                                }");
    fftBandsShad.link();
    fftBandsShad.addUniform("cl");
    fftBandsShad.addUniform("npoints");
    fftBandsShad.addUniform("rmax");
    fftBandsShad.addUniform("offset");
}

void BeatDetector2::draw() {
    if(currentReg == -1) return;

    //Paint rfft
    rfftVBO->subdata(&rfft[cP][0], 0, 512*sizeof(float));
    fftBandsShad.use();
    fftBandsShad("npoints", 512);
    fftBandsShad("rmax", 2.0f);
    fftBandsShad("cl", new glm::vec3(0.0, 0.2, 0.2));
    fftBandsShad("offset", 0.0f);
    rfftVAO->draw();

    //Paint masked rfft
    rfftVBO->subdata(showmaskedrfft, 0, 512*sizeof(float));
    fftBandsShad.use();
    fftBandsShad("npoints", 512);
    fftBandsShad("rmax", 2.0f);
    fftBandsShad("cl", new glm::vec3(0.2, 0.0, 0.2));
    fftBandsShad("offset", 1.0f/512.0f);
    rfftVAO->draw();


    waveVBO->subdata(&displayEnergies[0], 0, 1024*sizeof(float));
    waveShad.use();
    waveShad("npoints", 1024);
    waveShad("bb", new glm::vec3(1.0, 0.0, 0.0));
    waveShad("yOffset", 0.6f);
    waveShad("K", 1.0f);
    waveVAO->draw();

    waveVBO->subdata(&displayMaskedEnergies[0], 0, 1024*sizeof(float));
    waveShad.use();
    waveShad("npoints", 1024);
    waveShad("bb", new glm::vec3(0.6, 0.0, 0.2));
    waveShad("yOffset", 0.6f);
    waveShad("K", 1.0f);
    waveVAO->draw();


    waveVBO->subdata(&displayKickEnergy[0], 0, 1024*sizeof(float));
    waveShad.use();
    waveShad("npoints", 1024);
    waveShad("bb", new glm::vec3(0.0, 0.0, 1.0));
    waveShad("yOffset", 0.2f);
    waveShad("K", 0.2f);
    waveVAO->draw();

    waveVBO->subdata(&displaySnareEnergy[0], 0, 1024*sizeof(float));
    waveShad.use();
    waveShad("npoints", 1024);
    waveShad("bb", new glm::vec3(0.0, 0.0, 1.0));
    waveShad("yOffset", -0.4f);
    waveShad("K", 0.2f);
    waveVAO->draw();

    waveVBO->subdata(&displayCymbalEnergy[0], 0, 1024*sizeof(float));
    waveShad.use();
    waveShad("npoints", 1024);
    waveShad("bb", new glm::vec3(0.0, 0.0, 1.0));
    waveShad("yOffset", -0.995f);
    waveShad("K", 0.2f);
    waveVAO->draw();



    waveVBO->subdata(&displayKickEnergyComp[0], 0, 1024*sizeof(float));
    waveShad.use();
    waveShad("npoints", 1024);
    waveShad("bb", new glm::vec3(0.2, 0.0, 0.8));
    waveShad("yOffset", 0.2f);
    waveShad("K", 0.2f);
    waveVAO->draw();
    
    waveVBO->subdata(&displaySnareEnergyComp[0], 0, 1024*sizeof(float));
    waveShad.use();
    waveShad("npoints", 1024);
    waveShad("bb", new glm::vec3(0.2, 0.0, 0.8));
    waveShad("yOffset", -0.4f);
    waveShad("K", 0.2f);
    waveVAO->draw();
    
    waveVBO->subdata(&displayCymbalEnergyComp[0], 0, 1024*sizeof(float));
    waveShad.use();
    waveShad("npoints", 1024);
    waveShad("bb", new glm::vec3(0.2, 0.0, 0.8));
    waveShad("yOffset", -0.995f);
    waveShad("K", 0.2f);
    waveVAO->draw();
    


    waveVBO->subdata(&displayKickCValue[0], 0, 1024*sizeof(float));
    waveShad.use();
    waveShad("npoints", 1024);
    waveShad("bb", new glm::vec3(0.6, 0.6, 0.6));
    waveShad("yOffset", 0.2f);
    waveShad("K", 1.0f);
    waveVAO->draw();
    
    waveVBO->subdata(&displaySnareCValue[0], 0, 1024*sizeof(float));
    waveShad.use();
    waveShad("npoints", 1024);
    waveShad("bb", new glm::vec3(0.6, 0.6, 0.6));
    waveShad("yOffset", -0.4f);
    waveShad("K", 1.0f);
    waveVAO->draw();
    
    waveVBO->subdata(&displayCymbalCValue[0], 0, 1024*sizeof(float));
    waveShad.use();
    waveShad("npoints", 1024);
    waveShad("bb", new glm::vec3(0.6, 0.6, 0.6));
    waveShad("yOffset", -0.995f);
    waveShad("K", 1.0f);
    waveVAO->draw();
}
















