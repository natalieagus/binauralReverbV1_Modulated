//
//  Parameter.cpp
//  TheEngineSample
//
//  Created by Natalie Agus on 28/10/15.
//  Copyright © 2015 A Tasty Pixel. All rights reserved.
//

#include "Parameter.hpp"

void Parameter::setListenerLocation(Point2d Ratio){
    this->listenerXYRatio = Ratio;
    
    this->listenerLoc = Point2d(this->roomWidth * Ratio.x, this->roomHeight * Ratio.y);
    this->listenerLocLeftEar.x = this->listenerLoc.x - RADIUSOFHEAD;
    this->listenerLocLeftEar.y = this->listenerLoc.y;
    this->listenerLocRightEar.x = this->listenerLoc.x + RADIUSOFHEAD;
    this->listenerLocRightEar.y = this->listenerLoc.y;
}

void Parameter::setSoundLocation(Point2d Ratio){
    this->soundXYRatio = Ratio;
    this->soundSourceLoc = Point2d(this->roomWidth * Ratio.x, this->roomHeight * Ratio.y);
}

void Parameter::setRoomSize(float size){
    this->roomSizeRatio = size;
    this->roomSize = this->roomSizeRatio * ROOMSIZE;
    this->roomWidth = this->roomSize * (widthRatio/0.5f);
    this->roomHeight = this->roomSize * ((1.0f-widthRatio)/0.5f);
    setListenerLocation(this->listenerXYRatio);
    setSoundLocation(this->soundXYRatio);
}

void Parameter::setWidthRatio(float ratio){
    this->widthRatio = ratio;
    this->roomWidth = this->roomSize * (widthRatio/0.5f);
    this->roomHeight = this->roomSize * ((1.0f-widthRatio)/0.5f);
    setListenerLocation(this->listenerXYRatio);
    setSoundLocation(this->soundXYRatio);
//    printf("New roomWidth: %f, new roomHeight: %f, new ListenerXY %f %f new SoundXY %f %f \n", roomWidthCM, roomHeightCM, listenerLoc.x, listenerLoc.y, soundSourceLoc.x, soundSourceLoc.y);
}