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
    if(argc < 2) {
        std::cout << "To use run: Synesthesia path/to/your/video/file" << std::endl;
        exit(EXIT_SUCCESS);
    }

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
    s.loadFromString(GL_VERTEX_SHADER, "#version 330 core\
                     layout(location = 0) in vec3 vertexPosition;\
                     out vec2 UV;\
                     void main(){\
                         gl_Position =  vec4(vertexPosition,1);\
                         UV = vec2(1+vertexPosition.x, 1-vertexPosition.y)/2.0;\
                     }");
    s.loadFromString(GL_FRAGMENT_SHADER, "#version 330 core\
                     in vec2 UV;\
                     layout(location = 0) out vec4 color;\
                     uniform sampler2DRect tex;\
                     uniform vec3 mean;\
                     uniform vec2 size;\
                     void main(){\
                         if(UV.x*size.x < 20) color.xyz = mean;\
                         else color = texture(tex, UV*size-vec2(20,0));\
                        color.a = 1.0;\
                     }");
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
        std::cout << "Select the shot to play (from 0 to " << shotdetector.boundaries.size() << ") or -1 for the full video or -2 to exit: ";
        std::cin >> shotId;
        if(shotId == -2) break;
        
        if(shotId == -1) {
            glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
            ContextGLFW newWindow(width+20, height, "All Video", NULL, &context);
            newWindow.init();
            newWindow.use();
            
            player.setFrame(0);
            player.play();

            int currentBreak = 0;
            double lastTime = -1;
            while(newWindow.enabled && player.getFrameNum() != player.getTotalNumFrames()) {
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                if(player.getFrameNum() == shotdetector.boundaries[currentBreak]) {
                    if(lastTime == -1) {
                        lastTime = glfwGetTime();
                        player.pause();
                        glClearColor(1.0, 1.0, 1.0, 1.0);
                    } else if(glfwGetTime()-lastTime > 0.07) {
                        currentBreak++;
                        if(currentBreak >= shotdetector.boundaries.size()) currentBreak = 0;
                        lastTime = -1;
                        player.play();
                        glClearColor(0.0, 0.0, 0.0, 1.0);
                    }

                    newWindow.swap();
                    if(glfwGetKey(newWindow.window, GLFW_KEY_ESCAPE) && newWindow.enabled) newWindow.stop();
                    continue;
                } else {
                    player.update();
                }

                if(player.getTexture() != NULL) { player.getTexture()->bindToGLSL(0); }

                s.use();
                s("tex", 0);
                glm::vec2 size(width+20, height);
                glm::vec3 mean(histograms.rgbMeans[player.getFrameNum()].r,
                               histograms.rgbMeans[player.getFrameNum()].g,
                               histograms.rgbMeans[player.getFrameNum()].b);
                s("size", &size);
                if(player.getFrameNum() >= 0 && player.getFrameNum() <= histograms.rgbMeans.size())
                    s("mean", &mean);
                vao.draw();

                newWindow.swap();

                if(glfwGetKey(newWindow.window, GLFW_KEY_ESCAPE) && newWindow.enabled) newWindow.stop();
            }
            player.stop();
            newWindow.stop();
        }
        else {
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
                glm::vec2 size(width+20, height);
                glm::vec3 mean(histograms.rgbMeans[player.getFrameNum()].r,
                               histograms.rgbMeans[player.getFrameNum()].g,
                               histograms.rgbMeans[player.getFrameNum()].b);
                s("size", &size);
                if(player.getFrameNum() >= 0 && player.getFrameNum() <= histograms.rgbMeans.size())
                    s("mean", &mean);
                vao.draw();

                newWindow.swap();

                if(glfwGetKey(newWindow.window, GLFW_KEY_ESCAPE) && newWindow.enabled) newWindow.stop();
            }
            player.stop();
            newWindow.stop();
        }
    } while (shotId != -2);

    Tobago.log->flush();

    exit(EXIT_SUCCESS);
}
