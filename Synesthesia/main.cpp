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
#include "AFO/BeatDetector2.h"

#include <unistd.h>
#include "ffft/FFTReal.h"

int main(int argc, const char * argv[]) {
    TobagoInitGLFW(3, 3);

    ContextGLFW context = ContextGLFW(1280, 720, "Synesthesia", NULL, NULL);

    std::vector<Context*> contexts;
    contexts.push_back(&context);

    Tobago.init(contexts);
    Tobago.use(0);
    TOBAGO::initOCLwithCurrentOGLcontext();

    AudioInput::printDevicesInfo();
    AudioInput a(0, 0, 768);
    BeatDetector2 beat(&a);
    
    a.start();

    glDisable(GL_DEPTH_TEST);
    

    while(Tobago.enabled(0)) {
        Tobago.use(0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        Tobago.swap(0);

        if(glfwGetKey(context.window, GLFW_KEY_ESCAPE) && Tobago.enabled(0)) Tobago.stop(0);
        Tobago.log->flush();
    }
    
    Tobago.log->flush();

    exit(EXIT_SUCCESS);
}
