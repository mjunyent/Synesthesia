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
#include "VFO/Player/FrameGetter.h"
#include "VFO/Player/OSXFrameGetter.h"

#include <unistd.h>

int main(int argc, const char * argv[]) {
    TobagoInitGLFW(3, 3);

    glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
    ContextGLFW context = ContextGLFW(100, 100, "invisibleWindow", NULL, NULL);
    Tobago.init(&context);
    Tobago.use(0);
    TOBAGO::initOCLwithCurrentOGLcontext();

    std::string path = argv[1];
    std::cout << "Looking path: " << path << std::endl;

    OSXFrameGetter getter;
    getter.enableTextureCache();

    try {
        getter.load(path);
    } catch (FrameGetterException& e) {
        std::cout << "Error loading video " << path << ": " << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
    
    if(!getter.isReady()) {
        std::cout << "Could not load video file!" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::vector< std::pair<int, double> > frame2Timestamp;
    
    std::cout << "Analysing frames..." << std::endl;
    HistogramHSV histograms(&getter);
    while(!getter.isFinished()) {
        histograms.iterate();
        frame2Timestamp.push_back(std::make_pair(getter.getFrameNum(),getter.getCurrentTime()));
        getter.update();

        std::cout << "\r" << getter.getFrameNum() << " frames  -  " << getter.getCurrentTime() << " seconds";
    }
    std::cout << std::endl;
    
    std::cout << "Analysing shots..."; std::cout.flush();
    ShotDetector shotdetector(&histograms);
    shotdetector.process();
    std::cout << " Done" << std::endl;

    long width = getter.getWidth();
    long height = getter.getHeight();

    std::cout << std::endl;
    std::cout << "Found " << shotdetector.boundaries.size() << " shots." << std::endl;

//    player.play();
    OSXPlayer player;
    player.enableTextureCache();
    player.load(path);

    Shader s;
    s.loadFromFile(GL_VERTEX_SHADER, "simple.vert");
    s.loadFromFile(GL_FRAGMENT_SHADER, "simple2.frag");
    s.link();
    s.addUniform("tex");
    s.addUniform("mean");
    s.addUniform("size");
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

    int shotId;
    do {
        std::cout << "Select the shot to play (from 0 to " << shotdetector.boundaries.size() << ") or -1 to exit: ";
        std::cin >> shotId;
        if(shotId == -1) break;

        glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
        ContextGLFW newWindow(width+20, height, ("Shot n"+std::to_string(shotId)).c_str(), NULL, &context);
        newWindow.init();
        newWindow.use();

        player.setFrame(shotId==0?0:shotdetector.boundaries[shotId-1]);
        player.play();

        while(newWindow.enabled && player.getFrameNum() != shotdetector.boundaries[shotId]) {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            if(player.getTexture() != NULL) { player.getTexture()->bindToGLSL(0); }

            player.update();

            s.use();
            s("tex", 0);
            s("size", new glm::vec2(width+20, height));
            if(player.getFrameNum() >= 0 && player.getFrameNum() <= histograms.rgbMeans.size())
                s("mean", new glm::vec3(histograms.rgbMeans[player.getFrameNum()].r,
                                        histograms.rgbMeans[player.getFrameNum()].g,
                                        histograms.rgbMeans[player.getFrameNum()].b));
            vao.draw();

            newWindow.swap();

            if(glfwGetKey(newWindow.window, GLFW_KEY_ESCAPE) && newWindow.enabled) newWindow.stop();
        }
        player.stop();
        newWindow.stop();
    } while (shotId != -1);

    Tobago.log->flush();

    exit(EXIT_SUCCESS);
}
