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
    if (rkGlobalParameters.dailyLog) {
        if (whatever[0] == '>') {
            i += sprintf(msg, "         ");
        } else {
            i += strftime(msg, 16, "%T ", &tm);
        }
    } else {
        if (whatever[0] == '>') {
            i += sprintf(msg, "                    ");
        } else {
            i += strftime(msg, 32, "%Y/%m/%d %T ", &tm);
        }
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
        char *colored_whatever = (char *)malloc(RKMaximumPacketSize * sizeof(char));

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
                len += sprintf(colored_whatever + len, RKGreenColor);
            } else if (has_info) {
                len += sprintf(colored_whatever + len, RKSkyBlueColor);
            } else if (has_error) {
                len += sprintf(colored_whatever + len, RKRedColor);
            } else if (has_warning) {
                len += sprintf(colored_whatever + len, RKYellowColor);
            }
        }
        strncpy(colored_whatever + len, anchor, RKMaximumStringLength - len);
        
        i += vsprintf(msg + i, colored_whatever, args);
        
        if (rkGlobalParameters.showColor) {
            sprintf(msg + i, RKNoColor);
        }

        free(colored_whatever);
    } else {
        vsprintf(msg + i, anchor, args);
    }
    
    if (whatever[strlen(whatever) - 1] != '\n') {
        strncat(msg, "\n", RKMaximumStringLength - 1);
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
    if (strlen(folder) > RKMaximumPathLength - 64) {
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
    } else if (strlen(filename) >= RKMaximumPathLength) {
        return 1;
    }
    snprintf(rkGlobalParameters.logfile, RKMaximumPathLength, "%s", filename);
    return RKResultSuccess;
}

int RKSetLogfileToDefault(void) {
    if (strlen(rkGlobalParameters.rootDataFolder)) {
        snprintf(rkGlobalParameters.logfile, RKMaximumPathLength - 1, "%s", RKDefaultLogfile);
    } else {
        snprintf(rkGlobalParameters.logfile, RKMaximumPathLength - 1, "%s/%s", rkGlobalParameters.rootDataFolder, RKDefaultLogfile);
    }
    return RKResultSuccess;
}

#pragma mark - Screen Output

void RKShowTypeSizes(void) {
    SHOW_FUNCTION_NAME
    RKPulse *pulse = NULL;
    RKRay *ray = NULL;
    RKSweep *sweep = NULL;
    
    // Keeep current output stream and temporary change to screen output
    FILE *stream = rkGlobalParameters.stream;
    rkGlobalParameters.stream = stdout;
    
    SHOW_SIZE(void *)
    SHOW_SIZE(int)
    SHOW_SIZE(unsigned int)
    SHOW_SIZE(int8_t)
    SHOW_SIZE(uint8_t)
    SHOW_SIZE(int16_t)
    SHOW_SIZE(uint16_t)
    SHOW_SIZE(int32_t)
    SHOW_SIZE(uint32_t)
    SHOW_SIZE(int64_t)
    SHOW_SIZE(uint64_t)
    SHOW_SIZE(long)
    SHOW_SIZE(long long)
    SHOW_SIZE(unsigned long)
    SHOW_SIZE(unsigned long long)
    SHOW_SIZE(RKByte)
    SHOW_SIZE(RKFloat)
    SHOW_SIZE(RKInt16C)
    SHOW_SIZE(RKComplex)
    SHOW_SIZE(RKRadarDesc)
    SHOW_SIZE(RKConfig)
    SHOW_SIZE(RKHealth)
    SHOW_SIZE(RKNodalHealth)
    SHOW_SIZE(RKPosition)
    SHOW_SIZE(RKPulseHeader)
    SHOW_SIZE(RKPulseParameters)
    SHOW_SIZE_SIMD(pulse->headerBytes)
    SHOW_SIZE_SIMD(RKPulse)
    SHOW_SIZE(RKRayHeader)
    SHOW_SIZE_SIMD(ray->headerBytes)
    SHOW_SIZE(RKSweep)
    SHOW_SIZE(sweep->header)
    SHOW_SIZE(RKScratch)
    SHOW_SIZE(RKFileHeader)
    SHOW_SIZE(RKPreferenceObject)
    SHOW_SIZE(RKControl)
    SHOW_SIZE(RKStatus)
    SHOW_SIZE(RKFileMonitor)
    SHOW_SIZE(RKFilterAnchor)
    SHOW_SIZE(struct sockaddr)
    SHOW_SIZE(struct sockaddr_in)
    SHOW_SIZE(RKWaveform)
    SHOW_SIZE(RKWaveformCalibration)
    SHOW_SIZE(RKProductDesc)
    SHOW_SIZE(RKProductHeader)

    printf("\n");
    
    RKFilterAnchor anchor = RKFilterAnchorDefault;
    printf("RKFilterAnchorDefault:\n");
    printf(".%s\n", RKVariableInString("name", &anchor.name, RKValueTypeUInt32));
    printf(".%s\n", RKVariableInString("origin", &anchor.origin, RKValueTypeUInt32));
    printf(".%s\n", RKVariableInString("length", &anchor.length, RKValueTypeUInt32));
    printf(".%s\n", RKVariableInString("inputOrigin", &anchor.inputOrigin, RKValueTypeUInt32));
    printf(".%s\n", RKVariableInString("outputOrigin", &anchor.outputOrigin, RKValueTypeUInt32));
    printf(".%s\n", RKVariableInString("maxDataLength", &anchor.maxDataLength, RKValueTypeUInt32));
    printf(".%s\n", RKVariableInString("subCarrierFrequency", &anchor.subCarrierFrequency, RKValueTypeFloat));
    printf(".%s dB\n", RKVariableInString("sensitivityGain", &anchor.sensitivityGain, RKValueTypeFloat));
    printf(".%s dB\n", RKVariableInString("filterGain", &anchor.filterGain, RKValueTypeFloat));
    printf(".%s\n", RKVariableInString("fullScale", &anchor.fullScale, RKValueTypeFloat));
    
    printf("\n");
    
    RKFilterAnchor anchor2 = RKFilterAnchorDefaultWithMaxDataLength(1000);
    memcpy(&anchor, &anchor2, sizeof(RKFilterAnchor));
    printf("RKFilterAnchorDefaultWithMaxDataLength(1000):\n");
    printf(".%s\n", RKVariableInString("name", &anchor.name, RKValueTypeUInt32));
    printf(".%s\n", RKVariableInString("origin", &anchor.origin, RKValueTypeUInt32));
    printf(".%s\n", RKVariableInString("length", &anchor.length, RKValueTypeUInt32));
    printf(".%s\n", RKVariableInString("inputOrigin", &anchor.inputOrigin, RKValueTypeUInt32));
    printf(".%s\n", RKVariableInString("outputOrigin", &anchor.outputOrigin, RKValueTypeUInt32));
    printf(".%s\n", RKVariableInString("maxDataLength", &anchor.maxDataLength, RKValueTypeUInt32));
    printf(".%s\n", RKVariableInString("subCarrierFrequency", &anchor.subCarrierFrequency, RKValueTypeFloat));
    printf(".%s dB\n", RKVariableInString("sensitivityGain", &anchor.sensitivityGain, RKValueTypeFloat));
    printf(".%s dB\n", RKVariableInString("filterGain", &anchor.filterGain, RKValueTypeFloat));
    printf(".%s\n", RKVariableInString("fullScale", &anchor.fullScale, RKValueTypeFloat));

    printf("\n");

    int k = 0;
    while (k != RKResultCount) {
        printf(RKLimeColor "%-50s" RKPurpleColor "%2d" RKNoColor "\n", rkResultStrings[k], k);
        k++;
    }
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

static char *arrayHeadTailElementsInString(const float *d, const int length) {
    int c, k = 0;
    static char line[1024];
    for (c = 0; c < 3; c++) {
        k += sprintf(line + k, " %6.2f", *(d + c));
    }
    k += sprintf(line + k, " ...");
    for (c = length - 3; c < length; c++) {
        k += sprintf(line + k, " %6.2f", *(d + c));
    }
    return line;
}

void RKShowArray(const RKFloat *data, const char *letter, const int width, const int height) {
    int j, k = 0;
    char text[1024];
    k = sprintf(text, "         %s%s%s = [ %s ]\n",
                rkGlobalParameters.showColor ? RKYellowColor : "", letter,
                rkGlobalParameters.showColor ? RKNoColor : "",
                arrayHeadTailElementsInString(data, width));
    k += sprintf(text + k, "             [ %s ]\n", arrayHeadTailElementsInString(data + width, width));
    k += sprintf(text + k, "             [ %s ]\n", arrayHeadTailElementsInString(data + 2 * width, width));
    k += sprintf(text + k, "             [     ...\n");
    for (j = height - 3; j < height; j++) {
        k += sprintf(text + k, "             [ %s ]\n", arrayHeadTailElementsInString(data + j * width, width));
    }
    printf("%s", text);
}

char *RKStringFromValue(const void *value, RKValueType type) {
    char    *c = (char *)value;
    float    f = *((float *)value);
    double   d = *((double *)value);
    int      i = *((int *)value);
    long     l = *((long *)value);
    unsigned int u = *((unsigned int *)value);
    unsigned long ul = *((unsigned long *)value);
    int8_t    i8 = *((int8_t *)value);
    uint8_t   u8 = *((uint8_t *)value);
    int16_t  i16 = *((int16_t *)value);
    uint16_t u16 = *((uint16_t *)value);
    int32_t  i32 = *((int32_t *)value);
    uint32_t u32 = *((uint32_t *)value);
    int64_t  i64 = *((int64_t *)value);
    uint64_t u64 = *((uint64_t *)value);
    size_t     s = *((size_t *)value);
    ssize_t   ss = *((ssize_t *)value);
    switch (type) {
        case RKValueTypeInt:
            l = i;
        case RKValueTypeLong:
            c = RKIntegerToCommaStyleString(l);
            break;
        case RKValueTypeInt8:
            i16 = i8;
        case RKValueTypeInt16:
            i32 = i16;
        case RKValueTypeInt32:
            i64 = i32;
        case RKValueTypeInt64:
            c = RKIntegerToCommaStyleString(i64);
            break;
        case RKValueTypeUInt:
            ul = u;
        case RKValueTypeULong:
            c = RKUIntegerToCommaStyleString((unsigned long long)ul);
            break;
        case RKValueTypeUInt8:
            u16 = u8;
        case RKValueTypeUInt16:
            u32 = u16;
        case RKValueTypeUInt32:
            u64 = u32;
        case RKValueTypeUInt64:
            c = RKUIntegerToCommaStyleString((unsigned long long)u64);
            break;
        case RKValueTypeSize:
            c = RKUIntegerToCommaStyleString((unsigned long long)s);
            break;
        case RKValueTypeSSize:
            c = RKIntegerToCommaStyleString((long long)ss);
            break;
        case RKValueTypeFloat:
            d = f;
        case RKValueTypeDouble:
            c = RKFloatToCommaStyleString(d);
            break;
        case RKValueTypeNumericString:
        default:
            break;
    }
    return c;
}

char *RKVariableInString(const char *name, const void *value, RKValueType type) {
    static int ibuf = 0;
    static RKName stringBuffer[16];

    char *string = stringBuffer[ibuf]; string[RKNameLength - 1] = '\0';

    ibuf = ibuf == 15 ? 0 : ibuf + 1;

    char *c = RKStringFromValue(value, type);
    bool b = *((bool *)value);

    if (rkGlobalParameters.showColor) {
        if (type == RKValueTypeBool) {
            snprintf(string, RKNameLength - 1, RKOrangeColor "%s" RKNoColor " = " RKPurpleColor "%s" RKNoColor, name, (b) ? "True" : "False");
        } else if (type == RKValueTypeString) {
            snprintf(string, RKNameLength - 1, RKOrangeColor "%s" RKNoColor " = '" RKSalmonColor "%s" RKNoColor "'", name, c);
        } else {
            snprintf(string, RKNameLength - 1, RKOrangeColor "%s" RKNoColor " = " RKLimeColor "%s" RKNoColor, name, c);
        }
    } else {
        if (type == RKValueTypeBool) {
            snprintf(string, RKNameLength - 1, "%s = %s", name, (b) ? "True" : "False");
        } else if (type == RKValueTypeString) {
            snprintf(string, RKNameLength - 1, "%s = '%s'", name, c);
        } else {
            snprintf(string, RKNameLength - 1, "%s = %s", name, c);
        }
    }
    return string;
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
        RKLog("Error. The total pulse size %s does not conform to SIMD alignment.", RKUIntegerToCommaStyleString(pulseSize));
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
        pulse->header.i = -(uint64_t)slots + i;
        m += pulseSize;
        i++;
    }
    return bytes;
}

void RKPulseBufferFree(RKBuffer mem) {
    return free(mem);
}

// Get a pulse from a pulse buffer
RKPulse *RKGetPulse(RKBuffer buffer, const uint32_t k) {
    RKPulse *pulse = (RKPulse *)buffer;
    size_t headerSize = sizeof(pulse->headerBytes);
    size_t pulseSize = headerSize + 2 * pulse->header.capacity * (sizeof(RKInt16C) + 4 * sizeof(RKFloat));
    return (RKPulse *)(buffer + k * pulseSize);
}

// Get the raw data in 16-bit I/Q from a pulse
RKInt16C *RKGetInt16CDataFromPulse(RKPulse *pulse, const uint32_t c) {
    void *m = (void *)pulse->data;
    return (RKInt16C *)(m + c * pulse->header.capacity * sizeof(RKInt16C));
}

// Get the compressed I/Q data in RKComplex from a pulse
RKComplex *RKGetComplexDataFromPulse(RKPulse *pulse, const uint32_t c) {
    void *m = (void *)pulse->data;
    m += 2 * pulse->header.capacity * sizeof(RKInt16C);
    return (RKComplex *)(m + c * pulse->header.capacity * sizeof(RKComplex));
}

// Get the compressed I/Q data in RKIQZ from a pulse
RKIQZ RKGetSplitComplexDataFromPulse(RKPulse *pulse, const uint32_t c) {
    void *m = (void *)pulse->data;
    m += 2 * pulse->header.capacity * (sizeof(RKInt16C) + sizeof(RKComplex));
    m += c * pulse->header.capacity * 2 * sizeof(RKFloat);
    RKIQZ data = {(RKFloat *)m, (RKFloat *)(m + pulse->header.capacity * sizeof(RKFloat))};
    return data;
}

int RKClearPulseBuffer(RKBuffer buffer, const uint32_t slots) {
    for (uint32_t k = 0; k < slots; k++) {
        RKPulse *pulse = RKGetPulse(buffer, k);
        pulse->header.s = RKPulseStatusVacant;
        pulse->header.i = -(uint64_t)slots + k;
        pulse->header.gateCount = 0;
        memset(pulse->data, 0, 2 * pulse->header.capacity * (sizeof(RKInt16C) + 4 * sizeof(RKFloat)));
    }
    return RKResultSuccess;
}

#pragma mark - Ray

//
// Each slot should have a structure as follows
//
//    RayHeader          header;
//    int8_t             idata[RKBaseMomentCount][capacity];
//    float              fdata[RKBaseMomentCount][capacity];
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
    size_t raySize = headerSize + RKBaseMomentCount * capacity * (sizeof(uint8_t) + sizeof(float));
    if (raySize != (raySize / RKSIMDAlignSize) * RKSIMDAlignSize) {
        RKLog("Error. The total ray size %s does not conform to SIMD alignment.", RKUIntegerToCommaStyleString(raySize));
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
        ray->header.i = -(uint64_t)slots + i;
        m += raySize;
        i++;
    }
    return bytes;
}

void RKRayBufferFree(RKBuffer mem) {
    return free(mem);
}

// Get a ray from a ray buffer
RKRay *RKGetRay(RKBuffer buffer, const uint32_t k) {
    RKRay *ray = (RKRay *)buffer;
    size_t raySize = RKRayHeaderPaddedSize + RKBaseMomentCount * ray->header.capacity * (sizeof(uint8_t) + sizeof(float));
    return (RKRay *)((void *)ray + k * raySize);
}

// Get the data in uint8_t from a ray
uint8_t *RKGetUInt8DataFromRay(RKRay *ray, const RKBaseMomentIndex m) {
    void *d = (void *)ray->data;
    return (uint8_t *)(d + m * ray->header.capacity * sizeof(uint8_t));
}

// Get the data in float from a ray
float *RKGetFloatDataFromRay(RKRay *ray, const RKBaseMomentIndex m) {
    void *d = (void *)ray->data;
    d += RKBaseMomentCount * ray->header.capacity * sizeof(uint8_t);
    return (float *)(d + m * ray->header.capacity * sizeof(float));
}

int RKClearRayBuffer(RKBuffer buffer, const uint32_t slots) {
    for (uint32_t k = 0; k < slots; k++) {
        RKRay *ray = RKGetRay(buffer, k);
        ray->header.s = RKRayStatusVacant;
        ray->header.i = -(uint64_t)slots + k;
        ray->header.gateCount = 0;
        memset(ray->data, 0, RKBaseMomentCount * ray->header.capacity * (sizeof(uint8_t) + sizeof(float)));
    }
    return RKResultSuccess;
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
        memset(space->rcor[k], 0, capacity * sizeof(RKFloat));
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
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->ZDR, RKSIMDAlignSize, capacity * sizeof(RKFloat)));
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->PhiDP, RKSIMDAlignSize, capacity * sizeof(RKFloat)));
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->RhoHV, RKSIMDAlignSize, capacity * sizeof(RKFloat)));
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->KDP, RKSIMDAlignSize, capacity * sizeof(RKFloat)));
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->dcal, RKSIMDAlignSize, capacity * sizeof(RKFloat)));
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&space->pcal, RKSIMDAlignSize, capacity * sizeof(RKFloat)));
    memset(space->dcal, 0, capacity * sizeof(RKFloat));
    memset(space->pcal, 0, capacity * sizeof(RKFloat));

    bytes += 10;
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
    free(space->dcal);
    free(space->pcal);
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

    RKLog("%s Started.   file = %s\n", engine->name, engine->filename);

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
            RKLog("%s File '%s' modified.\n", engine->name, engine->filename);
            if (engine->callbackRoutine) {
                engine->callbackRoutine(engine);
            }
        }
    }
    return NULL;
}

RKFileMonitor *RKFileMonitorInit(const char *filename, void (*routine)(void *), void *userResource) {
    RKFileMonitor *engine = (RKFileMonitor *)malloc(sizeof(RKFileMonitor));
    if (engine == NULL) {
        RKLog("Error allocating a file monitor.\n");
        return NULL;
    }
    memset(engine, 0, sizeof(RKFileMonitor));
    sprintf(engine->name, "%s<UserFileMonitor>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(RKEngineColorMisc) : "",
            rkGlobalParameters.showColor ? RKNoColor : "");
    engine->state = RKEngineStateAllocated | RKEngineStateProperlyWired | RKEngineStateActivating;
    engine->memoryUsage = sizeof(RKFileMonitor);
    strncpy(engine->filename, filename, RKMaximumPathLength);
    engine->callbackRoutine = routine;
    engine->userResource = userResource;
    RKLog("%s Starting ...\n", engine->name);
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
            case '0':
                flag = (flag & !RKStreamStatusMask) | RKStreamStatusPositions;
                break;
            case '1':
                flag = (flag & !RKStreamStatusMask) | RKStreamStatusPulses;
                break;
            case '2':
                flag = (flag & !RKStreamStatusMask) | RKStreamStatusRays;
                break;
            case '3':
                flag = (flag & !RKStreamStatusMask) | RKStreamStatusIngest;
                break;
            case '4':
                flag = (flag & !RKStreamStatusMask) | RKStreamStatusEngines;
                break;
            case '5':
                flag = (flag & !RKStreamStatusMask) | RKStreamStatusBuffers;
                break;
            case '!':
                flag |= RKStreamStatusProcessorStatus;
                break;
            case 'h':
                flag |= RKStreamHealthInJSON;
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
    // Exclusive part from RKStreamStatusMask
    if (stream & RKStreamStatusPulses) {
        j += sprintf(string + j, "1");
    } else if (stream & RKStreamStatusRays) {
        j += sprintf(string + j, "2");
    } else if (stream & RKStreamStatusPositions) {
        j += sprintf(string + j, "3");
    } else if (stream & RKStreamStatusEngines) {
        j += sprintf(string + j, "4");
    } else if (stream & RKStreamStatusBuffers) {
        j += sprintf(string + j, "5");
    }
    if (stream & RKStreamStatusProcessorStatus) { j += sprintf(string + j, "!"); }
    if (stream & RKStreamHealthInJSON)          { j += sprintf(string + j, "h"); }
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
    static char string[RKNameLength];
    RKStringFromStream(string, stream);
    return string;
}

int RKGetNextProductDescription(char *symbol, char *name, char *unit, char *colormap, RKBaseMomentIndex *index, RKBaseMomentList *list) {
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
    RKBaseMomentList baseMoments[] = {
        RKBaseMomentListProductZ,
        RKBaseMomentListProductV,
        RKBaseMomentListProductW,
        RKBaseMomentListProductD,
        RKBaseMomentListProductP,
        RKBaseMomentListProductR,
        RKBaseMomentListProductK,
        RKBaseMomentListProductSh,
        RKBaseMomentListProductSv,
        0xFFFF
    };
    RKBaseMomentIndex baseMomentIndices[] = {
        RKBaseMomentIndexZ,
        RKBaseMomentIndexV,
        RKBaseMomentIndexW,
        RKBaseMomentIndexD,
        RKBaseMomentIndexP,
        RKBaseMomentIndexR,
        RKBaseMomentIndexK,
        RKBaseMomentIndexSh,
        0
    };
    int k = -1;
    if (*list & RKBaseMomentListProductZ) {
        k = 0;
    } else if (*list & RKBaseMomentListProductV) {
        k = 1;
    } else if (*list & RKBaseMomentListProductW) {
        k = 2;
    } else if (*list & RKBaseMomentListProductD) {
        k = 3;
    } else if (*list & RKBaseMomentListProductP) {
        k = 4;
    } else if (*list & RKBaseMomentListProductR) {
        k = 5;
    } else if (*list & RKBaseMomentListProductK) {
        k = 6;
    } else if (*list & RKBaseMomentListProductSh) {
        k = 7;
    } else if (*list & RKBaseMomentListProductSv) {
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
        *index = baseMomentIndices[k];
    }
    *list ^= baseMoments[k];
    return RKResultSuccess;
}

#pragma mark - JSON Stuff

size_t RKParseCommaDelimitedValues(void *valueStorage, RKValueType type, const size_t size, const char *valueString) {
    float *fv = (float *)valueStorage;
    double *dv = (double *)valueStorage;
    int32_t *i32v = (int32_t *)valueStorage;
    uint32_t *u32v = (uint32_t *)valueStorage;
    int64_t *i64v = (int64_t *)valueStorage;
    uint64_t *u64v = (uint64_t *)valueStorage;
    char *copy = (char *)malloc(strlen(valueString));
    strcpy(copy, valueString);
    char *c = copy;
    char *e;
    if ((e = strchr(copy, ',')) != NULL) {
        *e = '\0';
    }
    size_t s = 0;
    while (*c != '\0' && s < size) {
        switch (type) {
            case RKValueTypeFloat:
                fv[s] = (RKFloat)atof(c);
                break;
            case RKValueTypeDouble:
                dv[s] = atof(c);
                break;
            case RKValueTypeInt32:
                i32v[s] = (int32_t)atoi(c);
                break;
            case RKValueTypeUInt32:
                u32v[s] = (uint32_t)atoi(c);
                break;
            case RKValueTypeInt64:
                i64v[s] = (int64_t)atol(c);
                break;
            case RKValueTypeUInt64:
                u64v[s] = (uint64_t)atol(c);
                break;
            default:
                break;
        }
        //printf("s = %d   c = %s @ %16p / %16p\n", (int)s, c == NULL ? "(NULL)" : c, c, e);
        s++;
        if (e) {
            c = e + 1;
            if ((e = strchr(c, ',')) != NULL) {
                *e = '\0';
            }
        } else {
            break;
        }
    }
    free(copy);
    return s;
}

size_t RKParseNumericArray(void *valueStorage, RKValueType type, const size_t size, const char *valueString) {
    size_t k = 0;
    char *s, *e;
    if ((s = strchr(valueString, '[')) == NULL) {
        return RKParseCommaDelimitedValues(valueStorage, type, size, valueString);
    }
    if ((e = strchr(valueString, ']')) == NULL) {
        fprintf(stderr, "Expected a close bracket for an array that begins with '['.\n");
        return 0;
    }
    char *copy = (char *)malloc(e - s + 1);
    memcpy(copy, s + 1, e - s - 1);
    *(copy + (e - s - 1)) = '\0';
    //printf("copy = %s\n", copy);
    k = RKParseCommaDelimitedValues(valueStorage, type, size, copy);
    free(copy);
    return k;
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
        j += sprintf(string + j, "{\"Label\": \"%s\", \"Command\": \"%s\"}, ", control->label, control->command);
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
        fprintf(stderr, "RKFindCondition() - Expected '{'.\n");
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

int RKParseProductDescription(RKProductDesc *desc, const char *inputString) {
    size_t k;
    char *keyString;

    memset(desc, 0, sizeof(RKProductDesc));

    // Product name is mandatory
    keyString = RKGetValueOfKey(inputString, "name");
    if (keyString) {
        strncpy(desc->name, keyString, RKNameLength - 1);
    } else {
        return RKResultIncompleteProductDescription;
    }
    // Product symbol is mandatory
    keyString = RKGetValueOfKey(inputString, "symbol");
    if (keyString) {
        strncpy(desc->symbol, keyString, 7);
    } else {
        return RKResultIncompleteProductDescription;
    }
    // Piece count can be assumed to be 1 if not supplied
    keyString = RKGetValueOfKey(inputString, "PieceCount");
    if (keyString) {
        desc->pieceCount = atoi(keyString);
        if (desc->pieceCount > 8) {
            desc->pieceCount = 8;
            fprintf(stderr, "User product piece count truncated to 8.\n");
        }
    } else {
        desc->pieceCount = 1;
    }
    // The bias term, b, for 8-bit conversion must be supplied
    keyString = RKGetValueOfKey(inputString, "b");
    if (keyString) {
        k = RKParseNumericArray(desc->b, RKValueTypeFloat, desc->pieceCount, keyString);
        if (k != desc->pieceCount) {
            fprintf(stderr, "Parsed %zu values but %u is expected (desc->b).\n", k, desc->pieceCount);
        }
    } else {
        return RKResultIncompleteProductDescription;
    }
    // The weight term, w, for 8-bit conversion must be supplied
    keyString = RKGetValueOfKey(inputString, "w");
    if (keyString) {
        k = RKParseNumericArray(desc->w, RKValueTypeFloat, desc->pieceCount, keyString);
        if (k != desc->pieceCount) {
            fprintf(stderr, "Parsed %zu values but %u is expected (desc->w).\n", k, desc->pieceCount);
        }
    } else {
        return RKResultIncompleteProductDescription;
    }
    // Optional values
    keyString = RKGetValueOfKey(inputString, "unit");
    if (keyString) {
        strncpy(desc->unit, keyString, 5);
    }
    keyString = RKGetValueOfKey(inputString, "colormap");
    if (keyString) {
        strncpy(desc->colormap, keyString, 9);
    }
    keyString = RKGetValueOfKey(inputString, "minimumValue");
    if (keyString) {
        desc->mininimumValue = (RKFloat)atof(keyString);
    }
    keyString = RKGetValueOfKey(inputString, "maximumValue");
    if (keyString) {
        desc->mininimumValue = (RKFloat)atof(keyString);
    }
    return RKResultSuccess;
}

RKProductId RKProductIdFromString(const char *string) {
    return (RKProductId)strtoul(string, NULL, 10);
}

RKIdentifier RKIdentifierFromString(const char *string) {
    return (RKIdentifier)strtouq(string, NULL, 10);
}

#pragma mark - Simple Engine Free

int RKSimpleEngineFree(RKSimpleEngine *engine) {
    if (engine->state & RKEngineStateDeactivating) {
        if (engine->verbose) {
            RKLog("%s Info. Engine is being or has been deactivated.\n", engine->name);
        }
        return RKResultEngineDeactivatedMultipleTimes;
    }
    RKLog("%s Stopping ...\n", engine->name);
    engine->state |= RKEngineStateDeactivating;
    engine->state ^= RKEngineStateActive;
    if (engine->tid) {
        pthread_join(engine->tid, NULL);
    }
    engine->state ^= RKEngineStateDeactivating;
    RKLog("%s Stopped.\n", engine->name);
    if (engine->state != (RKEngineStateAllocated | RKEngineStateProperlyWired)) {
        RKLog("%s Inconsistent state 0x%04x\n", engine->name, engine->state);
    }
    free(engine);
    return RKResultSuccess;
}

