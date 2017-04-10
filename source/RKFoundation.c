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
    int i = 0;
    size_t len;
    va_list args;
    time_t utc;
    struct tm tm;
    char msg[RKMaximumStringLength];

    // Get the time
    time(&utc);
    memcpy(&tm, localtime(&utc), sizeof(struct tm));

    // Construct the string
    va_start(args, whatever);
    if (strlen(whatever) > RKMaximumStringLength - 256) {
        fprintf(stderr, "RKLog() could potential crash for string '%s'\n", whatever);
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
    if (rkGlobalParameters.dailyLog) {
        if (strlen(rkGlobalParameters.rootDataFolder)) {
            i = sprintf(rkGlobalParameters.logfile, "%s/log/%s-", rkGlobalParameters.rootDataFolder, rkGlobalParameters.program);
        } else {
            i = 0;
        }
        i += strftime(rkGlobalParameters.logfile + i, RKNameLength, "%Y%m%d.log", &tm);
    }
    if (strlen(rkGlobalParameters.logfile)) {
        RKPreparePath(rkGlobalParameters.logfile);
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

#pragma mark -

void RKShowTypeSizes(void) {
    RKPulse *pulse = NULL;
    RKRay *ray = NULL;
    FILE *stream = rkGlobalParameters.stream;
    rkGlobalParameters.stream = stdout;
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
    RKLog(">sizeof(RKPosition) = %d", (int)sizeof(RKPosition));
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
        pulse->header.i = (uint64_t)(-1) - slots + i;
        m += pulseSize;
        i++;
    }
    return bytes;
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

#pragma mark -

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
        ray->header.i = (uint64_t)(-1) - slots + i;
        m += raySize;
        i++;
    }
    return bytes;
}

RKRay *RKGetRay(RKRay *ray, const uint32_t k) {
    size_t raySize = RKRayHeaderPaddedSize + RKMaxProductCount * ray->header.capacity * (sizeof(uint8_t) + sizeof(float));
    return (RKRay *)((void *)ray + k * raySize);
}

uint8_t *RKGetUInt8DataFromRay(RKRay *ray, const uint32_t m) {
    void *d = (void *)ray->data;
    return (uint8_t *)(d + m * ray->header.capacity * sizeof(uint8_t));
}

float *RKGetFloatDataFromRay(RKRay *ray, const uint32_t m) {
    void *d = (void *)ray->data;
    d += RKMaxProductCount * ray->header.capacity * sizeof(uint8_t);
    return (float *)(d + m * ray->header.capacity * sizeof(float));
}

#pragma mark -

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
    space->capacity = capacity;
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
    free(space);
}

#pragma mark -

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
