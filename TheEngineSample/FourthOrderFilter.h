//
//  FourthOrderFilter.h
//  ViolinModel
//
//  Created by Hans on 7/7/15.
//  Copyright (c) 2015 A Tasty Pixel. All rights reserved.
//

#ifndef __ViolinModel__FourthOrderFilter__
#define __ViolinModel__FourthOrderFilter__

#include <stdio.h>

#define VECTOR_LEN 256

class FourthOrderFilter {
private:
    float b0, z1b, z2b, z3b, z4b, z1a, z2a, z3a, z4a;
    float A [4] = {}; // filter coefficients
    float B [4] = {}; // filter coefficients
    float temp [4] = {};
    float Zb [VECTOR_LEN] = {};
    float Za [VECTOR_LEN] = {};
    float* zb1;
    float* za1;
    float* ZbEnd = Zb + VECTOR_LEN - 1;
    float* ZaEnd = Za + VECTOR_LEN - 1;
    void reset();
    
public:
    FourthOrderFilter(float fc);
    FourthOrderFilter();
    float process(float sample);
    void setLowPass(float fc);
};

#endif /* defined(__ViolinModel__FourthOrderFilter__) */
