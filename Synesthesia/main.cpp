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
    ContextGLFW visualizer = ContextGLFW(600, 600, "Visualizer", NULL, &context);

    std::vector<Context*> contexts;
    contexts.push_back(&context);
    contexts.push_back(&visualizer);

    Tobago.init(contexts);
    Tobago.use(0);
    TOBAGO::initOCLwithCurrentOGLcontext();

//    AudioInput::printDevicesInfo();
    
    AudioInput a(7, 0, 1024);
    BeatDetector beat(&a, 4);
    
    std::cout << "Analyzing " << beat.numBands << " bands." << std::endl;

    
    Tobago.use(1);
    
    Shader padsShad;
    padsShad.loadFromString(GL_VERTEX_SHADER,
                            "#version 330 core\
                            layout(location = 0) in float isBeat;\
                            out float beat;\
                            void main(){\
                                beat = isBeat;\
                            gl_Position =  vec4(1.0/4.0*mod(gl_VertexID, 8)-1.0, -1.0/4.0*floor(gl_VertexID/8)+0.75, 0, 1);\
                            }");
    
    padsShad.loadFromString(GL_GEOMETRY_SHADER,
                            "#version 150 core\n\
                            layout (points) in;\n\
                            layout (triangle_strip, max_vertices=4) out;\n\
                            in float beat[];\n\
                            out VertexData {\n\
                            float beat;\n\
                            } VertexOut;\n\
                            void main(void){\n\
                            VertexOut.beat = beat[0];\n\
                            gl_Position = vec4(gl_in[0].gl_Position.xy+vec2(0.0,1.0/4.1), 0.0, 1.0);\n\
                            EmitVertex();\n\
                            gl_Position = vec4(gl_in[0].gl_Position.xy+vec2(1.0/4.1,1.0/4.1), 0.0, 1.0);\n\
                            EmitVertex();\n\
                            gl_Position = vec4(gl_in[0].gl_Position.xy, 0.0, 1.0);\n\
                            EmitVertex();\n\
                            gl_Position = vec4(gl_in[0].gl_Position.xy+vec2(1.0/4.1,0.0), 0.0, 1.0);\n\
                            EmitVertex();\n\
                            }");

    padsShad.loadFromString(GL_FRAGMENT_SHADER,
                            "#version 330 core\
                            in VertexData {\
                            float beat;\
                            } VertexIn;\
                            in float beat;\
                            layout(location = 0) out vec4 color;\
                            void main(){\
                            color = vec4(VertexIn.beat, VertexIn.beat, VertexIn.beat, 1.0);\
                            }");
    
    padsShad.link();
    
    VAO beatVAO(GL_POINTS);
    std::vector<float> beatPoints(beat.numBands, 1.0);
    VBO beatVBO(beat.fBeats);
    beatVAO.addAttribute(0, 1, &beatVBO);

    Tobago.use(0);

    a.start();

    glDisable(GL_DEPTH_TEST);
    

    while(Tobago.enabled(0)) {
        Tobago.use(0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        bool k = beat.renderBands();
        if(k) beat.renderWAVE(1.0);
        else beat.renderWAVE(0.0);
        beat.renderFFT();
        
        Tobago.swap(0);


        Tobago.use(1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
/*        beat.getBeats();
        for(int i=0; i<beat.numBands; i++) {
            if(beat.beats[i]) beatPoints[i] = 1.0;
            else beatPoints[i] = 0.0;
        }
        beatVBO.subdata(&beatPoints[0], 0, sizeof(float)*beat.numBands);*/
        beatVBO.subdata(&beat.fBeats[0], 0, sizeof(float)*beat.numBands);

        padsShad.use();
        beatVAO.draw();
        
        Tobago.swap(1);

        if(glfwGetKey(context.window, GLFW_KEY_ESCAPE) && Tobago.enabled(0)) Tobago.stop(0);
        Tobago.log->flush();
    }
    
    Tobago.log->flush();

    exit(EXIT_SUCCESS);
}
