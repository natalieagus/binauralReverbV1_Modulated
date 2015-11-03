//
//  CombFilter.cpp
//  TheEngineSample
//
//  Created by Natalie Agus on 2/11/15.
//  Copyright © 2015 A Tasty Pixel. All rights reserved.
//

#include "CombFilter.hpp"

CombFilter::CombFilter(){
    gX = gY = x1 = y1 = 0.0f;
}

void CombFilter::setHComb(float R, float td){
    
}

float CombFilter::process(float input){
    float output = gX * x1 + gY * y1 + input;
    x1 = input;
    y1 = output;
    return output;
}