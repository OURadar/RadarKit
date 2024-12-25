//
//  RKProduct.c
//  RadarKit
//
//  Created by Boonleng Cheong on 6/23/18.
//  Copyright © Boonleng Cheong. All rights reserved.
//

#include <RadarKit/RKProduct.h>

static size_t _rk_malloc_product(RKProduct *product, const uint32_t rayCount, const uint32_t gateCount) {
    uint32_t arraySize = gateCount * rayCount * sizeof(RKFloat);
    uint32_t halfSize = rayCount * sizeof(RKFloat);
    uint32_t fullSize = rayCount * sizeof(double);
    product->header.rayCount = rayCount;
    product->header.gateCount = gateCount;
    product->capacity = rayCount * gateCount;
    product->startAzimuth = (RKFloat *)RKMalloc(halfSize);
    product->endAzimuth = (RKFloat *)RKMalloc(halfSize);
    product->startElevation = (RKFloat *)RKMalloc(halfSize);
    product->endElevation = (RKFloat *)RKMalloc(halfSize);
    product->startTime = (double *)RKMalloc(fullSize);
    product->endTime = (double *)RKMalloc(fullSize);
    product->data = (RKFloat *)RKMalloc(arraySize);
    memset(product->startAzimuth, 0, halfSize);
    memset(product->endAzimuth, 0, halfSize);
    memset(product->startElevation, 0, halfSize);
    memset(product->endElevation, 0, halfSize);
    memset(product->startTime, 0, fullSize);
    memset(product->endTime, 0, fullSize);
    memset(product->data, 0, arraySize);
    product->totalBufferSize = (uint32_t)sizeof(RKProduct) + 4 * halfSize + 2 * fullSize + arraySize;
    return product->totalBufferSize;
}

size_t RKProductBufferAlloc(RKProduct **buffer, const uint32_t depth, const uint32_t rayCount, const uint32_t gateCount) {
    int i;

    RKProduct *products = (RKProduct *)malloc(depth * sizeof(RKProduct));
    if (products == NULL) {
        RKLog("Error. Unable to allocate product buffer.\n");
        return 0;
    }
    memset(products, 0, depth * sizeof(RKProduct));

    size_t size = 0;
    for (i = 0; i < depth; i++) {
        RKProduct *product = &products[i];
        // printf("New RKProduct @ %p\n", product);
        size += _rk_malloc_product(product, rayCount, gateCount);
    }
    *buffer = products;
    return size;
}

size_t RKProductBufferExtend(RKProduct **buffer, const uint32_t depth, const uint32_t extension) {
    int i;

    if (buffer == NULL) {
        RKLog("Error. Null buffer.\n");
        return 0;
    }
    RKProduct *products = (RKProduct *)realloc(*buffer, (depth + extension) * sizeof(RKProduct));
    if (products == NULL) {
        RKLog("Error. Unable to reallocate product buffer.\n");
        return 0;
    }
    for (i = 0; i < depth; i++) {
        RKProduct *newProduct = &products[i];
        RKProduct *oldProduct = &(*buffer)[i];
        if (newProduct != oldProduct) {
            memcpy(newProduct, oldProduct, sizeof(RKProduct));
        }
    }

    const uint32_t rayCount = products->header.rayCount;
    const uint32_t gateCount = products->header.gateCount;

    size_t size = 0;
    for (i = depth; i < depth + extension; i++) {
        RKProduct *product = &products[i];
        size += _rk_malloc_product(product, rayCount, gateCount);
    }
    *buffer = products;
    return size;
}

void RKProductBufferFree(RKProduct *buffer, const uint32_t depth) {
    int i;
    for (i = 0; i < depth; i++) {
        RKProduct *product = &buffer[i];
        free(product->startAzimuth);
        free(product->endAzimuth);
        free(product->startElevation);
        free(product->endElevation);
        free(product->startTime);
        free(product->endTime);
        free(product->data);
    }
    free(buffer);
}

int RKProductInitFromSweep(RKProduct *product, const RKSweep *sweep) {
    int k;

    // Required capacity, round to the next 90 ray count, next 100 gate count
    const uint32_t requiredCapacity = (uint32_t)ceilf(sweep->header.rayCount / 90.0f) * 90 * (uint32_t)ceilf(sweep->header.gateCount / 100.0f) * 100;

    // Sweep header
    memcpy(product->header.radarName, sweep->header.desc.name, sizeof(RKName));
    product->header.latitude = sweep->header.desc.latitude;
    product->header.longitude = sweep->header.desc.longitude;
    product->header.heading = sweep->header.desc.heading;
    product->header.radarHeight = sweep->header.desc.radarHeight;
    product->header.wavelength = sweep->header.desc.wavelength;
    product->header.volumeIndex = sweep->header.config.volumeIndex;
    product->header.sweepIndex = sweep->header.config.sweepIndex;
    product->header.sweepElevation = sweep->header.config.sweepElevation;
    product->header.sweepAzimuth = sweep->header.config.sweepAzimuth;
    product->header.startTime = sweep->header.startTime;
    product->header.endTime = sweep->header.endTime;
    product->header.isPPI = sweep->header.isPPI;
    product->header.isRHI = sweep->header.isRHI;
    product->header.rayCount = sweep->header.rayCount;
    product->header.gateCount = sweep->header.gateCount;
    product->header.gateSizeMeters = sweep->header.gateSizeMeters;

    // Sweep header config
    for (k = 0; k < RKMaximumFilterCount; k++) {
        product->header.pw[k] = sweep->header.config.pw[k];
        product->header.prt[k] = sweep->header.config.prt[k];
    }
    product->header.noise[0] = sweep->header.config.noise[0];
    product->header.noise[1] = sweep->header.config.noise[1];
    product->header.systemZCal[0] = sweep->header.config.systemZCal[0];
    product->header.systemZCal[1] = sweep->header.config.systemZCal[1];
    product->header.systemDCal = sweep->header.config.systemDCal;
    product->header.systemPCal = sweep->header.config.systemPCal;
    for (k = 0; k < RKMaximumFilterCount; k++) {
        product->header.ZCal[k][0] = sweep->header.config.ZCal[k][0];
        product->header.ZCal[k][1] = sweep->header.config.ZCal[k][1];
        product->header.DCal[k] = sweep->header.config.DCal[k];
        product->header.PCal[k] = sweep->header.config.PCal[k];
    }
    product->header.SNRThreshold = sweep->header.config.SNRThreshold;
    product->header.SQIThreshold = sweep->header.config.SQIThreshold;
    memcpy(product->header.waveformName, sweep->header.config.waveformName, sizeof(RKName));
    switch (sweep->header.config.momentMethod) {
        case RKMomentMethodPulsePair:
            sprintf(product->header.momentMethod, "pulse_pair");
            break;
        case RKMomentMethodPulsePairHop:
            sprintf(product->header.momentMethod, "pulse_pair_hop");
            break;
        case RKMomentMethodPulsePairATSR:
            sprintf(product->header.momentMethod, "pulse_pair_atsr");
            break;
        case RKMomentMethodMultiLag2:
            sprintf(product->header.momentMethod, "multilag_2");
            break;
        case RKMomentMethodMultiLag3:
            sprintf(product->header.momentMethod, "multilag_3");
            break;
        case RKMomentMethodMultiLag4:
            sprintf(product->header.momentMethod, "multilag_4");
            break;
        case RKMomentMethodSpectralMoment:
            sprintf(product->header.momentMethod, "spectral_moment");
            break;
        default:
            sprintf(product->header.momentMethod, "user_defined");
            break;
    }
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
        product->startTime[k]      = sweep->rays[k]->header.startTimeDouble;
        product->endTime[k]        = sweep->rays[k]->header.endTimeDouble;
    }
    if (product->desc.index < RKProductIndexCount) {
        RKFloat *x, *y = product->data;
        for (k = 0; k < product->header.rayCount; k++) {
            if (sweep->rays[k] == NULL) {
                RKLog("Error. Null ray.\n");
                continue;
            }
            x = RKGetFloatDataFromRay(sweep->rays[k], product->desc.index);
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
