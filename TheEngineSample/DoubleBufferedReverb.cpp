//
//  DoubleBufferedReverb.cpp
//  TheEngineSample
//
//  Created by Natalie Agus on 27/10/15.
//  Copyright © 2015 A Tasty Pixel. All rights reserved.
//

#include "DoubleBufferedReverb.h"
using namespace std;

DoubleBufferedReverb::DoubleBufferedReverb(void)
{
    // set reverb2 to be the back buffer reverb
    backReverb = &reverb2;
    
    // set the gain on reverb 1 to 1.0f;
    r1_gainTarget = 1.0f;
    r1_gain[DB_REVERB_BUFFER_SIZE-1] = 1.0f;
}

size_t min(size_t a, size_t b){ return a < b ? a : b; }

void DoubleBufferedReverb::processIFretlessBuffer(float* input, size_t numFrames, float * outputL, float* outputR){
   
    size_t framesToProcess = numFrames;
    while (framesToProcess > 0) {

        size_t framesInThisIteration = min(framesToProcess,DB_REVERB_BUFFER_SIZE);
        
        // process the reverbs into the buffers
        size_t outputIndex = numFrames - framesToProcess;
        reverb1.processIFretlessBuffer(&input[outputIndex], framesInThisIteration,  l_buffer_1,r_buffer_1);
        reverb2.processIFretlessBuffer(&input[outputIndex], framesInThisIteration, l_buffer_2, r_buffer_2);
//        
        // compute the gains for reverb1
        float r1gainAtFrameStart = r1_gain[DB_REVERB_BUFFER_SIZE-1];
        float r1gainAtFrameEnd = r1gainAtFrameStart + .5*(r1_gainTarget - r1gainAtFrameStart);
       // printf("gain 1 at frame start: %f, gain at frame end: %f\n", r1gainAtFrameStart, r1gainAtFrameEnd);
        vDSP_vgen(&r1gainAtFrameStart, &r1gainAtFrameEnd, r1_gain, 1, framesInThisIteration);
      //  printf("r1_gain: %f %f %f %f\n", r1_gain[0],r1_gain[1],r1_gain[2], r1_gain[3]);
        
        // compute the gains for reverb2
        float r2gainAtFrameStart = sqrt(1.0f - r1gainAtFrameStart);
        float r2gainAtFrameEnd = sqrt(1.0f - r1gainAtFrameEnd);
      //  printf("gain 2 at frame start: %f, gain at frame end: %f\n", r2gainAtFrameStart, r2gainAtFrameEnd);
        vDSP_vgen(&r2gainAtFrameStart, &r2gainAtFrameEnd, r2_gain, 1, framesInThisIteration);
       // printf("r2_gain: %f %f %f %f\n", r2_gain[0],r2_gain[1],r2_gain[2], r2_gain[3]);
//
        // adjust the volume in the buffers to fade between reverbs
        vDSP_vmul(r1_gain, 1, l_buffer_1, 1, l_buffer_1, 1, framesInThisIteration);
        vDSP_vmul(r1_gain, 1, r_buffer_1, 1, r_buffer_1, 1, framesInThisIteration);
        vDSP_vmul(r2_gain, 1, l_buffer_2, 1, l_buffer_2, 1, framesInThisIteration);
        vDSP_vmul(r2_gain, 1, r_buffer_2, 1, r_buffer_2, 1, framesInThisIteration);
        
        // sum the buffers into the outputs
    
        vDSP_vadd(l_buffer_1, 1, l_buffer_2, 1, &outputL[outputIndex], 1, framesInThisIteration);
        vDSP_vadd(r_buffer_1, 1, r_buffer_2, 1, &outputR[outputIndex], 1, framesInThisIteration);
        
        // update the number of frames left to process
        framesToProcess -= framesInThisIteration;
    }
}

void DoubleBufferedReverb::updateReverbSettings(){
    backReverb->setListenerLoc(listenerLocation);
    backReverb->setSoundSourceLoc(soundLocation);
    backReverb->setWidthRatio(widthRatio);
    backReverb->setRT60(RT60);
    backReverb->setRoomSize(roomSize);
}

void DoubleBufferedReverb::setListenerLocation(float* loc){
    Point2d L = Point2d(loc[0],loc[1]);
    this->listenerLocation = L;
        backReverb->setListenerLoc(listenerLocation);
    //updateReverbSettings();
    flip();
        backReverb->setListenerLoc(listenerLocation);

}

void DoubleBufferedReverb::setSoundLocation(float* loc){
    Point2d L = Point2d(loc[0],loc[1]);
    this->soundLocation = L;
        backReverb->setSoundSourceLoc(soundLocation);
    //updateReverbSettings();
    flip();
        backReverb->setSoundSourceLoc(soundLocation);
}

void DoubleBufferedReverb::setRoomSize(float size){
    this->roomSize = size;
        backReverb->setRoomSize(roomSize);
 //   updateReverbSettings();
    flip();
        backReverb->setRoomSize(roomSize);
}

void DoubleBufferedReverb::setWidthRatio(float widthRatio){
    this->widthRatio = widthRatio;
        backReverb->setWidthRatio(widthRatio);
   // updateReverbSettings();
    flip();
    backReverb->setWidthRatio(widthRatio);
}

void DoubleBufferedReverb::setRT60(float RT60){
    this->RT60 = RT60;
    backReverb->setRT60(RT60);
   // updateReverbSettings();
    flip();
    backReverb->setRT60(RT60);
}

void DoubleBufferedReverb::flip(){
    // if reverb 1 is on the back buffer
    if (backReverb == &reverb1) {
        // move reverb 1 to the front buffer by increasing the gain
        r1_gainTarget = 1.0f;
        backReverb = &reverb2;
    } else {
        // move reverb 1 to the back buffer by decreasing the gain
        r1_gainTarget = 0.0f;
        backReverb = &reverb1;
    }
}