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
#include "AFO/FFTR.h"

#include <unistd.h>

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

    FFT(1, 3, x, y);
    
    
    
    AudioInput::printDevicesInfo();


    AudioInput a;

    float *l = (float*)malloc(sizeof(float)*a.bufferSize);
    a.setCallback([&](float* b, int n, double t){
        memcpy(l, b, sizeof(float)*n);
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
    
    VAO waveVAO(GL_LINE_STRIP);
    VBO waveVBO(l, a.bufferSize);
    waveVAO.addAttribute(0, 1, &waveVBO);
    
    while(Tobago.enabled(0)) {
        waveVBO.subdata(l, 0, a.bufferSize*sizeof(float));
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        waveShad.use();

        waveShad("npoints", (int)a.bufferSize);
        waveVAO.draw();
        
        Tobago.swap(0);
        

        if(glfwGetKey(context.window, GLFW_KEY_ESCAPE) && Tobago.enabled(0)) Tobago.stop(0);
        Tobago.log->flush();
    }
    
    Tobago.log->flush();

    exit(EXIT_SUCCESS);
}
