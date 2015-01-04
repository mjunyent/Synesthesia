//
//  main.cpp
//  Synesthesia
//

#include <iostream>
#include "Tobago/Tobago.h"
#include "Tobago/init/ContextGLFW.h"
#include "VFO/Player/OSXPlayer.h"
#include "VFO/HistogramHSV.h"
#include "VFO/ShotDetector.h"
#include "VFO/VideoAsset.h"
#include "VFO/AssetLibrary.h"

#include "VFO/Player/OSXFrameGetter.h"

#include "AFO/AudioInput.h"

#include <unistd.h>
#include "ffft/FFTReal.h"

int main(int argc, const char * argv[]) {
    
    double x[] = {
        1.0, 2.0, //0 UP, LEFT
        3.0, 4.0, //1 UP, RIGHT
        5.0, 6.0, //0 UP, LEFT
        7.0, 8.0 //1 UP, RIGHT
    };

    double y[] = {
        0.0, 0.0, //0 UP, LEFT
        0.0, 0.0, //1 UP, RIGHT
        0.0, 0.0, //0 UP, LEFT
        0.0, 0.0 //1 UP, RIGHT
    };
    
    
    
    AudioInput::printDevicesInfo();

    AudioInput a;
    ffft::FFTReal<float> fft_object(a.bufferSize);

    
    float *wave = (float*)malloc(sizeof(float)*a.bufferSize);
    float *fft = (float*)malloc(sizeof(float)*a.bufferSize);
    float *rfft = (float*)malloc(sizeof(float)*a.bufferSize/2);
    float maxrfft = 1.0;
    a.setCallback([&](float* b, int n, double t){
        memcpy(wave, b, sizeof(float)*n);
        fft_object.do_fft(fft, b);
        rfft[0] = fft[0];
        maxrfft = rfft[0];
        for(int i=1; i<n/2; i++) {
            rfft[i] = sqrt(fft[i]*fft[i] + fft[i+n/2]*fft[i+n/2]);
            if(maxrfft < rfft[i]) maxrfft = rfft[i];
        }
    });

    a.start();

    TobagoInitGLFW(3, 3);

    ContextGLFW context = ContextGLFW(1280, 720, "Synesthesia", NULL, NULL);
    Tobago.init(&context);
    Tobago.use(0);
    TOBAGO::initOCLwithCurrentOGLcontext();
    
    Shader waveShad;
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
    
    
    Shader fftShad;
    fftShad.loadFromString(GL_VERTEX_SHADER,
                            "#version 330 core\
                            layout(location = 0) in float vertexPosition;\
                            uniform int npoints;\
                            uniform float rmax;\
                            out vec3 cl;\
                            void main(){\
                            gl_Position =  vec4((2.0*gl_VertexID)/float(npoints-1.0) - 1.0, vertexPosition/rmax-1.0, 0, 1);\
                            if(gl_VertexID == 19) cl = vec3(1.0,1.0,1.0);\
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

    VAO waveVAO(GL_LINE_STRIP);
    VBO waveVBO(wave, a.bufferSize);
    waveVAO.addAttribute(0, 1, &waveVBO);
    
    VAO fftVAO(GL_LINE_STRIP);
    VBO fftVBO(rfft, a.bufferSize/2);
    fftVAO.addAttribute(0, 1, &fftVBO);
    
    while(Tobago.enabled(0)) {
        waveVBO.subdata(wave, 0, a.bufferSize*sizeof(float));
        fftVBO.subdata(rfft, 0, a.bufferSize*sizeof(float)/2);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        waveShad.use();

        waveShad("npoints", (int)a.bufferSize);
        waveVAO.draw();
        
        fftShad.use();
        fftShad("npoints", (int)a.bufferSize/2);
        fftShad("rmax", 10.0f);
        std::cout << maxrfft << std::endl;
        fftVAO.draw();

        Tobago.swap(0);
        

        if(glfwGetKey(context.window, GLFW_KEY_ESCAPE) && Tobago.enabled(0)) Tobago.stop(0);
        Tobago.log->flush();
    }
    
    Tobago.log->flush();

    exit(EXIT_SUCCESS);
}
