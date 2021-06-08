//
//  RKCalibrator.c
//  RadarKit
//
//  Created by Boonleng Cheong on 4/24/2021.
//  Copyright (c) 2021 Boonleng Cheong. All rights reserved.
//

#include <RadarKit/RKCalibrator.h>

void RKCalibratorSimple(RKScratch *space, RKConfig *config) {
    int i, k, p;
    RKFloat r = 0.0f;
    RKFilterAnchor *filterAnchors = config->waveformDecimate->filterAnchors[0];
    for (k = 0; k < config->waveformDecimate->count; k++) {
        //RKLog("RKCalibratorSimple: k=%d: %d ... %d\n", k, filterAnchors[k].outputOrigin, MIN(filterAnchors[k].outputOrigin + filterAnchors[k].maxDataLength, space->gateCount));
        for (i = filterAnchors[k].inputOrigin; i < MIN(filterAnchors[k].outputOrigin + filterAnchors[k].maxDataLength, space->gateCount); i++) {
            r = (RKFloat)i * space->gateSizeMeters;
            for (p = 0; p < 2; p++) {
                space->S2Z[p][i] = 20.0f * log10f(r) + config->systemZCal[p] + config->ZCal[k][p] - filterAnchors[k].sensitivityGain - space->samplingAdjustment;
            }
            space->dcal[i] = config->systemDCal + config->DCal[k];
            space->pcal[i] = RKSingleWrapTo2PI(config->systemPCal + config->PCal[k]);
        }
    }
}
