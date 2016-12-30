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
//    RKInt16            X[2][RKGateCount];
//    RKComplex          Y[2][RKGateCount];
//    RKIQZ              Z[2];
//
size_t RKPulseBufferAlloc(void **mem, const int capacity, const int slots) {
    if (capacity - (1 << (int)log2f((float)capacity))) {
        RKLog("ERROR. Pulse capacity must of power of 2!");
        return 0;
    }
    if (capacity != (capacity / RKSIMDAlignSize) * RKSIMDAlignSize) {
        RKLog("ERROR. Pulse capacity must be multiple of %d!", RKSIMDAlignSize);
        return 0;
    }
    RKPulse *pulse;
    size_t headerSize = sizeof(pulse->headerBytes);
    if (headerSize != (headerSize / RKSIMDAlignSize) * RKSIMDAlignSize) {
        RKLog("ERROR. The framework has not been compiled with proper structure size.");
        return 0;
    }
    size_t channelCount = 2;
    size_t pulseSize = headerSize + channelCount * capacity * (sizeof(RKInt16) + 4 * sizeof(RKFloat));
    if (pulseSize != (pulseSize / RKSIMDAlignSize) * RKSIMDAlignSize) {
        RKLog("ERROR. The resultant size does not conform to SIMD alignment.");
        return 0;
    }
    size_t bytes = slots * pulseSize;
    posix_memalign(mem, RKSIMDAlignSize, bytes);
    if (*mem == NULL) {
        RKLog("ERROR. Unable to allocate pulse buffer.");
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
        pulse->data = m + headerSize;
        m += pulseSize;
        i++;
    }
    return bytes;
}

RKPulse *RKGetPulse(void *buffer, const int k) {
    RKPulse *pulse = (RKPulse *)buffer;
    size_t pulseSize = sizeof(pulse->headerBytes) + 2 * pulse->header.capacity * (sizeof(RKInt16) + 4 * sizeof(RKFloat));
    return (RKPulse *)(buffer + k * pulseSize);
}

RKInt16 *RKGetInt16DataFromPulse(RKPulse *pulse, const int p) {
    void *m = (void *)pulse;
    m += sizeof(pulse->headerBytes);
    //void *m = (void *)pulse->data;
    //pulse->data = m;
    return (RKInt16 *)(m + p * pulse->header.capacity * sizeof(RKInt16));
}

RKComplex *RKGetComplexDataFromPulse(RKPulse *pulse, const int p) {
//    void *m = (void *)pulse->data;
    void *m = (void *)pulse;
    m += sizeof(pulse->headerBytes);
    m += 2 * pulse->header.capacity * sizeof(RKInt16);
    return (RKComplex *)(m + p * pulse->header.capacity * sizeof(RKComplex));
}

RKIQZ RKGetSplitComplexDataFromPulse(RKPulse *pulse, const int p) {
    void *m = (void *)pulse->data;
    m += 2 * pulse->header.capacity * (sizeof(RKInt16) + sizeof(RKComplex));
    m += p * pulse->header.capacity * 2 * sizeof(RKFloat);
    RKIQZ data = {(RKFloat *)m, (RKFloat *)(m + pulse->header.capacity * sizeof(RKFloat))};
    return data;
}

//
//    int16_t            idata[2][RKGateCount];
//    float              fdata[2][RKGateCount];
//
size_t RKRayBufferAlloc(void **mem, const int capacity, const int slots) {
    if (capacity - (capacity / RKSIMDAlignSize) * RKSIMDAlignSize != 0) {
        RKLog("ERROR. Ray capacity must be multiple of %d!", RKSIMDAlignSize);
        return 0;
    }
    RKRay *ray;
    size_t headerSize = sizeof(ray->headerBytes);
    if (headerSize != (headerSize / RKSIMDAlignSize) * RKSIMDAlignSize) {
        RKLog("ERROR. The framework has not been compiled with proper structure size.");
        return 0;
    }
    size_t channelCount = 2;
    size_t raySize = headerSize + channelCount * capacity * (sizeof(int16_t) + sizeof(float));
    if (raySize != (raySize / RKSIMDAlignSize) * RKSIMDAlignSize) {
        RKLog("ERROR. The resultant size does not conform to SIMD alignment.");
        return 0;
    }
    size_t bytes = slots * raySize;
    posix_memalign(mem, RKSIMDAlignSize, bytes);
    if (*mem == NULL) {
        RKLog("ERROR. Unable to allocate ray buffer.");
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
        ray->data = m + headerSize;
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
