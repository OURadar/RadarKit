//
//  RKProduct.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 6/23/18.
//  Copyright © Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKProduct.h>

int RKProductInitFromSweep(RKProduct *product, const RKSweep *sweep) {
    int k;
    uint32_t requiredCapacity = sweep->header.rayCount * sweep->header.gateCount;
    // Sweep header
    memcpy(product->header.radarName, sweep->header.desc.name, sizeof(RKName));
    product->header.latitude = sweep->header.desc.latitude;
    product->header.longitude = sweep->header.desc.longitude;
    product->header.heading = sweep->header.desc.heading;
    product->header.radarHeight = sweep->header.desc.radarHeight;
    product->header.wavelength = sweep->header.desc.wavelength;
    product->header.sweepElevation = sweep->header.config.sweepElevation;
    product->header.sweepAzimuth = sweep->header.config.sweepAzimuth;
    product->header.startTime = sweep->header.startTime;
    product->header.endTime = sweep->header.endTime;
    product->header.isPPI = sweep->header.isPPI;
    product->header.isRHI = sweep->header.isRHI;
    product->header.filterCount = sweep->header.config.filterCount;
    product->header.rayCount = sweep->header.rayCount;
    product->header.gateCount = sweep->header.gateCount;
    product->header.gateSizeMeters = sweep->header.gateSizeMeters;
    // Sweep header config
    for (k = 0; k < product->header.filterCount; k++) {
        memcpy(&product->header.filterAnchors[k], &sweep->header.config.filterAnchors[k], sizeof(RKFilterAnchor));
        product->header.pw[k] = sweep->header.config.pw[k];
        product->header.prf[k] = sweep->header.config.prf[k];
    }
    product->header.noise[0] = sweep->header.config.noise[0];
    product->header.noise[1] = sweep->header.config.noise[1];
    product->header.systemZCal[0] = sweep->header.config.systemZCal[0];
    product->header.systemZCal[1] = sweep->header.config.systemZCal[1];
    product->header.systemDCal = sweep->header.config.systemDCal;
    product->header.systemPCal = sweep->header.config.systemPCal;
    for (k = 0; k < product->header.filterCount; k++) {
        product->header.ZCal[k][0] = sweep->header.config.ZCal[k][0];
        product->header.ZCal[k][1] = sweep->header.config.ZCal[k][1];
        product->header.DCal[k] = sweep->header.config.DCal[k];
        product->header.PCal[k] = sweep->header.config.PCal[k];
    }
    product->header.SNRThreshold = sweep->header.config.SNRThreshold;
    memcpy(product->header.waveform, sweep->header.config.waveform, sizeof(RKName));
    memcpy(product->header.vcpDefinition, sweep->header.config.vcpDefinition, sizeof(RKMaximumCommandLength));

    if (product->capacity < requiredCapacity) {
        RKFloat *mem = realloc(product->data, product->capacity * sizeof(RKFloat));
        if (mem == NULL) {
            RKLog("Error. Unable to expand space.");
            exit(EXIT_FAILURE);
        }
        if (mem != product->data) {
            RKLog("Info. Product space reallocated   %p -> %p\n", product->data, mem);
            if (product->data) {
                free(product->data);
            }
            product->data = mem;
            product->capacity = requiredCapacity;
        }
    }
    for (k = 0; k < product->header.rayCount; k++) {
        product->startAzimuth[k]   = sweep->rays[k]->header.startAzimuth;
        product->endAzimuth[k]     = sweep->rays[k]->header.endAzimuth;
        product->startElevation[k] = sweep->rays[k]->header.startElevation;
        product->endElevation[k]   = sweep->rays[k]->header.endElevation;
    }
    RKBaseMomentIndex momentIndex = RKBaseMomentIndexCount;
    if (!strcmp(product->desc.symbol, "Z")) {
        momentIndex = RKBaseMomentIndexZ;
    } else if (!strcmp(product->desc.symbol, "V")) {
        momentIndex = RKBaseMomentIndexV;
    } else if (!strcmp(product->desc.symbol, "W")) {
        momentIndex = RKBaseMomentIndexW;
    } else if (!strcmp(product->desc.symbol, "D")) {
        momentIndex = RKBaseMomentIndexD;
    } else if (!strcmp(product->desc.symbol, "P")) {
        momentIndex = RKBaseMomentIndexP;
    } else if (!strcmp(product->desc.symbol, "R")) {
        momentIndex = RKBaseMomentIndexR;
    } else if (!strcmp(product->desc.symbol, "K")) {
        momentIndex = RKBaseMomentIndexK;
    } else if (!strcmp(product->desc.symbol, "Sh")) {
        momentIndex = RKBaseMomentIndexSh;
    }
    if (momentIndex < RKBaseMomentIndexCount) {
        RKFloat *x, *y = product->data;
        for (k = 0; k < product->header.rayCount; k++) {
            if (sweep->rays[k] == NULL) {
                RKLog("Error. Null ray.\n");
                continue;
            }
            x = RKGetFloatDataFromRay(sweep->rays[k], momentIndex);
            if (x == NULL) {
                continue;
            }
            memcpy(y, x, product->header.gateCount * sizeof(RKFloat));
            y += product->header.gateCount;
        }
        // Synchronize config id
        product->i = sweep->header.config.i;
    }

    return RKResultSuccess;
}

RKProduct *RKProductInitFromFile(const char *filename) {
    RKProduct *product = NULL;
    return product;
}
