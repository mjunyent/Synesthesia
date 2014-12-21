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

#include <unistd.h>

int main(int argc, const char * argv[]) {
    TobagoInitGLFW(3, 3);
    
    ContextGLFW context = ContextGLFW(1280, 720, "Synesthesia", NULL, NULL);
    Tobago.init(&context);
    Tobago.use(0);

    TOBAGO::initOCLwithCurrentOGLcontext();

    std::string path = "/Users/marc/Desktop/";
    std::cout << "Using folder: " << path << std::endl;

    AssetLibrary al(path);

    std::cout << "Loading assets..." << std::endl;
    al.loadAssets();
    al.listAssets();
    
    int op;
    do {
        std::cout << "Menu" << std::endl;
        std::cout << "\t 1) List assets \n \t 2) Add asset \n \t 3) Remove asset \n   \t 4) Process assets \n \t 0) Exit \n";
        std::cout << "Choose: ";
        std::cin >> op;
        std::string t;
        
        switch (op) {
            case 1:
                al.listAssets();
                std::cout << std::endl;
                break;
            case 2:
                std::cout << "Asset path: ";
                std::cin >> t;
                try {
                    al.addAsset(bfs::path(t), true);
                } catch (exception& e) {
                    std::cout << e.what() << std::endl;
                }
                break;
            case 3:
                std::cout << "Asset name: ";
                std::cin >> t;
                al.removeAsset(t);
                break;
            case 4:
                al.process();
                break;
            default:
                break;
        }
    }
    while(op != 0);

    Tobago.log->flush();

    exit(EXIT_SUCCESS);
}
