//
//  Delays.hpp
//  TheEngineSample
//
//  Created by Natalie Agus on 13/11/15.
//  Copyright © 2015 A Tasty Pixel. All rights reserved.
//

#ifndef Delays_hpp
#define Delays_hpp

#include <stdio.h>
typedef struct Delays {
    Delays(){
        delay = 0.0f;
        index = 0;
    };
    Delays(float delay, unsigned idx){
        this->delay = delay;
        this->index = idx;
    }
    float delay;
    size_t index;
    
} Delays;


#endif /* Delays_hpp */
