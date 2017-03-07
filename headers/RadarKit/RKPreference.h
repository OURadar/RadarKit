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

#define RKPreferenceObjectCount   64

typedef struct rk_preference {
    char                  filename[RKMaximumStringLength];
    char                  waveform[RKNameLength];
    char                  vcp[RKNameLength];
    double                latitude;
    double                longitude;
    double                radarHeight;
    double                meanSeaLevel;
    RKPreferenceObject    objects[RKPreferenceObjectCount];
    uint32_t              memoryUsage;
} RKPreference;

RKPreference *RKPreferenceInitWithFile(const char *filename);
RKPreference *RKPreferenceInit(void);
void RKPreferenceFree(RKPreference *);

int RKPreferenceUpdate(RKPreference *preference);
RKPreferenceObject *RKPreferenceFindKeyword(RKPreference *preference, const char *keyword);

#endif
