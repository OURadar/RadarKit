//
//  RKPreference.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 2/19/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_Preference__
#define __RadarKit_Preference__

#include <RadarKit/RKFoundation.h>

typedef struct rk_preference {
    char    waveform[RKNameLength];
    char    vcp[RKNameLength];
    double  latitude;
    double  longitude;
    double  radarHeight;
    double  meanSeaLevel;
} RKPreference;

RKPreference *RKPreferenceInit(void);
void RKPreferenceFree(RKPreference *);

#endif
