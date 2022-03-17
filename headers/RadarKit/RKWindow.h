//
//  RKWindow.h
//  RadarKit
//
//  Created by Boonleng Cheong on 3/18/15.
//  Copyright (c) 2015 Boonleng Cheong. All rights reserved.
//

#ifndef __RadarKit_Window__
#define __RadarKit_Window__

#include <RadarKit/RKFoundation.h>

typedef int RKWindowType;
enum {
    RKWindowTypeBoxCar,
    RKWindowTypeHann,
    RKWindowTypeHamming,
    RKWindowTypeKaiser,
    RKWindowTypeTukey,
    RKWindowTypeTrapezoid
};

void RKWindowMake(RKFloat *buffer, RKWindowType type, const int length, ...);

#endif /* defined(__RadarKit_Window__) */
