//
//  Point2d.cpp
//  TheEngineSample
//
//  Created by Hans on 28/10/15.
//  Copyright © 2015 A Tasty Pixel. All rights reserved.
//

#include "Point2d.hpp"
#include <math.h>

float Point2d::distance(Point2d p){
    float dx = p.x - x;
    float dy = p.y - y;
    return sqrtf(dx*dx + dy*dy);
}

float Point2d::dotProduct(Point2d p){
    return x*p.x + y*p.y;
}