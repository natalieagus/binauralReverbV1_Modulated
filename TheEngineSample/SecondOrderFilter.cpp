//
//  SecondOrderFilter.cpp
//  TheEngineSample
//
//  Created by Natalie Agus on 2/11/15.
//  Copyright © 2015 A Tasty Pixel. All rights reserved.
//

#include "SecondOrderFilter.hpp"

SecondOrderFilter::SecondOrderFilter(){
    a0 = a1 = a2 = b0 = b1 = b2 = x1 = x2 = y1 = y2 = 0.0f;
}

//input is notch frequency and sampling rate
void SecondOrderFilter::setNotchFilter(float notch, float fc){
    
}

void SecondOrderFilter::setResonatorBlock(float G, float fs, float fc){
    
}
void SecondOrderFilter::setReflectionBlock(float G, float fs, float fc){
    
}

float SecondOrderFilter::process(float input){
    float output = b0 * input + b1 * x1 + b2 * x2 - ((a1 * y1 + a2 * y2) / a0);
    //update variables
    x2 = x1;
    x1 = input;
    y2 = y1;
    y1 = output;
    return output;
}
