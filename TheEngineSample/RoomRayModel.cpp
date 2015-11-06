//
//  RoomRayModel.c
//  TheEngineSample
//
//  Created by Hans on 6/11/15.
//  Copyright © 2015 A Tasty Pixel. All rights reserved.
//

#include "RoomRayModel.h"
#include "assert.h"

#define RRM_MAX_CORNERS 100

void RRM_setGains(float* inputGains, float* outputGains, size_t numTaps, Point2d* corners, size_t numCorners, Point2d listenerLocation, Point2d soundSourceLocation){
    
    // get normalized vectors to represent the orientation of each wall
    assert(numCorners < RRM_MAX_CORNERS);
    Point2d wallOrientations[RRM_MAX_CORNERS];
    for (size_t i = 1; i < numCorners; i++) {
        wallOrientations[i] = corners[i] - corners[i-1];
        wallOrientations[i].normalize();
    }
    wallOrientations[0] = corners[0] - corners[numCorners-1];
    wallOrientations[0].normalize();
    
}