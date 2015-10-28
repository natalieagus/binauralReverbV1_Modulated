//
//  Parameter.hpp
//  TheEngineSample
//
//  Created by Natalie Agus on 28/10/15.
//  Copyright © 2015 A Tasty Pixel. All rights reserved.
//

#ifndef Parameter_hpp
#define Parameter_hpp

#include <stdio.h>
#include "Point2d.hpp"

typedef struct Parameter {
    
    Parameter(Point2d listenerLoc, Point2d soundSourceLoc, float roomSize, float RT60, float widthRatio){
        this->listenerLoc = listenerLoc;
        this->soundSourceLoc = soundSourceLoc;
        this->roomSize = roomSize;
        this->RT60 = RT60;
        this->widthRatio = widthRatio;
    };
    
    Point2d listenerLoc;
    Point2d soundSourceLoc;
    float roomSize, RT60, widthRatio;
    
} Parameter;

#endif /* Parameter_hpp */
