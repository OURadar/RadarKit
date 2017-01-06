//
//  RKDSP.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/18/15.
//  Copyright (c) 2015 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_RKDSP__
#define __RadarKit_RKDSP__

#include <RadarKit/RKFoundation.h>

//#ifdef __cplusplus
//extern "C" {
//#endif

float RKGetSignedMinorSectorInDegrees(const float angle1, const float angle2);
float RKGetMinorSectorInDegrees(const float angle1, const float angle2);
float RKInterpolateAngles(const float angleLeft, const float angleRight, const float alpha);

//
// FIR + IIR Filters
//

// xcorr() ?
// ambiguity function
//


//#ifdef __cplusplus
//}
//#endif

#endif /* defined(__RadarKit_RKDSP__) */
