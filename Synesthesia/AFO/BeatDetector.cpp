//
//  BeatDetector.cpp
//  Synesthesia
//
//  Created by Marc Junyent Martín on 05/01/15.
//  Copyright (c) 2015 Marc Junyent Martín. All rights reserved.
//

#include "BeatDetector.h"

BeatDetector::BeatDetector(AudioInput* adc) : adc(adc) {
    fft_object = new ffft::FFTReal<float>(adc->bufferSize);

    
    fft = (float*)malloc(sizeof(float)*adc->bufferSize);
    rfft = (float*)malloc(sizeof(float)*adc->bufferSize/2);

    numBands = 64;
    numElementsInBand = adc->bufferSize/2/numBands;
    numRegs = 43;
    currentReg = -1;

    fftBands = std::vector< std::vector<float> >(numRegs, std::vector<float>(numBands, 0.0f));
    
    fftBandMeans = std::vector<float>(numBands, 0.0f);
    
#ifdef BEAT_DEBUG
    initDebug();
#endif

    adc->setCallback([&](float* b, int n, double t) {
#ifdef BEAT_DEBUG
        memcpy(wave, b, sizeof(float)*n);
#endif
        fft_object->do_fft(fft, b);
        rfft[0] = fft[0];
        for(int i=1; i<n/2; i++)
            rfft[i] = sqrt(fft[i]*fft[i] + fft[i+n/2]*fft[i+n/2]);

        //Calculate means
        for(int i=0; i<numBands; i++) {
            fftBandMeans[i] = 0.0f;
            for (int j=0; j<numRegs; j++) {
                fftBandMeans[i] += fftBands[j][i];
            }
            fftBandMeans[i] /= float(numRegs);
        }

        currentReg++;
        currentReg %= numRegs;
        
        //Fill bands
        int k = 0;
        for(int i=0; i<numBands && k<n/2; i++)  {
            fftBands[currentReg][i] = 0.0f;
            for(int j=0; j<pow(2,i+1); j++) {
                if(k > n/2) break;
                fftBands[currentReg][i] += rfft[k];
                k++;
            }
            fftBands[currentReg][i] *= pow(2,i+1)/(n/2);
        }
        
        std::cout << t << std::endl;
    });
}


#ifdef BEAT_DEBUG
void BeatDetector::initDebug() {
    
    wave = (float*)malloc(sizeof(float)*adc->bufferSize);
    
    waveShad.loadFromString(GL_VERTEX_SHADER,
                            "#version 330 core\
                            layout(location = 0) in float vertexPosition;\
                            uniform int npoints;\
                            void main(){\
                            gl_Position =  vec4((2.0*gl_VertexID)/float(npoints-1.0) - 1.0, vertexPosition, 0, 1);\
                            }");
    waveShad.loadFromString(GL_FRAGMENT_SHADER,
                            "#version 330 core\
                            layout(location = 0) out vec4 color;\
                            void main(){\
                            color = vec4(1.0, 0.0, 0.0, 1.0);\
                            }");
    waveShad.link();
    waveShad.addUniform("npoints");
    
    waveVAO = new VAO(GL_LINE_STRIP);
    waveVBO = new VBO(wave, adc->bufferSize);
    waveVAO->addAttribute(0, 1, waveVBO);
    
    
    fftShad.loadFromString(GL_VERTEX_SHADER,
                           "#version 330 core\
                           layout(location = 0) in float vertexPosition;\
                           uniform int npoints;\
                           uniform float rmax;\
                           out vec3 cl;\
                           void main(){\
                           gl_Position =  vec4((2.0*gl_VertexID)/float(npoints-1.0) - 1.0, vertexPosition/rmax-1.0, 0, 1);\
                           if(gl_VertexID == 10 || gl_VertexID == 20 || gl_VertexID == 41) cl = vec3(1.0,1.0,1.0);\
                           else cl = vec3(0.0,0.0,1.0);\
                           }");
    fftShad.loadFromString(GL_FRAGMENT_SHADER,
                           "#version 330 core\
                           in vec3 cl;\
                           layout(location = 0) out vec4 color;\
                           void main(){\
                           color = vec4(cl, 1.0);\
                           }");
    
    fftShad.link();
    fftShad.addUniform("npoints");
    fftShad.addUniform("rmax");
    
    
    fftVAO = new VAO(GL_LINE_STRIP);
    fftVBO = new VBO(rfft, adc->bufferSize/2);
    fftVAO->addAttribute(0, 1, fftVBO);
    
   
    fftBandsShad.loadFromString(GL_VERTEX_SHADER,
                           "#version 330 core\
                           layout(location = 0) in float vertexPosition;\
                           uniform int npoints;\
                           uniform float rmax;\
                           void main(){\
                           gl_Position =  vec4((2.0*gl_VertexID)/float(npoints-1.0) - 1.0, vertexPosition/rmax-1.0, 0, 1);\
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
    
    fftBandsVAO = new VAO(GL_LINE_STRIP);
    fftBandsVBO = new VBO(&fftBands[0][0], fftBands[0].size());
    fftBandsVAO->addAttribute(0, 1, fftBandsVBO);
}

void BeatDetector::renderWAVE() {
    waveVBO->subdata(wave, 0, adc->bufferSize*sizeof(float));

    waveShad.use();
    waveShad("npoints", (int)adc->bufferSize);
    waveVAO->draw();
}

void BeatDetector::renderFFT(float rmax) {
    fftVBO->subdata(rfft, 0, adc->bufferSize*sizeof(float)/2);

    fftShad.use();
    fftShad("npoints", (int)adc->bufferSize/2);
    fftShad("rmax", rmax);
    fftVAO->draw();
}

void BeatDetector::renderBands(float rmax) {
    if(currentReg != -1) {
        fftBandsVBO->subdata(&fftBands[currentReg][0], 0, numBands*sizeof(float));
    }
    
    fftBandsShad.use();
    fftBandsShad("npoints", numBands);
    fftBandsShad("rmax", rmax);
    fftBandsShad("cl", new glm::vec3(0.0,1.0,0.0));
    fftBandsVAO->draw();
    
    
    
    fftBandsVBO->subdata(&fftBandMeans[0], 0, numBands*sizeof(float));
    fftBandsShad.use();
    fftBandsShad("npoints", numBands);
    fftBandsShad("rmax", rmax);
    fftBandsShad("cl", new glm::vec3(1.0,1.0,1.0));
    fftBandsVAO->draw();
}
#endif













