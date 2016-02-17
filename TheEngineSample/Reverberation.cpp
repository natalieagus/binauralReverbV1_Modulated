//
//  Reverberation.cpp
//  TheEngineSample
//
//  Created by Natalie Agus on 17/2/16.
//  Copyright © 2016 A Tasty Pixel. All rights reserved.
//

#include "Reverberation.hpp"
using namespace std;

Reverberation::Reverberation(void)
{

}

size_t min(size_t a, size_t b){ return a < b ? a : b; }

void Reverberation::processIFretlessBuffer(float* input, size_t numFrames, float * outputL, float* outputR){
    
    size_t framesToProcess = numFrames;
    while (framesToProcess > 0) {
        
        size_t framesInThisIteration = min(framesToProcess,REVERB_BUFFER_SIZE);
        
        // process the reverbs into the buffers
        size_t outputIndex = numFrames - framesToProcess;
        reverb.processIFretlessBuffer(&input[outputIndex], framesInThisIteration, & outputL[outputIndex] ,&outputR[outputIndex]);


        // update the number of frames left to process
        framesToProcess -= framesInThisIteration;

    }
}

void Reverberation::updateReverbSettings(){
    reverb.setParameter(parameters);
}

void Reverberation::setListenerLocation(float* loc){
    Point2d Ratio = Point2d(loc[0],1.0f-loc[1]);
    printf("loc of listener is: %f %f \n", Ratio.x, Ratio.y);
    this->parameters.setListenerLocation(Ratio);
    updateReverbSettings();
    
}

void Reverberation::setSoundLocation(float* loc){
    Point2d Ratio = Point2d(loc[0],1.0f-loc[1]);
    this->parameters.setSoundLocation(Ratio);
    printf("loc of sound is: %f %f \n", Ratio.x, Ratio.y);
    updateReverbSettings();

}


void Reverberation::setSoundAndListenerLocation(float* locL, float* locS){
    Point2d RatioL = Point2d(locL[0],1.0f-locL[1]);
    //  printf("loc of listener is: %f %f \n", RatioL.x, RatioL.y);
    this->parameters.setListenerLocation(RatioL);
    Point2d RatioS = Point2d(locS[0],1.0f-locS[1]);
    //  printf("loc of sound is: %f %f \n", RatioS.x, RatioS.y);
    this->parameters.setSoundLocation(RatioS);
    updateReverbSettings();

}

void Reverberation::setRoomSize(float size){
    parameters.setRoomSize(size);
    updateReverbSettings();

}

void Reverberation::setWidthRatio(float widthRatio){
    this->parameters.setWidthRatio(widthRatio);
    updateReverbSettings();

}

void Reverberation::setRT60(float RT60){
    this->parameters.RT60 = RT60;
    updateReverbSettings();

}

void Reverberation::setRoomRayModel(bool roomRayModel){
    this->parameters.roomRayModelOn = roomRayModel;
    updateReverbSettings();

}



void Reverberation::setReverbONOFF(bool on){
    if (on){
        reverb.reverbPortionOn = 1.0f;

    }
    else{
        reverb.reverbPortionOn = 0.0f;

    }
}

void Reverberation::setDirectONOFF(bool on){
    if (on){
        reverb.directPortionOn = 1.0f;

    }
    else{
        reverb.directPortionOn = 0.0f;

    }
}