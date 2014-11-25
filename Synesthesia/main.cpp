//
//  main.cpp
//  Synesthesia
//

#include <iostream>
#include "Tobago/Tobago.h"
#include "Tobago/init/ContextGLFW.h"
#include "VFO/Player/OSX/ofxAVFoundationVideoPlayer.h"
#include "VFO/Player/OSXPlayer.h"

#include <unistd.h>

int main(int argc, const char * argv[]) {
    TobagoInitGLFW(3, 3);

    ContextGLFW context = ContextGLFW(1280, 720, "Synesthesia", NULL, NULL);
    Tobago.init(&context);
    Tobago.use(0);

    OSXPlayer player;
    player.enableTextureCache();
    player.load("/Users/marc/Downloads/IMG_0376.MOV");

    Shader s;
    s.loadFromFile(GL_VERTEX_SHADER, "simple.vert");
    s.loadFromFile(GL_FRAGMENT_SHADER, "simple2.frag");
    s.link();
    s.addUniform("tex");

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
    
//    Tobago.log->setOutput(&cout);
    (*Tobago.log)(Log::DEBUG) << "UAU goes!";

    while(Tobago.enabled(0)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        player.syncNextFrame();
        player.update();
        std::cout << "Frame: " << player.getFrameNum() << "/" << player.getTotalNumFrames() << " " << player.isMovieDone() << std::endl;
        
        tt = player.getTexture();
        if(tt != NULL) {
            tt->bindToGLSL(0);
        }

        s.use();
        s("tex", 0);
        vao.draw();

        Tobago.swap(0);

        if(glfwGetKey(context.window, GLFW_KEY_ESCAPE) && Tobago.enabled(0)) Tobago.stop(0);
    }

    Tobago.log->flush();
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
