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
    
    timeSize = 17;
    freqSize = 17;

    fft = std::vector< std::vector<float> >(timeSize, std::vector<float>(1024, 0.0));
    rfft = std::vector< std::vector<float> >(timeSize, std::vector<float>(512, 0.0));

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

    numRegs = 40;
    currentReg = -1;
    maskedrfft = std::vector< std::vector<float> > (numRegs, std::vector<float>(numBands, 0.0f));
    rfftBands = std::vector< std::vector<float> > (numRegs, std::vector<float>(numBands, 0.0f));
    maskedrfftMean = (float*)malloc(sizeof(float)*numBands);
    maskedVariances = (float*)malloc(sizeof(float)*numBands);
    rfftMean = (float*)malloc(sizeof(float)*numBands);
    Variances = (float*)malloc(sizeof(float)*numBands);

    cValue = (float*)malloc(sizeof(float)*numBands);

//    fBeats = std::vector<float>(numBands+1, 0.0f);

    //HARDCODED
    fBeats = std::vector<float>(20, 0.0f);

    cP = 0;


//    passthrough = std::ofstream("pass.raw", std::ios::binary);
//    percussion  = std::ofstream("perc.raw", std::ios::binary);

    adc->setCallback([&](float* in, float* out, int n, double t) { this->callback(in, out, n, t); });
}

void BeatDetector2::callback(float *in, float* out, int n, double t) {
    memcpy(tempBuffer, remaining, sizeof(float)*Nremaining);
    memcpy(tempBuffer+Nremaining, in, sizeof(float)*n);
    memcpy(remaining, in+(n-Nremaining), sizeof(float)*Nremaining);

    //passthrough.write((const char*)in, sizeof(float)*768);
    
    fft_object->do_fft(&fft[fftPointer][0], tempBuffer);

    rfft[fftPointer][0] = std::fabs(fft[fftPointer][0]);
    for(int i=1; i<512; i++) {
        rfft[fftPointer][i] = sqrt( fft[fftPointer][i]*fft[fftPointer][i] +
                                    fft[fftPointer][i+512]*fft[fftPointer][i+512]);
    }
    
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
    
    
    std::vector<float> median(timeSize);
    for(int i=0; i<512; i++) {
        for(int j=0; j<timeSize; j++) {
            median[j] = rfft[j][i];
        }
//        median[timeSize] = rfft[timeSize-1][i] + (rfft[timeSize-1][i] - rfft[timeSize-2][i]);
//        median[timeSize+1] = median[timeSize] + (median[timeSize] - median[timeSize-1]);
        sort(median.begin(), median.end());

        H[i] = median[(median.size())/2];
    }


    for(int i=0; i<numBands; i++) {
        maskedrfftMean[i] = 0.0f;
        for(int j=0; j<numRegs; j++) {
            maskedrfftMean[i] += maskedrfft[j][i];
            rfftMean[i] += rfftBands[j][i];
        }
        maskedrfftMean[i] /= float(numRegs);
        rfftMean[i] /= float(numRegs);
    }

    for(int i=0; i<numBands; i++) {
        maskedVariances[i] = 0.0f;
        for(int j=0; j<numRegs; j++) {
            maskedVariances[i] += std::pow(maskedrfft[j][i]-maskedrfftMean[i], 2.0);
            Variances[i] += std::pow(rfftBands[j][i]-rfftMean[i], 2.0);
        }
        maskedVariances[i] /= float(numRegs);
        Variances[i] /= float(numRegs);
    }
    
    currentReg++;
    currentReg %= numRegs;
    

    if(maskSelector == 0) {
        for(int i=0; i<512; i++) {
            float m = (P[i]*P[i]) / (H[i]*H[i] + P[i]*P[i]);
            masked[i] = m*fft[cP][i];
            masked[i+512] = m*fft[cP][i+512];
//            maskedrfft[maskedPointer][i] = m*rfft[cP][i];
        }

        maskedrfft[currentReg][0] = 0.0;
        rfftBands[currentReg][0] = 0.0;
        for(int i=0; i<3; i++) {
            float m = (P[i]*P[i]) / (H[i]*H[i] + P[i]*P[i]);
            maskedrfft[currentReg][0] += m*rfft[cP][i];
            rfftBands[currentReg][0] += rfft[cP][i];
        }
        maskedrfft[currentReg][0] /= 3.0;
        rfftBands[currentReg][0] /= 3.0;

        maskedrfft[currentReg][1] = 0.0;
        rfftBands[currentReg][1] = 0.0;
        for(int i=4; i<11; i++) {
            float m = (P[i]*P[i]) / (H[i]*H[i] + P[i]*P[i]);
            maskedrfft[currentReg][1] += m*rfft[cP][i];
            rfftBands[currentReg][1] += rfft[cP][i];
        }
        maskedrfft[currentReg][1] /= 7.0;
        rfftBands[currentReg][1] /= 7.0;

        maskedrfft[currentReg][2] = 0.0;
        rfftBands[currentReg][2] = 0.0;
        for(int i=0; i<10; i++) {
            float m = (P[i]*P[i]) / (H[i]*H[i] + P[i]*P[i]);
            maskedrfft[currentReg][2] += m*rfft[cP][i];
            rfftBands[currentReg][2] += rfft[cP][i];
        }
        maskedrfft[currentReg][2] /= 10.0;
        rfftBands[currentReg][2] /= 10.0;

        
        
        /*
        maskedrfft[currentReg][0] = 0.0;
        for(int i=0; i<2; i++) {
            float m = (P[i]*P[i]) / (H[i]*H[i] + P[i]*P[i]);
            maskedrfft[currentReg][0] += m*rfft[cP][i];
        }
        maskedrfft[currentReg][0] /= 2.0;

        maskedrfft[currentReg][1] = 0.0;
        for(int i=2; i<6; i++) {
            float m = (P[i]*P[i]) / (H[i]*H[i] + P[i]*P[i]);
            maskedrfft[currentReg][1] += m*rfft[cP][i];
        }
        maskedrfft[currentReg][1] /= 4.0;

        maskedrfft[currentReg][2] = 0.0;
        for(int i=6; i<21; i++) {
            float m = (P[i]*P[i]) / (H[i]*H[i] + P[i]*P[i]);
            maskedrfft[currentReg][2] += m*rfft[cP][i];
        }
        maskedrfft[currentReg][2] /= 15.0;*/

/*        maskedrfft[currentReg][0]  = (P[0]*P[0]) / (H[0]*H[0] + P[0]*P[0])*rfft[cP][0];
        maskedrfft[currentReg][0] += (P[1]*P[1]) / (H[1]*H[1] + P[1]*P[1])*rfft[cP][1];
        maskedrfft[currentReg][0] += (P[2]*P[2]) / (H[2]*H[2] + P[2]*P[2])*rfft[cP][2];
        maskedrfft[currentReg][0] += (P[3]*P[3]) / (H[3]*H[3] + P[3]*P[3])*rfft[cP][3];
        maskedrfft[currentReg][0] /= 4.0f;

        maskedrfft[currentReg][1]  = (P[5]*P[5]) / (H[5]*H[5] + P[5]*P[5])*rfft[cP][5];
        maskedrfft[currentReg][1] += (P[6]*P[6]) / (H[6]*H[6] + P[6]*P[6])*rfft[cP][6];
        maskedrfft[currentReg][1] += (P[7]*P[7]) / (H[7]*H[7] + P[7]*P[7])*rfft[cP][7];
        maskedrfft[currentReg][1] += (P[8]*P[8]) / (H[8]*H[8] + P[8]*P[8])*rfft[cP][8];
        maskedrfft[currentReg][1] += (P[9]*P[9]) / (H[9]*H[9] + P[9]*P[9])*rfft[cP][9];
        maskedrfft[currentReg][1] += (P[10]*P[10]) / (H[10]*H[10] + P[10]*P[10])*rfft[cP][10];
        maskedrfft[currentReg][1] += (P[11]*P[11]) / (H[11]*H[11] + P[11]*P[11])*rfft[cP][11];
        maskedrfft[currentReg][1] += (P[12]*P[12]) / (H[12]*H[12] + P[12]*P[12])*rfft[cP][12];
        maskedrfft[currentReg][1] /= 8.0f;*/
 
/*
        maskedrfft[currentReg][0]  = (P[0]*P[0]) / (H[0]*H[0] + P[0]*P[0])*rfft[cP][0];
        maskedrfft[currentReg][0] += (P[1]*P[1]) / (H[1]*H[1] + P[1]*P[1])*rfft[cP][1];
        maskedrfft[currentReg][0] += (P[2]*P[2]) / (H[2]*H[2] + P[2]*P[2])*rfft[cP][2];
        maskedrfft[currentReg][0] += (P[3]*P[3]) / (H[3]*H[3] + P[3]*P[3])*rfft[cP][3];
        maskedrfft[currentReg][0] += (P[4]*P[4]) / (H[4]*H[4] + P[4]*P[4])*rfft[cP][4];
        maskedrfft[currentReg][0] += (P[5]*P[5]) / (H[5]*H[5] + P[5]*P[5])*rfft[cP][5];
        maskedrfft[currentReg][0] += (P[6]*P[6]) / (H[6]*H[6] + P[6]*P[6])*rfft[cP][6];
        maskedrfft[currentReg][0] += (P[7]*P[7]) / (H[7]*H[7] + P[7]*P[7])*rfft[cP][7];
        maskedrfft[currentReg][0] += (P[8]*P[8]) / (H[8]*H[8] + P[8]*P[8])*rfft[cP][8];
        maskedrfft[currentReg][0] += (P[9]*P[9]) / (H[9]*H[9] + P[9]*P[9])*rfft[cP][9];
        maskedrfft[currentReg][0] += (P[10]*P[10]) / (H[10]*H[10] + P[10]*P[10])*rfft[cP][10];
        maskedrfft[currentReg][0] += (P[11]*P[11]) / (H[11]*H[11] + P[11]*P[11])*rfft[cP][11];
        maskedrfft[currentReg][0] /= 10.0f;
*/
        
        /*
        int rfftP = 0;
        int fftBandsP = 0;
        int numSamples = 1;
        for(int i=0; i<numOctaves; i++) {
            for(int j=0; j<bandsPerOctave[i]; j++) {
                maskedrfft[currentReg][fftBandsP] = 0.0f;

                for(int k=0; k<numSamples/bandsPerOctave[i]; k++) {
                    float m = (P[rfftP]*P[rfftP]) / (H[rfftP]*H[rfftP] + P[rfftP]*P[rfftP]);
                    maskedrfft[currentReg][fftBandsP] += m*rfft[cP][rfftP];
                    rfftP++;
                }

                maskedrfft[currentReg][fftBandsP] /= numSamples/float(bandsPerOctave[i]);

                fftBandsP++;
            }

            numSamples <<= 1;
        }*/




        fBeats[numBands] = 0.0;
        for(int i=0; i<numBands; i++) {
            float c = (-1.0/200.0)*maskedVariances[i] + 2.5;
            cValue[i] = c*maskedrfftMean[i];
            fBeats[i] = std::max(0.0, fBeats[i]-4.0*n/44100.0);
            if(maskedrfft[currentReg][i] >= 2.5*maskedrfftMean[i]) fBeats[i] = 1.2;
            fBeats[numBands] += fBeats[i];
        }
        fBeats[numBands] /= float(numBands);


        fBeats[9] = std::max(0.0, fBeats[9]-4.0*n/44100.0);
        if(maskedrfft[currentReg][0] >= 1.5*rfftMean[0]) fBeats[9] = 1.2;
        fBeats[10] = std::max(0.0, fBeats[10]-4.0*n/44100.0);
        if(maskedrfft[currentReg][1] >= 0.8*rfftMean[1]) fBeats[10] = 1.2;
        fBeats[numBands] = 0.0;

        
        int lastReg = currentReg-1;
        if(lastReg < 0) lastReg = numRegs-1;

        fBeats[17] = std::max(0.0, fBeats[17]-4.0*n/44100.0);
        float maskedDiff = maskedrfft[currentReg][0] - maskedrfft[lastReg][0];
        float diff = rfftBands[currentReg][0] - rfftBands[lastReg][0];
        if(maskedDiff > 3.0*diff && maskedDiff > 0) fBeats[17] = 1.2;

        fBeats[18] = std::max(0.0, fBeats[18]-4.0*n/44100.0);
        maskedDiff = maskedrfft[currentReg][1] - maskedrfft[lastReg][1];
        diff = rfftBands[currentReg][1] - rfftBands[lastReg][1];
        if(maskedDiff > 3.0*diff && maskedDiff > 0) fBeats[18] = 1.2;


//        fft_object->do_ifft(&fft[cP][0], output);
        fft_object->do_ifft(masked, output);
        fft_object->rescale(output);

        memcpy(out, output+256, sizeof(float)*768);
        //percussion.write((const char*)(output+256), sizeof(float)*768);
    }

    fftPointer++;
    fftPointer %= timeSize;
}

void BeatDetector2::setupDraw() {
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
                                gl_Position = vec4(gl_in[0].gl_Position.x+1.0/(3.0*npoints), -1.0, 0.0, 1.0);\
                                EmitVertex();\
                                gl_Position = gl_in[0].gl_Position;\
                                EmitVertex();\
                                gl_Position = vec4(gl_in[0].gl_Position.x+1.0/(3.0*npoints), gl_in[0].gl_Position.y, 0.0, 1.0);\
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
      //display fftr[cP] and maskedrfft they have size 512
/*    rfftVBO->subdata(&rfft[cP][0], 0, 512*sizeof(float));
    fftBandsShad.use();
    fftBandsShad("npoints", 512);
    fftBandsShad("rmax", 10.0f);
    fftBandsShad("cl", new glm::vec3(1.0, 1.0, 0.0));
    rfftVAO->draw();
*/

//    bsd[0] = maskedrfft[0] + maskedrfft[1] + maskedrfft[2] + maskedrfft[3];
//    bsd[1] = maskedrfft[5] + maskedrfft[6] + maskedrfft[7] + maskedrfft[8] + maskedrfft[9] + maskedrfft[10] + maskedrfft[11] + maskedrfft[12];
    if(currentReg == -1) return;
    
    rfftVBO->subdata(&maskedrfft[currentReg][0], 0, numBands*sizeof(float));
    fftBandsShad.use();
    fftBandsShad("npoints", numBands);
    fftBandsShad("rmax", 15.0f);
    fftBandsShad("cl", new glm::vec3(1.0, 0.0, 0.0));
    fftBandsShad("offset", 0.0f);
    rfftVAO->draw();

    rfftVBO->subdata(&rfftBands[currentReg][0], 0, numBands*sizeof(float));
    fftBandsShad.use();
    fftBandsShad("npoints", numBands);
    fftBandsShad("rmax", 15.0f);
    fftBandsShad("cl", new glm::vec3(0.8, 0.0, 0.0));
    fftBandsShad("offset", 0.2f);
    rfftVAO->draw();

    rfftVBO->subdata(cValue, 0, numBands*sizeof(float));
    fftBandsShad.use();
    fftBandsShad("npoints", numBands);
    fftBandsShad("rmax", 15.0f);
    fftBandsShad("cl", new glm::vec3(1.0, 1.0, 0.0));
    fftBandsShad("offset", 0.4f);
    rfftVAO->draw();

    rfftVBO->subdata(maskedrfftMean, 0, numBands*sizeof(float));
    fftBandsShad.use();
    fftBandsShad("npoints", numBands);
    fftBandsShad("rmax", 15.0f);
    fftBandsShad("cl", new glm::vec3(1.0, 1.0, 1.0));
    fftBandsShad("offset", 0.6f);
    rfftVAO->draw();

    rfftVBO->subdata(rfftMean, 0, numBands*sizeof(float));
    fftBandsShad.use();
    fftBandsShad("npoints", numBands);
    fftBandsShad("rmax", 15.0f);
    fftBandsShad("cl", new glm::vec3(0.8, 0.8, 0.8));
    fftBandsShad("offset", 0.8f);
    rfftVAO->draw();
    
    rfftVBO->subdata(maskedVariances, 0, numBands*sizeof(float));
    fftBandsShad.use();
    fftBandsShad("npoints", numBands);
    fftBandsShad("rmax", 100.0f);
    fftBandsShad("cl", new glm::vec3(0.0, 0.0, 1.0));
    fftBandsShad("offset", 1.2f);
    rfftVAO->draw();


    rfftVBO->subdata(Variances, 0, numBands*sizeof(float));
    fftBandsShad.use();
    fftBandsShad("npoints", numBands);
    fftBandsShad("rmax", 100.0f);
    fftBandsShad("cl", new glm::vec3(0.0, 0.0, 0.8));
    fftBandsShad("offset", 1.4f);
    rfftVAO->draw();
}
















