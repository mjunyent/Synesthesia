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
#include "AFO/BeatDetector.h"

#include <unistd.h>
#include "ffft/FFTReal.h"

int main(int argc, const char * argv[]) {
    TobagoInitGLFW(3, 3);

    ContextGLFW context = ContextGLFW(1280, 720, "Synesthesia", NULL, NULL);
    Tobago.init(&context);
    Tobago.use(0);
    TOBAGO::initOCLwithCurrentOGLcontext();
    
    
    AudioInput::printDevicesInfo();
    
    AudioInput a;
    BeatDetector beat(&a);

    a.start();

    

    while(Tobago.enabled(0)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        beat.renderWAVE();
        beat.renderFFT();
        beat.renderBands();
        
        Tobago.swap(0);

        if(glfwGetKey(context.window, GLFW_KEY_ESCAPE) && Tobago.enabled(0)) Tobago.stop(0);
        Tobago.log->flush();
    }
    
    Tobago.log->flush();

    exit(EXIT_SUCCESS);
}
