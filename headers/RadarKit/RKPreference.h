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

enum RKParameterType {
    RKParameterTypeInt,
    RKParameterTypeUInt,
    RKParameterTypeBool,
    RKParameterTypeFloat,
    RKParameterTypeDouble,
    RKParameterTypeString,
    RKParameterTypeWaveformCalibration
};

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

int RKPreferenceUpdate(RKPreference *);
RKPreferenceObject *RKPreferenceFindKeyword(RKPreference *, const char *keyword);
int RKPreferenceGetKeywordCount(RKPreference *, const char *keyword);
void RKPreferenceGetValueOfKeyword(RKPreference *, const int verb, const char *keyword, void *value, const int type, const int count);

#endif
