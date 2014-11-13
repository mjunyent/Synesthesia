//
//  Player.h
//  Synesthesia
//
//  Created by Marc Junyent Martín on 12/11/14.
//  Copyright (c) 2014 Marc Junyent Martín. All rights reserved.
//

#ifndef __Synesthesia__Player__
#define __Synesthesia__Player__

#include <string>

class Player {
public:
    //Player() = 0;
    
    virtual void loadFile(std::string url) = 0;
    
    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;
    
    virtual long framecount() = 0;
};


#endif /* defined(__Synesthesia__Player__) */
