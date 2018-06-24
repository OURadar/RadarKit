//
//  RKProduct.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 6/23/18.
//  Copyright © Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKProduct.h>

#pragma mark - Helper Functions

int RKProductInitFromSweep(RKProduct *product, const RKSweep *sweep) {
    int k;
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
    product->header.gateCount = sweep->header.gateSizeMeters;
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
    // Synchronize config id
    product->i = sweep->header.config.i;
    if (product->capacity < sweep->header.rayCount * sweep->header.gateCount) {
        RKLog("Allocating product space    %s   %s\n", RKVariableInString("i", &product->i, RKValueTypeIdentifier), RKVariableInString("capacity", &product->capacity, RKValueTypeUInt32));
        product->capacity = sweep->header.rayCount * sweep->header.gateCount;
        void *mem = realloc(product->data, product->capacity * sizeof(RKFloat));
        if (mem == NULL) {
            RKLog("Error. Unable to expand space.");
            exit(EXIT_FAILURE);
        }
        if (mem != product->data) {
            RKLog("Warning. Go read more about realloc().\n");
        }
    }
    for (k = 0; k < product->header.rayCount; k++) {
        product->startAzimuth[k]   = sweep->rays[k]->header.startAzimuth;
        product->endAzimuth[k]     = sweep->rays[k]->header.endAzimuth;
        product->startElevation[k] = sweep->rays[k]->header.startElevation;
        product->endElevation[k]   = sweep->rays[k]->header.endElevation;
    }
    if (!strcmp(product->desc.symbol, "Z")) {
        RKLog("Copying Z from swepp ...\n");
    } else if (!strcmp(product->desc.symbol, "V")) {
        RKLog("Copying %s from swepp ...\n", product->desc.symbol);
    } else if (!strcmp(product->desc.symbol, "W")) {
        RKLog("Copying %s from swepp ...\n", product->desc.symbol);
    } else if (!strcmp(product->desc.symbol, "D")) {
        RKLog("Copying %s from swepp ...\n", product->desc.symbol);
    } else if (!strcmp(product->desc.symbol, "P")) {
        RKLog("Copying %s from swepp ...\n", product->desc.symbol);
    } else if (!strcmp(product->desc.symbol, "R")) {
        RKLog("Copying %s from swepp ...\n", product->desc.symbol);
    } else if (!strcmp(product->desc.symbol, "K")) {
        RKLog("Copying %s from swepp ...\n", product->desc.symbol);
    } else if (!strcmp(product->desc.symbol, "Sh")) {
        RKLog("Copying %s from swepp ...\n", product->desc.symbol);
    }
    return RKResultSuccess;
}

#pragma mark - Delegate Workers

#pragma mark - Life Cycle

#pragma mark - Properties

#pragma mark - Interactions
