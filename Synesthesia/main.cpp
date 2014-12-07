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

#include <unistd.h>

int main(int argc, const char * argv[]) {
    AssetLibrary a("/Users/marc/Desktop/");

    TobagoInitGLFW(3, 3);
    
    ContextGLFW context = ContextGLFW(1280, 720, "Synesthesia", NULL, NULL);
    Tobago.init(&context);
    Tobago.use(0);

    TOBAGO::initOCLwithCurrentOGLcontext();

    VideoAsset asset("/Users/marc/Documents/Developing/Synesthesia/build/Europe.mp4");

    std::cout << "Init" << std::endl;
    OSXPlayer player;
    player.enableTextureCache();
    player.load("Europe.mp4");
//    player.load("Toying.MOV");

    std::cout << "Video lodaded" << std::endl;
    HistogramHSV histograms(&player);

    Shader s;
    s.loadFromFile(GL_VERTEX_SHADER, "simple.vert");
    s.loadFromFile(GL_FRAGMENT_SHADER, "simple2.frag");
    s.link();
    s.addUniform("tex");
    s.addUniform("mean");

    float quad[] = {
        -1.0f,  1.0f,  0.0f, //0 UP, LEFT
        1.0f,  1.0f,  0.0f, //1 UP, RIGHT
        1.0f, -1.0f,  0.0f, //2 DOWN, RIGHT
        -1.0f, -1.0f,  0.0f  //3 DOWN, LEFT
    };

    GLushort quad_I[] = {
        0, 3, 1,
        1, 3, 2
    };

    VBO qv(quad, 12);
    IBO qi(quad_I, 6);

    VAO vao(GL_TRIANGLES);
    vao.addAttribute(0, 3, &qv);
    vao.addIBO(&qi);

    Texture *tt;

#define CALCULATE 1
#ifdef CALCULATE
    while(Tobago.enabled(0) && !player.isMovieDone()) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        histograms.iterate();

        std::cout << player.getFrameNum() << "/" << player.getTotalNumFrames() << "    " << player.getPosition() << std::endl;

        tt = player.getTexture();
        if(tt != NULL) { tt->bindToGLSL(0); }

        player.syncNextFrame();
//        player.update();

        s.use();
        s("tex", 0);
        s("mean", new glm::vec3(histograms.rgbMeans.back().r, histograms.rgbMeans.back().g, histograms.rgbMeans.back().b));
        vao.draw();

        Tobago.swap(0);

        if(glfwGetKey(context.window, GLFW_KEY_ESCAPE) && Tobago.enabled(0)) Tobago.stop(0);
        
        Tobago.log->flush();
    }
    
    histograms.save();
#else
    std::cout << "LOADING" << std::endl;
    histograms.load();
    std::cout << "Loaded" << std::endl;
    
    std::cout << "CALCULATING" << std::endl;
    ShotDetector shotdetector(&histograms);
    shotdetector.process();
    std::cout << "Calculated" << std::endl;
    
    int index=0;

    while(Tobago.enabled(0) && player.getFrameNum() != player.getTotalNumFrames()) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if(player.getFrameNum() == shotdetector.boundaries[index]) {
            index++;
            std::cout << "Boundary" << std::endl;
            int a; cin >> a;
        }
        
        tt = player.getTexture();
        if(tt != NULL) { tt->bindToGLSL(0); }
        
        player.syncNextFrame();
//        player.update();
        
        s.use();
        s("tex", 0);
        s("mean", new glm::vec3(histograms.rgbMeans.back().r, histograms.rgbMeans.back().g, histograms.rgbMeans.back().b));
        vao.draw();
        
        Tobago.swap(0);
        
        if(glfwGetKey(context.window, GLFW_KEY_ESCAPE) && Tobago.enabled(0)) Tobago.stop(0);
    }
    
    histograms.save();
#endif

    Tobago.log->flush();
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
