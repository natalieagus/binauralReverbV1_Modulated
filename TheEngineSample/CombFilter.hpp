//
//  CombFilter.hpp
//  TheEngineSample
//
//  Created by Natalie Agus on 2/11/15.
//  Copyright © 2015 A Tasty Pixel. All rights reserved.
//

#ifndef CombFilter_hpp
#define CombFilter_hpp

#include <stdio.h>
class CombFilter{
public:
    CombFilter();
    float process(float input);
    void setHComb(float R, float td);
    
private:
    float gX, gY, x1, y1;
};
#endif /* CombFilter_hpp */
