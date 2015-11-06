//
//  RoomRayModel.h
//  TheEngineSample
//
//  Created by Hans on 6/11/15.
//  Copyright © 2015 A Tasty Pixel. All rights reserved.
//

#ifndef RoomRayModel_h
#define RoomRayModel_h

#include <stdio.h>
#include "Point2d.hpp"
#include "mactypes.h"

#define RRM_MAX_CORNERS 100

class RoomRayModel {
private:
    Point2d corners[RRM_MAX_CORNERS];
    Point2d wallOrientations[RRM_MAX_CORNERS];
    float wallLengths[RRM_MAX_CORNERS];
    size_t numCorners;
    float totalWallLength;
    void setBouncePoints(Point2d* bouncePoints, Point2d wallOrientation, Point2d wallStart, float wallLength, size_t numPoints);

    
public:
    RoomRayModel();
    
    void setRoomGeometry(Point2d* corners, size_t numCorners);
    
    void setLocation(float* inputGains, float* outputGains, float* rayLengths,size_t numTaps, Point2d listenerLocation, Point2d soundSourceLocation, Point2d* bouncePoints);

};

#endif /* RoomRayModel_h */
