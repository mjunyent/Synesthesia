//
//  main.cpp
//  Synesthesia
//

#include <iostream>
#include "Tobago/Tobago.h"
#include "Tobago/init/ContextGLFW.h"
#include "VFO/Player/OSX/ofxAVFoundationVideoPlayer.h"
#include "VFO/Player/OSXPlayer.h"

int main(int argc, const char * argv[]) {
//    ofxAVFoundationVideoPlayer testplayer = ofxAVFoundationVideoPlayer();
    TobagoInitGLFW(3, 3);

    ContextGLFW context = ContextGLFW(1280, 720, "Synesthesia", NULL, NULL);
    Tobago.init(&context);
    Tobago.use(0);

//    ofxAVFoundationVideoPlayer player = ofxAVFoundationVideoPlayer();
//    player.loadMovie("/Users/marc/Downloads/IMG_0376.MOV");
//    player.play();
    
    OSXPlayer player;
    player.enableTextureCache();
    player.load("/Users/marc/Downloads/IMG_0376.MOV");
    player.play();

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
//    player.setPixelFormat(GL_RGBA);
    
//    Tobago.log->setOutput(&cout);
    (*Tobago.log)(Log::DEBUG) << "UAU goes!";

    while(Tobago.enabled(0)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        player.update();
        
        tt = player.getTexture();
        if(tt != NULL) {
            tt->bindToGLSL(0);
            std::cout << player.getPosition() << " " << tt->id << ": " << tt->width << ", " << tt->height << std::endl;
        }

//        Texture *t = player.getTexture();
//        if(t == NULL) std::cout << "T IS NULL" << std::endl;
//        else std::cout << t->id << ": " << t->width << ", " << t->height << std::endl;
//        mn.bindToGLSL(1);
        
        /*
        unsigned char *rr = player.getPixels();
        if(rr != NULL) {
            tt = new Texture(GL_TEXTURE_2D);
            tt->setData(player.getWidth(), player.getHeight(), rr);
            tt->setMinificationFilter(GL_LINEAR);
            tt->setMagnificationFilter(GL_LINEAR);
            tt->bindToGLSL(0);
        }*/

/*        unsigned char * rr = player.getPixels();
        if(rr != NULL) LodePNG::encode("image.png", rr, 1920, 1080); //LodePNG_saveFile(rr, 1920*1080*4, "image.png");
        int mm; std::cin >> mm; */

//        tt->unbind();
//        tt->bindToGLSL(0);
        //        if(t != NULL) t->bindToGLSL(0);

/*        Texture mn("/Users/marc/Downloads/a.png", Texture::PNG);
        mn.setMinificationFilter(GL_LINEAR);
        mn.setMagnificationFilter(GL_LINEAR);
        std::cout << mn.width << ", " << mn.height << std::endl;
        mn.bindToGLSL(0);*/
        
        s.use();
        s("tex", 0);
        vao.draw();

//        if(t != NULL) t->unbind();

        Tobago.swap(0);

        if(glfwGetKey(context.window, GLFW_KEY_ESCAPE) && Tobago.enabled(0)) Tobago.stop(0);
    }

    Tobago.log->flush();
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
