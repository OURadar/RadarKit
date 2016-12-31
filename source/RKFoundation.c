//
//  RKFoundation
//  RadarKit
//
//  Created by Boon Leng Cheong on 9/13/16.
//
//

#include <RadarKit/RKFoundation.h>

// Strip out \r, \n, white space, \10 (BS), etc.
void stripTrailingUnwanted(char *str) {
    char *c = str + strlen(str) - 1;
    while (c >= str && (*c == '\r' || *c == '\n' || *c == ' ' || *c == 10)) {
        *c-- = '\0';
    }
}

//
// Each slot should have a structure as follows
//
//    RKPulseHeader      header;
//    RKPulseParameters  parameters;
//    RKInt16C           X[2][capacity];
//    RKComplex          Y[2][capacity];
//    RKIQZ              Z[2];
//
size_t RKPulseBufferAlloc(void **mem, const int capacity, const int slots) {
    if (capacity - (1 << (int)log2f((float)capacity))) {
        RKLog("Error. Pulse capacity must of power of 2!");
        return 0;
    }
    if (capacity != (capacity / RKSIMDAlignSize) * RKSIMDAlignSize) {
        RKLog("Error. Pulse capacity must be multiple of %d!", RKSIMDAlignSize);
        return 0;
    }
    RKPulse *pulse;
    size_t headerSize = sizeof(pulse->headerBytes);
    if (headerSize != (headerSize / RKSIMDAlignSize) * RKSIMDAlignSize) {
        RKLog("Error. The framework has not been compiled with proper structure size.");
        return 0;
    }
    size_t channelCount = 2;
    size_t pulseSize = headerSize + channelCount * capacity * (sizeof(RKInt16C) + 4 * sizeof(RKFloat));
    if (pulseSize != (pulseSize / RKSIMDAlignSize) * RKSIMDAlignSize) {
        RKLog("Error. The resultant size does not conform to SIMD alignment.");
        return 0;
    }
    size_t bytes = slots * pulseSize;
    posix_memalign(mem, RKSIMDAlignSize, bytes);
    if (*mem == NULL) {
        RKLog("Error. Unable to allocate pulse buffer.");
        return 0;
    }
    memset(*mem, 0, bytes);
    // Set the pulse capacity
    int i = 0;
    void *m = *mem;
    while (i < slots) {
        RKPulse *pulse = (RKPulse *)m;
        pulse->header.capacity = capacity;
        pulse->header.i = slots - i;
        m += pulseSize;
        i++;
    }
    return bytes;
}

RKPulse *RKGetPulse(void *buffer, const int k) {
    RKPulse *pulse = (RKPulse *)buffer;
    size_t pulseSize = sizeof(pulse->headerBytes) + 2 * pulse->header.capacity * (sizeof(RKInt16C) + 4 * sizeof(RKFloat));
    return (RKPulse *)(buffer + k * pulseSize);
}

RKInt16C *RKGetInt16DataFromPulse(RKPulse *pulse, const int c) {
    char *m = (char *)pulse->data;
    return (RKInt16C *)(m + c * pulse->header.capacity * sizeof(RKInt16C));
}

RKComplex *RKGetComplexDataFromPulse(RKPulse *pulse, const int c) {
    void *m = (void *)pulse->data;
    m += 2 * pulse->header.capacity * sizeof(RKInt16C);
    return (RKComplex *)(m + c * pulse->header.capacity * sizeof(RKComplex));
}

RKIQZ RKGetSplitComplexDataFromPulse(RKPulse *pulse, const int c) {
    void *m = (void *)pulse->data;
    m += 2 * pulse->header.capacity * (sizeof(RKInt16C) + sizeof(RKComplex));
    m += c * pulse->header.capacity * 2 * sizeof(RKFloat);
    RKIQZ data = {(RKFloat *)m, (RKFloat *)(m + pulse->header.capacity * sizeof(RKFloat))};
    return data;
}

//
// Each slot should have a structure as follows
//
//    RayHeader          header;
//    int16_t            idata[2][capacity];
//    float              fdata[2][capacity];
//
size_t RKRayBufferAlloc(void **mem, const int capacity, const int slots) {
    if (capacity - (capacity / RKSIMDAlignSize) * RKSIMDAlignSize != 0) {
        RKLog("Error. Ray capacity must be multiple of %d!", RKSIMDAlignSize);
        return 0;
    }
    RKRay *ray;
    size_t headerSize = sizeof(ray->headerBytes);
    if (headerSize != (headerSize / RKSIMDAlignSize) * RKSIMDAlignSize) {
        RKLog("Error. The framework has not been compiled with proper structure size.");
        return 0;
    }
    size_t productCount = 7;
    size_t raySize = headerSize + productCount * capacity * (sizeof(int16_t) + sizeof(float));
    if (raySize != (raySize / RKSIMDAlignSize) * RKSIMDAlignSize) {
        RKLog("Error. The resultant size does not conform to SIMD alignment.");
        return 0;
    }
    size_t bytes = slots * raySize;
    posix_memalign(mem, RKSIMDAlignSize, bytes);
    if (*mem == NULL) {
        RKLog("Error. Unable to allocate ray buffer.");
        return 0;
    }
    memset(*mem, 0, bytes);
    // Set the ray capacity
    int i = 0;
    void *m = *mem;
    while (i < slots) {
        RKRay *ray = (RKRay *)m;
        ray->header.capacity = capacity;
        ray->header.i = slots - i;
        m += raySize;
        i++;
    }
    return bytes;
}

RKRay *RKGetRay(void *buffer, const int k) {
    RKRay *ray = (RKRay *)buffer;
    size_t raySize = sizeof(ray->headerBytes) + 2 * ray->header.capacity * (sizeof(int16_t) + sizeof(float));
    return (RKRay *)(buffer + k * raySize);
}

int16_t *RKGetInt16DataFromRay(RKRay *ray, const int p) {
    void *m = (void *)ray->data;
    return (int16_t *)(m + p * ray->header.capacity * sizeof(int16_t));
}

float *RKGetFloatDataFromRay(RKRay *ray, const int p) {
    void *m = (void *)ray->data;
    m += 2 * ray->header.capacity * sizeof(int16_t);
    return (float *)(m + p * ray->header.capacity * sizeof(float));
}

#pragma mark -

RKScratch *RKScratchInit(const size_t capacity) {
    RKScratch *space = (RKScratch *)malloc(sizeof(RKScratch));
    if (space == NULL) {
        RKLog("Error. Unable to allocate a momment scratch space.\n");
        return NULL;
    }
    int j, k;
    size_t mem = 0;

    for (k = 0; k < 2; k++) {
        posix_memalign((void **)&space->mX[k].i, RKSIMDAlignSize, capacity * sizeof(RKFloat));
        posix_memalign((void **)&space->mX[k].q, RKSIMDAlignSize, capacity * sizeof(RKFloat));
        posix_memalign((void **)&space->vX[k].i, RKSIMDAlignSize, capacity * sizeof(RKFloat));
        posix_memalign((void **)&space->vX[k].q, RKSIMDAlignSize, capacity * sizeof(RKFloat));
        mem += 4;
        for (j = 0; j < RKMaxLag; j++) {
            posix_memalign((void **)&space->R[k][j].i, RKSIMDAlignSize, capacity * sizeof(RKFloat));
            posix_memalign((void **)&space->R[k][j].q, RKSIMDAlignSize, capacity * sizeof(RKFloat));
            posix_memalign((void **)&space->aR[k][j], RKSIMDAlignSize, capacity * sizeof(RKFloat));
            mem += 3;
        }
    }
    posix_memalign((void **)&space->sC.i, RKSIMDAlignSize, capacity * sizeof(RKFloat));
    posix_memalign((void **)&space->sC.q, RKSIMDAlignSize, capacity * sizeof(RKFloat));
    posix_memalign((void **)&space->ts.i, RKSIMDAlignSize, capacity * sizeof(RKFloat));
    posix_memalign((void **)&space->ts.q, RKSIMDAlignSize, capacity * sizeof(RKFloat));
    mem += 4;
    for (j = 0; j < 2 * RKMaxLag - 1; j++) {
        posix_memalign((void **)&space->C[j].i, RKSIMDAlignSize, capacity * sizeof(RKFloat));
        posix_memalign((void **)&space->C[j].q, RKSIMDAlignSize, capacity * sizeof(RKFloat));
        posix_memalign((void **)&space->aC[j], RKSIMDAlignSize, capacity * sizeof(RKFloat));
        mem += 3 ;
    }
    posix_memalign((void **)&space->gC, RKSIMDAlignSize, capacity * sizeof(RKFloat));
    mem++;
    mem *= capacity * sizeof(RKFloat);
    return space;
}

void RKScratchFree(RKScratch *space) {
    int j, k;
    for (k = 0; k < 2; k++) {
        free(space->mX[k].i);
        free(space->mX[k].q);
        free(space->vX[k].i);
        free(space->vX[k].q);
        for (j = 0; j < RKMaxLag; j++) {
            free(space->R[k][j].i);
            free(space->R[k][j].q);
            free(space->aR[k][j]);
        }
    }
    free(space->sC.i);
    free(space->sC.q);
    free(space->ts.i);
    free(space->ts.q);
    for (j = 0; j < 2 * RKMaxLag - 1; j++) {
        free(space->C[j].i);
        free(space->C[j].q);
        free(space->aC[j]);
    }
    free(space->gC);
    free(space);
}
