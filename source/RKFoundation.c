//
//  RKFoundation
//  RadarKit
//
//  Created by Boon Leng Cheong on 9/13/16.
//
//

#include <RadarKit/RKFoundation.h>

#pragma mark - Logger

int RKLog(const char *whatever, ...) {
    if (rkGlobalParameters.stream == NULL && rkGlobalParameters.logfile[0] == 0) {
        return 0;
    }
    int i = 0;
    size_t len;
    time_t utc;
    va_list args;
    struct tm tm;
    char *msg = (char *)malloc(RKMaximumStringLength * sizeof(char));
    char *filename = (char *)malloc(RKMaximumPathLength * sizeof(char));
    if (msg == NULL || filename == NULL) {
        fprintf(stderr, "Error in RKLog().\n");
        free(filename);
        free(msg);
        return -1;
    }

    // Get the time
    time(&utc);
    memcpy(&tm, localtime(&utc), sizeof(struct tm));

    // Construct the string
    va_start(args, whatever);
    if (strlen(whatever) > RKMaximumStringLength - 256) {
        fprintf(stderr, "RKLog() could potential crash for string '%s'\n", whatever);
        free(filename);
        free(msg);
        return 1;
    }
    if (whatever[0] == '>') {
        i += sprintf(msg, "                    ");
    } else {
        i += strftime(msg, 32, "%Y/%m/%d %T ", &tm);
    }
    char *okay_str = strcasestr(whatever, "ok");
    char *info_str = strcasestr(whatever, "info");
    char *error_str = strcasestr(whatever, "error");
    char *warning_str = strcasestr(whatever, "warning");
    bool has_ok = okay_str != NULL;
    bool has_info = info_str != NULL;
    bool has_error =  error_str != NULL;
    bool has_warning = warning_str != NULL;
    
    char *anchor = (char *)whatever + (whatever[0] == '>' ? 1 : 0);

    if (has_ok || has_info || has_error || has_warning) {
        char colored_whatever[RKMaximumStringLength];
        
        if (has_ok) {
            len = (size_t)(okay_str - anchor);
        } else if (has_info) {
            len = (size_t)(info_str - anchor);
        } else if (has_error) {
            len = (size_t)(error_str - anchor);
        } else if (has_warning) {
            len = (size_t)(warning_str - anchor);
        } else {
            len = 0;
        }
        
        strncpy(colored_whatever, anchor, len);
        colored_whatever[len] = '\0';
        anchor += len;
        
        if (rkGlobalParameters.showColor) {
            if (has_ok) {
                len += sprintf(colored_whatever + len, "\033[1;92m");
            } else if (has_info) {
                len += sprintf(colored_whatever + len, "\033[1;96m");
            } else if (has_error) {
                len += sprintf(colored_whatever + len, "\033[1;91m");
            } else if (has_warning) {
                len += sprintf(colored_whatever + len, "\033[1;93m");
            }
        }
        strncpy(colored_whatever + len, anchor, RKMaximumStringLength - len);
        
        i += vsprintf(msg + i, colored_whatever, args);
        
        if (rkGlobalParameters.showColor) {
            sprintf(msg + i, "\033[0m");
        }
    } else {
        vsprintf(msg + i, anchor, args);
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
    FILE *logFileID = NULL;
    if (rkGlobalParameters.dailyLog) {
        if (strlen(rkGlobalParameters.rootDataFolder)) {
            i = sprintf(filename, "%s/log/%s-", rkGlobalParameters.rootDataFolder, rkGlobalParameters.program);
        } else {
            i = 0;
        }
        strftime(filename + i, RKNameLength - i, "%Y%m%d.log", &tm);
        if (i) {
            RKPreparePath(filename);
        }
        logFileID = fopen(filename, "a");
    } else if (strlen(rkGlobalParameters.logfile)) {
        RKPreparePath(rkGlobalParameters.logfile);
        logFileID = fopen(rkGlobalParameters.logfile, "a");
    }
    if (logFileID) {
        fprintf(logFileID, "%s", msg);
        fclose(logFileID);
    }
    free(filename);
    free(msg);
    return 0;
}

#pragma mark - Global Preferences

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

void RKSetUseDailyLog(const bool dailyLog) {
    rkGlobalParameters.dailyLog = dailyLog;
}

int RKSetProgramName(const char *name) {
    if (strlen(name) >= RKNameLength) {
        return 1;
    }
    snprintf(rkGlobalParameters.program, RKNameLength, "%s", name);
    return RKResultSuccess;
}

int RKSetRootFolder(const char *folder) {
    if (strlen(folder) > RKNameLength - 64) {
        fprintf(stderr, "WARNING. Very long root folder.\n");
    }
    sprintf(rkGlobalParameters.rootDataFolder, "%s", folder);
    size_t len = strlen(rkGlobalParameters.rootDataFolder);
    while (rkGlobalParameters.rootDataFolder[len - 1] == '/') {
        len--;
    }
    rkGlobalParameters.rootDataFolder[len] = '\0';
    return RKResultSuccess;
}

int RKSetLogfile(const char *filename) {
    if (filename == NULL) {
        rkGlobalParameters.logfile[0] = '\0';
        return 0;
    } else if (strlen(filename) >= RKNameLength) {
        return 1;
    }
    snprintf(rkGlobalParameters.logfile, RKNameLength, "%s", filename);
    return RKResultSuccess;
}

int RKSetLogfileToDefault(void) {
    if (strlen(rkGlobalParameters.rootDataFolder)) {
        snprintf(rkGlobalParameters.logfile, RKNameLength, "%s", RKDefaultLogfile);
    } else {
        snprintf(rkGlobalParameters.logfile, RKNameLength, "%s/%s", rkGlobalParameters.rootDataFolder, RKDefaultLogfile);
    }
    return RKResultSuccess;
}

#pragma mark - Screen Output

void RKShowTypeSizes(void) {
    RKPulse *pulse = NULL;
    RKRay *ray = NULL;
    RKSweep *sweep = NULL;
    // Keeep current output stream and temporary change to screen output
    FILE *stream = rkGlobalParameters.stream;
    rkGlobalParameters.stream = stdout;
    RKLog(">sizeof(void *) = %d\n", (int)sizeof(void *));
    RKLog(">sizeof(RKByte) = %d\n", (int)sizeof(RKByte));
    RKLog(">sizeof(RKFloat) = %d\n", (int)sizeof(RKFloat));
    RKLog(">sizeof(RKInt16C) = %d\n", (int)sizeof(RKInt16C));
    RKLog(">sizeof(RKComplex) = %d\n", (int)sizeof(RKComplex));
    RKLog(">sizeof(RKRadarDesc) = %s\n", RKIntegerToCommaStyleString(sizeof(RKRadarDesc)));
    RKLog(">sizeof(RKConfig) = %s\n", RKIntegerToCommaStyleString(sizeof(RKConfig)));
    RKLog(">sizeof(RKHealth) = %s\n", RKIntegerToCommaStyleString(sizeof(RKHealth)));
    RKLog(">sizeof(RKNodalHealth) = %d\n", (int)sizeof(RKNodalHealth));
    RKLog(">sizeof(RKPosition) = %d\n", (int)sizeof(RKPosition));
    RKLog(">sizeof(RKPulseHeader) = %d\n", (int)sizeof(RKPulseHeader));
    RKLog(">sizeof(RKPulseParameters) = %d\n", (int)sizeof(RKPulseParameters));
    RKLog(">sizeof(pulse->headerBytes) = %d  (SIMD aligned: %s)\n", (int)sizeof(pulse->headerBytes), sizeof(pulse->headerBytes) % RKSIMDAlignSize == 0 ? "true" : "false");
    RKLog(">sizeof(RKPulse) = %d\n", (int)sizeof(RKPulse));
    RKLog(">sizeof(RKRayHeader) = %d\n", (int)sizeof(RKRayHeader));
    RKLog(">sizeof(ray->headerBytes) = %d  (SIMD aligned: %s)\n", (int)sizeof(ray->headerBytes), sizeof(ray->headerBytes) % RKSIMDAlignSize == 0 ? "true" : "false");
    RKLog(">sizeof(RKRay) = %d\n", (int)sizeof(RKRay));
    RKLog(">sizeof(RKSweep) = %s\n", RKIntegerToCommaStyleString(sizeof(RKSweep)));
    RKLog(">sizeof(sweep->header) = %s\n", RKIntegerToCommaStyleString(sizeof(sweep->header)));
    RKLog(">sizeof(RKScratch) = %d\n", (int)sizeof(RKScratch));
    RKLog(">sizeof(RKFileHeader) = %s\n", RKIntegerToCommaStyleString(sizeof(RKFileHeader)));
    RKLog(">sizeof(RKPreferenceObject) = %s\n", RKIntegerToCommaStyleString(sizeof(RKPreferenceObject)));
    RKLog(">sizeof(RKControl) = %s\n", RKIntegerToCommaStyleString(sizeof(RKControl)));
    RKLog(">sizeof(RKStatus) = %d\n", (int)sizeof(RKStatus));
    RKLog(">sizeof(RKFileMonitor) = %s\n", RKIntegerToCommaStyleString(sizeof(RKFileMonitor)));
    RKLog(">sizeof(RKFilterAnchor) = %d\n", (int)sizeof(RKFilterAnchor));
    RKLog(">sizeof(struct sockaddr) = %d\n", (int)sizeof(struct sockaddr));
    RKLog(">sizeof(struct sockaddr_in) = %d\n", (int)sizeof(struct sockaddr_in));
    // Restoring previous output stream
    rkGlobalParameters.stream = stream;
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
    char *str = (char *)malloc(RKMaximumStringLength);
    if (str == NULL) {
        fprintf(stderr, "Error allocating string buffer.\n");
        return;
    }
    k = sprintf(str, "%s[", name);
    while (i < n && k < RKMaximumStringLength - 20) {
        k += sprintf(str + k, "%+9.4f%+9.4fi ", p->i[i], p->q[i]);
        i++;
    }
    sprintf(str + k, "]");
    printf("%s\n", str);
    fflush(stdout);
    free(str);
}

#pragma mark - Buffer

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

#pragma mark - Pulse

//
// Each slot should have a structure as follows
//
//    RKPulseHeader      header;
//    RKPulseParameters  parameters;
//    RKInt16C           X[2][capacity];
//    RKComplex          Y[2][capacity];
//    RKIQZ              Z[2];
//
size_t RKPulseBufferAlloc(RKBuffer *mem, const uint32_t capacity, const uint32_t slots) {
    if (capacity != (capacity * sizeof(RKInt16C) / RKSIMDAlignSize) * RKSIMDAlignSize / sizeof(RKInt16C)) {
        RKLog("Error. Pulse capacity must be multiple of %d!", RKSIMDAlignSize / sizeof(RKInt16C));
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
        RKLog("Error. The total pulse size %s does not conform to SIMD alignment.", RKIntegerToCommaStyleString(pulseSize));
        return 0;
    }
    size_t bytes = slots * pulseSize;
    if (posix_memalign((void **)mem, RKSIMDAlignSize, bytes)) {
        RKLog("Error. Unable to allocate pulse buffer.");
        exit(EXIT_FAILURE);
    }
    memset(*mem, 0, bytes);
    // Set the pulse capacity
    int i = 0;
    void *m = *mem;
    while (i < slots) {
        RKPulse *pulse = (RKPulse *)m;
        pulse->header.capacity = capacity;
        pulse->header.i = (uint64_t)(-slots) + i;
        m += pulseSize;
        i++;
    }
    return bytes;
}

void RKPulseBufferFree(RKBuffer mem) {
    return free(mem);
}

RKPulse *RKGetPulse(RKBuffer buffer, const uint32_t k) {
    RKPulse *pulse = (RKPulse *)buffer;
    size_t headerSize = sizeof(pulse->headerBytes);
    size_t pulseSize = headerSize + 2 * pulse->header.capacity * (sizeof(RKInt16C) + 4 * sizeof(RKFloat));
    return (RKPulse *)(buffer + k * pulseSize);
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

#pragma mark - Ray

//
// Each slot should have a structure as follows
//
//    RayHeader          header;
//    int8 _t            idata[RKMaxProductCount][capacity];
//    float              fdata[RKMaxProductCount][capacity];
//
size_t RKRayBufferAlloc(RKBuffer *mem, const uint32_t capacity, const uint32_t slots) {
    if (capacity != (capacity / RKSIMDAlignSize) * RKSIMDAlignSize) {
        RKLog("Error. Ray capacity must be a multiple of %d!", RKSIMDAlignSize);
        return 0;
    }
    RKRay *ray;
    size_t headerSize = sizeof(ray->headerBytes);
    if (headerSize != (headerSize / RKSIMDAlignSize) * RKSIMDAlignSize) {
        RKLog("Error. The framework has not been compiled with proper structure size.");
        return 0;
    }
    size_t raySize = headerSize + RKMaxProductCount * capacity * (sizeof(uint8_t) + sizeof(float));
    if (raySize != (raySize / RKSIMDAlignSize) * RKSIMDAlignSize) {
        RKLog("Error. The total ray size %s does not conform to SIMD alignment.", RKIntegerToCommaStyleString(raySize));
        return 0;
    }
    size_t bytes = slots * raySize;
    if (posix_memalign((void **)mem, RKSIMDAlignSize, bytes)) {
        RKLog("Error. Unable to allocate ray buffer.");
        exit(EXIT_FAILURE);
    }
    memset(*mem, 0, bytes);
    // Set the ray capacity
    int i = 0;
    void *m = *mem;
    while (i < slots) {
        RKRay *ray = (RKRay *)m;
        ray->header.capacity = capacity;
        ray->header.i = (uint64_t)(-slots) + i;
        m += raySize;
        i++;
    }
    return bytes;
}

void RKRayBufferFree(RKBuffer mem) {
    return free(mem);
}

RKRay *RKGetRay(RKBuffer buffer, const uint32_t k) {
    RKRay *ray = (RKRay *)buffer;
    size_t raySize = RKRayHeaderPaddedSize + RKMaxProductCount * ray->header.capacity * (sizeof(uint8_t) + sizeof(float));
    return (RKRay *)((void *)ray + k * raySize);
}

uint8_t *RKGetUInt8DataFromRay(RKRay *ray, const RKProductIndex m) {
    void *d = (void *)ray->data;
    return (uint8_t *)(d + m * ray->header.capacity * sizeof(uint8_t));
}

float *RKGetFloatDataFromRay(RKRay *ray, const RKProductIndex m) {
    void *d = (void *)ray->data;
    d += RKMaxProductCount * ray->header.capacity * sizeof(uint8_t);
    return (float *)(d + m * ray->header.capacity * sizeof(float));
}

#pragma mark - Scratch Space

size_t RKScratchAlloc(RKScratch **buffer, const uint32_t capacity, const uint8_t lagCount, const bool showNumbers) {
    if (capacity - (capacity * sizeof(RKFloat) / RKSIMDAlignSize) * RKSIMDAlignSize / sizeof(RKFloat)) {
        RKLog("Error. Scratch space capacity must be an integer multiple of %s!",
              RKIntegerToCommaStyleString(RKSIMDAlignSize / sizeof(RKFloat)));
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
    space->capacity = MAX(1, (capacity / RKSIMDAlignSize)) * RKSIMDAlignSize;
    space->lagCount = lagCount;
    space->showNumbers = showNumbers;
    
    int j, k;
    size_t bytes = 0;
    for (k = 0; k < 2; k++) {
        POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->mX[k].i, RKSIMDAlignSize, capacity * sizeof(RKFloat)));
        POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->mX[k].q, RKSIMDAlignSize, capacity * sizeof(RKFloat)));
        POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->vX[k].i, RKSIMDAlignSize, capacity * sizeof(RKFloat)));
        POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->vX[k].q, RKSIMDAlignSize, capacity * sizeof(RKFloat)));
        POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->S[k], RKSIMDAlignSize, capacity * sizeof(RKFloat)));
        POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->Z[k], RKSIMDAlignSize, capacity * sizeof(RKFloat)));
        POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->V[k], RKSIMDAlignSize, capacity * sizeof(RKFloat)));
        POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->W[k], RKSIMDAlignSize, capacity * sizeof(RKFloat)));
        POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->SNR[k], RKSIMDAlignSize, capacity * sizeof(RKFloat)));
        POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->rcor[k], RKSIMDAlignSize, capacity * sizeof(RKFloat)));
        bytes += 10;
        for (j = 0; j < space->lagCount; j++) {
            POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->R[k][j].i, RKSIMDAlignSize, capacity * sizeof(RKFloat)));
            POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->R[k][j].q, RKSIMDAlignSize, capacity * sizeof(RKFloat)));
            POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->aR[k][j], RKSIMDAlignSize, capacity * sizeof(RKFloat)));
            bytes += 3;
        }
    }
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->sC.i, RKSIMDAlignSize, capacity * sizeof(RKFloat)));
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->sC.q, RKSIMDAlignSize, capacity * sizeof(RKFloat)));
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->ts.i, RKSIMDAlignSize, capacity * sizeof(RKFloat)));
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->ts.q, RKSIMDAlignSize, capacity * sizeof(RKFloat)));
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->ZDR,   RKSIMDAlignSize, capacity * sizeof(RKFloat)));
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->PhiDP, RKSIMDAlignSize, capacity * sizeof(RKFloat)));
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->RhoHV, RKSIMDAlignSize, capacity * sizeof(RKFloat)));
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->KDP,   RKSIMDAlignSize, capacity * sizeof(RKFloat)));
    bytes += 8;
    for (j = 0; j < 2 * space->lagCount - 1; j++) {
        POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->C[j].i, RKSIMDAlignSize, capacity * sizeof(RKFloat)));
        POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->C[j].q, RKSIMDAlignSize, capacity * sizeof(RKFloat)));
        POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->aC[j], RKSIMDAlignSize, capacity * sizeof(RKFloat)));
        bytes += 3 ;
    }
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->gC, RKSIMDAlignSize, capacity * sizeof(RKFloat)));
    bytes++;
    bytes *= capacity * sizeof(RKFloat);
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->mask, RKSIMDAlignSize, capacity * sizeof(int8_t)));
    bytes += capacity * sizeof(int8_t);
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
        free(space->S[k]);
        free(space->Z[k]);
        free(space->V[k]);
        free(space->W[k]);
        free(space->SNR[k]);
        free(space->rcor[k]);
        for (j = 0; j < space->lagCount; j++) {
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
    for (j = 0; j < 2 * space->lagCount - 1; j++) {
        free(space->C[j].i);
        free(space->C[j].q);
        free(space->aC[j]);
    }
    free(space->gC);
    free(space->mask);
    free(space);
}

#pragma mark - File Monitor

static void *fileMonitorRunLoop(void *in) {
    RKFileMonitor *engine = (RKFileMonitor *)in;

    int s;
    struct stat fileStat;
    
    engine->state |= RKEngineStateActive;
    engine->state ^= RKEngineStateActivating;

    stat(engine->filename, &fileStat);
    time_t mtime = fileStat.st_mtime;

    RKLog("%s Started.   file = %s B\n", engine->name, engine->filename);

    while (engine->state & RKEngineStateActive) {
        engine->state |= RKEngineStateSleep1;
        s = 0;
        while (s++ < 10 && engine->state & RKEngineStateActive) {
            if (engine->verbose > 2) {
                RKLog("%s", engine->name);
            }
            usleep(100000);
        }
        engine->state ^= RKEngineStateSleep1;
        stat(engine->filename, &fileStat);
        if (mtime != fileStat.st_mtime) {
            mtime = fileStat.st_mtime;
            RKLog("%s %s modified.\n", engine->name, engine->filename);
            if (engine->callbackRoutine) {
                engine->callbackRoutine(engine);
            }
        }
    }
    return NULL;
}

RKFileMonitor *RKFileMonitorInit(const char *filename, void (*routine)(void *)) {
    RKFileMonitor *engine = (RKFileMonitor *)malloc(sizeof(RKFileMonitor));
    if (engine == NULL) {
        RKLog("Error allocating a file monitor.\n");
        return NULL;
    }
    memset(engine, 0, sizeof(RKFileMonitor));
    sprintf(engine->name, "%s<FileMonitor>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(RKEngineColorMisc) : "",
            rkGlobalParameters.showColor ? RKNoColor : "");
    engine->state = RKEngineStateAllocated | RKEngineStateProperlyWired | RKEngineStateActivating;
    engine->memoryUsage = sizeof(RKFileMonitor);
    strncpy(engine->filename, filename, RKMaximumPathLength);
    engine->callbackRoutine = routine;
    if (engine->verbose) {
        RKLog("%s Starting ...\n", engine->name);
    }
    if (pthread_create(&engine->tid, NULL, fileMonitorRunLoop, engine)) {
        RKLog("%s Error creating file monitor.\n", engine->name);
        free(engine);
        return NULL;
    }
    while (!(engine->state & RKEngineStateActive)) {
        usleep(100000);
    }
    return engine;
}

int RKFileMonitorFree(RKFileMonitor *engine) {
    return RKSimpleEngineFree((RKSimpleEngine *)engine);
}

#pragma mark - Moment Stuff

RKStream RKStreamFromString(const char * string) {
	int j = 0;
	char *c = (char *)string;
	RKStream flag = RKStreamNull;
	while (j++ < strlen(string)) {
		switch (*c) {
			case 'h':
				flag |= RKStreamStatusHealth;
				break;
			case '1':
				flag |= RKStreamStatusPulses;
				break;
			case '2':
				flag |= RKStreamStatusRays;
				break;
			case '3':
				flag |= RKStreamStatusPositions;
				break;
			case '4':
				flag |= RKStreamStatusEngines;
				break;
			case '!':
				flag |= RKStreamStatusProcessorStatus;
				break;
			case 'z':
				flag |= RKStreamDisplayZ;
				break;
			case 'Z':
				flag |= RKStreamProductZ;
				break;
			case 'v':
				flag |= RKStreamDisplayV;
				break;
			case 'V':
				flag |= RKStreamProductV;
				break;
			case 'w':
				flag |= RKStreamDisplayW;
				break;
			case 'W':
				flag |= RKStreamProductW;
				break;
			case 'd':
				flag |= RKStreamDisplayD;
				break;
			case 'D':
				flag |= RKStreamProductD;
				break;
			case 'p':
				flag |= RKStreamDisplayP;
				break;
			case 'P':
				flag |= RKStreamProductP;
				break;
			case 'r':
				flag |= RKStreamDisplayR;
				break;
			case 'R':
				flag |= RKStreamProductR;
				break;
			case 'k':
				flag |= RKStreamDisplayK;
				break;
			case 'K':
				flag |= RKStreamProductK;
				break;
			case 's':
				flag |= RKStreamDisplaySh;
				break;
			case 'S':
				flag |= RKStreamProductSh;
				break;
			case 't':
				flag |= RKStreamDisplaySv;
				break;
			case 'T':
				flag |= RKStreamProductSv;
				break;
			case 'i':
				flag |= RKStreamDisplayIQ;
				break;
			case 'I':
				flag |= RKStreamProductIQ;
				break;
			case 'Y':
				flag |= RKStreamSweepZ;
				break;
			case 'U':
				flag |= RKStreamSweepV;
				break;
			case 'X':
				flag |= RKStreamSweepW;
				break;
			case 'C':
				flag |= RKStreamSweepD;
				break;
			case 'O':
				flag |= RKStreamSweepP;
				break;
			case 'Q':
				flag |= RKStreamSweepR;
				break;
			case 'J':
				flag |= RKStreamSweepK;
				break;
			default:
				break;
		}
		c++;
	}
	return flag;
}

int RKStringFromStream(char *string, RKStream stream) {
	int j = 0;
	if (stream & RKStreamStatusPulses)          { j += sprintf(string + j, "1"); }
	if (stream & RKStreamStatusRays)            { j += sprintf(string + j, "2"); }
	if (stream & RKStreamStatusPositions)       { j += sprintf(string + j, "3"); }
	if (stream & RKStreamStatusEngines)         { j += sprintf(string + j, "4"); }
	if (stream & RKStreamStatusProcessorStatus) { j += sprintf(string + j, "!"); }
	if (stream & RKStreamStatusHealth)          { j += sprintf(string + j, "h"); }
	if (stream & RKStreamDisplayZ)              { j += sprintf(string + j, "z"); }
	if (stream & RKStreamProductZ)              { j += sprintf(string + j, "Z"); }
	if (stream & RKStreamDisplayV)              { j += sprintf(string + j, "v"); }
	if (stream & RKStreamProductV)              { j += sprintf(string + j, "V"); }
	if (stream & RKStreamDisplayW)              { j += sprintf(string + j, "w"); }
	if (stream & RKStreamProductW)              { j += sprintf(string + j, "W"); }
	if (stream & RKStreamDisplayD)              { j += sprintf(string + j, "d"); }
	if (stream & RKStreamProductD)              { j += sprintf(string + j, "D"); }
	if (stream & RKStreamDisplayP)              { j += sprintf(string + j, "p"); }
	if (stream & RKStreamProductP)              { j += sprintf(string + j, "P"); }
	if (stream & RKStreamDisplayR)              { j += sprintf(string + j, "r"); }
	if (stream & RKStreamProductR)              { j += sprintf(string + j, "R"); }
	if (stream & RKStreamDisplayK)              { j += sprintf(string + j, "k"); }
	if (stream & RKStreamProductK)              { j += sprintf(string + j, "K"); }
	if (stream & RKStreamDisplaySh)             { j += sprintf(string + j, "s"); }
	if (stream & RKStreamProductSh)             { j += sprintf(string + j, "S"); }
	if (stream & RKStreamDisplaySv)             { j += sprintf(string + j, "t"); }
	if (stream & RKStreamProductSv)             { j += sprintf(string + j, "T"); }
	if (stream & RKStreamDisplayIQ)             { j += sprintf(string + j, "i"); }
	if (stream & RKStreamProductIQ)             { j += sprintf(string + j, "I"); }
	if (stream & RKStreamSweepZ)                { j += sprintf(string + j, "Y"); }
	if (stream & RKStreamSweepV)                { j += sprintf(string + j, "U"); }
	if (stream & RKStreamSweepW)                { j += sprintf(string + j, "X"); }
	if (stream & RKStreamSweepD)                { j += sprintf(string + j, "C"); }
	if (stream & RKStreamSweepP)                { j += sprintf(string + j, "O"); }
	if (stream & RKStreamSweepR)                { j += sprintf(string + j, "Q"); }
	if (stream & RKStreamSweepK)                { j += sprintf(string + j, "J"); }
	string[j] = '\0';
	return j;
}

char *RKStringOfStream(RKStream stream) {
	static string[RKNameLength];
	RKStringFromStream(string, stream);
	return string;
}

int RKGetNextProductDescription(char *symbol, char *name, char *unit, char *colormap, uint32_t *index, uint32_t *list) {
    if (list == NULL || *list == 0) {
        return RKResultNullInput;
    }
    RKName symbols[] = {
        "Z",
        "V",
        "W",
        "D",
        "P",
        "R",
        "K",
        "Sh",
        "Sv",
        "U"
    };
    RKName names[] = {
        "Corrected_Intensity",
        "Radial_Velocity",
        "Width",
        "Differential_Reflectivity",
        "PhiDP",
        "RhoHV",
        "KDP",
        "Signal_Power_H",
        "Signal_Power_V",
        "Uknown"
    };
    RKName units[] = {
        "dBZ",
        "MetersPerSecond",
        "MetersPerSecond",
        "dB",
        "Degrees",
        "Unitless",
        "DegreesPerMeter",
        "dBm",
        "dBm",
        "Undefined"
    };
    RKName colormaps[] = {
        "Reflectivity",
        "Velocity",
        "Width",
        "Differential_Reflectivity",
        "PhiDP",
        "RhoHV",
        "KDP",
        "Power",
        "Power",
        "Default"
    };
    uint32_t products[] = {
        RKProductListProductZ,
        RKProductListProductV,
        RKProductListProductW,
        RKProductListProductD,
        RKProductListProductP,
        RKProductListProductR,
        RKProductListProductK,
        RKProductListProductSh,
        RKProductListProductSv,
        0xFFFF
    };
    uint32_t productIndices[] = {
        RKProductIndexZ,
        RKProductIndexV,
        RKProductIndexW,
        RKProductIndexD,
        RKProductIndexP,
        RKProductIndexR,
        RKProductIndexK,
        RKProductIndexSh,
        0
    };
    int k = -1;
    if (*list & RKProductListProductZ) {
        k = 0;
    } else if (*list & RKProductListProductV) {
        k = 1;
    } else if (*list & RKProductListProductW) {
        k = 2;
    } else if (*list & RKProductListProductD) {
        k = 3;
    } else if (*list & RKProductListProductP) {
        k = 4;
    } else if (*list & RKProductListProductR) {
        k = 5;
    } else if (*list & RKProductListProductK) {
        k = 6;
    } else if (*list & RKProductListProductSh) {
        k = 7;
    } else if (*list & RKProductListProductSv) {
        k = 8;
    }
    if (k < 0) {
        RKLog("Unable to get description for k = %d\n", k);
        return RKResultNullInput;
    }
    if (symbol) {
        sprintf(symbol, "%s", symbols[k]);
    }
    if (name) {
        sprintf(name, "%s", names[k]);
    }
    if (unit) {
        sprintf(unit, "%s", units[k]);
    }
    if (colormap) {
        sprintf(colormap, "%s", colormaps[k]);
    }
    if (index) {
        *index = productIndices[k];
    }
    *list ^= products[k];
    return RKResultSuccess;
}

#pragma mark - JSON Stuff


void RKParseCommaDelimitedValues(void *valueStorage, RKValueType type, const size_t size, const char *valueString) {
    float *fv;
    double *fd;
    int32_t *i32v;
    uint32_t *u32v;
    char *copy = (char *)malloc(strlen(valueString));
    strcpy(copy, valueString);
    char *c = copy;
    char *e = strchr(copy, ',');
    if (e) {
        *e = '\0';
    }
    size_t s = 0;
    while (c != NULL && s < size) {
        switch (type) {
            case RKValueTypeFloat:
                fv = (float *)valueStorage;
                fv[s] = atof(c);
                break;
            case RKValueTypeDouble:
                fd = (double *)valueStorage;
                fd[s] = atof(c);
                break;
            case RKValueTypeInt32:
                i32v = (int32_t *)valueStorage;
                i32v[s] = (int32_t)atoi(c);
                break;
            case RKValueTypeUInt32:
                u32v = (uint32_t *)valueStorage;
                u32v[s] = (uint32_t)atoi(c);
                break;
            default:
                break;
        }
        s++;
        if (e) {
            c = e + 1;
            if ((e = strchr(c, ',')) != NULL) {
                *e = '\0';
            }
        }
    }
    free(copy);
}

void RKParseQuotedStrings(const char *source, ...) {
    va_list args;
    va_start(args, source);
    
    char *s = (char *)source, *e, q;
    char *string = va_arg(args, char *);
    size_t length = 0;
    
    while (string != NULL) {
        // Look for the beginning quote
        while (*s != '"' && *s != '\'' && *s != '\0') {
            s++;
        }
        if (*s == '\0') {
            break;
        }
        q = *s++;
        // Look for the ending quote
        e = s;
        while (*e != q && *e != q) {
            e++;
        }
        length = (size_t)(e - s);
        strncpy(string, s, length);
        string[length] = '\0';
        s = e + 1;
        // Get the next string
        string = va_arg(args, char *);
    }
}

void RKMakeJSONStringFromControls(char *string, RKControl *controls, uint32_t count) {
    int i, j = 0;
    RKControl *control = controls;
    for (i = 0; i < count; i++) {
        if (control->label[0] == 0) {
            break;
        }
        j += sprintf(string + j, "{\"Label\":\"%s\",\"Command\":\"%s\"}, ", control->label, control->command);
        control++;
    }
    if (j > 2) {
        string[j - 2] = '\0';
    } else {
        string[0] = '\0';
    }
}

// RKStatusEnum zones:
//              x <= tooLow        --> RKStatusEnumCritical
// tooLow     < x <= low           --> RKStatusEnumTooLow
// low        < x <= normalLow     --> RKStatusEnumLow
// normalLow  < x <= normalHigh    --> RKStatusEnumNormal
// normalHigh < x <= high          --> RKStatusEnumHigh
// high       < x <= tooHigh       --> RKStatusEnumTooHigh       
// tooHigh    < x                  --> RKStatusEnumCritical

RKStatusEnum RKValueToEnum(RKConst value, RKConst tlo, RKConst lo, RKConst nlo, RKConst nhi, RKConst hi, RKConst thi) {
    RKStatusEnum status = !isfinite(value) ? RKStatusEnumUnknown :
    (value <= tlo ? RKStatusEnumCritical :
     (value <= lo ? RKStatusEnumTooLow :
      (value <= nlo ? RKStatusEnumLow :
       (value <= nhi ? RKStatusEnumNormal :
        (value <= hi ? RKStatusEnumHigh :
         (value <= thi ? RKStatusEnumTooHigh : RKStatusEnumCritical))))));
    return status;
}

// Typical commercial electronics operates 0 to 70 C
RKStatusEnum RKStatusFromTemperatureForCE(RKConst value) {
    return RKValueToEnum(value, -20.0f, -10.0, 0.0f, 70.0f, 80.0f, 90.0f);
}

// Typical industrial electronics operates -40 to 85 C
RKStatusEnum RKStatusFromTemperatureForIE(RKConst value) {
    return RKValueToEnum(value, -60.0f, -50.0, -40.0f, 85.0f, 95.0f, 105.0f);
}

// Typical computers operates 0 to 25 C
RKStatusEnum  RKStatusFromTemperatureForComputers(RKConst value) {
    return RKValueToEnum(value, -20.0f, -10.0, 0.0f, 25.0f, 26.0f, 27.0f);
}

//
// Examine if any status is critical
// Input:
//     const char *string       - JSON description
//     const bool showEnum      - true or false to show the details
// Output:
//     char *firstKey           - (nullable) the key of first critical value
//     char *firstValue         - (nullable) the object value of the first critical key
//
bool RKAnyCritical(const char *string, const bool showEnum, char *firstKey, char *firstValue) {
    return RKFindCondition(string, RKStatusEnumCritical, showEnum, firstKey, firstValue);
}

//
// Examine if any status is (target)
// Input:
//     const char *string         - JSON description
//     const RKStatusEnum target  - The target RKStatusEnum
//     const bool showEnum        - true or false to show the details
// Output:
//     char *firstKey             - (nullable) the key of first critical value
//     char *firstValue           - (nullable) the object value of the first critical key
//
bool RKFindCondition(const char *string, const RKStatusEnum target, const bool showEnum, char *firstKey, char *firstValue) {
    if (string == NULL || strlen(string) == 0) {
        return false;
    }
    char *str = (char *)malloc(strlen(string) + 1);
    char *key = (char *)malloc(RKNameLength);
    char *obj = (char *)malloc(RKNameLength);
    char *subKey = (char *)malloc(RKNameLength);
    char *subObj = (char *)malloc(RKNameLength);
    uint8_t type;
    uint8_t subType;

    int v;
    char *ks;
    char *sks;
    if (*string != '{') {
        fprintf(stderr, "RKGoThroughKeywords() - Expected '{'.\n");
    }

    strcpy(str, string);

    bool found = false;

    ks = str + 1;
    while (*ks != '\0' && *ks != '}') {
        ks = RKExtractJSON(ks, &type, key, obj);
        if (type != RKJSONObjectTypeObject) {
            continue;
        }
        sks = obj + 1;
        while (*sks != '\0' && *sks != '}') {
            sks = RKExtractJSON(sks, &subType, subKey, subObj);
            if (strcmp("Enum", subKey)) {
                continue;
            }
            v = atoi(subObj);
            if (v == target && found == false) {
                if (firstKey) {
                    strcpy(firstKey, key);
                }
                if (firstValue) {
                    strcpy(firstValue, obj);
                }
                found = true;
            }
            if (showEnum) {
                fprintf(stderr, "%s --> '%s' --> %d%s%s%s\n", key, subObj, v,
                        rkGlobalParameters.showColor ? "\033[38;5;204m" : "",
                        v == RKStatusEnumCritical ? "  *" : "",
                        rkGlobalParameters.showColor ? RKNoColor : "");
            }
        }
    }

    free(subKey);
    free(subObj);
    free(str);
    free(key);
    free(obj);

    return found;
}

#pragma mark - Simple Engine Free

int RKSimpleEngineFree(RKSimpleEngine *engine) {
    if (engine->state & RKEngineStateDeactivating) {
        if (engine->verbose) {
            RKLog("%s Info. Engine is being or has been deactivated.\n", engine->name);
        }
        return RKResultEngineDeactivatedMultipleTimes;
    }
    if (engine->verbose) {
        RKLog("%s Stopping ...\n", engine->name);
    }
    engine->state |= RKEngineStateDeactivating;
    engine->state ^= RKEngineStateActive;
    if (engine->tid) {
        pthread_join(engine->tid, NULL);
    }
    engine->state ^= RKEngineStateDeactivating;
    if (engine->verbose) {
        RKLog("%s Stopped.\n", engine->name);
    }
    if (engine->state != (RKEngineStateAllocated | RKEngineStateProperlyWired)) {
        RKLog("%s Inconsistent state 0x%04x\n", engine->name, engine->state);
    }
    free(engine);
    return RKResultSuccess;
}
