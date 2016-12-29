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

size_t RKPulseBufferAlloc(void **mem, const int capacity, const int slots) {
    if (capacity - (1 << (int)log2f((float)capacity))) {
        RKLog("ERROR. Pulse capacity must of power of 2!");
        return 0;
    }
    size_t pulseSize = (sizeof(RKPulseHeader) + sizeof(RKPulseParameters) + 2 * capacity * (sizeof(RKInt16) + sizeof(RKComplex) + 2 * sizeof(RKFloat)));
    size_t bytes = slots * pulseSize;
    if ((bytes / RKSIMDAlignSize) * RKSIMDAlignSize != bytes) {
        RKLog("ERROR. The resultant size does not conform to SIMD alignment.");
        return 0;
    }
    posix_memalign(mem, RKSIMDAlignSize, bytes);
    if (*mem == NULL) {
        RKLog("ERROR. Unable to allocate buffer.");
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
        pulse->data = m + sizeof(RKPulseHeader) + sizeof(RKPulseParameters);
        m += pulseSize;
        i++;
    }
    return bytes;
}

RKPulse *RKGetPulse(void *buffer, const int k) {
    RKPulse *pulse = (RKPulse *)buffer;
    size_t pulseSize = (sizeof(RKPulseHeader) + sizeof(RKPulseParameters) + 2 * pulse->header.capacity * (sizeof(RKInt16) + sizeof(RKComplex) + 2 * sizeof(RKFloat)));
    return (RKPulse *)(buffer + k * pulseSize + sizeof(RKPulseHeader));
}

RKInt16 *RKGetInt16DataFromPulse(RKPulse *pulse, const int p) {
    void *m = (void *)pulse;
    m += sizeof(RKPulseHeader) + sizeof(RKPulseParameters);
    return (RKInt16 *)(m + p * pulse->header.capacity * sizeof(RKInt16));
}

RKComplex *RKGetComplexDataFromPulse(RKPulse *pulse, const int p) {
    void *m = (void *)pulse;
    m += sizeof(RKPulseHeader) + sizeof(RKPulseParameters);
    m += 2 * pulse->header.capacity * sizeof(RKInt16);
    return (RKComplex *)(m + p * pulse->header.capacity * sizeof(RKComplex));
}

RKIQZ *RKGetSplitComplexDataFromPulse(RKPulse *pulse, const int p) {
    void *m = (void *)pulse;
    m += sizeof(RKPulseHeader) + sizeof(RKPulseParameters);
    m += 2 * pulse->header.capacity * sizeof(RKInt16);
    m += 2 * pulse->header.capacity * sizeof(RKComplex);
    return (RKIQZ *)(m + p * pulse->header.capacity * 2 * sizeof(RKFloat));
}
