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

void RoomRayModel::setBouncePoints(Point2d* bouncePoints, Point2d wallOrientation, Point2d wallStart, float wallLength, size_t numPoints, float* outputGains2, float* inputGains2){
    // average space between each pair of points
    float pointSpacing = wallLength / numPoints;
    
    Point2d prevStart = wallStart;
    // set the points at even but randomly jittered locations
    for (size_t i = 0; i < numPoints; i++) {
        float randFlt = (float)rand() / (float)RAND_MAX;
        bouncePoints[i] = getBP(pointSpacing, wallStart, i, wallOrientation, randFlt);
        
        if(i>0){
            Point2d start = prevStart;
            Point2d difference = (bouncePoints[i] - bouncePoints[i-1]).scalarMul(0.5f);
            Point2d end = bouncePoints[i-1] + difference;
            
  //      Point2d start = wallStart + wallOrientation.scalarMul(pointSpacing * i);
        //    printf("\nstart at: %f %f ", start.x, start.y);
    //    Point2d end = wallStart + wallOrientation.scalarMul(pointSpacing * (i+1));
          //  printf("end at: %f %f \n", end.x, end.y);
            outputGains2[i-1] = xAlignedIntegration(listenerLoc, start, end);
            inputGains2[i-1] = xAlignedIntegration(soundSourceLoc, start, end);
           // printf("Result of outgain: %f \n", outputGains2[i-1]);
           // printf("Result of ingain: %f \n", inputGains2[i-1]);
            prevStart = Point2d(end.x, end.y);
        }
        
        
    }
    //do the last gain
    Point2d end = wallStart + wallOrientation.scalarMul(wallLength);
  //  printf("start at: %f %f ", prevStart.x, prevStart.y);
    //    Point2d end = wallStart + wallOrientation.scalarMul(pointSpacing * (i+1));
 //   printf("end at: %f %f \n", end.x, end.y);
    outputGains2[numPoints-1] = xAlignedIntegration(listenerLoc, prevStart, end);
    inputGains2[numPoints-1] = xAlignedIntegration(soundSourceLoc, prevStart, end);
  //  printf("Outgain: %f \n", outputGains2[numPoints-1]);
  //  printf("Ingain: %f \n", outputGains2[numPoints-1]);
    
    
//    //test
//    Point2d testStart = Point2d(-1.f, 2.f);
//    Point2d testEnd = Point2d(-3.f, 6.f);
//    Point2d testListLoc = Point2d(-10.f, -15.f);
//    
//    float testGainSimple = xAlignedIntegration(testListLoc, testStart, testEnd);
//    float testGainNormal = getGain(testStart, testEnd, testListLoc);
//    printf("Test gain simple : %f \n Test Gain Normal : %f \n", testGainSimple, testGainNormal);
//
//    
   // printf("One wall orientation done ======== \n \n \n");
    
}

Point2d RoomRayModel::getBP(float pointSpacing, Point2d wallStart, size_t i, Point2d wallOrientation, float randFlt){
    float distance = (((float)i+randFlt) * pointSpacing);
    Point2d bp = wallStart + wallOrientation.scalarMul(distance);
    return bp;
}

//Raylength isnt used?
void RoomRayModel::setLocation(float* rayLengths, size_t numTaps, Point2d listenerLocation, Point2d soundSourceLocation, Point2d* bouncePoints, float* outputGains2, float* inputGains2){
    
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
    
  //  printf("Total taps is: %lu \n", totalTaps);
    
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
        setBouncePoints(&bouncePoints[j], wallOrientations[i], corners[i], wallLengths[i], numTapsOnWall[i],&outputGains2[j],&inputGains2[j]);
        j += numTapsOnWall[i];
        
        // find the input gain scale for each point (angle)
        while (k < j) {
            Point2d sourceToBouncePoint = (soundSourceLocation - bouncePoints[k]).normalize();
            Point2d wallNormal = wallOrientations[i].normal();
            inGainScale[k] = fabsf( wallNormal.dotProduct(sourceToBouncePoint));
       //     printf("inGainScale: %lu is %f\n", k, inGainScale[k]);
            //TODO: does ingainscale have to be +ve?
            k++;
        }
    }
    
    
//    // set input gain porportional to inGainScale/distance(source,bouncePoint)
//    float totalSquaredInputGain = 0.0f;
//    for (size_t i = 0; i < numTaps; i++) {
//        inputGains[i] = inGainScale[i] / soundSourceLocation.distance(bouncePoints[i]);
//      //  printf("inputGain: %f\n",inputGains[i]);
//        totalSquaredInputGain += inputGains[i]*inputGains[i];
//    }
    
    
//    // normalize the total input gain to 1.0f
//    float inGainNormalize = 1.0f / sqrt(totalSquaredInputGain);
//    for (size_t i = 0; i < numTaps; i++) {
//        inputGains[i] *= inGainNormalize;
//    }
    
    // set input gain porportional to inGainScale
    float totalSquaredInputGain2 = 0.0f;
    for (size_t i = 0; i < numTaps; i++) {
        inputGains2[i] *= inGainScale[i];
        //   printf("inputGain2: %f\n",inputGains2[i]);
        totalSquaredInputGain2 += inputGains2[i]*inputGains2[i];
    }
    
    // normalize the total input gain to 1.0f
    float inGainNormalize2 = 1.0f / sqrt(totalSquaredInputGain2);
    for (size_t i = 0; i < numTaps; i++) {
        inputGains2[i] *= inGainNormalize2;
      //  printf("inputGain2 normalised: %f\n",inputGains2[i]);
    }
//    
//    // set output gain porportional to 1/distance(bouncePoint,ListenerLocation)
//    float totalSquaredOutputGain = 0.0f;
//    for (size_t i = 0; i < numTaps; i++) {
//        outputGains[i] = 1.0f / listenerLocation.distance(bouncePoints[i]);
//        totalSquaredOutputGain += outputGains[i]*outputGains[i];
//    }
    
//    // normalize the total out gain to 1.0f
//    float outGainNormalize = 1.0f / sqrt(totalSquaredOutputGain);
//    for (size_t i = 0; i < numTaps; i++) {
//        outputGains[i] *= outGainNormalize;
//    }
    
    //normalize the total out gain2 to 1.0f
    float totalSquaredOutputGain2 = 0.0f;
    for (size_t i = 0; i< numTaps; i++){
       // printf("OutputGain2 : %f ", outputGains2[i]);
        totalSquaredOutputGain2 += outputGains2[i]*outputGains2[i];
      //  printf("totalSquare outputGain2 : %f \n", totalSquaredOutputGain2);
    }
  //  printf("totalSquare outputGain2 : %f \n", totalSquaredOutputGain2);
    float outputGain2Normalize = 1.0f / sqrtf(totalSquaredOutputGain2);
    for (size_t i = 0; i< numTaps; i++){
        outputGains2[i] *= outputGain2Normalize;
        //printf("Normalized outputGain2 : %f \n", outputGains2[i]);
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

float RoomRayModel::getGain(Point2d start, Point2d end, Point2d loc){
    float tE = (end-start).length();
    float tS = 0.0f;
    
    Point2d normalized = (end - start).normalize();
    
    //  printf("Sound x %f, Sound y %f \n", soundSourceLoc.x, soundSourceLoc.y);
    //  printf("Normalized: %f %f ", normalized.x, normalized.y);

    
    float startVal = integrate(start, end, tS, loc, normalized);
    float endVal = integrate(start, end, tE, loc, normalized);
    
    return endVal - startVal;
    
}

float RoomRayModel::integrate(Point2d start,  Point2d end, float t, Point2d loc, Point2d vn){
    float a = 1.0f ;
    float b = start.x * vn.x - loc.x * vn.x + t * vn.x * vn.x + vn.y * (start.y - loc.y + t * vn.y);
    float c = 1.0f ;
    float d = (start.x - loc.x + t * vn.x) * (start.x - loc.x + t * vn.x) + (start.y - loc.y) * (start.y - loc.y) + 2 * t * (start.y - loc.y) * vn.y + t * t * vn.y * vn.y;
    return (a * logf(b + c * sqrtf(d)));
}

//Simpler integration method
float RoomRayModel::integrationSimple(Point2d loc, float x){
    float value = (x-loc.x)/loc.y;
   // printf("Value is : %f \n", value);
    return asinhf(value);
}

Point2d  RoomRayModel::align(Point2d point, Point2d wallvector){
    //normalize wall vector
    wallvector.normalize();
  //  printf("Normalized wall vector : % f %f ", wallvector.x, wallvector.y);
    float x = wallvector.x * point.x + wallvector.y * point.y;
    float y = -1.0f*wallvector.y * point.x + wallvector.x * point.y;
    return Point2d(x,y);
}

//this returns the gain, can be used for both input and output
float  RoomRayModel::xAlignedIntegration(Point2d loc, Point2d ptStart, Point2d ptEnd){
    Point2d wallVector = ptEnd - ptStart;
  //  printf("Wall vector: %f %f \n", wallVector.x, wallVector.y);
    
    Point2d alignedStart = align(ptStart, wallVector);
    Point2d alignedEnd = align(ptEnd, wallVector);
    Point2d alignedLoc = align(loc, wallVector);

 //   printf("Aligned start: %f %f \n", alignedStart.x, alignedStart.y);
  //  printf("Aligned end: %f %f \n", alignedEnd.x, alignedEnd.y)
  // printf("Aligned loc: %f %f \n", alignedLoc.x, alignedLoc.y);
    
    alignedEnd = alignedEnd - alignedStart;
    alignedLoc = alignedLoc - alignedStart;
    
    float endVal = integrationSimple(alignedLoc, alignedEnd.x);
   // printf("endVal: %f ", endVal);
    float startVal = integrationSimple(alignedLoc, 0.0f);
  //  printf("startVal: %f \n", startVal);
    
    return endVal - startVal;
}
