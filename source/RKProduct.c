//
//  RKProduct.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 6/23/18.
//  Copyright © Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKProduct.h>

#pragma mark - Helper Functions

int RKProductSetMetaDataFromSweep(RKProduct *product, const RKSweep *sweep) {
    int k;
    memcpy(product->header.name, sweep->header.desc.name, sizeof(RKName));
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
    return RKResultSuccess;
}

#pragma mark - Delegate Workers

#pragma mark - Life Cycle

#pragma mark - Properties

#pragma mark - Interactions
