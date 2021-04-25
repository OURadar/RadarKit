//
//  RKProduct.c
//  RadarKit
//
//  Created by Boonleng Cheong on 6/23/18.
//  Copyright © Boonleng Cheong. All rights reserved.
//

#include <RadarKit/RKProduct.h>

size_t RKProductBufferAlloc(RKProduct **buffer, const uint32_t depth, const uint32_t rayCount, const uint32_t gateCount) {
    int i;
    
    RKProduct *products = (RKProduct *)malloc(depth * sizeof(RKProduct));
    if (products == NULL) {
        RKLog("Error. Unable to allocate product buffer.\n");
        return 0;
    }
    memset(products, 0, depth * sizeof(RKProduct));
    
    size_t size = 0;
    uint32_t capacity = gateCount * rayCount;
    uint32_t headSize = rayCount * sizeof(RKFloat);
    uint32_t dataSize = capacity * sizeof(RKFloat);
    
    for (i = 0; i < depth; i++) {
        RKProduct *product = &products[i];
        product->header.rayCount = rayCount;
        product->header.gateCount = gateCount;
        product->startAzimuth = (RKFloat *)malloc(headSize);
        product->endAzimuth = (RKFloat *)malloc(headSize);
        product->startElevation = (RKFloat *)malloc(headSize);
        product->endElevation = (RKFloat *)malloc(headSize);
        product->data = (RKFloat *)malloc(dataSize);
        product->capacity = capacity;
        memset(product->startAzimuth, 0, headSize);
        memset(product->endAzimuth, 0, headSize);
        memset(product->startElevation, 0, headSize);
        memset(product->endElevation, 0, headSize);
        memset(product->data, 0, dataSize);
        product->totalBufferSize = (uint32_t)sizeof(RKProduct) + 4 * headSize + dataSize;
        size += product->totalBufferSize;
    }
    *buffer = products;
    return size;
}

void RKProductBufferFree(RKProduct *buffer, const int depth) {
    int i;
    for (i = 0; i < depth; i++) {
        RKProduct *product = &buffer[i];
        free(product->startAzimuth);
        free(product->endAzimuth);
        free(product->startElevation);
        free(product->endElevation);
        free(product->data);
    }
    free(buffer);
}

int RKProductInitFromSweep(RKProduct *product, const RKSweep *sweep) {
    int k;
    
    // Required capacity
    const uint32_t requiredCapacity = (uint32_t)ceilf(sweep->header.rayCount / 90.0f) * 90 * (uint32_t)ceilf(sweep->header.gateCount / 100.0f) * 100;

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
        product->header.prt[k] = sweep->header.config.prt[k];
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
    memcpy(product->header.waveformName, sweep->header.config.waveformName, sizeof(RKName));
    memcpy(product->header.vcpDefinition, sweep->header.config.vcpDefinition, sizeof(RKMaximumCommandLength));
    
    // Expand if the current capacity is not sufficient
    if (product->capacity < requiredCapacity) {
        product->data = (RKFloat *)realloc(product->data, requiredCapacity * sizeof(RKFloat));
        if (product->data == NULL) {
            RKLog("Error. Unable to expand space.\n");
            exit(EXIT_FAILURE);
        }
        product->capacity = requiredCapacity;
        product->totalBufferSize = sizeof(RKProduct) + (4 * RKMaximumRaysPerSweep + product->capacity) * sizeof(RKFloat);
    }
    for (k = 0; k < product->header.rayCount; k++) {
        product->startAzimuth[k]   = sweep->rays[k]->header.startAzimuth;
        product->endAzimuth[k]     = sweep->rays[k]->header.endAzimuth;
        product->startElevation[k] = sweep->rays[k]->header.startElevation;
        product->endElevation[k]   = sweep->rays[k]->header.endElevation;
    }
    
    // Copy over the data if this is one of the base moments
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
    }

    return RKResultSuccess;
}

void RKProductFree(RKProduct *product) {
    free(product);
}
