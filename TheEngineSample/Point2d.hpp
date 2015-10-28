//
//  Point2d.hpp
//  TheEngineSample
//
//  Created by Hans on 28/10/15.
//  Copyright © 2015 A Tasty Pixel. All rights reserved.
//

#ifndef Point2d_hpp
#define Point2d_hpp

#include <stdio.h>

typedef struct Point2d {
    Point2d(float x, float y){this
        ->x = x; this->y = y; this->mark = true;};
    Point2d(){x = 0.0f; y = 0.0f;};
    float distance(Point2d p);
    float dotProduct(Point2d p);
    float x,y;
    
    bool mark;
} Point2d;

#endif /* Point2d_hpp */
