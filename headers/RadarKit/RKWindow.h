//
//  RKWindow.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/18/15.
//  Copyright (c) 2015 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_Window__
#define __RadarKit_Window__

#include <RadarKit/RKFoundation.h>

//#ifdef __cplusplus
//extern "C" {
//#endif

typedef uint8_t RKWindowType;
enum RKWindowType{
    RKWindowTypeBoxCar,
    RKWindowTypeHann,
    RKWindowTypeHamming,
    RKWindowTypeKaiser,
    RKWindowTypeTukey
};

void RKWindowMake(RKFloat *buffer, RKWindowType type, const int length, ...);

// hamming
// hanning
// blackman
// chebyshev

//#ifdef __cplusplus
//}
//#endif

#endif /* defined(__RadarKit_Window__) */
