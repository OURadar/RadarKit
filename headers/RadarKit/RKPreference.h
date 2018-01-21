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

#define RKPreferenceObjectCount   128

typedef struct rk_preference {
    RKPreferenceObject    objects[RKPreferenceObjectCount];
    uint32_t              count;
    uint32_t              memoryUsage;

    // Internal variables
    uint32_t              previousIndex;
    char                  previousKeyword[RKNameLength];
    
    // These may not be committed
    char                  filename[RKMaximumStringLength];
    char                  waveform[RKNameLength];
    char                  vcp[RKNameLength];
    double                latitude;
    double                longitude;
    double                radarHeight;
    double                meanSeaLevel;
} RKPreference;

RKPreference *RKPreferenceInitWithFile(const char *filename);
RKPreference *RKPreferenceInit(void);
void RKPreferenceFree(RKPreference *);

int RKPreferenceUpdate(RKPreference *preference);
RKPreferenceObject *RKPreferenceFindKeyword(RKPreference *preference, const char *keyword);
int RKPreferenceGetKeywordCount(RKPreference *preference, const char *keyword);

#endif
