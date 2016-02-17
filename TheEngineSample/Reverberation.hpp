//
//  Reverberation.hpp
//  TheEngineSample
//
//  Created by Natalie Agus on 17/2/16.
//  Copyright © 2016 A Tasty Pixel. All rights reserved.
//

#ifndef Reverberation_hpp
#define Reverberation_hpp

#include <stdio.h>
#import "FDN.h"
#define REVERB_BUFFER_SIZE 256
#import "Parameter.hpp"

class Reverberation{
public:
    
    Reverberation(void);
    
    Parameter parameters = Parameter();
    
    FDN reverb;
    
    float roomSize = 0.15;
    float widthRatio = 0.5;
    float RT60 = 0.7f;
    Point2d listenerLocation = Point2d(0.5f, (1.0f/3.0f));
    Point2d soundLocation = Point2d(0.5f, (2.0f/3.0f));
    
    void setListenerLocation(float* loc);
    void setSoundLocation(float* loc);
    void setSoundAndListenerLocation(float* locL, float* locS);
    void setRoomSize(float size);
    void setWidthRatio(float size);
    void setRT60(float RT60);
    void updateReverbSettings();
    void setRoomRayModel(bool roomRay);
    void processIFretlessBuffer(float* IOBuffer, size_t frames, float * outputL, float* outputR);
    
    
    void setReverbONOFF(bool on);
    void setDirectONOFF(bool onO);
    
private:
    float l_buffer_1[REVERB_BUFFER_SIZE];
    float r_buffer_1[REVERB_BUFFER_SIZE];

};
#endif /* Reverberation_hpp */
