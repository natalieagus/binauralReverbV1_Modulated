//
//  FourthOrderFilter.cpp
//  ViolinModel
//
//  Created by Hans on 7/7/15.
//  Copyright (c) 2015 A Tasty Pixel. All rights reserved.
//

#include "FourthOrderFilter.h"
#include <Accelerate/Accelerate.h>
#include "Constants.h"

#define ALPHA 2.6131259297
#define BETA  3.4142135623

FourthOrderFilter::FourthOrderFilter(float fc){
    setLowPass(fc);
    reset();
}

FourthOrderFilter::FourthOrderFilter(){
    b0 = 0.0f; // this prevents audio ouput until filter coefficients are set
    reset();
}

void FourthOrderFilter::reset(){
    z1a = z1b = z2a = z2b = z3a = z3b = z4a = z4b = 0.0f;
    za1 = ZaEnd - 3;
    zb1 = ZbEnd - 3;
    vDSP_vclr(Za, 1, VECTOR_LEN);
    vDSP_vclr(Zb, 1, VECTOR_LEN);
}

float FourthOrderFilter::process(float sample){
    float Aout, Bout;
    vDSP_vmul(A, 1, za1, 1, temp, 1, 4);
    vDSP_sve(temp, 1, &Aout, 4);
    vDSP_vmul(B, 1, zb1, 1, temp, 1, 4);
    vDSP_sve(temp, 1, &Bout, 4);
    float output = b0*sample + Bout - Aout;
    
    za1--;
    zb1--;
    
    if (za1 < Za) { // if we have reached the beginning of the buffer
    
        // copy za2, za3, za4, zb2, zb3, zb4 back to the end of the array
        *ZaEnd       = *(Za + 2);
        *(ZaEnd - 1) = *(Za + 1);
        *(ZaEnd - 2) = *Za;
        *ZbEnd       = *(Za + 2);
        *(ZbEnd - 1) = *(Zb + 1);
        *(ZbEnd - 2) = *Zb;
        
        // reset pointers
        za1 = ZaEnd - 3;
        zb1 = ZbEnd - 3;
    }
    
    *za1 = output;
    *zb1 = sample;
    
    return output;
}

void FourthOrderFilter::setLowPass(float fc){
    float gamma = tanf(M_PI * fc / SAMPLE_RATE_F);
    float gamma_2 = gamma*gamma;
    float beta_gamma_2 = BETA * gamma_2;
    float gamma_4 = gamma_2*gamma_2;
    float alpha_gamma = ALPHA * gamma;
    float alpha_gamma_3 = gamma * gamma_2 * ALPHA;
    float two_x_gamma_4 = 2.0f * gamma_4;
    float one_over_denominator = 1.0f / (gamma_4 + alpha_gamma_3 + beta_gamma_2 + alpha_gamma + 1.0f);
    
    b0 = gamma_4 * one_over_denominator;
    B[0] = 4.0f * b0;
    B[1] = 6.0f * b0;
    B[2] = B[0];
    B[3] = b0;
    
    A[0] = 2.0f * (two_x_gamma_4 + alpha_gamma_3 - alpha_gamma - 2.0f) * one_over_denominator;
    A[1] = 2.0f * (3.0f * gamma_4 - beta_gamma_2 + 3.0f) * one_over_denominator;
    A[2] = 2.0f * (two_x_gamma_4 - alpha_gamma_3 + alpha_gamma - 2.0f) * one_over_denominator;
    A[3] = (gamma_4 - alpha_gamma_3 + beta_gamma_2 - alpha_gamma + 1.0f) * one_over_denominator;
}