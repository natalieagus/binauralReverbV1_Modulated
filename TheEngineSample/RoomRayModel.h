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
    void setBouncePoints(Point2d* bouncePoints, Point2d wallOrientation, Point2d wallStart, float wallLength, size_t numPoints, float* outputGains2, float* inputGains2);
    Point2d getBP(float pointSpacing, Point2d wallStart, size_t i, Point2d wallOrientation, float randFlt);
        
    Point2d soundSourceLoc; Point2d listenerLoc;
    
    //original integration method
    float getGain(Point2d start, Point2d end, Point2d loc);
    float integrate(Point2d start, Point2d end, float t, Point2d loc, Point2d vn);
    
    //Simple integration method
    float integrationSimple(Point2d loc, float x, bool listLoc);
    Point2d align(Point2d point, Point2d wallvector);
    float xAlignedIntegration(Point2d loc, Point2d ptStart, Point2d ptEnd, bool listLoc);
    void setFloorBouncePointsGain(Point2d* bouncePoints, float* inputGain, float* outputGain, size_t floorTaps);
    void gridBP(Point2d* floorBouncePoints, size_t floorTaps);
    float pythagorasGain(Point2d loc, Point2d* bouncePoint, float height);
    
    size_t floorTapsPerDimension;
    
    float getMaxGain(float xLower, float xUpper, float yLower, float yUpper);
    float calcMaxGain(float x, float y);
    float maxFloorGain = 1.55463f;
    float gridArea;
public:
    RoomRayModel();
    
    void setRoomGeometry(Point2d* corners, size_t numCorners);
    
    void setLocation(float* rayLengths,size_t numTaps, Point2d listenerLocation, Point2d soundSourceLocation, Point2d* bouncePoints, float* outputGains2, float* inputGains2, Point2d* floorBouncePoints, size_t floorTaps);
    
    

};

#endif /* RoomRayModel_h */
