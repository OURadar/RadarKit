//
//  RKMoment.c
//  RadarKit
//
//  Created by Boonleng Cheong on 5/7/2021.
//  Copyright (c) Boonleng Cheong. All rights reserved.
//

#include <RadarKit/RKScratch.h>

#pragma mark - Helper Functions

#pragma mark - Scratch Space

// Allocate a scratch space for pulse compression
size_t RKCompressionScratchAlloc(RKCompressionScratch **buffer, const uint32_t capacity, const uint8_t verbose) {
    if (capacity == 0 || capacity != (capacity * sizeof(RKFloat) / RKMemoryAlignSize) * RKMemoryAlignSize / sizeof(RKFloat)) {
        RKLog("Error. Scratch space capacity must be greater than 0 and an integer multiple of %s!",
              RKIntegerToCommaStyleString(RKMemoryAlignSize / sizeof(RKFloat)));
        return 0;
    }
    *buffer = (RKCompressionScratch *)malloc(sizeof(RKCompressionScratch));
    if (*buffer == NULL) {
        RKLog("Error. Unable to allocate a compression scratch space.\n");
        return 0;
    }
    memset(*buffer, 0, sizeof(RKCompressionScratch));

    const uint32_t nfft = 1 << (int)ceilf(log2f((float)capacity));

    RKCompressionScratch *scratch = (RKCompressionScratch *)*buffer;
    scratch->capacity = capacity;
    scratch->verbose = verbose;

    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&scratch->inBuffer, RKMemoryAlignSize, nfft * sizeof(fftwf_complex)))
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&scratch->outBuffer, RKMemoryAlignSize, nfft * sizeof(fftwf_complex)))
    if (scratch->inBuffer == NULL || scratch->outBuffer == NULL) {
        RKLog("Error. Unable to allocate resources for FFTW.\n");
        return 0;
    }
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&scratch->zi, RKMemoryAlignSize, nfft * sizeof(RKFloat)))
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&scratch->zo, RKMemoryAlignSize, nfft * sizeof(RKFloat)))
    if (scratch->zi == NULL || scratch->zo == NULL) {
        RKLog("Error. Unable to allocate resources for FFTW.\n");
        return 0;
    }
    size_t mem = 2 * nfft * sizeof(fftwf_complex) + 2 * nfft * sizeof(RKFloat);
    return mem;
}

void RKCompressionScratchFree(RKCompressionScratch *scratch) {
    free(scratch->inBuffer);
    free(scratch->outBuffer);
    free(scratch->zi);
    free(scratch->zo);
    free(scratch);
}

// Allocate a scratch space for moment processors
size_t RKMomentScratchAlloc(RKMomentScratch **buffer, const uint32_t capacity, const uint8_t verbose) {
    if (capacity == 0 || capacity != (capacity * sizeof(RKFloat) / RKMemoryAlignSize) * RKMemoryAlignSize / sizeof(RKFloat)) {
        RKLog("Error. Scratch space capacity must be greater than 0 and an integer multiple of %s!",
              RKIntegerToCommaStyleString(RKMemoryAlignSize / sizeof(RKFloat)));
        return 0;
    }
    *buffer = malloc(sizeof(RKMomentScratch));
    if (*buffer == NULL) {
        RKLog("Error. Unable to allocate a momment scratch space.\n");
        return 0;
    }
    memset(*buffer, 0, sizeof(RKMomentScratch));

    const uint32_t nfft = 1 << (int)ceilf(log2f((float)RKMaximumPulsesPerRay));

    RKMomentScratch *space = *buffer;
    space->capacity = capacity;
    space->verbose = verbose;

    if (space->verbose) {
        RKLog("Info. %s <-- %s",
              RKVariableInString("space->capacity", &space->capacity, RKValueTypeUInt32),
              RKVariableInString("capacity", &capacity, RKValueTypeUInt32));
    }

    int j, k;
    size_t bytes = sizeof(RKMomentScratch);

    int s = 0;
    for (k = 0; k < 2; k++) {
        POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->mX[k].i, RKMemoryAlignSize, space->capacity * sizeof(RKFloat)));
        POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->mX[k].q, RKMemoryAlignSize, space->capacity * sizeof(RKFloat)));
        POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->vX[k].i, RKMemoryAlignSize, space->capacity * sizeof(RKFloat)));
        POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->vX[k].q, RKMemoryAlignSize, space->capacity * sizeof(RKFloat)));
        POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->S[k], RKMemoryAlignSize, space->capacity * sizeof(RKFloat)));
        POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->Z[k], RKMemoryAlignSize, space->capacity * sizeof(RKFloat)));
        POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->V[k], RKMemoryAlignSize, space->capacity * sizeof(RKFloat)));
        POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->W[k], RKMemoryAlignSize, space->capacity * sizeof(RKFloat)));
        POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->Q[k], RKMemoryAlignSize, space->capacity * sizeof(RKFloat)));
        POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->SNR[k], RKMemoryAlignSize, space->capacity * sizeof(RKFloat)));
        POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->S2Z[k], RKMemoryAlignSize, space->capacity * sizeof(RKFloat)));
        memset(space->S2Z[k], 0, space->capacity * sizeof(RKFloat));
        s += 11;
        for (j = 0; j < RKMaximumLagCount; j++) {
            POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->R[k][j].i, RKMemoryAlignSize, space->capacity * sizeof(RKFloat)));
            POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->R[k][j].q, RKMemoryAlignSize, space->capacity * sizeof(RKFloat)));
            POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->aR[k][j], RKMemoryAlignSize, space->capacity * sizeof(RKFloat)));
            s += 3;
        }
    }
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->sC.i, RKMemoryAlignSize, space->capacity * sizeof(RKFloat)));
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->sC.q, RKMemoryAlignSize, space->capacity * sizeof(RKFloat)));
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->ts.i, RKMemoryAlignSize, space->capacity * sizeof(RKFloat)));
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->ts.q, RKMemoryAlignSize, space->capacity * sizeof(RKFloat)));
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->ZDR, RKMemoryAlignSize, space->capacity * sizeof(RKFloat)));
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->PhiDP, RKMemoryAlignSize, space->capacity * sizeof(RKFloat)));
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->RhoHV, RKMemoryAlignSize, space->capacity * sizeof(RKFloat)));
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->KDP, RKMemoryAlignSize, space->capacity * sizeof(RKFloat)));
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->userArray1, RKMemoryAlignSize, space->capacity * sizeof(RKFloat)));
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->userArray2, RKMemoryAlignSize, space->capacity * sizeof(RKFloat)));
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->userArray3, RKMemoryAlignSize, space->capacity * sizeof(RKFloat)));
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->userArray4, RKMemoryAlignSize, space->capacity * sizeof(RKFloat)));
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->dcal, RKMemoryAlignSize, space->capacity * sizeof(RKFloat)));
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->pcal, RKMemoryAlignSize, space->capacity * sizeof(RKFloat)));
    memset(space->dcal, 0, space->capacity * sizeof(RKFloat));
    memset(space->pcal, 0, space->capacity * sizeof(RKFloat));
    s += 14;

    for (j = 0; j < 2 * RKMaximumLagCount - 1; j++) {
        POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->C[j].i, RKMemoryAlignSize, space->capacity * sizeof(RKFloat)));
        POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->C[j].q, RKMemoryAlignSize, space->capacity * sizeof(RKFloat)));
        POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->aC[j], RKMemoryAlignSize, space->capacity * sizeof(RKFloat)));
        s += 3;
    }
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->gC, RKMemoryAlignSize, space->capacity * sizeof(RKFloat)));
    s++;
    bytes += s * sizeof(RKFloat);

    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->mask, RKMemoryAlignSize, space->capacity * sizeof(uint8_t)));
    bytes += space->capacity * sizeof(uint8_t);

    space->inBuffer = (fftwf_complex **)malloc(space->capacity * sizeof(fftwf_complex *));
    space->outBuffer = (fftwf_complex **)malloc(space->capacity * sizeof(fftwf_complex *));
    bytes += 2 * space->capacity * sizeof(fftwf_complex *);
    for (j = 0; j < space->capacity; j++) {
        POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->inBuffer[j], RKMemoryAlignSize, nfft * sizeof(fftwf_complex)));
        POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->outBuffer[j], RKMemoryAlignSize, nfft * sizeof(fftwf_complex)));
    }
    bytes += space->capacity * 2 * nfft * sizeof(fftwf_complex);
    return bytes;
}

void RKMomentScratchFree(RKMomentScratch *space) {
    int j, k;
    for (k = 0; k < 2; k++) {
        free(space->mX[k].i);
        free(space->mX[k].q);
        free(space->vX[k].i);
        free(space->vX[k].q);
        free(space->S[k]);
        free(space->Z[k]);
        free(space->V[k]);
        free(space->W[k]);
        free(space->Q[k]);
        free(space->SNR[k]);
        free(space->S2Z[k]);
        for (j = 0; j < RKMaximumLagCount; j++) {
            free(space->R[k][j].i);
            free(space->R[k][j].q);
            free(space->aR[k][j]);
        }
    }
    free(space->sC.i);
    free(space->sC.q);
    free(space->ts.i);
    free(space->ts.q);
    free(space->ZDR);
    free(space->PhiDP);
    free(space->RhoHV);
    free(space->KDP);
    free(space->userArray1);
    free(space->userArray2);
    free(space->userArray3);
    free(space->userArray4);
    free(space->dcal);
    free(space->pcal);
    for (j = 0; j < 2 * RKMaximumLagCount - 1; j++) {
        free(space->C[j].i);
        free(space->C[j].q);
        free(space->aC[j]);
    }
    free(space->gC);
    free(space->mask);
    for (k = 0; k < space->capacity; k++) {
        free(space->inBuffer[k]);
        free(space->outBuffer[k]);
    }
    free(space->inBuffer);
    free(space->outBuffer);
    free(space);
}

#pragma mark -

int prepareScratch(RKMomentScratch *space) {
    space->fftOrder = -1;
    return 0;
}

// This function converts the float data calculated from a chosen processor to uint8_t type, which also represent
// the display data for the front end
int makeRayFromScratch(RKMomentScratch *space, RKRay *ray) {
    int k;
    uint8_t *mask;
    float SNRh, SNRv;
    float SNRThreshold, SQIThreshold;

    // Grab the relevant data from scratch space for float to 16-bit quantization
    RKFloat *iHmi  = space->mX[0].i;      int16_t *oHmi  = RKGetInt16DataFromRay(ray, RKMomentIndexHmi);
    RKFloat *iHmq  = space->mX[0].q;      int16_t *oHmq  = RKGetInt16DataFromRay(ray, RKMomentIndexHmq);
    RKFloat *iHR0  = space->aR[0][0];     int16_t *oHR0  = RKGetInt16DataFromRay(ray, RKMomentIndexHR0);
    RKFloat *iHR1i = space->R[0][1].i;    int16_t *oHR1i = RKGetInt16DataFromRay(ray, RKMomentIndexHR1i);
    RKFloat *iHR1q = space->R[0][1].q;    int16_t *oHR1q = RKGetInt16DataFromRay(ray, RKMomentIndexHR1q);
    RKFloat *iHR2  = space->aR[0][2];     int16_t *oHR2  = RKGetInt16DataFromRay(ray, RKMomentIndexHR2);
    RKFloat *iHR3  = space->aR[0][3];     int16_t *oHR3  = RKGetInt16DataFromRay(ray, RKMomentIndexHR3);
    RKFloat *iHR4  = space->aR[0][3];     int16_t *oHR4  = RKGetInt16DataFromRay(ray, RKMomentIndexHR4);
    RKFloat *iVmi  = space->mX[1].i;      int16_t *oVmi  = RKGetInt16DataFromRay(ray, RKMomentIndexVmi);
    RKFloat *iVmq  = space->mX[1].q;      int16_t *oVmq  = RKGetInt16DataFromRay(ray, RKMomentIndexVmq);
    RKFloat *iVR0  = space->aR[1][0];     int16_t *oVR0  = RKGetInt16DataFromRay(ray, RKMomentIndexVR0);
    RKFloat *iVR1i = space->R[1][1].i;    int16_t *oVR1i = RKGetInt16DataFromRay(ray, RKMomentIndexVR1i);
    RKFloat *iVR1q = space->R[1][1].q;    int16_t *oVR1q = RKGetInt16DataFromRay(ray, RKMomentIndexVR1q);
    RKFloat *iVR2  = space->aR[1][2];     int16_t *oVR2  = RKGetInt16DataFromRay(ray, RKMomentIndexVR2);
    RKFloat *iVR3  = space->aR[1][3];     int16_t *oVR3  = RKGetInt16DataFromRay(ray, RKMomentIndexVR3);
    RKFloat *iVR4  = space->aR[1][3];     int16_t *oVR4  = RKGetInt16DataFromRay(ray, RKMomentIndexVR4);

    // Convert float to 16-bit representation (-32768 - 32767) using ...
    for (k = 0; k < MIN(space->capacity, space->gateCount); k++) {
        *oHmi++  = (int16_t)(*iHmi++);
        *oHmq++  = (int16_t)(*iHmq++);
        *oHR0++  = (int16_t)1000.0f * log2f(*iHR0++);
        *oHR1i++ = (int16_t)1000.0f * log2f(*iHR1i++);
        *oHR1q++ = (int16_t)1000.0f * log2f(*iHR1q++);
        *oHR2++  = (int16_t)1000.0f * log2f(*iHR2++);
        *oHR3++  = (int16_t)1000.0f * log2f(*iHR3++);
        *oHR4++  = (int16_t)1000.0f * log2f(*iHR4++);
        *oVmi++  = (int16_t)(*iVmi++);
        *oVmq++  = (int16_t)(*iVmq++);
        *oVR0++  = (int16_t)1000.0f * log2f(*iVR0++);
        *oVR1i++ = (int16_t)1000.0f * log2f(*iVR1i++);
        *oVR1q++ = (int16_t)1000.0f * log2f(*iVR1q++);
        *oVR2++  = (int16_t)1000.0f * log2f(*iVR2++);
        *oVR3++  = (int16_t)1000.0f * log2f(*iVR3++);
        *oVR4++  = (int16_t)1000.0f * log2f(*iVR4++);
    }
    // ? Recover the float from 16-bit so that output is the same as generating products from level 15 data?
    #if defined(EMULATE_15)
    iHmi  = space->mX[0].i;      oHmi  = RKGetInt16DataFromRay(ray, RKMomentIndexHmi);
    iHmq  = space->mX[0].q;      oHmq  = RKGetInt16DataFromRay(ray, RKMomentIndexHmq);
    iHR0  = space->aR[0][0];     oHR0  = RKGetInt16DataFromRay(ray, RKMomentIndexHR0);
    iHR1i = space->R[0][1].i;    oHR1i = RKGetInt16DataFromRay(ray, RKMomentIndexHR1i);
    iHR1q = space->R[0][1].q;    oHR1q = RKGetInt16DataFromRay(ray, RKMomentIndexHR1q);
    iHR2  = space->aR[0][2];     oHR2  = RKGetInt16DataFromRay(ray, RKMomentIndexHR2);
    iHR3  = space->aR[0][3];     oHR3  = RKGetInt16DataFromRay(ray, RKMomentIndexHR3);
    iHR4  = space->aR[0][3];     oHR4  = RKGetInt16DataFromRay(ray, RKMomentIndexHR4);
    iVmi  = space->mX[1].i;      oVmi  = RKGetInt16DataFromRay(ray, RKMomentIndexVmi);
    iVmq  = space->mX[1].q;      oVmq  = RKGetInt16DataFromRay(ray, RKMomentIndexVmq);
    iVR0  = space->aR[1][0];     oVR0  = RKGetInt16DataFromRay(ray, RKMomentIndexVR0);
    iVR1i = space->R[1][1].i;    oVR1i = RKGetInt16DataFromRay(ray, RKMomentIndexVR1i);
    iVR1q = space->R[1][1].q;    oVR1q = RKGetInt16DataFromRay(ray, RKMomentIndexVR1q);
    iVR2  = space->aR[1][2];     oVR2  = RKGetInt16DataFromRay(ray, RKMomentIndexVR2);
    iVR3  = space->aR[1][3];     oVR3  = RKGetInt16DataFromRay(ray, RKMomentIndexVR3);
    iVR4  = space->aR[1][3];     oVR4  = RKGetInt16DataFromRay(ray, RKMomentIndexVR4);
    for (k = 0; k < MIN(space->capacity, space->gateCount); k++) {
        *iHmi++  = (RKFloat)(*oHmi++);
        *iHmq++  = (RKFloat)(*oHmq++);
        *iHR0++  = powf(2.0f, 0.001f * (RKFloat)*oHR0++);
        *iHR1i++ = powf(2.0f, 0.001f * (RKFloat)*oHR1i++);
        *iHR1q++ = powf(2.0f, 0.001f * (RKFloat)*oHR1q++);
        *iHR2++  = powf(2.0f, 0.001f * (RKFloat)*oHR2++);
        *iHR3++  = powf(2.0f, 0.001f * (RKFloat)*oHR3++);
        *iHR4++  = powf(2.0f, 0.001f * (RKFloat)*oHR4++);
        *iVmi++  = (RKFloat)(*oVmi++);
        *iVmq++  = (RKFloat)(*oVmq++);
        *iVR0++  = powf(2.0f, 0.001f * (RKFloat)*oVR0++);
        *iVR1i++ = powf(2.0f, 0.001f * (RKFloat)*oVR1i++);
        *iVR1q++ = powf(2.0f, 0.001f * (RKFloat)*oVR1q++);
        *iVR2++  = powf(2.0f, 0.001f * (RKFloat)*oVR2++);
        *iVR3++  = powf(2.0f, 0.001f * (RKFloat)*oVR3++);
        *iVR4++  = powf(2.0f, 0.001f * (RKFloat)*oVR4++);
    }
    #endif

    // Grab the data from scratch space.
    RKFloat *Si = space->S[0],  *So = RKGetFloatDataFromRay(ray, RKBaseProductIndexSh);
    RKFloat *Ti = space->S[1],  *To = RKGetFloatDataFromRay(ray, RKBaseProductIndexSv);
    RKFloat *Zi = space->Z[0],  *Zo = RKGetFloatDataFromRay(ray, RKBaseProductIndexZ);
    RKFloat *Vi = space->V[0],  *Vo = RKGetFloatDataFromRay(ray, RKBaseProductIndexV);
    RKFloat *Wi = space->W[0],  *Wo = RKGetFloatDataFromRay(ray, RKBaseProductIndexW);
    RKFloat *Qi = space->Q[0],  *Qo = RKGetFloatDataFromRay(ray, RKBaseProductIndexQ);
    RKFloat *Oi = space->Q[1];
    RKFloat *Di = space->ZDR,   *Do = RKGetFloatDataFromRay(ray, RKBaseProductIndexD);
    RKFloat *Pi = space->PhiDP, *Po = RKGetFloatDataFromRay(ray, RKBaseProductIndexP);
    RKFloat *Ki = space->KDP,   *Ko = RKGetFloatDataFromRay(ray, RKBaseProductIndexK);
    RKFloat *Ri = space->RhoHV, *Ro = RKGetFloatDataFromRay(ray, RKBaseProductIndexR);

    SNRThreshold = powf(10.0f, 0.1f * space->config->SNRThreshold);
    SQIThreshold = space->config->SQIThreshold;
    mask = space->mask;
    // Masking based on SNR and SQI
    for (k = 0; k < MIN(space->capacity, space->gateCount); k++) {
        SNRh = *Si / space->noise[0];
        SNRv = *Ti / space->noise[1];
        *So++ = 10.0f * log10f(*Si++) - 80.0f;                    // Still need the mapping coefficient from ADU-dB to dBm
        *To++ = 10.0f * log10f(*Ti++) - 80.0f;
        *Qo++ = *Qi;
        *mask = RKCellMaskNull;
        if (SNRh > SNRThreshold && *Qi > SQIThreshold) {
            *mask |= RKCellMaskKeepH;
        }
        if (SNRv > SNRThreshold && *Oi > SQIThreshold) {
            *mask |= RKCellMaskKeepV;
        }
        mask++;
        Qi++;
        Oi++;
    }
    // Simple despeckling: censor the current cell if the next cell is censored
    mask = space->mask;
    for (k = 0; k < MIN(space->capacity, space->gateCount) - 1; k++) {
        if (!(*(mask + 1) & RKCellMaskKeepH)) {
            *mask &= ~RKCellMaskKeepH;
        }
        if (!(*(mask + 1) & RKCellMaskKeepV)) {
            *mask &= ~RKCellMaskKeepV;
        }
        mask++;
    }
    // Now we copy out the values based on mask
    mask = space->mask;
    for (k = 0; k < MIN(space->capacity, space->gateCount); k++) {
        if (*mask & RKCellMaskKeepH) {
            *Zo++ = *Zi;
            *Vo++ = *Vi;
            *Wo++ = *Wi;
        } else {
            *Zo++ = NAN;
            *Vo++ = NAN;
            *Wo++ = NAN;
        }
        if (*mask == RKCellMaskKeepBoth) {
            *Do++ = *Di;
            *Po++ = *Pi;
            *Ko++ = *Ki;
            *Ro++ = *Ri;
        } else {
            *Do++ = NAN;
            *Po++ = NAN;
            *Ko++ = NAN;
            *Ro++ = NAN;
        }
        mask++;
        Zi++;
        Vi++;
        Wi++;
        Di++;
        Pi++;
        Ki++;
        Ri++;
    }
    // Record down the down-sampled gate count
    ray->header.gateCount = k;
    RKFloat lhma[4];
    const int K = (ray->header.gateCount * sizeof(RKFloat) + sizeof(RKVec) - 1) / sizeof(RKVec);

    // Convert float to 8-bit representation (0.0 - 255.0) using M * (value) + A; RhoHV is special
    RKSLHMAC   RKVec sl = _rk_mm_set1(lhma[0]);  RKVec sh = _rk_mm_set1(lhma[1]);  RKVec sm = _rk_mm_set1(lhma[2]);  RKVec sa = _rk_mm_set1(lhma[3]);
    RKZLHMAC   RKVec zl = _rk_mm_set1(lhma[0]);  RKVec zh = _rk_mm_set1(lhma[1]);  RKVec zm = _rk_mm_set1(lhma[2]);  RKVec za = _rk_mm_set1(lhma[3]);
    RKV2LHMAC  RKVec vl = _rk_mm_set1(lhma[0]);  RKVec vh = _rk_mm_set1(lhma[1]);  RKVec vm = _rk_mm_set1(lhma[2]);  RKVec va = _rk_mm_set1(lhma[3]);
    RKWLHMAC   RKVec wl = _rk_mm_set1(lhma[0]);  RKVec wh = _rk_mm_set1(lhma[1]);  RKVec wm = _rk_mm_set1(lhma[2]);  RKVec wa = _rk_mm_set1(lhma[3]);
    RKQLHMAC   RKVec ql = _rk_mm_set1(lhma[0]);  RKVec qh = _rk_mm_set1(lhma[1]);  RKVec qm = _rk_mm_set1(lhma[2]);  RKVec qa = _rk_mm_set1(lhma[3]);
    RKDLHMAC   RKVec dl = _rk_mm_set1(lhma[0]);  RKVec dh = _rk_mm_set1(lhma[1]);  RKVec dm = _rk_mm_set1(lhma[2]);  RKVec da = _rk_mm_set1(lhma[3]);
    RKPLHMAC   RKVec pl = _rk_mm_set1(lhma[0]);  RKVec ph = _rk_mm_set1(lhma[1]);  RKVec pm = _rk_mm_set1(lhma[2]);  RKVec pa = _rk_mm_set1(lhma[3]);
    RKKLHMAC   RKVec kl = _rk_mm_set1(lhma[0]);  RKVec kh = _rk_mm_set1(lhma[1]);  RKVec km = _rk_mm_set1(lhma[2]);  RKVec ka = _rk_mm_set1(lhma[3]);
    RKRLHMAC   RKVec rl = _rk_mm_set1(lhma[0]);  RKVec rh = _rk_mm_set1(lhma[1]);
    RKVec *Si_pf = (RKVec *)RKGetFloatDataFromRay(ray, RKBaseProductIndexSh); RKVec *So_pf = (RKVec *)space->S[0];
    RKVec *Ti_pf = (RKVec *)RKGetFloatDataFromRay(ray, RKBaseProductIndexSv); RKVec *To_pf = (RKVec *)space->S[1];
    RKVec *Zi_pf = (RKVec *)RKGetFloatDataFromRay(ray, RKBaseProductIndexZ);  RKVec *Zo_pf = (RKVec *)space->Z[0];
    RKVec *Vi_pf = (RKVec *)RKGetFloatDataFromRay(ray, RKBaseProductIndexV);  RKVec *Vo_pf = (RKVec *)space->V[0];
    RKVec *Wi_pf = (RKVec *)RKGetFloatDataFromRay(ray, RKBaseProductIndexW);  RKVec *Wo_pf = (RKVec *)space->W[0];
    RKVec *Qi_pf = (RKVec *)RKGetFloatDataFromRay(ray, RKBaseProductIndexQ);  RKVec *Qo_pf = (RKVec *)space->Q[0];
    RKVec *Di_pf = (RKVec *)RKGetFloatDataFromRay(ray, RKBaseProductIndexD);  RKVec *Do_pf = (RKVec *)space->ZDR;
    RKVec *Pi_pf = (RKVec *)RKGetFloatDataFromRay(ray, RKBaseProductIndexP);  RKVec *Po_pf = (RKVec *)space->PhiDP;
    RKVec *Ki_pf = (RKVec *)RKGetFloatDataFromRay(ray, RKBaseProductIndexK);  RKVec *Ko_pf = (RKVec *)space->KDP;
    RKVec *Ri_pf = (RKVec *)RKGetFloatDataFromRay(ray, RKBaseProductIndexR);  RKVec *Ro_pf = (RKVec *)space->RhoHV;
    for (k = 0; k < K; k++) {
        *So_pf++ = _rk_mm_add(_rk_mm_mul(_rk_mm_min(_rk_mm_max(*Si_pf++, sl), sh), sm), sa);
        *To_pf++ = _rk_mm_add(_rk_mm_mul(_rk_mm_min(_rk_mm_max(*Ti_pf++, sl), sh), sm), sa);
        *Zo_pf++ = _rk_mm_add(_rk_mm_mul(_rk_mm_min(_rk_mm_max(*Zi_pf++, zl), zh), zm), za);
        *Vo_pf++ = _rk_mm_add(_rk_mm_mul(_rk_mm_min(_rk_mm_max(*Vi_pf++, vl), vh), vm), va);
        *Wo_pf++ = _rk_mm_add(_rk_mm_mul(_rk_mm_min(_rk_mm_max(*Wi_pf++, wl), wh), wm), wa);
        *Qo_pf++ = _rk_mm_add(_rk_mm_mul(_rk_mm_min(_rk_mm_max(*Qi_pf++, ql), qh), qm), qa);
        *Do_pf++ = _rk_mm_add(_rk_mm_mul(_rk_mm_min(_rk_mm_max(*Di_pf++, dl), dh), dm), da);
        *Po_pf++ = _rk_mm_add(_rk_mm_mul(_rk_mm_min(_rk_mm_max(*Pi_pf++, pl), ph), pm), pa);
        *Ko_pf++ = _rk_mm_add(_rk_mm_mul(_rk_mm_min(_rk_mm_max(*Ki_pf++, kl), kh), km), ka);
        *Ro_pf++ = _rk_mm_min(_rk_mm_max(*Ri_pf++, rl), rh);
    }
    // Convert to uint8 type
    Si = space->S[0];  uint8_t *su = RKGetUInt8DataFromRay(ray, RKBaseProductIndexSh);
    Ti = space->S[1];  uint8_t *tu = RKGetUInt8DataFromRay(ray, RKBaseProductIndexSv);
    Zi = space->Z[0];  uint8_t *zu = RKGetUInt8DataFromRay(ray, RKBaseProductIndexZ);
    Vi = space->V[0];  uint8_t *vu = RKGetUInt8DataFromRay(ray, RKBaseProductIndexV);
    Wi = space->W[0];  uint8_t *wu = RKGetUInt8DataFromRay(ray, RKBaseProductIndexW);
    Qi = space->Q[0];  uint8_t *qu = RKGetUInt8DataFromRay(ray, RKBaseProductIndexQ);
    Di = space->ZDR;   uint8_t *du = RKGetUInt8DataFromRay(ray, RKBaseProductIndexD);
    Pi = space->PhiDP; uint8_t *pu = RKGetUInt8DataFromRay(ray, RKBaseProductIndexP);
    Ki = space->KDP;   uint8_t *ku = RKGetUInt8DataFromRay(ray, RKBaseProductIndexK);
    Ri = space->RhoHV; uint8_t *ru = RKGetUInt8DataFromRay(ray, RKBaseProductIndexR);
    for (k = 0; k < ray->header.gateCount; k++) {
        *su++ = *Si++;
        *tu++ = *Ti++;
        if (isfinite(*Zi)) {
            *zu++ = (uint8_t)*Zi;
            *vu++ = (uint8_t)*Vi;
            *wu++ = (uint8_t)*Wi;
            *qu++ = (uint8_t)*Qi;
            *du++ = (uint8_t)*Di;
            *pu++ = (uint8_t)*Pi;
            *ku++ = (uint8_t)*Ki;
            *ru++ = (uint8_t)RKRho2Uint8(*Ri);
        } else {
            // Uint8 = 0 = transparent color
            *zu++ = 0;
            *vu++ = 0;
            *wu++ = 0;
            *qu++ = 0;
            *du++ = 0;
            *pu++ = 0;
            *ku++ = 0;
            *ru++ = 0;
        }
        Zi++;
        Vi++;
        Wi++;
        Qi++;
        Di++;
        Pi++;
        Ki++;
        Ri++;
    }
    // If the space has been used for the same gateCount calculations, it should remain zero
    if (*Zi != 0.0 || *zu != 0) {
        memset(su, 0, (ray->header.capacity - ray->header.gateCount) * sizeof(uint8_t));
        memset(tu, 0, (ray->header.capacity - ray->header.gateCount) * sizeof(uint8_t));
        memset(zu, 0, (ray->header.capacity - ray->header.gateCount) * sizeof(uint8_t));
        memset(vu, 0, (ray->header.capacity - ray->header.gateCount) * sizeof(uint8_t));
        memset(wu, 0, (ray->header.capacity - ray->header.gateCount) * sizeof(uint8_t));
        memset(qu, 0, (ray->header.capacity - ray->header.gateCount) * sizeof(uint8_t));
        memset(du, 0, (ray->header.capacity - ray->header.gateCount) * sizeof(uint8_t));
        memset(pu, 0, (ray->header.capacity - ray->header.gateCount) * sizeof(uint8_t));
        memset(ku, 0, (ray->header.capacity - ray->header.gateCount) * sizeof(uint8_t));
        memset(ru, 0, (ray->header.capacity - ray->header.gateCount) * sizeof(uint8_t));
        memset(Si, 0, (ray->header.capacity - ray->header.gateCount) * sizeof(RKFloat));
        memset(Ti, 0, (ray->header.capacity - ray->header.gateCount) * sizeof(RKFloat));
        memset(Zi, 0, (ray->header.capacity - ray->header.gateCount) * sizeof(RKFloat));
        memset(Vi, 0, (ray->header.capacity - ray->header.gateCount) * sizeof(RKFloat));
        memset(Wi, 0, (ray->header.capacity - ray->header.gateCount) * sizeof(RKFloat));
        memset(Qi, 0, (ray->header.capacity - ray->header.gateCount) * sizeof(RKFloat));
        memset(Di, 0, (ray->header.capacity - ray->header.gateCount) * sizeof(RKFloat));
        memset(Pi, 0, (ray->header.capacity - ray->header.gateCount) * sizeof(RKFloat));
        memset(Ki, 0, (ray->header.capacity - ray->header.gateCount) * sizeof(RKFloat));
        memset(Ri, 0, (ray->header.capacity - ray->header.gateCount) * sizeof(RKFloat));
        ray->header.marker |= RKMarkerMemoryManagement;
    }
    ray->header.baseProductList = RKBaseProductListFloatZVWDPRKSQ | RKBaseProductListUInt8ZVWDPRKSQ;
    if (space->fftOrder > 0) {
        ray->header.fftOrder = (uint8_t)space->fftOrder;
    }
    return k;
}

int RKNullProcessor(RKMomentScratch *space, RKPulse **pulses, const uint16_t count) {
    return 0;
}
