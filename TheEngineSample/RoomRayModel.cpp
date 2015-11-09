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

void RoomRayModel::setBouncePoints(Point2d* bouncePoints, Point2d wallOrientation, Point2d wallStart, float wallLength, size_t numPoints){
    // average space between each pair of points
    float pointSpacing = wallLength / numPoints;
    
    // set the points at even but randomly jittered locations
    for (size_t i = 0; i < numPoints; i++) {
        float randFlt = (float)rand() / (float)RAND_MAX;
        float distance = (((float)i+randFlt) * pointSpacing);
        bouncePoints[i] = wallStart + wallOrientation.scalarMul(distance);
       // printf("BouncePoints : %f %f \n", bouncePoints[i].x, bouncePoints[i].y);
    }
}


//Raylength isnt used?
void RoomRayModel::setLocation(float* inputGains, float* outputGains, float* rayLengths, size_t numTaps, Point2d listenerLocation, Point2d soundSourceLocation, Point2d* bouncePoints){
    
    assert(numCorners > 0); // the geometry must be initialised before now
    
    
    // set the number of taps on each wall proportional to the
    // length of the wall
    size_t numTapsOnWall[RRM_MAX_CORNERS];
    size_t totalTaps = 0;
    for (size_t i = 0; i < RRM_MAX_CORNERS; i++) {
        numTapsOnWall[i] = (size_t)floor(wallLengths[i]/totalWallLength * (float)numTaps);
        totalTaps += numTapsOnWall[i];
    }
    
    printf("Total taps is: %lu \n", totalTaps);
    
    // if the number of taps now assigned isn't enough, add one tap to
    // each wall until we have the desired number
    size_t i = 0;
    while (totalTaps < numTaps) {
        numTapsOnWall[i]++;
        i++;
        totalTaps++;
        if (i == RRM_MAX_CORNERS) i = 0;
    }
    
    float inGainScale[NUMTAPSSTD];
    
    // set bounce points for each wall
    size_t j = 0;
    size_t k = 0;
    for (size_t i = 0; i < numCorners; i++) {
        //must be corner i-1 or shift the corner values firston
      //  printf(" i = %lu, Wall orientation : %f %f, wallStart : %f %f, wallLength : %f TapsOnWall : %lu\n",i, wallOrientations[i].x, wallOrientations[i].y, corners[i].x, corners[i].y, wallLengths[i], numTapsOnWall[i] );
        setBouncePoints(&bouncePoints[j], wallOrientations[i], corners[i], wallLengths[i], numTapsOnWall[i]);
        j += numTapsOnWall[i];
        
        // find the input gain scale for each point
        while (k < j) {
            Point2d sourceToBouncePoint = (soundSourceLocation - bouncePoints[k]).normalize();
            Point2d wallNormal = wallOrientations[i].normal();
            inGainScale[k] = wallNormal.dotProduct(sourceToBouncePoint);
          //  printf("inGainScale: %lu is %f\n", k, inGainScale[k]);
            //TODO: does ingainscale have to be +ve?
            k++;
        }
    }
    
    // set input gain porportional to inGainScale/distance(source,bouncePoint)
    float totalSquaredInputGain = 0.0f;
    for (size_t i = 0; i < numTaps; i++) {
        inputGains[i] = inGainScale[i] / soundSourceLocation.distance(bouncePoints[i]);
        totalSquaredInputGain += inputGains[i]*inputGains[i];
    }
    
    // normalize the total input gain to 1.0f
    float inGainNormalize = 1.0f / sqrt(totalSquaredInputGain);
    for (size_t i = 0; i < numTaps; i++) {
        inputGains[i] *= inGainNormalize;
    }
    
    // set output gain porportional to 1/distance(bouncePoint,ListenerLocation)
    float totalSquaredOutputGain = 0.0f;
    for (size_t i = 0; i < numTaps; i++) {
        outputGains[i] = 1.0f / listenerLocation.distance(bouncePoints[i]);
        totalSquaredOutputGain += outputGains[i]*outputGains[i];
    }
    
    // normalize the total input gain to 1.0f
    float outGainNormalize = 1.0f / sqrt(totalSquaredOutputGain);
    for (size_t i = 0; i < numTaps; i++) {
        outputGains[i] *= outGainNormalize;
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
    
    
//    printf("Total wall length: %f\n", totalWallLength);
//    for (int i = 0; i<numCorners; i++){
//        printf("Wall orientation %d is %f %f \n", i, wallOrientations[i].x, wallOrientations[i].y);
//        printf("Corners : %f %f \n", corners[i].x, corners[i].y);
//    }
}