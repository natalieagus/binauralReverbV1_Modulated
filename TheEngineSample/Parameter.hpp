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
#include <math.h>

#define INITIALROOMSIZE 0.15f
#define INITIALWIDTHRATIO 0.5f
#define INITIALLISTENERX 0.5f
#define INITIALLISTENERY (1.0f/3.0f)
#define INITIALSOUNDX 0.5f
#define INITIALSOUNDY (2.0f/3.0f)
#define INITIALRT60 0.4f
#define RADIUSOFHEAD 0.08f //8cm radius of head
#define ROOMSIZE 20.f //20 metres max
#define ROOMCEILING 2.f
#define INITIALDIRECTGAIN 1.f ///(4.f*M_PI);
#define INITIALREVERBGAIN 1.f ///(4.f*M_PI);
#define REFERENCEDISTANCE 1.f; //The original volume is as loud as we can hear within 1 metre away from the soundsource

typedef struct Parameter {
    
    //Initial values of parameter
    Parameter(){
        this->roomSize = INITIALROOMSIZE * ROOMSIZE;
        this->roomSizeRatio = INITIALROOMSIZE;
        this->widthRatio = INITIALWIDTHRATIO;
        this->roomWidth = this->roomSize * (widthRatio/0.5f);
        this->roomHeight = this->roomSize * ((1.0f-widthRatio)/0.5f);
        this->roomCeiling = ROOMCEILING;
        
        Point2d L = Point2d(INITIALLISTENERX, INITIALLISTENERY);
        setListenerLocation(L);
        Point2d S = Point2d(INITIALSOUNDX, INITIALSOUNDY);
        setSoundLocation(S);
        this->listenerXYRatio = Point2d(INITIALLISTENERX, INITIALLISTENERY);
        this->soundXYRatio = Point2d(INITIALSOUNDX, INITIALSOUNDY);
    
        this->RT60 = INITIALRT60;
        
        this->directGain = INITIALDIRECTGAIN;
        this->reverbGain = INITIALREVERBGAIN;
        
        this->roomRayModelOn = true;
        
    }
    
    void setListenerLocation(Point2d Ratio);
    void setSoundLocation(Point2d Ratio);
    void setRoomSize(float size);
    void setWidthRatio(float ratio);
    
    Point2d listenerXYRatio;
    Point2d soundXYRatio;
    
    Point2d listenerLoc;
    Point2d listenerLocLeftEar;
    Point2d listenerLocRightEar;
    Point2d soundSourceLoc;
    float roomSizeRatio, RT60, widthRatio,roomSize;
    float roomWidth, roomHeight;
    float roomCeiling;
    float directGain, reverbGain;
    
    bool roomRayModelOn;
    
} Parameter;

#endif /* Parameter_hpp */
