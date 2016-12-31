//
//  RKDSP.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/18/15.
//  Copyright (c) 2015 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKDSP.h>

float RKGetSignedMinorSectorInDegrees(const float angle1, const float angle2) {
    float delta = angle1 - angle2;
    if (delta > 180.0f) {
        delta -= 360.0f;
    } else if (delta < -180.0f) {
        delta += 360.0f;
    }
    return delta;
}

float RKGetMinorSectorInDegrees(const float angle1, const float angle2) {
    return fabs(RKGetSignedMinorSectorInDegrees(angle1, angle2));
}
