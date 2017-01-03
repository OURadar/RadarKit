//
//  RKFoundation
//  RadarKit
//
//  Created by Boon Leng Cheong on 9/13/16.
//
//

#include <RadarKit/RKFoundation.h>

#pragma mark -

int RKLog(const char *whatever, ...) {
    if (rkGlobalParameters.stream == NULL && rkGlobalParameters.logfile[0] == 0) {
        return 0;
    }
    // Construct the string
    int i = 0;
    va_list args;
    va_start(args, whatever);
    char msg[2048];
    if (strlen(whatever) > 1600) {
        fprintf(stderr, "RKLog() could potential crash for string '%s'\n", whatever);
    }
    if (whatever[0] == '>') {
        i += snprintf(msg, 2040, "                    : [%s] ", rkGlobalParameters.program);
    } else {
        i += snprintf(msg, 2040, "%s : [%s] ", RKNow(), rkGlobalParameters.program);
    }
    bool has_ok = (strcasestr(whatever, "ok") != NULL);
    bool has_not_ok = (strcasestr(whatever, "error") != NULL);
    bool has_warning = (strcasestr(whatever, "warning") != NULL);
    if (rkGlobalParameters.showColor) {
        if (has_ok) {
            i += snprintf(msg + i, 2040 - i, "\033[1;32m");
        } else if (has_not_ok) {
            i += snprintf(msg + i, 2040 - i, "\033[1;31m");
        } else if (has_warning) {
            i += snprintf(msg + i, 2040 - i, "\033[1;33m");
        }
    }
    if (whatever[0] == '>') {
        i += vsprintf(msg + i, whatever + 1, args);
    } else {
        i += vsprintf(msg + i, whatever, args);
    }
    if (rkGlobalParameters.showColor && (has_ok || has_not_ok || has_warning)) {
        snprintf(msg + i, 2040 - i, "\033[0m");
    }
    if (whatever[strlen(whatever) - 1] != '\n') {
        strncat(msg, "\n", 2040);
    }
    va_end(args);
    // Produce the string to the specified stream
    if (rkGlobalParameters.stream) {
        fprintf(rkGlobalParameters.stream, "%s", msg);
        fflush(rkGlobalParameters.stream);
    }
    // Write the string to a file if specified
    if (rkGlobalParameters.logfile[0] != '\0' && strlen(rkGlobalParameters.logfile) > 0) {
        FILE *logFileID = fopen(rkGlobalParameters.logfile, "a");
        if (logFileID == NULL) {
            fprintf(stderr, "Unable to log.\n");
            return 1;
        }
        fprintf(logFileID, "%s", msg);
        fclose(logFileID);
    }
    return 0;
}

#pragma mark -

void RKSetWantColor(const bool showColor) {
    rkGlobalParameters.showColor = showColor;
}

void RKSetWantScreenOutput(const bool yes) {
    if (yes) {
        rkGlobalParameters.stream = stdout;
    } else {
        rkGlobalParameters.stream = NULL;
    }
}

int RKSetProgramName(const char *name) {
    if (strlen(name) >= RKMaximumStringLength) {
        return 1;
    }
    snprintf(rkGlobalParameters.program, RKMaximumStringLength, "%s", name);
    return 0;
}

int RKSetLogfile(const char *filename) {
    if (filename == NULL) {
        rkGlobalParameters.logfile[0] = '\0';
        return 0;
    } else if (strlen(filename) >= RKMaximumStringLength) {
        return 1;
    }
    snprintf(rkGlobalParameters.logfile, RKMaximumStringLength, "%s", filename);
    return 0;
}

int RKSetLogfileToDefault(void) {
    snprintf(rkGlobalParameters.logfile, RKMaximumStringLength, "%s", RKDefaultLogfile);
    return 0;
}

#pragma mark -

void RKShowTypeSizes(void) {
    RKPulse *pulse = NULL;
    RKRay *ray = NULL;
    RKLog(">sizeof(void *) = %d", (int)sizeof(void *));
    RKLog(">sizeof(RKByte) = %d", (int)sizeof(RKByte));
    RKLog(">sizeof(RKInt16C) = %d", (int)sizeof(RKInt16C));
    RKLog(">sizeof(RKFloat) = %d", (int)sizeof(RKFloat));
    RKLog(">sizeof(RKComplex) = %d", (int)sizeof(RKComplex));
    RKLog(">sizeof(RKPulseHeader) = %d", (int)sizeof(RKPulseHeader));
    RKLog(">sizeof(RKPulseParameters) = %d", (int)sizeof(RKPulseParameters));
    RKLog(">sizeof(pulse->headerBytes) = %d", (int)sizeof(pulse->headerBytes));
    RKLog(">sizeof(RKRayHeader) = %d", (int)sizeof(RKRayHeader));
    RKLog(">sizeof(ray->headerBytes) = %d", (int)sizeof(ray->headerBytes));
}

void RKShowVecFloat(const char *name, const float *p, const int n) {
    int i = 0;
    int k = 0;
    char str[RKMaximumStringLength];
    k = sprintf(str, "%s[", name);
    while (i < n && k < RKMaximumStringLength - 20)
        k += sprintf(str + k, "%+9.4f           ", p[i++]);
    sprintf(str + k, "]");
    printf("%s\n", str);
}


void RKShowVecIQZ(const char *name, const RKIQZ *p, const int n) {
    int i = 0;
    int k = 0;
    char str[RKMaximumStringLength];
    k = sprintf(str, "%s[", name);
    while (i < n && k < RKMaximumStringLength - 20) {
        k += sprintf(str + k, "%+9.4f%+9.4fi ", p->i[i], p->q[i]);
        i++;
    }
    sprintf(str + k, "]");
    printf("%s\n", str);
}

#pragma mark -

void RKZeroOutFloat(RKFloat *data, const uint32_t capacity) {
    memset(data, 0, capacity * sizeof(RKFloat));
}

void RKZeroOutIQZ(RKIQZ *data, const uint32_t capacity) {
    memset(data->i, 0, capacity * sizeof(RKFloat));
    memset(data->q, 0, capacity * sizeof(RKFloat));
}

void RKZeroTailFloat(RKFloat *data, const uint32_t capacity, const uint32_t origin) {
    memset(&data[origin], 0, (capacity - origin) * sizeof(RKFloat));
}

void RKZeroTailIQZ(RKIQZ *data, const uint32_t capacity, const uint32_t origin) {
    memset(&data->i[origin], 0, (capacity - origin) * sizeof(RKFloat));
    memset(&data->q[origin], 0, (capacity - origin) * sizeof(RKFloat));
}

#pragma mark -

//
// Each slot should have a structure as follows
//
//    RKPulseHeader      header;
//    RKPulseParameters  parameters;
//    RKInt16C           X[2][capacity];
//    RKComplex          Y[2][capacity];
//    RKIQZ              Z[2];
//
size_t RKPulseBufferAlloc(RKPulse **mem, const uint32_t capacity, const uint32_t slots) {
    if (capacity - (1 << (int)log2f((float)capacity))) {
        RKLog("Error. Pulse capacity must be power of 2!");
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
    posix_memalign((void **)mem, RKSIMDAlignSize, bytes);
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
        pulse->header.i = i - slots;
        m += pulseSize;
        i++;
    }
    return bytes;
}

RKPulse *RKGetPulse(RKPulse *pulse, const uint32_t k) {
    size_t pulseSize = sizeof(pulse->headerBytes) + 2 * pulse->header.capacity * (sizeof(RKInt16C) + 4 * sizeof(RKFloat));
    return (RKPulse *)((void *)pulse + k * pulseSize);
}

RKInt16C *RKGetInt16CDataFromPulse(RKPulse *pulse, const uint32_t c) {
    void *m = (void *)pulse->data;
    return (RKInt16C *)(m + c * pulse->header.capacity * sizeof(RKInt16C));
}

RKComplex *RKGetComplexDataFromPulse(RKPulse *pulse, const uint32_t c) {
    void *m = (void *)pulse->data;
    m += 2 * pulse->header.capacity * sizeof(RKInt16C);
    return (RKComplex *)(m + c * pulse->header.capacity * sizeof(RKComplex));
}

RKIQZ RKGetSplitComplexDataFromPulse(RKPulse *pulse, const uint32_t c) {
    void *m = (void *)pulse->data;
    m += 2 * pulse->header.capacity * (sizeof(RKInt16C) + sizeof(RKComplex));
    m += c * pulse->header.capacity * 2 * sizeof(RKFloat);
    RKIQZ data = {(RKFloat *)m, (RKFloat *)(m + pulse->header.capacity * sizeof(RKFloat))};
    return data;
}

#pragma mark -

//
// Each slot should have a structure as follows
//
//    RayHeader          header;
//    int16_t            idata[2][capacity];
//    float              fdata[2][capacity];
//
size_t RKRayBufferAlloc(RKRay **mem, const uint32_t capacity, const uint32_t slots) {
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
    size_t raySize = headerSize + RKMaxProductCount * capacity * (sizeof(int16_t) + sizeof(float));
    if (raySize != (raySize / RKSIMDAlignSize) * RKSIMDAlignSize) {
        RKLog("Error. The resultant size does not conform to SIMD alignment.");
        return 0;
    }
    size_t bytes = slots * raySize;
    posix_memalign((void **)mem, RKSIMDAlignSize, bytes);
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
        ray->header.i = i - slots;
        m += raySize;
        i++;
    }
    return bytes;
}

RKRay *RKGetRay(RKRay *ray, const uint32_t k) {
    size_t raySize = sizeof(ray->headerBytes) + 2 * ray->header.capacity * (sizeof(int16_t) + sizeof(float));
    return (RKRay *)((void *)ray + k * raySize);
}

int16_t *RKGetInt16DataFromRay(RKRay *ray, const uint32_t p) {
    void *m = (void *)ray->data;
    return (int16_t *)(m + p * ray->header.capacity * sizeof(int16_t));
}

float *RKGetFloatDataFromRay(RKRay *ray, const uint32_t p) {
    void *m = (void *)ray->data;
    m += 2 * ray->header.capacity * sizeof(int16_t);
    return (float *)(m + p * ray->header.capacity * sizeof(float));
}

#pragma mark -

size_t RKScratchAlloc(RKScratch **buffer, const uint32_t capacity, const uint8_t lagCount, const bool showNumbers) {
    if (capacity - (1 << (int)log2f((float)capacity))) {
        RKLog("Error. Scratch space capacity must be power of 2!");
        return 0;
    }
    if (lagCount > RKLagCount) {
        RKLog("Error. Lag count must not exceed the hard-coded limit %d\n", lagCount);
        return 0;
    }
    *buffer = malloc(sizeof(RKScratch));
    if (*buffer == NULL) {
        RKLog("Error. Unable to allocate a momment scratch space.\n");
        return 0;
    }
    memset(*buffer, 0, sizeof(RKScratch));

    RKScratch *space = *buffer;
    space->lagCount = lagCount;
    space->showNumbers = showNumbers;
    
    int j, k;
    size_t bytes = 0;
    for (k = 0; k < 2; k++) {
        posix_memalign((void **)&space->mX[k].i, RKSIMDAlignSize, capacity * sizeof(RKFloat));
        posix_memalign((void **)&space->mX[k].q, RKSIMDAlignSize, capacity * sizeof(RKFloat));
        posix_memalign((void **)&space->vX[k].i, RKSIMDAlignSize, capacity * sizeof(RKFloat));
        posix_memalign((void **)&space->vX[k].q, RKSIMDAlignSize, capacity * sizeof(RKFloat));
        bytes += 4;
        for (j = 0; j < RKLagCount; j++) {
            posix_memalign((void **)&space->R[k][j].i, RKSIMDAlignSize, capacity * sizeof(RKFloat));
            posix_memalign((void **)&space->R[k][j].q, RKSIMDAlignSize, capacity * sizeof(RKFloat));
            posix_memalign((void **)&space->aR[k][j], RKSIMDAlignSize, capacity * sizeof(RKFloat));
            bytes += 3;
        }
    }
    posix_memalign((void **)&space->sC.i, RKSIMDAlignSize, capacity * sizeof(RKFloat));
    posix_memalign((void **)&space->sC.q, RKSIMDAlignSize, capacity * sizeof(RKFloat));
    posix_memalign((void **)&space->ts.i, RKSIMDAlignSize, capacity * sizeof(RKFloat));
    posix_memalign((void **)&space->ts.q, RKSIMDAlignSize, capacity * sizeof(RKFloat));
    bytes += 4;
    for (j = 0; j < 2 * RKLagCount - 1; j++) {
        posix_memalign((void **)&space->C[j].i, RKSIMDAlignSize, capacity * sizeof(RKFloat));
        posix_memalign((void **)&space->C[j].q, RKSIMDAlignSize, capacity * sizeof(RKFloat));
        posix_memalign((void **)&space->aC[j], RKSIMDAlignSize, capacity * sizeof(RKFloat));
        bytes += 3 ;
    }
    posix_memalign((void **)&space->gC, RKSIMDAlignSize, capacity * sizeof(RKFloat));
    bytes++;
    bytes *= capacity * sizeof(RKFloat);
    bytes += sizeof(RKScratch);
    return bytes;
}

void RKScratchFree(RKScratch *space) {
    int j, k;
    for (k = 0; k < 2; k++) {
        free(space->mX[k].i);
        free(space->mX[k].q);
        free(space->vX[k].i);
        free(space->vX[k].q);
        for (j = 0; j < RKLagCount; j++) {
            free(space->R[k][j].i);
            free(space->R[k][j].q);
            free(space->aR[k][j]);
        }
    }
    free(space->sC.i);
    free(space->sC.q);
    free(space->ts.i);
    free(space->ts.q);
    for (j = 0; j < 2 * RKLagCount - 1; j++) {
        free(space->C[j].i);
        free(space->C[j].q);
        free(space->aC[j]);
    }
    free(space->gC);
    free(space);
}
