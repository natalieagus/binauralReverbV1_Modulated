//
//  RoomRayModel.c
//  TheEngineSample
//
//  Created by Hans on 6/11/15.
//  Copyright © 2015 A Tasty Pixel. All rights reserved.
//

#include "RoomRayModel.h"
#include "assert.h"
#include "string.h"
#include "math.h"
#include "FDN.h"

RoomRayModel::RoomRayModel(){
    numCorners = 0;
}

void RoomRayModel::setBouncePoints(Point2d* bouncePoints, Point2d wallOrientation, Point2d wallStart, float wallLength, size_t numPoints, float* outputGains, float* inputGains, bool bpSet){
    // average space between each pair of points
    float pointSpacing = wallLength / numPoints;
    
    Point2d prevStart = wallStart;
    // set the points at even but randomly jittered locations
    for (size_t i = 0; i < numPoints; i++) {
        float randFlt = (float)rand() / (float)RAND_MAX;
        
        if (!bpSet)
        bouncePoints[i] = getBP(pointSpacing, wallStart, i, wallOrientation, randFlt);
        
        if(i>0){
            Point2d start = prevStart;
            Point2d difference = (bouncePoints[i] - bouncePoints[i-1]).scalarMul(0.5f);
            Point2d end = bouncePoints[i-1] + difference;
            outputGains[i-1] = xAlignedIntegration(listenerLoc, start, end, true);
            inputGains[i-1] = xAlignedIntegration(soundSourceLoc, start, end, false);
            prevStart = Point2d(end.x, end.y);
        }
        
    }
    
    //do the last gain
    Point2d end = wallStart + wallOrientation.scalarMul(wallLength);
    outputGains[numPoints-1] = xAlignedIntegration(listenerLoc, prevStart, end, true);
    inputGains[numPoints-1] = xAlignedIntegration(soundSourceLoc, prevStart, end, false);
    
}

Point2d RoomRayModel::getBP(float pointSpacing, Point2d wallStart, size_t i, Point2d wallOrientation, float randFlt){
    float distance = (((float)i+randFlt) * pointSpacing);
    Point2d bp = wallStart + wallOrientation.scalarMul(distance);
    return bp;
}


void RoomRayModel::gridBP(Point2d* floorBouncePoints, size_t floorTaps){
    float xSpacing = wallLengths[0] / floorTaps;
    float ySpacing = wallLengths[1] / floorTaps;
    
    for (size_t i = 0; i < floorTaps; i++){ //y
        for (size_t j = 0; j < floorTaps; j++) { //x
            float randFltX = (float)rand() / (float)RAND_MAX;
            float randFltY = (float)rand() / (float)RAND_MAX;
            //printf("index  %lu : ", i*floorTaps+j);
            floorBouncePoints[i*floorTaps + j] = Point2d(((float)j + randFltX) * xSpacing, ((float)i + randFltY) * ySpacing);
            //printf(" --- PT x %f y %f \n", floorBouncePoints[i*floorTaps + j].x, floorBouncePoints[i*floorTaps + j].y);
        }
    }
}

void RoomRayModel::setFloorBouncePointsGain(Point2d* bouncePoints, float* inputGain, float* outputGain, size_t floorTaps){
    for (size_t i = 0; i < floorTaps; i++){
        inputGain[i] = pythagorasGain(soundSourceLoc, bouncePoints[i], 1.65f);
        outputGain[i] = pythagorasGain(listenerLoc, bouncePoints[i], 1.65f);
    }
}

float RoomRayModel::pythagorasGain(Point2d loc, Point2d bouncePoint, float height){
    return 1.0f/sqrtf( powf(loc.distance(bouncePoint), 2.f) + powf((1.f + (height-1.f)*(float)rand()/float(RAND_MAX)), 2.f));
}

//Raylength isnt used?
void RoomRayModel::setLocation(float* rayLengths, size_t numTaps, Point2d listenerLocation, Point2d soundSourceLocation, Point2d* bouncePoints, float* outputGains, float* inputGains, bool bpSet, Point2d* floorBouncePoints, size_t floorTaps){
    
    if (bpSet){
        printf("BP is set, no need to generate new points\n");
    }
    
    printf("Numtaps is: %lu, floorTaps is : %lu", numTaps, floorTaps);
    numTaps -= floorTaps;
    floorTapsPerDimension = (size_t) sqrtf(floorTaps);
    
    assert(numCorners > 0); // the geometry must be initialised before now
    soundSourceLoc = soundSourceLocation;
    listenerLoc = listenerLocation;
    
    // set the number of taps on each wall proportional to the
    // length of the wall
    size_t numTapsOnWall[RRM_MAX_CORNERS];
    size_t totalTaps = 0;
    for (size_t i = 0; i < RRM_MAX_CORNERS; i++) {
        numTapsOnWall[i] = (size_t)floor(wallLengths[i]/totalWallLength * (float)numTaps);
        totalTaps += numTapsOnWall[i];
    }
    
    // if the number of taps now assigned isn't enough, add one tap to
    // each wall until we have the desired number
    size_t i = 0;
    while (totalTaps < numTaps) {
        numTapsOnWall[i]++;
        i++;
        totalTaps++;
        if (i == RRM_MAX_CORNERS) i = 0;
    }
    
//    float inGainScale[NUMTAPSSTD];
    
    // set bounce points for each wall
    size_t j = 0;
    for (size_t i = 0; i < numCorners; i++) {
        //must be corner i-1 or shift the corner values firston
        setBouncePoints(&bouncePoints[j], wallOrientations[i], corners[i], wallLengths[i], numTapsOnWall[i],&outputGains[j],&inputGains[j], bpSet);
        j += numTapsOnWall[i];
    }
    
 
    // set bounce points for the floor
    if (!bpSet){
    gridBP(&bouncePoints[j], floorTapsPerDimension);
    }
    setFloorBouncePointsGain(&bouncePoints[j], &inputGains[j], &outputGains[j], floorTaps);
    
    //printout Bouncepoints
    
    numTaps += floorTaps;
    
//    printf("Bouncepoints X & Y printout: \n");
//    for (int i = 0; i < numTaps ; i++){
//        printf("index %d x %f, y %f \n",i, bouncePoints[i].x, bouncePoints[i].y);
//    }
    
    // normalize the total input gain to 1.0f
    float totalSquaredInputGain = 0.0f;
    for (size_t i = 0; i < numTaps; i++) {
        totalSquaredInputGain += inputGains[i]*inputGains[i];
    }

    float inGainNormalize = 1.0f / sqrt(totalSquaredInputGain);
    for (size_t i = 0; i < numTaps; i++) {
        inputGains[i] *= inGainNormalize;
        printf("inputGains[%lu] : %f \n", i, inputGains[i]);
    }
    
    //normalize the total out gain to 1.0f
    float totalSquaredOutputGain = 0.0f;
    for (size_t i = 0; i< numTaps; i++){
        totalSquaredOutputGain += outputGains[i]*outputGains[i];
    }

    float outputGainNormalize = 1.0f / sqrtf(totalSquaredOutputGain);
    for (size_t i = 0; i< numTaps; i++){
        outputGains[i] *= outputGainNormalize;
        printf("OutputGain[%lu] : %f \n", i, outputGains[i]);
    }
    

}


void RoomRayModel::setRoomGeometry(Point2d* corners, size_t numCorners){
    assert(numCorners >= 3);
    
    this->numCorners = numCorners;
    
    // save the locations of the corners
    memcpy(this->corners,corners,sizeof(Point2d)*numCorners);
    
    // get normalized vectors to represent the orientation of each wall
    // and get length of each wall
    assert(numCorners < RRM_MAX_CORNERS);
    totalWallLength = 0.0f;
    for (size_t i = 1; i < numCorners; i++) {
        // get orientation vector
        wallOrientations[i] = corners[i] - corners[i-1];

        
        // get wall length
        wallLengths[i] = wallOrientations[i].length();
        totalWallLength += wallLengths[i];
        
        // normalize the orientation vector
        wallOrientations[i].normalize();
    }
    
    // get the values that wrap around from the end of the for loop above
    wallOrientations[0] = corners[0] - corners[numCorners-1];
    wallLengths[0] = wallOrientations[0].length();
    totalWallLength += wallLengths[0];
    wallOrientations[0].normalize();
    
    assert(totalWallLength > 0.0f);
    
    //change the corner indexes to match the wallOrientation indexes for setLocation method
    Point2d lastCorner = this->corners[numCorners-1];
    Point2d prevCorner = this->corners[0];
    Point2d currCorner;
    for (size_t i = 1; i<numCorners; i++){
        currCorner = this->corners[i];
        this->corners[i] = prevCorner;
        prevCorner = currCorner;
    }
    this->corners[0] = lastCorner;
}

float RoomRayModel::getGain(Point2d start, Point2d end, Point2d loc){
    float tE = (end-start).length();
    float tS = 0.0f;
    
    Point2d normalized = (end - start).normalize();
    
    float startVal = integrate(start, end, tS, loc, normalized);
    float endVal = integrate(start, end, tE, loc, normalized);
    
    return endVal - startVal;
    
}

//Original integration method
float RoomRayModel::integrate(Point2d start,  Point2d end, float t, Point2d loc, Point2d vn){
    float a = 1.0f ;
    float b = start.x * vn.x - loc.x * vn.x + t * vn.x * vn.x + vn.y * (start.y - loc.y + t * vn.y);
    float c = 1.0f ;
    float d = (start.x - loc.x + t * vn.x) * (start.x - loc.x + t * vn.x) + (start.y - loc.y) * (start.y - loc.y) + 2 * t * (start.y - loc.y) * vn.y + t * t * vn.y * vn.y;
    return (a * logf(b + c * sqrtf(d)));
}

//Simpler integration method
float RoomRayModel::integrationSimple(Point2d loc, float x, bool listLoc){
    float value = (x-loc.x)/loc.y;
   // printf("Value is : %f \n", value);
    if (listLoc){
    return asinhf(value);
    }
    else{
    return atanf(value);
    }
}

Point2d  RoomRayModel::align(Point2d point, Point2d wallvector){
    //normalize wall vector
    wallvector.normalize();
    float x = wallvector.x * point.x + wallvector.y * point.y;
    float y = -1.0f*wallvector.y * point.x + wallvector.x * point.y;
    return Point2d(x,y);
}

//this returns the gain, can be used for both input and output
float  RoomRayModel::xAlignedIntegration(Point2d loc, Point2d ptStart, Point2d ptEnd, bool listLoc){
    Point2d wallVector = ptEnd - ptStart;
    
    Point2d alignedStart = align(ptStart, wallVector);
    Point2d alignedEnd = align(ptEnd, wallVector);
    Point2d alignedLoc = align(loc, wallVector);
    
    alignedEnd = alignedEnd - alignedStart;
    alignedLoc = alignedLoc - alignedStart;
    
    float endVal = integrationSimple(alignedLoc, alignedEnd.x, listLoc);
    float startVal = integrationSimple(alignedLoc, 0.0f, listLoc);
    
    return endVal - startVal;
}
