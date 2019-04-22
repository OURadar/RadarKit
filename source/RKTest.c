//
//  RKTest.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 10/25/16.
//  Copyright (c) 2016 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKTest.h>
#include <getopt.h>

#define RKFMT                               "%5d"
#define RKSIMD_TEST_DESC_FORMAT             "%65s"
#define RKSIMD_TEST_TIME_FORMAT             "%0.4f"
#define RKSIMD_TEST_RESULT(clr, str, res)   clr ? \
    printf(RKSIMD_TEST_DESC_FORMAT " : %s.\033[0m\n", str, res ? "\033[32msuccessful" : "\033[31mfailed") : \
    printf(RKSIMD_TEST_DESC_FORMAT " : %s.\n", str, res ? "successful" : "failed");
#define OXSTR(x)                       x ? "\033[32mo\033[0m" : "\033[31mx\033[0m"
#define PEDESTAL_SAMPLING_TIME         0.01
#define HEALTH_RELAY_SAMPLING_TIME     0.1

#define TEST_RESULT(clr, str, res)   clr ? \
printf("%s %s\033[0m\n", str, res ? "\033[32mokay" : "\033[31mtoo high") : \
printf("%s %s\n", str, res ? "okay" : "too high");

// Make some private functions available

int prepareScratch(RKScratch *);
int makeRayFromScratch(RKScratch *, RKRay *);
size_t RKScratchAlloc(RKScratch **space, const uint32_t capacity, const uint8_t lagCount, const uint8_t fftOrder, const bool);
void RKScratchFree(RKScratch *);

#pragma mark - Static Functions

static void RKTestCallback(void *in) {
    RKFileMonitor *engine = (RKFileMonitor *)in;
    RKLog("%s I am a callback function.\n", engine->name);
}

#pragma mark - Test Wrapper and Help Text

char *RKTestByNumberDescription(const int indent) {
    static char text[4096];
    char helpText[] =
    " 0 - Show types\n"
    " 1 - Show colors\n"
    " 2 - Test pretty strings\n"
    " 3 - Test modulo-math macros\n"
    " 4 - Test parsing comma delimited values\n"
    " 5 - Test parsing values in a JSON string\n"
    " 6 - Test initializing a file manager - RKFileManagerInit()\n"
    " 7 - Test reading a preference file - RKPreferenceInit()\n"
    " 8 - Test counting files using RKCountFilesInPath()\n"
    " 9 - Test the file monitor module - RKFileMonitor()\n"
    "10 - Test the internet monitor module - RKHostMonitorInit()\n"
    "11 - Test initializing a radar system - RKRadarInit()\n"
    "12 - Test converting a temperature reading to status\n"
    "13 - Test getting a country name from position\n"
    "14 - Test generating text for buffer overview\n"
    "15 - Test reading a netcdf file using RKSweepRead(); -T15 FILENAME\n"
    "16 - Test reading a netcdf file using RKProductRead(); -T16 FILENAME\n"
    "17 - Test reading multiple netcdf files using RKProductCollectionInitWithFilename()\n"
    "18 - Test writing a netcdf file using RKProductFileWriterNC()\n"
    "\n"
    "20 - SIMD quick test\n"
    "21 - SIMD test with numbers shown\n"
    "22 - Show window types\n"
    "23 - Hilbert transform\n"
    "24 - Optimize FFT performance and generate an fft-wisdom file\n"
    "25 - Show ring filter coefficients\n"
    "\n"
    "30 - Make a frequency hopping sequence\n"
    "31 - Make a TFM waveform\n"
    "32 - Generate a waveform file\n"
    "33 - Test waveform down-sampling\n"
    "34 - Test showing built-in waveform properties\n"
    "35 - Test showing waveform properties; -T35 WAVEFORM_FILE\n"
    "36 - Test generating a hopping chirp\n"
    "\n"
    "40 - Pulse compression using simple cases\n"
    "41 - Calculating one ray using the Pulse Pair method\n"
    "42 - Calculating one ray using the Pulse Pair Hop method\n"
    "43 - Calculating one ray using the Multi-Lag method with L = 2\n"
    "44 - Calculating one ray using the Multt-Lag method with L = 3\n"
    "45 - Calculating one ray using the Multi-Lag method with L = 4\n"
    "46 - Calculating one ray using the Spectral Moment method\n"
    "\n"
    "50 - Measure the speed of SIMD calculations\n"
    "51 - Measure the speed of pulse compression\n"
    "52 - Measure the speed of various moment methods\n"
    "53 - Measure the speed of cached write\n";
    RKIndentCopy(text, helpText, indent);
    if (strlen(text) > 3000) {
        fprintf(stderr, "Warning. Approaching limit. (%lu)\n", strlen(text));
    }
    return text;
}

void RKTestByNumber(const int number, const void *arg) {
    RKSetWantScreenOutput(true);
    switch (number) {
        case 0:
            RKShowTypeSizes();
            printf("\n");
            RKNetworkShowPacketTypeNumbers();
            break;
        case 1:
            RKTestTerminalColors();
            break;
        case 2:
            RKTestPrettyStrings();
            break;
        case 3:
            RKTestModuloMath();
            break;
        case 4:
            RKTestParseCommaDelimitedValues();
            break;
        case 5:
            RKTestParseJSONString();
            break;
        case 6:
            RKTestFileManager();
            break;
        case 7:
            RKTestPreferenceReading();
            break;
        case 8:
            RKTestCountFiles();
            break;
        case 9:
            RKTestFileMonitor();
            break;
        case 10:
            RKTestHostMonitor();
            break;
        case 11:
            RKTestInitializingRadar();
            break;
        case 12:
            RKTestTemperatureToStatus();
            break;
        case 13:
            RKTestGetCountry();
            break;
        case 14:
            RKTestBufferOverviewText((char *)arg);
            break;
        case 15:
            if (arg == NULL) {
                RKLog("No filename given.\n");
                exit(EXIT_FAILURE);
            }
            RKTestSweepRead((char *)arg);
            break;
        case 16:
            if (arg == NULL) {
                RKLog("No filename given.\n");
                exit(EXIT_FAILURE);
            }
            RKTestProductRead((char *)arg);
            break;
        case 17:
            if (arg == NULL) {
                RKLog("No filename given.\n");
                exit(EXIT_FAILURE);
            }
            RKProductCollectionInitWithFilename((char *)arg);
            break;
        case 18:
            RKTestProductWrite();
            break;
        case 20:
            RKTestSIMD(RKTestSIMDFlagNull);
            break;
        case 21:
            RKTestSIMD(RKTestSIMDFlagShowNumbers);
            break;
        case 22:
            RKTestWindow();
            break;
        case 23:
            RKTestHilbertTransform();
            break;
        case 24:
            RKTestWriteFFTWisdom();
            break;
        case 25:
            RKTestRingFilterShowCoefficients();
            break;
        case 30:
            RKTestMakeHops();
            break;
        case 31:
            RKTestWaveformTFM();
            break;
        case 32:
            RKTestWaveformWrite();
            break;
        case 33:
            RKTestWaveformDownsampling();
            break;
        case 34:
            RKTestWaveformShowProperties();
            break;
        case 35:
            if (arg == NULL) {
                RKLog("No filename given.\n");
                exit(EXIT_FAILURE);
            }
            RKTestWaveformShowUserWaveformProperties((char *)arg);
            break;
        case 36:
            RKTestWaveformHoppingChirp();
            break;
        case 40:
            RKTestPulseCompression(RKTestFlagVerbose | RKTestFlagShowResults);
            break;
        case 41:
            RKTestOneRay(RKPulsePair, 0);
            break;
        case 42:
            RKTestOneRay(RKPulsePairHop, 0);
            break;
        case 43:
            RKTestOneRay(RKMultiLag, 2);
            break;
        case 44:
            RKTestOneRay(RKMultiLag, 3);
            break;
        case 45:
            RKTestOneRay(RKMultiLag, 4);
            break;
        case 46:
            RKTestOneRay(RKSpectralMoment, 0);
            break;
        case 50:
            RKTestSIMD(RKTestSIMDFlagPerformanceTestAll);
            break;
        case 51:
            RKTestPulseCompressionSpeed();
            break;
        case 52:
            RKTestMomentProcessorSpeed();
            break;
        case 53:
            RKTestCacheWrite();
            break;
        case 60:
            RKTestExperiment();
            break;
        default:
            RKLog("Test %d is invalid.\n", number);
            break;
    }
}

#pragma mark - Fundamental Functions

void RKTestTerminalColors(void) {
    SHOW_FUNCTION_NAME
    for (int k = 0; k < 17; k++) {
        printf("%s<BackgroundColor %2d>%s    %s<Color %2d>%s\n", RKGetBackgroundColorOfIndex(k), k, RKNoColor, RKGetColorOfIndex(k), k, RKNoColor);
    }
    printf("\n");
    int c;
    for (int i = 0; i < 6; i++) {
        for (int k = 0; k < 6; k++) {
            for (int j = 0; j < 6; j++) {
                c = k * 100 + j * 10 + i;
                printf("%s %03d %s", RKGetBackgroundColorOfCubeIndex(c), c, RKNoColor);
            }
            printf("\n");
        }
        printf("\n");
    }
}

void RKTestPrettyStrings(void) {
    SHOW_FUNCTION_NAME
    int k;
    float f;
    printf("\n");
    for (k = 1020; k < (1 << 30); k += k ^ 2) {
        printf("k = %11d -> %s\n", k, RKIntegerToCommaStyleString(k));
    }
    f = +INFINITY; printf("f = %11.2f -> %s\n", f, RKFloatToCommaStyleString(f));
    f = -INFINITY; printf("f = %11.2f -> %s\n", f, RKFloatToCommaStyleString(f));
    f = NAN;       printf("f = %11.2f -> %s\n", f, RKFloatToCommaStyleString(f));
    printf("\n");
    char *c;
    bool tf = true; printf("%s\n", RKVariableInString("tf", &tf, RKValueTypeBool));
    int8_t i8 = 100; printf("%s\n", RKVariableInString("i8", &i8, RKValueTypeInt8));
    uint8_t u8 = 100; printf("%s\n", RKVariableInString("u8", &u8, RKValueTypeUInt8));
    int16_t i16 = 100; printf("%s\n", RKVariableInString("i16", &i16, RKValueTypeInt16));
    uint16_t u16 = 100; printf("%s\n", RKVariableInString("u16", &u16, RKValueTypeUInt16));
    int32_t i32 = 100; printf("%s\n", RKVariableInString("i32", &i32, RKValueTypeInt32));
    uint64_t i64 = 1234567;
    c = RKVariableInString("u64", RKIntegerToCommaStyleString(i64), RKValueTypeNumericString);
    printf("%s (len = %zu)\n", c, strlen(c));
    char name[] = "RadarKit";
    c = RKVariableInString("name", name, RKValueTypeString);
    printf("%s (len = %zu)\n", c, strlen(c));
    i32 = 0x303f;
    c = RKVariableInString("mask", &i32, RKValueTypeInt32InHex);
    printf("%s (len = %zu)\n", c, strlen(c));
}

void RKTestModuloMath(void) {
    SHOW_FUNCTION_NAME
    int k;
    const int N = 4;

    RKLog("Test with SlotCount = %d, N = %d\n", RKBuffer0SlotCount, N);
    k = 0;                      RKLog("k = " RKFMT " --> Next N = " RKFMT "\n", k, RKNextNModuloS(k, N, RKBuffer0SlotCount));
    k = RKBuffer0SlotCount - 4; RKLog("k = " RKFMT " --> Next N = " RKFMT "\n", k, RKNextNModuloS(k, N, RKBuffer0SlotCount));
    k = RKBuffer0SlotCount - 3; RKLog("k = " RKFMT " --> Next N = " RKFMT "\n", k, RKNextNModuloS(k, N, RKBuffer0SlotCount));
    k = RKBuffer0SlotCount - 2; RKLog("k = " RKFMT " --> Next N = " RKFMT "\n", k, RKNextNModuloS(k, N, RKBuffer0SlotCount));
    k = RKBuffer0SlotCount - 1; RKLog("k = " RKFMT " --> Next N = " RKFMT "\n", k, RKNextNModuloS(k, N, RKBuffer0SlotCount));
    k = RKBuffer0SlotCount - 1; RKLog("k = " RKFMT " --> Prev N = " RKFMT "\n", k, RKPreviousNModuloS(k, N, RKBuffer0SlotCount));
    k = 0;                      RKLog("k = " RKFMT " --> Prev N = " RKFMT "\n", k, RKPreviousNModuloS(k, N, RKBuffer0SlotCount));
    k = 1;                      RKLog("k = " RKFMT " --> Prev N = " RKFMT "\n", k, RKPreviousNModuloS(k, N, RKBuffer0SlotCount));
    k = 2;                      RKLog("k = " RKFMT " --> Prev N = " RKFMT "\n", k, RKPreviousNModuloS(k, N, RKBuffer0SlotCount));
    k = 3;                      RKLog("k = " RKFMT " --> Prev N = " RKFMT "\n", k, RKPreviousNModuloS(k, N, RKBuffer0SlotCount));
    k = 4;                      RKLog("k = " RKFMT " --> Prev N = " RKFMT "\n", k, RKPreviousNModuloS(k, N, RKBuffer0SlotCount));
    k = 4899;                   RKLog("k = " RKFMT " --> Next N = " RKFMT "\n", k, RKNextNModuloS(k, 100 - 1, RKBuffer0SlotCount));

    struct timeval t0, t1;
    gettimeofday(&t1, NULL); t1.tv_sec -= 1;
    gettimeofday(&t0, NULL);
    if (RKTimevalDiff(t0, t1) < 0.1) {
        RKLog("First iteraction failed.\n");
    } else {
        RKLog("First iteraction is as expected.\n");
    }
}

void RKTestParseCommaDelimitedValues(void) {
    SHOW_FUNCTION_NAME
    char string[] = "1200,2000, 3000,  5000";
    float v[4];
    int32_t i[4];
    RKParseCommaDelimitedValues(v, RKValueTypeFloat, 4, string);
    RKLog("%s -> %.2f %.2f %.2f %.2f\n", string, v[0], v[1], v[2], v[3]);
    RKParseCommaDelimitedValues(i, RKValueTypeInt32, 4, string);
    RKLog("%s -> %d %d %d %d\n", string, i[0], i[1], i[2], i[3]);
}

void RKTestParseJSONString(void) {
    SHOW_FUNCTION_NAME
    char string[] = "{"
    "\"Transceiver\":{\"Value\":true,\"Enum\":0}, "
    "\"Pedestal\":{\"Value\":true,\"Enum\":0}, "
    "\"Health Relay\":{\"Value\":true,\"Enum\":0}, "
    "\"Network\":{\"Value\":true,\"Enum\":0}, "
    "\"Recorder (Coming Soon)\":{\"Value\":true,\"Enum\":3}, "
    "\"10-MHz Clock\":{\"Value\":true,\"Enum\":0}, "
    "\"DAC PLL\":{\"Value\":true,\"Enum\":0}, "
    "\"FPGA Temp\":{\"Value\":\"69.3degC\",\"Enum\":0}, "
    "\"Core Volt\":{\"Value\":\"1.00 V\",\"Enum\":4}, "
    "\"Aux. Volt\":{\"Value\":\"2.469 V\",\"Enum\":0}, "
    "\"XMC Volt\":{\"Value\":\"11.649 V\",\"Enum\":0}, "
    "\"XMC 3p3\":{\"Value\":\"3.250 V\",\"Enum\":0}, "
    "\"PRF\":{\"Value\":\"5,008 Hz\",\"Enum\":0,\"Target\":\"5,000 Hz\"}, "
    "\"Transmit H\":{\"Value\":\"69.706 dBm\",\"Enum\":0,\"MaxIndex\":2,\"Max\":\"-1.877 dBm\",\"Min\":\"-2.945 dBm\"}, "
    "\"Transmit V\":{\"Value\":\"69.297 dBm\",\"Enum\":0,\"MaxIndex\":2,\"Max\":\"-2.225 dBm\",\"Min\":\"-3.076 dBm\"}, "
    "\"DAC QI\":{\"Value\":\"0.913\",\"Enum\":0}, "
    "\"Waveform\":{\"Value\":\"h4011\",\"Enum\":0}, "
    "\"UnderOver\":[0,-897570], "
    "\"Lags\":[-139171212,-139171220,-159052813], \"NULL\":[149970], "
    "\"Pedestal AZ Interlock\":{\"Value\":true,\"Enum\":0}, "
    "\"Pedestal EL Interlock\":{\"Value\":true,\"Enum\":0}, "
    "\"VCP Active\":{\"Value\":true,\"Enum\":0}, "
    "\"Pedestal AZ Position\":{\"Value\":\"26.21 deg\",\"Enum\":0}, "
    "\"Pedestal EL Position\":{\"Value\":\"2.97 deg\",\"Enum\":0}, "
    "\"TWT Power\":{\"Value\":true,\"Enum\":0}, "
    "\"TWT Warmed Up\":{\"Value\":true,\"Enum\":0}, "
    "\"TWT High Voltage\":{\"Value\":true,\"Enum\":0}, "
    "\"TWT Full Power\":{\"Value\":true,\"Enum\":0}, "
    "\"TWT VSWR\":{\"Value\":true,\"Enum\":0}, "
    "\"TWT Duty Cycle\":{\"Value\":true,\"Enum\":0}, "
    "\"TWT Fans\":{\"Value\":true,\"Enum\":0}, "
    "\"TWT Interlock\":{\"Value\":true,\"Enum\":0}, "
    "\"TWT Faults Clear\":{\"Value\":true,\"Enum\":0}, "
    "\"TWT Cathode Voltage\":{\"Value\":\"-21.54 kV\",\"Enum\":0}, "
    "\"TWT Body Current\":{\"Value\":\"0.09 A\",\"Enum\":0}, "
    "\"GPS Valid\":{\"Value\":true,\"Enum\":0}, "
    "\"GPS Latitude\":{\"Value\":\"35.1812820\",\"Enum\":0}, "
    "\"GPS Longitude\":{\"Value\":\"-97.4373016\",\"Enum\":0}, "
    "\"GPS Heading\":{\"Value\":\"88.0 deg\", \"Enum\":0}, "
    "\"Ground Speed\":{\"Value\":\"0.30 km/h\", \"Enum\":0}, "
    "\"Platform Pitch\":{\"Value\":\"-0.23 deg\",\"Enum\":0}, "
    "\"Platform Roll\":{\"Value\":\"0.04 deg\",\"Enum\":0}, "
    "\"I2C Chip\":{\"Value\":\"30.50 degC\",\"Enum\":0}, "
    "\"Event\":\"none\", \"Log Time\":1493410480"
    "}";
    printf("%s (%d characters)\n\n", string, (int)strlen(string));

    // Test getting a specific key
    printf("Getting specific key:\n");
    printf("---------------------\n");
    char *stringObject, *stringValue, *stringEnum;
    if ((stringObject = RKGetValueOfKey(string, "latitude")) != NULL) {
        printf("stringObject = '%s'\n", stringObject);
        stringValue = RKGetValueOfKey(stringObject, "value");
        stringEnum = RKGetValueOfKey(stringObject, "enum");
        if (stringValue != NULL && stringEnum != NULL && atoi(stringEnum) == RKStatusEnumNormal) {
            printf("latitude = %.7f\n", atof(stringValue));
        }
    }
    printf("\n");
    if ((stringObject = RKGetValueOfKey(string, "longitude")) != NULL) {
        printf("stringObject = '%s'\n", stringObject);
        stringValue = RKGetValueOfKey(stringObject, "value");
        stringEnum = RKGetValueOfKey(stringObject, "enum");
        if (stringValue != NULL && stringEnum != NULL && atoi(stringEnum) == RKStatusEnumNormal) {
            printf("longitude = %.7f\n", atof(stringValue));
        }
    }

    printf("\n");
    printf("Getting all keys:\n");
    printf("-----------------\n");
    char criticalKey[RKNameLength];
    char criticalValue[RKNameLength];
    bool anyCritical = RKAnyCritical(string, true, criticalKey, criticalValue);
    printf("anyCritical = %s%s%s%s%s\n",
           rkGlobalParameters.showColor ? "\033[38;5;207m" : "",
           anyCritical ? "true" : "false",
           rkGlobalParameters.showColor ? RKNoColor : "",
           anyCritical ? " --> " : "",
           anyCritical ? criticalKey : ""
           );

    printf("\n");
    char strObj[] = "0";
    stringObject = strObj;
    printf("stringObject = '%s'\n", stringObject);
    stringValue = RKGetValueOfKey(stringObject, "value");
    stringEnum = RKGetValueOfKey(stringObject, "enum");
    printf("stringValue = %s\n", stringValue == NULL ? "(NULL)" : stringValue);
    printf("stringEnum = %s\n", stringEnum == NULL ? "(NULL)" : stringEnum);
    if (stringValue != NULL && stringEnum != NULL && atoi(stringEnum) == RKStatusEnumNormal) {
        printf("longitude = %.7f\n", atof(stringValue));
    }

    printf("\n===\n\n");
    
    int k;
    size_t s;
    const int N = 8;
    float *nums = (float *)malloc(N * sizeof(float));
    sprintf(string, "{'name':'U', 'PieceCount': 1, 'b':-32, 'w':[0.5, 0.6]}");
    printf("%s (%d characters)\n\n", string, (int)strlen(string));

    stringObject = RKGetValueOfKey(string, "name");
    printf("name = %s\n", stringObject);
    stringObject = RKGetValueOfKey(string, "PieceCount");
    printf("Piece Count = %s\n", stringObject);

    stringObject = RKGetValueOfKey(string, "b");
    printf("b = %s -->", stringObject);
    s = RKParseNumericArray(nums, RKValueTypeFloat, N, stringObject);
    for (k = 0; k < s; k++) {
        printf(" %.2f", nums[k]);
    }
    printf(" (count = %zu)\n", s);

    stringObject = RKGetValueOfKey(string, "w");
    printf("w = %s --> ", stringObject);
    s = RKParseNumericArray(nums, RKValueTypeFloat, N, stringObject);
    for (k = 0; k < s; k++) {
        printf(" %.2f", nums[k]);
    }
    printf(" (count = %zu)\n", s);

    free(nums);

    printf("\n===\n\n");

    sprintf(string, "{"
            "\"Health\":{\"Value\":true,\"Enum\":1}, "
            "\"Transceiver\":{\"Value\":true,\"Enum\":1}, "
            "\"GPS Latitude\":{\"Value\":\"+35.0 deg\",\"Enum\":1}, "
            "\"GPS Longitude\":{\"Value\":\"-97.0 deg\",\"Enum\":1}, "
            "}");
    printf("string = %s\n", string);
    RKReplaceEnumOfKey(string, "Health", RKStatusEnumOld);
    printf("string = %s\n", string);
    RKReplaceAllValuesOfKey(string, "Enum", RKStatusEnumOld);
    printf("string = %s\n", string);

    printf("\n===\n\n");
    sprintf(string, "{"
            "\"Health\":{\"Value\":\"true\",\"Enum\":1}, "
            "\"Transceiver\":{\"Value\":\"False\",\"Enum\":1}, "
            "}");
    printf("string = %s\n", string);
    RKReviseLogicalValues(string);
    printf("string = %s\n", string);
}

void RKTestFileManager(void) {
    SHOW_FUNCTION_NAME
    int e, j, k, s;
    RKName filename;
    struct timeval tm;
    gettimeofday(&tm, NULL);
    time_t startTime = tm.tv_sec;
    float es[] = {2.0f, 4.0f, 6.0f, 8.0f};
    const int ne = sizeof(es) / sizeof(float);
    RKName ss[] = {"Z", "V", "D", "R"};
    const int ns = sizeof(ss) / sizeof(RKName);
    FILE *fid;
    size_t filesize = 1024 * 1024;
    char *payload = (char *)malloc(filesize);
    if (payload == NULL) {
        fprintf(stderr, "Unable to allocate payload.\n");
        exit(EXIT_FAILURE);
    }
    
    const char dataPath[] = "data";
    RKFileManager *fileManager = RKFileManagerInit();
    RKFileManagerSetPathToMonitor(fileManager, dataPath);
    RKFileManagerSetDiskUsageLimit(fileManager, 200 * 1024 * 1024);
    RKFileManagerSetVerbose(fileManager, 3);
    
    RKLog("Warning. All data will be erased!\n");
    printf("Press Enter to continue ... or Ctrl-C to terminate.\n");
    
    if (system("rm -rf data/moment") == -1) {
        RKLog("Error. Failed during system().\n");
    }
    
    getchar();
    
    RKFileManagerStart(fileManager);
    
    e = 0;
    for (j = 0; j < 50; j++) {
        startTime += 3000;
        k = sprintf(filename, "%s/%s/", dataPath, RKDataFolderMoment);
        k += strftime(filename + k, 9, "%Y%m%d", gmtime(&startTime));
        k += sprintf(filename + k, "/RK-");
        k += strftime(filename + k, 16, "%Y%m%d-%H%M%S", gmtime(&startTime));
        k += sprintf(filename + k, "-E%.1f", es[e]);
        
        for (s = 0; s < ns; s++) {
            sprintf(filename + k, "-%s.nc", ss[s]);
            
            RKPreparePath(filename);
            
            fid = fopen(filename, "w");
            if (fid == NULL) {
                fprintf(stderr, "Unable to create file.\n");
                exit(EXIT_FAILURE);
            }
            fwrite(payload, filesize, 1, fid);
            fclose(fid);
            
            RKFileManagerAddFile(fileManager, filename, RKFileTypeMoment);
        }
        e = RKNextModuloS(e, ne);
        usleep(500000);
    }
    
    RKFileManagerFree(fileManager);
}

void RKTestPreferenceReading(void) {
    SHOW_FUNCTION_NAME
    int k;
    RKName name;
    double numbers[4];
    RKControl control;
    RKWaveformCalibration cali;
    RKPreference *preference = RKPreferenceInit();;
    RKPreferenceObject *object = NULL;
    object = RKPreferenceFindKeyword(preference, "PedzyHost");
    if (object) {
        printf("pedzy host = %s\n", object->valueString);
    }
    object = RKPreferenceFindKeyword(preference, "TweetaHost");
    if (object) {
        printf("tweeta host = %s\n", object->valueString);
    }
    RKPreferenceGetValueOfKeyword(preference, 1, "Name", name, RKParameterTypeString, RKNameLength);
    RKPreferenceGetValueOfKeyword(preference, 1, "FilePrefix", name, RKParameterTypeString, RKNameLength);
    RKPreferenceGetValueOfKeyword(preference, 1, "Latitude", numbers, RKParameterTypeDouble, 1);
    RKPreferenceGetValueOfKeyword(preference, 1, "Longitude", numbers, RKParameterTypeDouble, 1);
    RKPreferenceGetValueOfKeyword(preference, 1, "SystemZCal", numbers, RKParameterTypeDouble, 2);
    RKPreferenceGetValueOfKeyword(preference, 1, "SystemDCal", numbers, RKParameterTypeDouble, 1);
    k = 0;
    while (RKPreferenceGetValueOfKeyword(preference, 1, "Shortcut", &control, RKParameterTypeControl, 0) == RKResultSuccess) {
        k++;
    }
    RKLog(">Preference.Shortcut count = %d\n", k);
    k = 0;
    while (RKPreferenceGetValueOfKeyword(preference, 1, "WaveformCal", &cali, RKParameterTypeWaveformCalibration, 0) == RKResultSuccess) {
        k++;
    }
    RKLog(">Preference.WaveformCal count = %d\n", k);
    RKPreferenceFree(preference);
}

void RKTestCountFiles(void) {
    SHOW_FUNCTION_NAME
    const char *folder = "data";
    long count = RKCountFilesInPath(folder);
    printf("%ld files in %s\n", count, folder);
}

void RKTestFileMonitor(void) {
    SHOW_FUNCTION_NAME
    const char *file = "pref.conf";
    RKFileMonitor *mon = RKFileMonitorInit(file, &RKTestCallback, NULL);
    RKLog("Touching file %s ...\n", file);
    char command[strlen(file) + 10];
    sprintf(command, "touch %s", file);
    int k = system(command);
    if (k) {
        RKLog("Error. Failed using system() -> %d   errno = %d\n", k, errno);
    }
    sleep(2);
    RKFileMonitorFree(mon);
}

void RKTestHostMonitor(void) {
    SHOW_FUNCTION_NAME
    RKHostMonitor *o = RKHostMonitorInit();
    if (o == NULL) {
        fprintf(stderr, "Unable to allocate a Host Monitor.\n");
        return;
    }
    RKHostMonitorSetVerbose(o, 2);
    RKHostMonitorAddHost(o, "www.amazon.com");
    RKHostMonitorStart(o);
    sleep(RKHostMonitorPingInterval * 3 + RKHostMonitorPingInterval - 1);
    RKHostMonitorFree(o);
}

void RKTestInitializingRadar(void) {
    SHOW_FUNCTION_NAME
    RKRadar *aRadar = RKInitLean();
    RKShowOffsets(aRadar, NULL);
    RKFree(aRadar);
}

void RKTestTemperatureToStatus(void) {
    SHOW_FUNCTION_NAME
    int k;
    float values[] = {-51.0f, -12.0f, -4.5f, 0.5f, 30.0f, 72.5f, 83.0f, 90.5f};
    for (k = 0; k < sizeof(values) / sizeof(float); k++) {
        printf("value = %7.2f -> %d\n", values[k], RKStatusFromTemperatureForCE(values[k]));
    }
}

void RKTestGetCountry(void) {
    SHOW_FUNCTION_NAME
    int k;
    double latitude;
    double longitude;
    char *country;
    bool correct;

    double coords[][2] = {
        { 35.222567, -97.439478},  // United States
        { 36.391592, 127.031428},  // South Korea
        {-11.024860, -75.279694},  // Peru
        { 34.756300, 135.615895}   // Japan
    };
    
    RKName answers[] = {
        "United States",
        "South Korea",
        "Peru",
        "Japan"
    };

    for (k = 0; k < sizeof(coords) / sizeof(coords[0]); k++) {
        latitude = coords[k][0];
        longitude = coords[k][1];
        country = RKCountryFromPosition(latitude, longitude);
        correct = !strcmp(answers[k], country);
        printf("%d. (%10.6f, %10.6f) --> %s (%s%s%s)\n", k, latitude, longitude, country,
               correct ? (rkGlobalParameters.showColor ? RKGreenColor : "") : (rkGlobalParameters.showColor ? RKRedColor : ""),
               correct ? "Correct" : "Incorrect",
               rkGlobalParameters.showColor ? RKNoColor : ""
               );
    }
}

void RKTestBufferOverviewText(const char *options) {
    SHOW_FUNCTION_NAME
    RKRadar *radar = RKInitLean();
    char *text = (char *)malloc(8192);
    RKTextPreferences textPreferences = RKTextPreferencesShowColor | RKTextPreferencesDrawBackground | RKTextPreferencesWindowSize120x80;
    if (options) {
        textPreferences = (RKTextPreferences)strtol(options, NULL, 16);
        RKLog("%s options = %s -> 0x%02x\n", __FUNCTION__, options, textPreferences);
        RKLog(">%s %s %s\n",
              textPreferences & RKTextPreferencesShowColor ? "RKTextPreferencesShowColor" : "",
              textPreferences & RKTextPreferencesDrawBackground ? "RKTextPreferencesDrawBackground" : "",
              ((textPreferences & RKTextPreferencesWindowSizeMask) == RKTextPreferencesWindowSize120x80 ? "RKTextPreferencesWindowSize120x80" :
               ((textPreferences & RKTextPreferencesWindowSizeMask) == RKTextPreferencesWindowSize120x50 ? "RKTextPreferencesWindowSize120x50" :
                ((textPreferences & RKTextPreferencesWindowSizeMask) == RKTextPreferencesWindowSize80x50 ? "RKTextPreferencesWindowSize80x50" :
                 ((textPreferences & RKTextPreferencesWindowSizeMask) == RKTextPreferencesWindowSize80x40 ? "RKTextPreferencesWindowSize80x50" :
                  ((textPreferences & RKTextPreferencesWindowSizeMask) == RKTextPreferencesWindowSize80x25 ? "RKTextPreferencesWindowSize80x25" : ""))))));
    }
    RKBufferOverview(radar, text, textPreferences);
    printf("%s\n", text);
    RKFree(radar);
    free(text);
}

void RKTestSweepRead(const char *file) {
    SHOW_FUNCTION_NAME
    RKSweep *sweep = RKSweepFileRead(file);
    if (sweep) {
        RKSweepFree(sweep);
    }
}

void RKTestProductRead(const char *file) {
    SHOW_FUNCTION_NAME
    RKProduct *product = RKProductFileReaderNC(file, true);
    if (product) {
        RKProductFree(product);
    }
}

void RKTestProductWrite(void) {
    SHOW_FUNCTION_NAME
    int g, k;
    float *v;
    RKProduct *product;
    RKProductBufferAlloc(&product, 1, 360, 8);
    product->desc.type = RKProductTypePPI;
    sprintf(product->desc.name, "Reflectivity");
    sprintf(product->desc.unit, "dBZ");
    sprintf(product->desc.colormap, "Reflectivity");
    sprintf(product->header.radarName, "RadarKit");
    product->header.latitude = 35.23682;
    product->header.longitude = -97.46381;
    product->header.heading = 0.0;
    product->header.wavelength = 0.0314;
    product->header.sweepElevation = 2.4;
    product->header.rayCount = 360;
    product->header.gateCount = 8;
    product->header.gateSizeMeters = 7500.0;
    product->header.prf[0] = 1000;
    product->header.isPPI = true;
    product->header.startTime = 201443696;
    product->header.endTime = 201443696 + 10;
    float az = 90.0;
    for (k = 0; k < 360; k++) {
        product->startAzimuth[k] = az;
        if (az >= 359.0) {
            az = az - 359.0;
        } else {
            az += 1.0;
        }
        product->endAzimuth[k] = az;
        product->startElevation[k] = 2.4;
        product->endElevation[k] = 2.4;
    }
    v = product->data;
    for (k = 0; k < 360; k++) {
        for (g = 0; g < 8; g++) {
            *v++ = NAN;
        }
    }
    RKProductFileWriterNC(product, "blank.nc");
    v = product->data;
    for (k = 0; k < 360; k++) {
        for (g = 0; g < 8; g++) {
            *v++ = (float)(k % 15) * 5.0 - 5.0;
        }
    }
    RKProductFileWriterNC(product, "rainbow.nc");
    RKProductBufferFree(product, 1);
}

#pragma mark -

void RKTestSIMD(const RKTestSIMDFlag flag) {
    SHOW_FUNCTION_NAME
    RKSIMD_show_info();

    int i;
    const int n = RKSIMDAlignSize / sizeof(RKFloat) * 2;

    for (i = 1; i <= 8; i++) {
        RKSIMD_show_count(i);
    }

    // PKIQZ struct variables are usually allocated somewhere else
    RKIQZ s, d, c;

    // In a local context, they are usually in reference form
    RKIQZ *src = &s;
    RKIQZ *dst = &d;
    RKIQZ *cpy = &c;
    RKComplex *cs;
    RKComplex *cd;
    RKComplex *cc;

    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&src->i, RKSIMDAlignSize, RKMaximumGateCount * sizeof(RKFloat)))
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&src->q, RKSIMDAlignSize, RKMaximumGateCount * sizeof(RKFloat)))
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&dst->i, RKSIMDAlignSize, RKMaximumGateCount * sizeof(RKFloat)));
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&dst->q, RKSIMDAlignSize, RKMaximumGateCount * sizeof(RKFloat)));
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&cpy->i, RKSIMDAlignSize, RKMaximumGateCount * sizeof(RKFloat)));
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&cpy->q, RKSIMDAlignSize, RKMaximumGateCount * sizeof(RKFloat)));
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&cs,     RKSIMDAlignSize, RKMaximumGateCount * sizeof(RKComplex)));
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&cd,     RKSIMDAlignSize, RKMaximumGateCount * sizeof(RKComplex)));
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&cc,     RKSIMDAlignSize, RKMaximumGateCount * sizeof(RKComplex)));

    const RKFloat tiny = 1.0e-3f;
    bool good;
    bool all_good = true;

    //

    for (i = 0; i < n; i++) {
        src->i[i] = (RKFloat)i;
        src->q[i] = (RKFloat)-i;
    }

    RKSIMD_zcpy(src, dst, n);

    if (flag & RKTestSIMDFlagShowNumbers) {
        printf("====\n");
    }
    all_good = true;
    for (i = 0; i < n; i++) {
        good = src->i[i] == dst->i[i] && src->q[i] == dst->q[i];
        if (flag & RKTestSIMDFlagShowNumbers) {
            printf("src[%2d] = %9.2f%+9.2fi   dst[%2d] = %9.2f%+9.2fi\n", i, src->i[i], src->q[i], i, dst->i[i], dst->q[i]);
        }
        all_good &= good;
    }
    RKSIMD_TEST_RESULT(rkGlobalParameters.showColor, "Complex Vector Copy -  zcpy", all_good);

    //

    memset(dst->i, 0, n * sizeof(RKFloat));
    memset(dst->q, 0, n * sizeof(RKFloat));

    RKSIMD_zscl(src, 3.0f, dst, n);

    if (flag & RKTestSIMDFlagShowNumbers) {
        printf("====\n");
    }
    all_good = true;
    for (i = 0; i < n; i++) {
        // Answers should be 0, 3-3i, 6-6i, 9-9i, ...
        good = fabsf(dst->i[i] - 3.0f * i) < tiny && fabs(dst->q[i] + 3.0f * i) < tiny;
        if (flag & RKTestSIMDFlagShowNumbers) {
            printf("%9.2f%+9.2fi x 3.0 -> %9.2f%+9.2fi  %s\n", src->i[i], src->q[i], dst->i[i], dst->q[i], OXSTR(good));
        }
        all_good &= good;
    }
    RKSIMD_TEST_RESULT(rkGlobalParameters.showColor, "Complex Vector Scaling by a Float -  zscl", all_good);

    //

    RKSIMD_zadd(src, src, dst, n);

    if (flag & RKTestSIMDFlagShowNumbers) {
        printf("====\n");
    }
    all_good = true;
    for (i = 0; i < n; i++) {
        // Answers should be 0, 2-2i, 3-3i, 4-4i, ...
        good = fabsf(dst->i[i] - (float)(2 * i)) < tiny && fabs(dst->q[i] - (float)(-2 * i)) < tiny;
        if (flag & RKTestSIMDFlagShowNumbers) {
            printf("%9.2f%+9.2fi ++ -> %9.2f%+9.2fi  %s\n", src->i[i], src->q[i], dst->i[i], dst->q[i], OXSTR(good));
        }
        all_good &= good;
    }
    RKSIMD_TEST_RESULT(rkGlobalParameters.showColor, "Complex Vector Addition -  zadd", all_good);

    //

    RKSIMD_zcpy(src, dst, n);
    RKSIMD_izadd(src, dst, n);

    if (flag & RKTestSIMDFlagShowNumbers) {
        printf("====\n");
    }
    all_good = true;
    for (int i = 0; i < n; i++) {
        // Answers should be 0, 2-2i, 3-3i, 4-4i, ...
        good = fabsf(dst->i[i] - (float)(2 * i)) < tiny && fabs(dst->q[i] - (float)(-2 * i)) < tiny;
        if (flag & RKTestSIMDFlagShowNumbers) {
            printf("%9.2f%+9.2fi ++ -> %9.2f%+9.2fi  %s\n", src->i[i], src->q[i], dst->i[i], dst->q[i], OXSTR(good));
        }
        all_good &= good;
    }
    RKSIMD_TEST_RESULT(rkGlobalParameters.showColor, "In-place Complex Vector Addition - izadd", all_good);

    //

    RKSIMD_zmul(src, src, dst, n, false);

    if (flag & RKTestSIMDFlagShowNumbers) {
        printf("====\n");
    }
    all_good = true;
    for (i = 0; i < n; i++) {
        // Answers should be 0, -2i, -8i, -18i, ...
        good = fabsf(dst->i[i]) < tiny && fabs(dst->q[i] - (float)(-2 * i * i)) < tiny;
        if (flag & RKTestSIMDFlagShowNumbers) {
            printf("%9.2f%+9.2fi ** -> %9.2f%+9.2fi  %s\n", src->i[i], src->q[i], dst->i[i], dst->q[i], OXSTR(good));
        }
        all_good &= good;
    }
    RKSIMD_TEST_RESULT(rkGlobalParameters.showColor, "Complex Vector Multiplication -  zmul", all_good);

    //

    RKSIMD_zcpy(src, dst, n);
    RKSIMD_izmul(src, dst, n, false);

    if (flag & RKTestSIMDFlagShowNumbers) {
        printf("====\n");
    }
    all_good = true;
    for (i = 0; i < n; i++) {
        // Answers should be 0, -2i, -8i, -18i, ...
        good = fabsf(dst->i[i]) < tiny && fabs(dst->q[i] - (float)(-2 * i * i)) < tiny;
        if (flag & RKTestSIMDFlagShowNumbers) {
            printf("%9.2f%+9.2fi ** -> %9.2f%+9.2fi  %s\n", src->i[i], src->q[i], dst->i[i], dst->q[i], OXSTR(good));
        }
        all_good &= good;
    }
    RKSIMD_TEST_RESULT(rkGlobalParameters.showColor, "In-place Complex Vector Multiplication - izmul", all_good);

    //

    // Populate some numbers
    for (i = 0; i < n; i++) {
        cs[i].i = (RKFloat)i;
        cs[i].q = (RKFloat)(-i);
        cd[i].i = (RKFloat)(i + 1);
        cd[i].q = (RKFloat)(-i);
    }
    memcpy(cc, cd, n * sizeof(RKComplex));

    RKSIMD_iymul(cs, cd, n);

    if (flag & RKTestSIMDFlagShowNumbers) {
        printf("====\n");
    }
    all_good = true;
    for (i = 0; i < n; i++) {
        // Answers should be 0, 1-3i, 2-10i, 3-21i, ...
        good = fabsf(cd[i].i - (RKFloat)i) < tiny && fabsf(cd[i].q - (RKFloat)(-2 * i * i - i)) < tiny;
        if (flag & RKTestSIMDFlagShowNumbers) {
            printf("%+9.2f%+9.2fi * %+9.2f%+9.2fi = %+9.2f%+9.2fi  %s\n", cs[i].i, cs[i].q, cc[i].i, cc[i].q, cd[i].i, cd[i].q, OXSTR(good));
        }
        all_good &= good;
    }
    RKSIMD_TEST_RESULT(rkGlobalParameters.showColor, "In-place Deinterleaved Complex Vector Multiplication - iymul", all_good);

    //

    // Populate some numbers
    for (i = 0; i < n; i++) {
        cs[i].i = (RKFloat)i;
        cs[i].q = (RKFloat)(-i);
        cd[i].i = (RKFloat)(i + 1);
        cd[i].q = (RKFloat)(-i);
    }
    memcpy(cc, cd, n * sizeof(RKComplex));

    RKSIMD_iymul2(cs, cd, n, false);

    if (flag & RKTestSIMDFlagShowNumbers) {
        printf("====\n");
    }
    all_good = true;
    for (i = 0; i < n; i++) {
        // Answers should be 0, 1-3i, 2-10i, 3-21i, ...
        good = fabsf(cd[i].i - (RKFloat)i) < tiny && fabsf(cd[i].q - (RKFloat)(-2 * i * i - i)) < tiny;
        if (flag & RKTestSIMDFlagShowNumbers) {
            printf("%+9.2f%+9.2fi * %+9.2f%+9.2fi = %+9.2f%+9.2fi  %s\n", cs[i].i, cs[i].q, cc[i].i, cc[i].q, cd[i].i, cd[i].q, OXSTR(good));
        }
        all_good &= good;
    }
    RKSIMD_TEST_RESULT(rkGlobalParameters.showColor, "In-place Deinterleaved Complex Vector Multiplication - iymul2", all_good);

    //

    for (i = 0; i < n; i++) {
        cc[i].i = (RKFloat)i;
        cc[i].q = (RKFloat)(-i);
    }
    RKSIMD_Complex2IQZ(cc, src, n);
    for (i = 0; i < n; i++) {
        cc[i].i = (RKFloat)(i + 1);
        cc[i].q = (RKFloat)(-i);
    }
    RKSIMD_Complex2IQZ(cc, dst, n);
    memcpy(cpy->i, dst->i, n * sizeof(RKFloat));
    memcpy(cpy->q, dst->q, n * sizeof(RKFloat));
    RKSIMD_izmul(src, dst, n, false);
    RKSIMD_IQZ2Complex(dst, cc, n);
    if (flag & RKTestSIMDFlagShowNumbers) {
        printf("====\n");
    }
    all_good = true;
    for (i = 0; i < n; i++) {
        // Answers should be 0, 1-3i, 2-10i, 3-21i, ...
        good = fabsf(cc[i].i - (RKFloat)i) < tiny && fabsf(cc[i].q - (RKFloat)(-2 * i * i - i)) < tiny;
        if (flag & RKTestSIMDFlagShowNumbers) {
            printf("%+9.2f%+9.2fi * %+9.2f%+9.2fi = %+9.2f%+9.2fi  %s\n", src->i[i], src->q[i], cpy->i[i], cpy->q[i], dst->i[i], dst->q[i], OXSTR(good));
        }
        all_good &= good;
    }
    RKSIMD_TEST_RESULT(rkGlobalParameters.showColor, "Deinterleave, Multiply Using iymul, and Interleave", all_good);

    //

    RKInt16C *is = (RKInt16C *)src->i;

    for (i = 0; i < n; i++) {
        is[i].i = (i % 2 == 0 ? 1 : -1);
        is[i].q = (i - n / 2) * (i % 2 == 0 ? -1 : 1);
    }
    memset(cd, 0, n * sizeof(RKComplex));

    RKSIMD_Int2Complex(is, cd, n);

    if (flag & RKTestSIMDFlagShowNumbers) {
        printf("====\n");
    }
    all_good = true;
    for (i = 0; i < n; i++) {
        // Answers should be 0-16i, 1-15i, 2-14i, 3-131i, ...
        good = fabsf(cd[i].i - (RKFloat)(i % 2 == 0 ? 1 : -1)) < tiny && fabsf(cd[i].q - (RKFloat)(i - n / 2) * (i % 2 == 0 ? -1 : 1)) < tiny;
        if (flag & RKTestSIMDFlagShowNumbers) {
            printf("%+3d%+3di -> %+5.1f%+5.1fi  %s\n", is[i].i, is[i].q, cd[i].i, cd[i].q, OXSTR(good));
        }
        all_good &= good;
    }
    RKSIMD_TEST_RESULT(rkGlobalParameters.showColor, "Conversion from i16 to float", all_good);

    if (flag & RKTestSIMDFlagPerformanceTestAll) {
        printf("\n==== Performance Test ====\n\n");
        printf("Using %s gates\n", RKIntegerToCommaStyleString(RKMaximumGateCount));

        int k;
        const int m = 20000;
        struct timeval t1, t2;

        if (flag & RKTestSIMDFlagPerformanceTestArithmetic) {
            gettimeofday(&t1, NULL);
            for (k = 0; k < m; k++) {
                RKSIMD_zmul(src, src, dst, RKMaximumGateCount, false);
            }
            gettimeofday(&t2, NULL);
            printf("Regular SIMD multiplication time for %dK loops = %.3f s\n", m / 1000, RKTimevalDiff(t2, t1));

            gettimeofday(&t1, NULL);
            for (k = 0; k < m; k++) {
                RKSIMD_izmul(src, dst, RKMaximumGateCount, false);
            }
            gettimeofday(&t2, NULL);
            printf("In-place SIMD multiplication time for %dK loops = %.3f s\n", m / 1000, RKTimevalDiff(t2, t1));

            printf("Vectorized Complex Multiplication (%dK loops):\n", m / 1000);
            gettimeofday(&t1, NULL);
            for (k = 0; k < m; k++) {
                RKSIMD_iymul_reg(cs, cd, RKMaximumGateCount);
            }
            gettimeofday(&t2, NULL);
            printf("              reg: " RKSIMD_TEST_TIME_FORMAT " ms (Compiler Optimized -O2)\n", 1.0e3 / m * RKTimevalDiff(t2, t1));

            gettimeofday(&t1, NULL);
            for (k = 0; k < m; k++) {
                RKSIMD_iymul(cs, cd, RKMaximumGateCount);
            }
            gettimeofday(&t2, NULL);
            printf("            iymul: " RKSIMD_TEST_TIME_FORMAT " ms (Normal interleaved I/Q)\n", 1.0e3 / m * RKTimevalDiff(t2, t1));

            gettimeofday(&t1, NULL);
            for (k = 0; k < m; k++) {
                RKSIMD_izmul((RKIQZ *)src, (RKIQZ *)dst, RKMaximumGateCount, false);
            }
            gettimeofday(&t2, NULL);
            printf("            izmul: " RKSIMD_TEST_TIME_FORMAT " ms (Deinterleaved I/Q)\n", 1.0e3 / m * RKTimevalDiff(t2, t1));

            gettimeofday(&t1, NULL);
            for (k = 0; k < m; k++) {
                RKSIMD_Complex2IQZ(cc, src, RKMaximumGateCount);
                RKSIMD_izmul((RKIQZ *)src, (RKIQZ *)dst, RKMaximumGateCount, false);
                RKSIMD_IQZ2Complex(dst, cc, RKMaximumGateCount);
            }
            gettimeofday(&t2, NULL);
            printf("    E + izmul + D: " RKSIMD_TEST_TIME_FORMAT " ms (D, Multiply, I)\n", 1.0e3 / m * RKTimevalDiff(t2, t1));
        }

        if (flag & RKTestSIMDFlagPerformanceTestConversion) {
            printf("Copy (%dK loops):\n", m / 1000);
            gettimeofday(&t1, NULL);
            for (k = 0; k < m; k++) {
                memcpy(src->i, dst->i, RKMaximumGateCount * sizeof(RKFloat));
                memcpy(src->q, dst->q, RKMaximumGateCount * sizeof(RKFloat));
            }
            gettimeofday(&t2, NULL);
            printf("       memcpy x 2: " RKSIMD_TEST_TIME_FORMAT " ms (Compiler Optimized -O2)\n", 1.0e3 / m * RKTimevalDiff(t2, t1));

            gettimeofday(&t1, NULL);
            for (k = 0; k < m; k++) {
                RKSIMD_zcpy(src, dst, RKMaximumGateCount);
            }
            gettimeofday(&t2, NULL);
            printf("             zcpy: " RKSIMD_TEST_TIME_FORMAT " ms (SIMD)\n", 1.0e3 / m * RKTimevalDiff(t2, t1));

            printf("Conversions (%dK loops):\n", m / 1000);
            gettimeofday(&t1, NULL);
            for (k = 0; k < m; k++) {
                RKSIMD_Int2Complex_reg(is, cd, RKMaximumGateCount);
            }
            gettimeofday(&t2, NULL);
            printf("              reg: " RKSIMD_TEST_TIME_FORMAT " ms (Compiler Optimized -O2)\n", 1.0e3 / m * RKTimevalDiff(t2, t1));

            gettimeofday(&t1, NULL);
            for (k = 0; k < m; k++) {
                RKSIMD_Int2Complex(is, cd, RKMaximumGateCount);
            }
            gettimeofday(&t2, NULL);
            printf("      cvtepi32_ps: " RKSIMD_TEST_TIME_FORMAT " ms (SIMD)\n", 1.0e3 / m * RKTimevalDiff(t2, t1));
        }

        printf("\n==========================\n");
    }

    free(src->i);
    free(src->q);
    free(dst->i);
    free(dst->q);
    free(cpy->i);
    free(cpy->q);
    free(cs);
    free(cd);
    free(cc);
}


void RKTestWindow(void) {
    int k;
    int n = 6;
    double param;
    RKFloat *window = (RKFloat *)malloc(n * sizeof(RKFloat));

    printf("=================\nWindow Functions:\n=================\n\n");

    printf("Hamming:\n");
    RKWindowMake(window, RKWindowTypeHamming, n);
    for (k = 0; k < n; k++) {
        printf("w[%d] = %.4f\n", k, window[k]);
    }
    printf("\n");

    param = 0.5;
    printf("Kaiser @ %.4f:\n", param);
    RKWindowMake(window, RKWindowTypeKaiser, n, param);
    for (k = 0; k < n; k++) {
        printf("w[%d] = %.4f\n", k, window[k]);
    }
    printf("\n");

    param = 0.8;
    printf("Trapezoid @ %.4f:\n", param);
    RKWindowMake(window, RKWindowTypeTrapezoid, n, param);
    for (k = 0; k < n; k++) {
        printf("w[%d] = %.4f\n", k, window[k]);
    }
    printf("\n");

    param = 0.5;
    printf("Tukey @ %.4f:\n", param);
    RKWindowMake(window, RKWindowTypeTukey, n, param);
    for (k = 0; k < n; k++) {
        printf("w[%d] = %.4f\n", k, window[k]);
    }
    printf("\n");

    free(window);
}

void RKTestHilbertTransform(void) {
    SHOW_FUNCTION_NAME
    int i;
    const int n = 10;
    RKFloat a = 0.0f;
    RKFloat *x = (RKFloat *)malloc(n * sizeof(RKFloat));
    RKComplex *y = (RKComplex *)malloc(n * sizeof(RKComplex));

    // Simple example of Xr = [1 2 3 4] like hilbert() in MATLAB
    memset(x, 0, n * sizeof(RKFloat));
    x[0] = 1.0f;
    x[1] = 2.0f;
    x[2] = 3.0f;
    x[3] = 4.0f;
    RKHilbertTransform(x, y, 4);

    printf("\n");
    printf("Example 1:\n");
    printf("----------\n");
    printf("\nX =\n\n");
    for (i = 0; i < 4; i++) {
        printf("    [ %7.4f ]\n", x[i]);
    }
    printf("\nH =\n\n");
    for (i = 0; i < 4; i++) {
        printf("    [ %7.4f %s %7.4fi ]\n", y[i].i, y[i].q < 0 ? "-" : "+", y[i].q < 0.0 ? -y[i].q : y[i].q);
    }

    // Another example to illustrate the noise gain is more or less preserved.
    for (i = 0; i < n; i++) {
        x[i] = cosf(0.1f * 2.0f * M_PI * (float)i + 0.2);
    }
    RKHilbertTransform(x, y, n);
    for (i = 0; i < n; i++) {
        a += y[i].i * y[i].i + y[i].q * y[i].q;
    }

    printf("\n");
    printf("Example 2:\n");
    printf("----------\n");
    printf("\nX =\n\n");
    for (i = 0; i < n; i++) {
        printf("    [ %7.4f ]\n", x[i]);
    }
    printf("\nH =\n\n");
    for (i = 0; i < n; i++) {
        printf("    [ %7.4f %s %7.4fi ]\n", y[i].i, y[i].q < 0 ? "-" : "+", y[i].q < 0.0 ? -y[i].q : y[i].q);
    }
    printf("\na = %.4f\n\n", a);
    free(x);
    free(y);
}

void RKTestWriteFFTWisdom(void) {
    SHOW_FUNCTION_NAME
    fftwf_plan plan;
    fftwf_complex *in, *out;
    int nfft = 1 << (int)ceilf(log2f((float)RKMaximumGateCount));
    in = fftwf_malloc(nfft * sizeof(fftwf_complex));
    out = fftwf_malloc(nfft * sizeof(fftwf_complex));
    RKLog("Generating FFT wisdom ...\n");
    while (nfft > 2) {
        RKLog("NFFT %s\n", RKIntegerToCommaStyleString(nfft));
        plan = fftwf_plan_dft_1d(nfft, in, in, FFTW_FORWARD, FFTW_MEASURE);
        fftwf_destroy_plan(plan);
        plan = fftwf_plan_dft_1d(nfft, in, out, FFTW_FORWARD, FFTW_MEASURE);
        fftwf_destroy_plan(plan);
        plan = fftwf_plan_dft_1d(nfft, out, out, FFTW_BACKWARD, FFTW_MEASURE);
        fftwf_destroy_plan(plan);
        plan = fftwf_plan_dft_1d(nfft, out, in, FFTW_BACKWARD, FFTW_MEASURE);
        fftwf_destroy_plan(plan);
        nfft >>= 1;
    }
    fftwf_free(in);
    fftwf_free(out);
    RKLog("Exporting FFT wisdom ...\n");
    fftwf_export_wisdom_to_filename(RKFFTWisdomFile);
    RKLog("Done.\n");
}

void RKTestRingFilterShowCoefficients(void) {
    int i, k;
    RKFilterType type;
    RKIIRFilter *filter = (RKIIRFilter *)malloc(sizeof(RKIIRFilter));
    char *string = (char *)malloc(1024);
    if (filter == NULL || string == NULL) {
        fprintf(stderr, "Error allocation memory.\n");
        exit(EXIT_FAILURE);
    }
    for (type = RKFilterTypeNull; type < RKFilterTypeCount; type++) {
        RKGetFilterCoefficients(filter, type);
        RKLog("%s:\n", filter->name);
        i = sprintf(string, "b = [");
        for (k = 0; k < filter->bLength; k++) {
            i += sprintf(string + i, "%s%.4f", k > 0 ? ", " : "", filter->B[k].i);
        }
        sprintf(string + i, "]");
        RKLog(">%s", string);
        i = sprintf(string, "a = [");
        for (k = 0; k < filter->aLength; k++) {
            i += sprintf(string + i, "%s%.4f", k > 0 ? ", " : "", filter->A[k].i);
        }
        sprintf(string + i, "]");
        RKLog(">%s", string);
    }
    free(string);
    free(filter);
}

#pragma mark - Waveform Tests

void RKTestMakeHops(void) {
    SHOW_FUNCTION_NAME
    for (int k = 3; k < 13; k++) {
        printf(UNDERLINE("%d Hops:\n"), k);
        RKBestStrideOfHops(k, true);
        printf("\n");
    }
}

void RKTestWaveformTFM(void) {
    SHOW_FUNCTION_NAME
    const char filename[] = "waveforms/test-tfm.rkwav";
    RKWaveform *waveform = RKWaveformInitAsFakeTimeFrequencyMultiplexing(2.0, 1.0, 0.5, 100);
    RKWaveformSummary(waveform);
    RKWaveformWrite(waveform, filename);
    RKWaveformFree(waveform);
}

void RKTestWaveformWrite(void) {
    SHOW_FUNCTION_NAME
    RKWaveform *waveform = RKWaveformInitWithCountAndDepth(14, 100);
    RKWaveformFrequencyHops(waveform, 20.0e6, 0.0, 16.0e6);

    char filename[160];
    snprintf(filename, 159, "waveforms/%s.rkwav", waveform->name);
    RKLog("Creating waveform file '%s' ...\n", filename);
    RKWaveformWrite(waveform, filename);

    RKLog("Reading waveform file '%s' ...\n", filename);
    RKWaveform *loadedWaveform = RKWaveformInitFromFile(filename);
    RKWaveformSummary(loadedWaveform);

    RKWaveformFree(waveform);
    RKWaveformFree(loadedWaveform);

    RKLog("Removing waveform file ...\n");
    if (remove(filename)) {
        RKLog("Error removing file '%s'.\n", filename);
    } else {
        RKLog("Done.\n");
    }
}

void RKTestWaveformDownsampling(void) {
    SHOW_FUNCTION_NAME
    RKWaveform *waveform = RKWaveformInitAsTimeFrequencyMultiplexing(160.0e6, 0.0, 4.0e6, 50.0e6);
    RKWaveformSummary(waveform);
    
    RKWaveformDecimate(waveform, 32);
    RKWaveformSummary(waveform);
}

void RKTestWaveformShowProperties(void) {
    SHOW_FUNCTION_NAME
    //RKWaveform *waveform = RKWaveformInitFromFile("waveforms/barker03.rkwav");
    RKWaveform *waveform = RKWaveformInitAsSingleTone(160.0e6, 50.0e6, 1.0e-6);
    RKWaveformSummary(waveform);

    printf("\n");

    RKWaveformDownConvert(waveform);
    RKWaveformSummary(waveform);
    RKWaveformFree(waveform);

    printf("\n");

//    waveform = RKWaveformInitFromFile("waveforms/ofm.rkwav");
//    RKWaveformSummary(waveform);
//
//    printf("\n");
//
//    RKWaveformDownConvert(waveform);
//    RKWaveformSummary(waveform);
//    RKWaveformFree(waveform);
//
//    printf("\n");

    waveform = RKWaveformInitAsLinearFrequencyModulation(160.0e6, 50.0e6, 1.0e-6, 0.0);
    RKWaveformSummary(waveform);
    RKWaveformFree(waveform);

    printf("\n");

    waveform = RKWaveformInitAsLinearFrequencyModulation(160.0e6, 50.0e6, 2.0e-6, 1.0e6);
    RKWaveformSummary(waveform);
    RKWaveformFree(waveform);

    printf("\n");

    waveform = RKWaveformInitAsLinearFrequencyModulation(160.0e6, 50.0e6, 4.0e-6, 2.0e6);
    RKWaveformDownConvert(waveform);
    RKWaveformSummary(waveform);
    RKWaveformFree(waveform);
}

void RKTestWaveformShowUserWaveformProperties(const char *filename) {
    SHOW_FUNCTION_NAME
    RKWaveform *waveform = RKWaveformInitFromFile(filename);
    RKWaveformSummary(waveform);
    RKWaveformFree(waveform);
}

void RKTestWaveformHoppingChirp(void) {
    SHOW_FUNCTION_NAME
    RKWaveform *waveform = RKWaveformInitAsFrequencyHoppingChirp(200.0e6, 0.0, 20.0e6, 1.5e-6, 5);
    RKWaveformSummary(waveform);

    printf("\n");

    RKWaveformDecimate(waveform, 4);
    RKWaveformSummary(waveform);
    RKWaveformFree(waveform);

    printf("\n");
    
    waveform = RKWaveformInitAsFrequencyHoppingChirp(200.0e6, 0.0, 25.0e6, 2.0e-6, 5);
    RKWaveformSummary(waveform);

    printf("\n");

    RKWaveformDecimate(waveform, 4);
    RKWaveformSummary(waveform);
    RKWaveformFree(waveform);
}

#pragma mark - Radar Signal Processing

void RKTestPulseCompression(RKTestFlag flag) {
    SHOW_FUNCTION_NAME
    int k;
    RKPulse *pulse;
    RKInt16C *X;
    RKComplex *F;
    RKComplex *Y;
    RKIQZ Z;

    RKRadar *radar = RKInitLean();
    RKSetProcessingCoreCounts(radar, 2, 1, 1);

    // Increases verbosity if set
    if (flag & RKTestFlagVerbose) {
        RKSetVerbosity(radar, 1);
    }

    RKGoLive(radar);

    // Filter #2
    RKComplex filter2[] = {{1.0f, 1.0f}};
    RKFilterAnchor anchor2 = RKFilterAnchorDefaultWithMaxDataLength(8);

    // Filter #3
    RKComplex filter3[] = {{1.0f, 0.0f}, {0.0f, 1.0f}, {-1.0f, 0.0f}, {0.0f, -1.0f}};
    RKFilterAnchor anchor3 = RKFilterAnchorOfLengthAndMaxDataLength(4, 8);

    for (k = 0; k < 4; k++) {
        switch (k) {
            default:
                // Default is impulse [1];
                RKPulseCompressionSetFilterToImpulse(radar->pulseCompressionEngine);
                break;
            case 1:
                // Two-tap running average [1, 1]
                RKPulseCompressionSetFilterTo11(radar->pulseCompressionEngine);
                break;
            case 2:
                // Change filter to filter #2: [1 + 1i]
                RKPulseCompressionSetFilter(radar->pulseCompressionEngine, filter2, anchor2, 0, 0);
                break;
            case 3:
                // Change filter to filter #3
                RKPulseCompressionSetFilter(radar->pulseCompressionEngine, filter3, anchor3, 0, 0);
                break;
        }

        pulse = RKGetVacantPulse(radar);
        pulse->header.gateCount = 6;

        X = RKGetInt16CDataFromPulse(pulse, 0);
        memset(X, 0, pulse->header.gateCount * sizeof(RKInt16C));
        X[0].i = 1;
        X[0].q = 0;
        X[1].i = 2;
        X[1].q = 0;
        X[2].i = 4;
        X[2].q = 0;
        X[3].i = 2;
        X[3].q = 0;
        X[4].i = 1;
        X[4].q = 0;
        RKSetPulseReady(radar, pulse);

        while (radar->state & RKRadarStateLive && (pulse->header.s & RKPulseStatusCompressed) == 0) {
            usleep(1000);
        }

        F = &radar->pulseCompressionEngine->filters[0][0][0];
        Y = RKGetComplexDataFromPulse(pulse, 0);
        Z = RKGetSplitComplexDataFromPulse(pulse, 0);

        if (flag & RKTestFlagShowResults) {
            printf("\033[4mTest %d:\n\033[24m", k);
            printf("X =               F =                    Y =                        Z =\n");
            for (int k = 0; k < 8; k++) {
                printf("    [ %2d %s %2di ]      [ %5.2f %s %5.2fi ]      [ %6.2f %s %6.2fi ]      [ %6.2f %s %6.2fi ]\n",
                       X[k].i, X[k].q < 0 ? "-" : "+", abs(X[k].q),
                       F[k].i, F[k].q < 0.0f ? "-" : "+", fabs(F[k].q),
                       Y[k].i, Y[k].q < 0.0f ? "-" : "+", fabs(Y[k].q),
                       Z.i[k], Z.q[k] < 0.0f ? "-" : "+", fabs(Z.q[k]));
            }
            printf("\n");
        }
    } // for (k = 0; k < 3; ...)

    RKFree(radar);
}

void RKTestOneRay(int method(RKScratch *, RKPulse **, const uint16_t), const int lag) {
    SHOW_FUNCTION_NAME
    int k, p, n, g;
    RKScratch *space;
    RKFFTModule *fftModule;
    RKBuffer pulseBuffer;
    const int gateCount = 6;
    const int pulseCount = 10;
    const int pulseCapacity = 64;

    RKLog("Allocating buffers ...\n");

    RKScratchAlloc(&space, pulseCapacity, RKMaximumLagCount, (uint8_t)ceilf(log2f((float)pulseCount)), true);
    fftModule = RKFFTModuleInit(pulseCapacity, 0);
    space->fftModule = fftModule;
    space->gateCount = gateCount;
    space->gateSizeMeters = 30.0f;

    RKPulseBufferAlloc(&pulseBuffer, pulseCapacity, pulseCount);
    RKPulse *pulses[pulseCount];
    
    for (k = 0; k < pulseCount; k++) {
        RKPulse *pulse = RKGetPulseFromBuffer(pulseBuffer, k);
        pulse->header.t = k;
        pulse->header.i = (uint64_t)(-1) - pulseCount + k;
        pulse->header.gateCount = gateCount;
        pulse->header.downSampledGateCount = gateCount;
        // Fill in the data...
        for (p = 0; p < 2; p++) {
            RKIQZ X = RKGetSplitComplexDataFromPulse(pulse, p);
            RKComplex *Y = RKGetComplexDataFromPulse(pulse, p);

            // Some seemingly random pattern for testing
            n = pulse->header.i % 3 * (pulse->header.i % 2 ? 1 : -1) + p;
            for (g = 0; g < gateCount; g++) {
                if (g % 2 == 0) {
                    X.i[g] = (int16_t)((g * n) + p);
                    X.q[g] = (int16_t)((n - 2) * (g - 1));
                } else {
                    X.i[g] = (int16_t)(-g * (n - 1));
                    X.q[g] = (int16_t)((g * p) + n);
                }
                Y[g].i = X.i[g];
                Y[g].q = X.q[g];
            }
        }
        pulses[k] = pulse;
    }

    //printf("pcal[0] = %.2f\n", space->pcal[0]);

    if (method == RKPulsePairHop) {
        RKLog("Info. Pulse Pair for Frequency Hopping.\n");
    } else if (method == RKPulsePair) {
        RKLog("Info. Pulse Pair.\n");
    } else if (method == RKMultiLag) {
        space->userLagChoice = lag;
        space->velocityFactor = 1.0f;
        space->widthFactor = 1.0f;
        RKLog("Info. Multilag (N = %d).\n", space->userLagChoice);
    } else if (method == RKSpectralMoment) {
        RKLog("Info. Spectral Moment.\n");
    } else {
        RKLog("Warning. Unknown method.\n");
        method = RKPulsePair;
    }
    method(space, pulses, pulseCount);

    // Some known results
    RKFloat err;
    RKName str;

    // Results for pulse-pair, pulse-pair for hops, multilag for lags 2, 3, and 4
    RKFloat D[5][7] = {
        { 1.7216, -2.5106, -1.9448,  -1.3558, -0.2018,  -0.9616},
        { 2.2780,  2.6324,  2.7621,   2.3824,  3.1231,   2.3561},
        { 4.3376, -7.4963, -7.8030, -11.6505, -1.1906, -11.4542},
        { 2.7106, -8.4965, -7.8061,  -9.1933, -0.7019,  -8.4546},
        { 3.7372, -4.2926, -4.1635,  -6.0751, -0.7788,  -5.9091}
    };
    RKFloat P[5][7] = {
        { 0.4856, -0.4533, -0.4636, -0.5404, -0.4298, -0.5248},
        { 0.4155, -0.8567, -0.7188, -0.7400, -0.4405, -0.6962},
        { 0.4856, -0.4533, -0.4636, -0.5404, -0.4298, -0.5248},
        { 0.4856, -0.4533, -0.4636, -0.5404, -0.4298, -0.5248},
        { 0.4856, -0.4533, -0.4636, -0.5404, -0.4298, -0.5248}
    };
    RKFloat R[5][7] = {
        {0.9024, 0.7063, 0.8050, 0.7045, 0.8717, 0.7079},
        {0.9858, 0.8144, 0.8593, 0.9104, 0.9476, 0.9236},
        {1.8119, 2.5319, 2.9437, 6.7856, 2.6919, 8.4917},
        {1.0677, 1.1674, 1.3540, 2.2399, 1.3389, 2.6234},
        {1.3820, 1.4968, 1.6693, 2.4468, 1.6047, 2.7012}
    };

    // Select the row for the correct answer
    int row = 0;
    if (method == RKPulsePairHop) {
        row = 1;
    } else if (method == RKMultiLag && lag >=2 && lag <= 4) {
        row = lag;
    }

    // Error of ZDR
    err = 0.0;
    for (k = 0; k < gateCount; k++) {
        err += D[row][k] - space->ZDR[k];
    }
    err /= (RKFloat)gateCount;
    sprintf(str, "Delta ZDR = %.4e", err);
    TEST_RESULT(rkGlobalParameters.showColor, str, fabsf(err) < 1.0e-4);

    // Error of PhiDP
    err = 0.0;
    for (k = 0; k < gateCount; k++) {
        err += P[row][k] - space->PhiDP[k];
    }
    err /= (RKFloat)gateCount;
    sprintf(str, "Delta PhiDP = %.4e", err);
    TEST_RESULT(rkGlobalParameters.showColor, str, fabsf(err) < 1.0e-4);

    // Error of RhoHV
    err = 0.0;
    for (k = 0; k < gateCount; k++) {
        err += R[row][k] - space->RhoHV[k];
    }
    err /= (RKFloat)gateCount;
    sprintf(str, "Delta RhoHV = %.4e", err);
    TEST_RESULT(rkGlobalParameters.showColor, str, fabsf(err) < 1.0e-4);

    RKLog("Deallocating buffers ...\n");

    RKScratchFree(space);
    RKFFTModuleFree(fftModule);
    RKPulseBufferFree(pulseBuffer);
}

#pragma mark - Performance Tests

void RKTestPulseCompressionSpeed(void) {
    SHOW_FUNCTION_NAME
    int p, i, j, k;
    const size_t nfft = 1 << 13;
    fftwf_complex *f, *in, *out;
    RKInt16C *X;
    RKComplex *Y;
    const int testCount = 5000;
    struct timeval tic, toc;
    double mint, t;

    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&X, RKSIMDAlignSize, nfft * sizeof(RKInt16C)));
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&Y, RKSIMDAlignSize, nfft * sizeof(RKComplex)));
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&f, RKSIMDAlignSize, nfft * sizeof(fftwf_complex)))
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&in, RKSIMDAlignSize, nfft * sizeof(fftwf_complex)))
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&out, RKSIMDAlignSize, nfft * sizeof(fftwf_complex)))
    if (in == NULL || out == NULL) {
        RKLog("Error. Unable to allocate resources for FFTW.\n");
        return;
    }

    fftwf_plan planForwardInPlace = fftwf_plan_dft_1d(nfft, in, in, FFTW_FORWARD, FFTW_MEASURE);
    fftwf_plan planForwardOutPlace = fftwf_plan_dft_1d(nfft, in, out, FFTW_FORWARD, FFTW_MEASURE);
    fftwf_plan planBackwardInPlace = fftwf_plan_dft_1d(nfft, out, out, FFTW_FORWARD, FFTW_MEASURE);

    // Set some real values so that we don't have see NAN in the data / results, which could slow down FFTW
    for (k = 0; k < nfft; k++) {
        X[k].i = rand();
        X[k].q = 0;
        f[k][0] = 1.0f;
        f[k][1] = 0.0f;
    }

    RKLog(UNDERLINE("PulseCompression") "\n");

    mint = INFINITY;
    for (i = 0; i < 3; i++) {
        gettimeofday(&tic, NULL);
        for (j = 0; j < testCount; j++) {
            for (p = 0; p < 2; p++) {
                // Converting complex int16_t ADC samples to complex float
                RKSIMD_Int2Complex(X, (RKComplex *)in, nfft);
                fftwf_execute_dft(planForwardInPlace, in, in);
                fftwf_execute_dft(planForwardOutPlace, f, out);
                //memcpy(out, f, nfft * sizeof(RKComplex));
                RKSIMD_iymulc((RKComplex *)in, (RKComplex *)out, nfft);
                fftwf_execute_dft(planBackwardInPlace, out, out);
                RKSIMD_iyscl((RKComplex *)out, 1.0f / nfft, nfft);
                // Copy the output
                for (k = 0; k < nfft; k++) {
                    Y[k].i = out[k][0];
                    Y[k].q = out[k][1];
                }
            }
        }
        gettimeofday(&toc, NULL);
        t = RKTimevalDiff(toc, tic);
        RKLog(">Test %d -> %.3f ms / pulse\n", i, 1.0e3 * t / testCount);
        mint = MIN(mint, t);
    }
    RKLog(">Time for each pulse (%s gates) = %.3f ms / pulse (Best of 3)\n",
          RKIntegerToCommaStyleString(nfft), 1.0e3 * mint / testCount);
    RKLog(">Speed: %.2f pulses / sec / core\n", testCount / mint);

    fftwf_destroy_plan(planForwardInPlace);
    fftwf_destroy_plan(planForwardOutPlace);
    fftwf_destroy_plan(planBackwardInPlace);

    free(X);
    free(Y);
    free(f);
    free(in);
    free(out);
}

void RKTestMomentProcessorSpeed(void) {
    SHOW_FUNCTION_NAME
    int i, j, k;
    RKFFTModule *fftModule;
    RKScratch *space;
    RKBuffer pulseBuffer;
    RKBuffer rayBuffer;
    const int testCount = 500;
    const int pulseCount = 100;
    const int pulseCapacity = 1 << 12;

    RKPulseBufferAlloc(&pulseBuffer, pulseCapacity, pulseCount);
    RKRayBufferAlloc(&rayBuffer, pulseCapacity, 1);

    RKScratchAlloc(&space, pulseCapacity, 5, (uint8_t)ceilf(log2f((float)pulseCount)), true);
    fftModule = RKFFTModuleInit(pulseCapacity, 0);
    space->fftModule = fftModule;
    space->gateCount = pulseCapacity;

    RKPulse *pulses[pulseCount];
    RKComplex *X;
    RKIQZ Y;
    for (k = 0; k < pulseCount; k++) {
        RKPulse *pulse = RKGetPulseFromBuffer(pulseBuffer, k);
        pulse->header.t = k;
        pulse->header.gateCount = pulseCapacity;
        X = RKGetComplexDataFromPulse(pulse, 0);
        Y = RKGetSplitComplexDataFromPulse(pulse, 0);
        for (j = 0; j < pulseCapacity; j++) {
            X[j].i = (RKFloat)rand() / RAND_MAX - 0.5f;
            X[j].q = (RKFloat)rand() / RAND_MAX - 0.5f;
            Y.i[j] = X[j].i;
            Y.q[j] = X[j].q;
        }
        X = RKGetComplexDataFromPulse(pulse, 1);
        Y = RKGetSplitComplexDataFromPulse(pulse, 1);
        for (j = 0; j < pulseCapacity; j++) {
            X[j].i = (RKFloat)rand() / RAND_MAX - 0.5f;
            X[j].q = (RKFloat)rand() / RAND_MAX - 0.5f;
            Y.i[j] = X[j].i;
            Y.q[j] = X[j].q;
        }
        pulses[k] = pulse;
    }

    double t, mint;
    struct timeval tic, toc;
    int (*method)(RKScratch *, RKPulse **, const uint16_t);

    RKRay *ray = RKGetRayFromBuffer(rayBuffer, 0);

    for (j = 0; j < 6; j++) {
        switch (j) {
            default:
                method = RKPulsePair;
                RKLog(UNDERLINE("PulsePair:") "\n");
                break;
            case 1:
                method = RKPulsePairHop;
                RKLog(UNDERLINE("PulsePairHop:") "\n");
                break;
            case 2:
                method = RKMultiLag;
                space->userLagChoice = 2;
                RKLog(UNDERLINE("MultiLag (L = %d):") "\n", space->userLagChoice);
                break;
            case 3:
                method = RKMultiLag;
                space->userLagChoice = 3;
                RKLog(UNDERLINE("MultiLag (L = %d):") "\n", space->userLagChoice);
                break;
            case 4:
                method = RKMultiLag;
                space->userLagChoice = 4;
                RKLog(UNDERLINE("MultiLag (L = %d):") "\n", space->userLagChoice);
                break;
            case 5:
                method = RKSpectralMoment;
                space->fftOrder = (uint8_t)ceilf(log2f((float)pulseCount));
                RKLog(UNDERLINE("SpectralMoment:") "\n");
                break;
        }
        mint = INFINITY;
        for (i = 0; i < 3; i++) {
            gettimeofday(&tic, NULL);
            for (k = 0; k < testCount; k++) {
                method(space, pulses, pulseCount);
                makeRayFromScratch(space, ray);
            }
            gettimeofday(&toc, NULL);
            t = RKTimevalDiff(toc, tic);
            RKLog(">Test %d -> %.2f ms\n", i, 1.0e3 * t / testCount);
            mint = MIN(mint, t);
        }
        RKLog(">Time for each ray (%s pulses x %s gates) = %.2f ms (Best of 3)\n",
              RKIntegerToCommaStyleString(pulseCount), RKIntegerToCommaStyleString(pulseCapacity), 1.0e3 * mint / testCount);
        RKLog(">Speed: %.2f rays / sec / core\n", testCount / mint);
    }

    RKFFTModuleFree(fftModule);
    RKScratchFree(space);
    free(pulseBuffer);
    free(rayBuffer);
}

void RKTestCacheWrite(void) {
    RKRawDataRecorder *fileEngine = RKRawDataRecorderInit();
    fileEngine->fd = open("._testwrite", O_CREAT | O_WRONLY, 0000644);
    if (fileEngine->fd < 0) {
        RKLog("Error. Unable to open file.\n");
        exit(EXIT_FAILURE);
    }

#ifdef FUNDAMENTAL_CACHE_WRITE_TEST

    RKRawDataRecorderSetCacheSize(fileEngine, 4);
    RKRawDataRecorderCacheWrite(fileEngine, bytes, 4);
    RKRawDataRecorderCacheWrite(fileEngine, &bytes[4], 2);
    RKRawDataRecorderCacheWrite(fileEngine, &bytes[6], 1);
    RKRawDataRecorderCacheFlush(fileEngine);

#endif

    RKBuffer pulseBuffer;
    RKPulseBufferAlloc(&pulseBuffer, 8192, 100);

    struct timeval time;
    double t0, t1;

    gettimeofday(&time, NULL);
    t1 = (double)time.tv_sec + 1.0e-6 * (double)time.tv_usec;

    int j, k;
    uint32_t len = 0;
    for (k = 1, j = 1; k < 50000; k++) {
        RKPulse *pulse = RKGetPulseFromBuffer(pulseBuffer, k % 100);
        pulse->header.gateCount = 16000;

        len += RKRawDataRecorderCacheWrite(fileEngine, &pulse->header, sizeof(RKPulseHeader));
        len += RKRawDataRecorderCacheWrite(fileEngine, RKGetInt16CDataFromPulse(pulse, 0), pulse->header.gateCount * sizeof(RKInt16C));
        len += RKRawDataRecorderCacheWrite(fileEngine, RKGetInt16CDataFromPulse(pulse, 1), pulse->header.gateCount * sizeof(RKInt16C));

        if (k % 2000 == 0) {
            RKRawDataRecorderCacheFlush(fileEngine);

            gettimeofday(&time, NULL);
            t0 = (double)time.tv_sec + 1.0e-6 * (double)time.tv_usec;
            printf("Speed = %.2f MBps (%d)\n", 1.0e-6 * len / (t0 - t1), len);

            if (j++ % 5 == 0) {
                printf("\n");
                t1 = t0;
                len = 0;
            }
        }
    }

    close(fileEngine->fd);

    // Remove the files that was just created.
    j = system("rm -f ._testwrite");
    if (j) {
        RKLog("Error. System call failed.   errno = %d\n", errno);
    }

    RKRawDataRecorderFree(fileEngine);
}

#pragma mark - Transceiver Emulator

//
// The transceiver emulator only has minimal functionality to get things going
// This isn't meant to fully simulate actual target returns
//
void *RKTestTransceiverRunLoop(void *input) {
    RKTestTransceiver *transceiver = (RKTestTransceiver *)input;
    RKRadar *radar = transceiver->radar;

    int j, k, p, g, s, w;
    double t = 0.0;
    double dt = 0.0;
    long tic = 0;
    struct timeval t0, t1, t2;
    bool even = true;
    
    // Update the engine state
    transceiver->state |= RKEngineStateWantActive;
    transceiver->state &= ~RKEngineStateActivating;

    // Show some info
    if (radar->desc.initFlags & RKInitFlagVerbose) {
        RKLog("%s fs = %s MHz (%.2f m)   %sPRF = %s Hz   (PRT = %.3f ms, %s)\n",
              transceiver->name,
              RKFloatToCommaStyleString(1.0e-6 * transceiver->fs),
              transceiver->gateSizeMeters,
              transceiver->sprt > 1 ? "Base " : "",
              RKIntegerToCommaStyleString((long)(1.0 / transceiver->prt)),
              1000.0 * transceiver->prt,
              transceiver->sprt == 1 ? "Normal" :
              (transceiver->sprt == 2 ? "2:3 Staggered" :
               (transceiver->sprt == 3 ? "3:4 Staggered" :
                (transceiver->sprt == 4 ? "4:5 Staggered" : "Normal"))));
        RKLog("%s gateCount = %s   R = %.1f km   chunkSize = %s   tics = %s\n",
              transceiver->name,
              RKIntegerToCommaStyleString(transceiver->gateCount),
              transceiver->gateCount * transceiver->gateSizeMeters * 1.0e-3,
              RKIntegerToCommaStyleString(transceiver->chunkSize),
              RKFloatToCommaStyleString(1.0e6 * transceiver->prt));
    }
    
    double periodTotal;

    float a;
    float r;
    float phi;
    float cosv;
    float sinv;
    int16_t noise;

    float *ra = (float *)malloc(transceiver->gateCount * sizeof(float));
    int16_t *rn = (int16_t *)malloc(transceiver->gateCount * sizeof(int16_t));
    if (ra == NULL || rn == NULL) {
        RKLog("Error. Unable to allocate memory in RKTransceiverRunLoop.\n");
        exit(EXIT_FAILURE);
    }
    memset(ra, 0, transceiver->gateCount * sizeof(float));
    memset(rn, 0, transceiver->gateCount * sizeof(int16_t));
    for (g = 0; g < transceiver->gateCount; g++) {
        r = (float)g * transceiver->gateSizeMeters * 0.1f;
        a = 600.0f * (cos(0.001f * r)
                     + 0.8f * cosf(0.003f * r + 0.8f) * cosf(0.003f * r + 0.8f) * cosf(0.003f * r + 0.8f)
                     + 0.3f * cosf(0.007f * r) * cosf(0.007f * r)
                     + 0.2f * cosf(0.01f * r + 0.3f)
                     + 0.5f);
        a *= (1000.0 / r) * fabsf(1.0f - (float)g / 50000.0f);
        ra[g] = a;
        rn[g] = (int16_t)(5.0f * ((float)rand() / RAND_MAX - 0.5f));
    }

    float dphi = transceiver->gateSizeMeters * 0.1531995963856f;
    while (dphi >= M_PI) {
        dphi -= 2.0 * M_PI;
    }
    while (dphi < -M_PI) {
        dphi += 2.0 * M_PI;
    }
    if (dphi < -M_PI || dphi >= M_PI) {
        RKLog("Error. Value of dphi = %.4f out of range!\n", dphi);
    }

    RKWaveform *waveform = transceiver->waveformCache[0];
    unsigned int waveformCacheIndex = 0;

#if defined(DEBUG_IIR)

    int16_t seq[] = {-3, -2 , -1, 0, 1, 2, 3, 2, 1, 0, -2};
    int len = sizeof(seq) / sizeof(int16_t);
    int ir = 0;
    int iq = 3;

#endif

    int nn;
    float temp;
    float volt;
    float room;
    RKHealth *health;
    
    RKLog("%s Started.   mem = %s B\n", transceiver->name, RKUIntegerToCommaStyleString(transceiver->memoryUsage));

    // Use a counter that mimics microsecond increments
    RKSetPulseTicsPerSeconds(radar, 1.0e6);
    
    transceiver->state |= RKEngineStateActive;

    // Wait until the radar has been declared live. Otherwise the pulseIndex is never advanced properly.
    s = 0;
    transceiver->state |= RKEngineStateSleep0;
    while (!(radar->state & RKRadarStateLive)) {
        usleep(10000);
        if (++s % 10 == 0 && transceiver->verbose > 1) {
            RKLog("%s sleep 0/%.1f s\n", transceiver->name, (float)s * 0.01f);
        }
    }
    transceiver->state ^= RKEngineStateSleep0;

    RKAddConfig(radar, RKConfigKeyPRF, (uint32_t)roundf(1.0f / transceiver->prt), RKConfigKeyNull);

    gettimeofday(&t1, NULL);
    gettimeofday(&t2, NULL);


    // g gate index
    // j sample index
    // k pseudo-random sequence to choose the pre-defined random numbers
    w = 0;   // waveform index
    while (transceiver->state & RKEngineStateWantActive) {

        periodTotal = 0.0;

        for (j = 0; j < transceiver->chunkSize && transceiver->state & RKEngineStateWantActive; j++) {
            RKPulse *pulse = RKGetVacantPulse(radar);
            
            // Fill in the header
            pulse->header.t = (uint64_t)(1.0e6 * t);
            pulse->header.gateCount = transceiver->gateCount;
            pulse->header.gateSizeMeters = transceiver->gateSizeMeters;

            // Fill in the data...
            if (waveformCacheIndex != transceiver->waveformCacheIndex) {
                waveformCacheIndex = transceiver->waveformCacheIndex;
                waveform = transceiver->waveformCache[transceiver->waveformCacheIndex];
                w = 0;
            }

            // Go through both polarizations
            for (p = 0; p < 2; p++) {
                RKInt16C *X = RKGetInt16CDataFromPulse(pulse, p);
                RKInt16C *S = waveform->iSamples[w];
                // Some random pattern for testing
                k = rand() % transceiver->gateCount;
                // Populate the pulse
                for (g = 1; g < waveform->depth; g++) {
                    noise = rn[k];
                    X->i = S->i + noise;
                    X->q = S->q + noise;
                    k = RKNextModuloS(k, transceiver->gateCount);
                    X++;
                    S++;
                }
                // Phase as a function of time (tic) wrapped into [-PI, PI]
                phi = fmod((double)(tic & 0xFFFF) / 655.36 * M_PI + M_PI, 2.0 * M_PI) - M_PI;
                //for (; g < transceiver->gateCount; g++) {
                for (; g < MIN(50000, transceiver->gateCount); g++) {
                    // sinf() and cosf() run faster with angle within 0 and 2 PI
                    phi += dphi;
                    if (phi < -3.14159265f) {
                        phi += 6.28318531f;
                    } else if (phi > 3.14159265f) {
                        phi -= 6.28318531f;
                    }
                    noise = rn[k];

                    //
                    // The following are using faster approximation of sin() and cos().
                    // They are less precise than the native functions but way faster.
                    // For a more precise version, use RKFastSineCosine()
                    //
                    // X->i = (int16_t)(ra[g] * cosf(phi) + noise);
                    // X->q = (int16_t)(ra[g] * sinf(phi) + noise);
                    //
                    RKFasterSineCosine(phi, &sinv, &cosv);
                    X->i = (int16_t)(ra[g] * cosv + noise);
                    X->q = (int16_t)(ra[g] * sinv + noise);

                    k = RKNextModuloS(k, transceiver->gateCount);
                    X++;
                }
                for (; g < transceiver->gateCount; g++) {
                    noise = rn[k];
                    X->i = noise;
                    X->q = noise;
                    k = RKNextModuloS(k, transceiver->gateCount);
                    X++;
                }

#if defined(DEBUG_IIR)

                X = RKGetInt16CDataFromPulse(pulse, p);

                // Override certain range gates
                X->i = seq[ir];
                X->q = seq[iq];
                X += 2;
                X->i = seq[ir];
                X->q = seq[iq];
                X += 2;
                X->i = seq[ir];
                X->q = seq[iq];
                X += 2;
                X->i = seq[ir];
                X->q = seq[iq];
                X += 2;
                X->i = seq[ir];
                X->q = seq[iq];

                X = RKGetInt16CDataFromPulse(pulse, p);
                X += 128;

                X->i = seq[ir];
                X->q = seq[iq];
                X += 2;
                X->i = seq[ir];
                X->q = seq[iq];
                X += 2;
                X->i = seq[ir];
                X->q = seq[iq];
                X += 2;
                X->i = seq[ir];
                X->q = seq[iq];
                X += 2;
                X->i = seq[ir];
                X->q = seq[iq];

#endif

            }

#if defined(DEBUG_IIR)

            ir = RKNextModuloS(ir, len);
            iq = RKNextModuloS(iq, len);

#endif

            RKSetPulseHasData(radar, pulse);

            if (even) {
                tic += transceiver->ticEven;
            } else {
                tic += transceiver->ticOdd;
            }

            w = RKNextModuloS(w, waveform->count);

            if (transceiver->sleepInterval > 0 && tic > 0 && tic % transceiver->sleepInterval == 0) {
                RKLog("%s sleeping at counter = %s / %s ... %.2e %.2e / %.2e %.2e\n",
                      transceiver->name,
                      RKIntegerToCommaStyleString((long)(transceiver->prt * transceiver->counter)),
                      RKIntegerToCommaStyleString(transceiver->sleepInterval),
                      radar->pulseClock->a, radar->pulseClock->b, radar->positionClock->a, radar->positionClock->b);
                transceiver->counter = 0;
                even = true;
                tic = 0;
                t += 2.0;
                sleep(2);
            }
            if (even) {
                periodTotal += transceiver->periodEven;
                t += transceiver->periodEven;
            } else {
                periodTotal += transceiver->periodOdd;
                t += transceiver->periodOdd;
            }
            even = !even;
        }

        // Report health
        gettimeofday(&t0, NULL);
        dt = RKTimevalDiff(t0, t2);
        if (dt > 0.1) {
            t2 = t0;
            nn = rand();
            temp = 1.0f * nn / RAND_MAX + 79.5f;
            volt = 1.0f * nn / RAND_MAX + 11.5f;
            room = 1.0f * nn / RAND_MAX + 21.5f + (transceiver->simFault && transceiver->transmitting ? 10.0f : 0.0f);
            health = RKGetVacantHealth(radar, RKHealthNodeTransceiver);
            if (health) {
                sprintf(health->string,
                        "{\"Trigger\":{\"Value\":true,\"Enum\":%d}, "
                        "\"PLL Clock\":{\"Value\":true,\"Enum\":%d}, "
                        "\"Target PRF\":{\"Value\":\"%s Hz\", \"Enum\":0}, "
                        "\"FPGA Temp\":{\"Value\":\"%.1fdegC\",\"Enum\":%d}, "
                        "\"XMC Voltage\":{\"Value\":\"%.1f V\",\"Enum\":%d}, "
                        "\"Ambient Temp\":{\"Value\":\"%.1fdegC\",\"Enum\":%d}, "
                        "\"Transmit H\":{\"Value\":\"%s dBm\", \"Enum\":%d}, "
                        "\"Transmit V\":{\"Value\":\"%s dBm\", \"Enum\":%d}, "
                        "\"Waveform\":{\"Value\":\"%s\", \"Enum\":0}, "
                        "\"TransceiverCounter\": %ld}",
                        RKStatusEnumActive,
                        RKStatusEnumNormal,
                        RKIntegerToCommaStyleString((long)(1.0 / transceiver->prt)),
                        temp, RKStatusFromTemperatureForCE(temp),
                        volt, volt > 12.2f ? RKStatusEnumHigh : RKStatusEnumNormal,
                        room, RKStatusFromTemperatureForComputers(room),
                        transceiver->transmitting ? RKFloatToCommaStyleString((float)50.0f + 0.001f * ((nn + 111) & 0x3ff)) : "-inf",
                        transceiver->transmitting ? RKStatusEnumActive : RKStatusEnumOff,
                        transceiver->transmitting ? RKFloatToCommaStyleString((float)50.0f + 0.001f * ((nn + 222) & 0x3ff)) : "-inf",
                        transceiver->transmitting ? RKStatusEnumActive : RKStatusEnumOff,
                        transceiver->waveformCache[transceiver->waveformCacheIndex]->name,
                        transceiver->counter);
                RKSetHealthReady(radar, health);
            }
        }

        // Wait to simulate the PRF
        s = 0;
        do {
            gettimeofday(&t0, NULL);
            dt = RKTimevalDiff(t0, t1);
            usleep(1000);
            if (++s % 1000 == 0) {
                printf("Sleeping ... n = %d ...\n", s);
            }
        } while (transceiver->state & RKEngineStateWantActive && dt < periodTotal);
        t1 = t0;
    }

    free(ra);
    free(rn);

    transceiver->state ^= RKEngineStateActive;
    return NULL;
}

RKTransceiver RKTestTransceiverInit(RKRadar *radar, void *input) {

    int i, j, k;
    RKTestTransceiver *transceiver = (RKTestTransceiver *)malloc(sizeof(RKTestTransceiver));
    if (transceiver == NULL) {
        RKLog("Error. Unable to allocate a test transceiver.\n");
        exit(EXIT_FAILURE);
    }
    memset(transceiver, 0, sizeof(RKTestTransceiver));
    sprintf(transceiver->name, "%s<TransceiverCast>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(RKEngineColorTransceiver) : "",
            rkGlobalParameters.showColor ? RKNoColor : "");
    transceiver->state = RKEngineStateAllocated;
    transceiver->radar = radar;
    transceiver->memoryUsage = sizeof(RKTestTransceiver);
    transceiver->gateCapacity = RKGetPulseCapacity(radar);
    transceiver->gateCount = transceiver->gateCapacity;
    transceiver->fs = transceiver->gateCount >= 16000 ? 50.0e6 :
                     (transceiver->gateCount >= 8000 ? 25.0e6 :
                     (transceiver->gateCount >= 4000 ? 10.0e6 : 5.0e6));
    transceiver->gateSizeMeters = 1.5e3 / transceiver->fs;
    transceiver->prt = 0.001;
    transceiver->sprt = 1;
    //transceiver->waveformCache[0] = RKWaveformInitAsFrequencyHops(transceiver->fs, 0.0, 1.0e-6, 0.0, 1);
    transceiver->waveformCache[0] = RKWaveformInitAsSingleTone(transceiver->fs, 0.0, 1.0e-6);
    sprintf(transceiver->waveformCache[0]->name, "s01");

    // Parse out input parameters
    if (input) {
        char *sb = (char *)input, *se = NULL, *sv = NULL;
        while (*sb == ' ') {
            sb++;
        }
        if (radar->desc.initFlags & RKInitFlagVeryVeryVerbose) {
            RKLog("%s Parsing input.\n", transceiver->name);
        }
        while ((se = strchr(sb, ' ')) != NULL) {
            sv = se + 1;
            switch (*sb) {
                case 'F':
                    transceiver->fs = atof(sv);
                    if (radar->desc.initFlags & RKInitFlagVeryVeryVerbose) {
                        RKLog(">%s fs = %s Hz", transceiver->name, RKIntegerToCommaStyleString((long)transceiver->fs));
                    }
                    break;
                case 'f':
                    i = sscanf(sv, "%d,%d", &j, &k);
                    transceiver->prt = 1.0 / (double)j;
                    if (i == 2) {
                        transceiver->sprt = k;
                    } else {
                        transceiver->sprt = 1;
                    }
                    if (radar->desc.initFlags & RKInitFlagVeryVeryVerbose) {
                        RKLog("%s PRF = %s Hz   (PRT = %.3f ms, %s)\n",
                              transceiver->name,
                              RKIntegerToCommaStyleString((long)j),
                              1000.0 * transceiver->prt,
                              transceiver->sprt == 1 ? "Normal" :
                              (transceiver->sprt == 2 ? "2:3 Staggered" :
                               (transceiver->sprt == 3 ? "3:4 Staggered" :
                                (transceiver->sprt == 4 ? "4:5 Staggered" : "Normal"))));
                    }
                    break;
                case 'g':
                    transceiver->gateCount = atoi(sv);
                    uint32_t capacity = RKGetPulseCapacity(radar);
                    if (transceiver->gateCount > capacity) {
                        RKLog("%s Warning. gateCount %s will be clamped to the capacity %s\n",
                              transceiver->name, RKIntegerToCommaStyleString(transceiver->gateCount), RKIntegerToCommaStyleString(capacity));
                        transceiver->gateCount = capacity;
                    }
                    if (radar->desc.initFlags & RKInitFlagVeryVeryVerbose) {
                        RKLog(">%s gateCount = %s\n", transceiver->name, RKIntegerToCommaStyleString(transceiver->gateCount));
                    }
                    break;
                case 'z':
                    transceiver->sleepInterval = atoi(sv);
                    if (radar->desc.initFlags & RKInitFlagVeryVeryVerbose) {
                        RKLog(">%s sleepInterval = %s", transceiver->name, RKIntegerToCommaStyleString(transceiver->sleepInterval));
                    }
                    break;
            }
            sb = strchr(sv, ' ');
            if (sb == NULL) {
                break;
            } else {
                while (*sb == ' ') {
                    sb++;
                }
            }
        }
    }
    
    // Derive some calculated parameters
    transceiver->periodEven = transceiver->prt;
    transceiver->periodOdd =
    transceiver->sprt == 2 ? transceiver->prt * 3.0 / 2.0 :
    (transceiver->sprt == 3 ? transceiver->prt * 4.0 / 3.0 :
     (transceiver->sprt == 4 ? transceiver->prt * 5.0 / 4.0 : transceiver->prt));
    transceiver->ticEven = (long)(transceiver->periodEven * 1.0e6);
    transceiver->ticOdd = (long)(transceiver->periodOdd * 1.0e6);
    transceiver->chunkSize = (transceiver->periodOdd + transceiver->periodEven) >= 0.02 ? 1 : MAX(1, (int)floor(0.05 / transceiver->prt));
    transceiver->gateSizeMeters = 1.5e8f / transceiver->fs;
    transceiver->gateCount = MIN(transceiver->gateCapacity, (1.5e8 * transceiver->prt) / transceiver->gateSizeMeters);

    transceiver->state |= RKEngineStateActivating;
    if (pthread_create(&transceiver->tidRunLoop, NULL, RKTestTransceiverRunLoop, transceiver)) {
        RKLog("%s. Unable to create transceiver run loop.\n", transceiver->name);
    }
    while (!(transceiver->state & RKEngineStateActive)) {
        usleep(10000);
    }

    return (RKTransceiver)transceiver;
}

int RKTestTransceiverExec(RKTransceiver transceiverReference, const char *command, char *response) {
    RKTestTransceiver *transceiver = (RKTestTransceiver *)transceiverReference;
    RKRadar *radar = transceiver->radar;

    int j, k;
    char *c;
    double bandwidth;
    double pulsewidth;
    double value;
    unsigned int pulsewidthSampleCount;

    RKWaveform *waveform = NULL;
    char string[RKMaximumPathLength];

    if (!(radar->state & RKRadarStatePulseCompressionEngineInitialized)) {
        if (transceiver->verbose) {
            RKLog("%s Warning. No I/Q processors for '%s'.", transceiver->name, command);
        }
        if (response != NULL) {
            sprintf(response, "NAK. No I/Q processors yet." RKEOL);
        }
        return RKResultFailedToExecuteCommand;
    }
    
    switch (command[0]) {
        case 'd':
            if (!strcmp(command, "disconnect")) {
                if (transceiver->state & RKEngineStateWantActive) {
                    transceiver->state ^= RKEngineStateWantActive;
                    if (radar->desc.initFlags & RKInitFlagVerbose) {
                        RKLog("%s Disconnecting ...", transceiver->name);
                    }
                } else {
                    if (radar->desc.initFlags & RKInitFlagVerbose) {
                        RKLog("%s Deactivate multiple times\n", transceiver->name);
                    }
                    return RKResultEngineDeactivatedMultipleTimes;
                }
                transceiver->state |= RKEngineStateDeactivating;
                pthread_join(transceiver->tidRunLoop, NULL);
                transceiver->state ^= RKEngineStateDeactivating;
                if (response != NULL) {
                    sprintf(response, "ACK. Transceiver stopped." RKEOL);
                }
                if (radar->desc.initFlags & RKInitFlagVerbose) {
                    RKLog("%s Stopped.\n", transceiver->name);
                }
            }
            break;
        case 'h':
            if (response != NULL) {
                sprintf(response,
                        "Commands:\n"
                        UNDERLINE("help") " - Help list.\n"
                        UNDERLINE("prt") " [value] - PRT set to value\n"
                        UNDERLINE("z") " [value] - Sleep interval set to value.\n"
                        );
            }
            break;
        case 'p':
            if (!strncmp(command, "prt", 3)) {
                k = sscanf(command, "%s %lf", string, &value);
                if (k == 2) {
                    transceiver->prt = value;
                    if (response != NULL) {
                        sprintf(response, "ACK. New PRT = %.3f ms" RKEOL, 1.0e3 * transceiver->prt);
                    }
                } else if (response != NULL) {
                    sprintf(response, "ACK. Current PRT = %.3f ms" RKEOL, 1.0e3 * transceiver->prt);
                    break;
                }
            } else if (!strncmp(command, "prf", 3)) {
                k = sscanf(command, "%s %lf", string, &value);
                if (k == 2) {
                    transceiver->prt = 1.0 / value;
                    if (response != NULL) {
                        sprintf(response, "ACK. New PRF = %.0f Hz" RKEOL, 1.0 / transceiver->prt);
                    }
                } else if (response != NULL) {
                    sprintf(response, "ACK. Current PRF = %.0f Hz" RKEOL, 1.0 / transceiver->prt);
                    break;
                }
            }
            transceiver->periodEven = transceiver->prt;
            transceiver->periodOdd =
            transceiver->sprt == 2 ? transceiver->prt * 3.0 / 2.0 :
            (transceiver->sprt == 3 ? transceiver->prt * 4.0 / 3.0 :
             (transceiver->sprt == 4 ? transceiver->prt * 5.0 / 4.0 : transceiver->prt));
            transceiver->ticEven = (long)(transceiver->periodEven * 1.0e6);
            transceiver->ticOdd = (long)(transceiver->periodOdd * 1.0e6);
            transceiver->chunkSize = (transceiver->periodOdd + transceiver->periodEven) >= 0.02 ? 1 : MAX(1, (int)floor(0.1 / transceiver->prt));
            value = 1.5e8 * transceiver->prt;
            transceiver->gateCount = MIN(transceiver->gateCapacity, value / transceiver->gateSizeMeters);
            RKAddConfig(radar, RKConfigKeyPRF, (uint32_t)roundf(1.0f / transceiver->prt), RKConfigKeyNull);
            if (radar->desc.initFlags & RKInitFlagVerbose) {
                RKLog("%s PRT = %s ms   gateCount = %s\n", transceiver->name,
                      RKFloatToCommaStyleString(1.0e3 * transceiver->prt),
                      RKIntegerToCommaStyleString(transceiver->gateCount));
            }
            break;
        case 's':
            if (!strcmp(command, "stop")) {
                RKLog("%s Stop transmitting.\n", transceiver->name);
                if (response != NULL) {
                    sprintf(response, "ACK. Transmitter Off." RKEOL);
                }
                break;
            }
            transceiver->sleepInterval = atoi(command + 1);
            RKLog("%s sleepInterval = %s", transceiver->name, RKIntegerToCommaStyleString(transceiver->sleepInterval));
            break;
        case 't':
            // Pretend a slow command
            RKPerformMasterTaskInBackground(radar, "w");
            if (response != NULL) {
                sprintf(response, "ACK. Command executed." RKEOL);
            }
            break;
        case 'w':
            // Waveform
            c = ((char *)command);
            if (command[1] == ' ') {
                c += 2;
            } else {
                c++;
            }
            if ((*c == 's' || *c == 't' || *c == 'q') && c[1] >= '0' && c[1] <= '9' && c[2] >= '0' && c[2] <= '9') {
                // Something like s01, t02, q05, etc.
                strcpy(string, c);
                RKStripTail(string);
                pulsewidth = 1.0e-6 * atof(c + 1);
                pulsewidthSampleCount = pulsewidth * transceiver->fs;
                RKLog("%s Waveform '%s'   pw = %.2f us\n", transceiver->name, c, 1.0e6 * pulsewidth);
                waveform = RKWaveformInitWithCountAndDepth(1, pulsewidthSampleCount);
                if (*c == 's') {
                    // Rectangular single tone
                    RKWaveformSingleTone(waveform, transceiver->fs, 0.0);
                } else if (*c == 't') {
                    // Rectangular single tone at 0.1 MHz
                    RKWaveformSingleTone(waveform, transceiver->fs, 0.1e6);
                } else if (*c == 'q') {
                    // LFM at half of the bandwidth capacity
                    RKWaveformLinearFrequencyModulation(waveform, transceiver->fs, -0.25 * transceiver->fs, pulsewidth, 0.5 * transceiver->fs);
                }
                // Override the waveform name
                strncpy(waveform->name, c, RKNameLength);
            } else if (*c == 'h' && c[1] >= '0' && c[1] <= '9' && c[2] >= '0' && c[2] <= '9' && c[3] >= '0' && c[3] <= '9' && c[4] >= '0' && c[4] <= '9') {
                // Something like h1005, h2007, etc.
                string[0] = c[1]; string[1] = c[2]; string[2] = '\0';
                bandwidth = 1.0e6 * atof(string);
                string[0] = c[3]; string[1] = c[4];
                k = atoi(string);
                if (strlen(c) > 5) {
                    pulsewidth = 1.0e-6 * atof(c + 5);
                } else {
                    pulsewidth = 1.0e-6;
                }
                RKLog("%s Waveform '%s'   hops over bw = %.2f MHz @ %d pairs   pw = %.2f us\n", transceiver->name, c, 1.0e-6 * bandwidth, k, 1.0e6 * pulsewidth);
                if (bandwidth > transceiver->fs) {
                    RKLog("%s Warning. Aliasing.   bandwidth = %.2f MHz > fs = %.2f MHz\n", transceiver->name, 1.0e-6 * bandwidth, 1.0e-6 * transceiver->fs);
                }
                // Frequency hop at the specified pulsewidth, bandwith and hop count
                waveform = RKWaveformInitAsFrequencyHops(transceiver->fs, 0.0, pulsewidth, bandwidth, k);
            } else if (*c == 'x') {
                // Experimental waveform
                waveform = RKWaveformInitAsTimeFrequencyMultiplexing(transceiver->fs, 0.0, 4.0e6, 50.0e-6);
            } else {
                // Load from a file
                sprintf(string, "%s/%s.rkwav", RKWaveformFolder, c);
                if (RKFilenameExists(string)) {
                    RKLog("Loading waveform from file '%s'...\n", string);
                    waveform = RKWaveformInitFromFile(string);
                    if (waveform == NULL) {
                        return RKResultFailedToSetWaveform;
                    }
                    pulsewidth = waveform->depth / waveform->fs;
                    RKWaveformSummary(waveform);
                    k = round(waveform->fs / transceiver->fs);
                    if (k > 1) {
                        RKLog("%s Adjusting waveform to RX sampling rate = %.2f MHz (x %d)    pw = %.2f us\n", transceiver->name, 1.0e-6 * transceiver->fs, k, 1.0e6 * pulsewidth);
                        RKWaveformDownConvert(waveform);
                        RKWaveformDecimate(waveform, k);
                    }
                } else if (response != NULL) {
                    sprintf(response, "NAK. Waveform '%s' not found." RKEOL, string);
                    return RKResultFailedToSetWaveform;
                }
            }
            if (waveform == NULL) {
                RKLog("%s No waveform.\n", transceiver->name);
                return RKResultFailedToSetWaveform;
            }
            if (radar->desc.pulseCapacity < waveform->depth) {
                RKLog("%s Warning. Waveform '%s' with %s samples not allowed (capacity = %s).\n", transceiver->name, string,
                      RKIntegerToCommaStyleString(waveform->depth), RKIntegerToCommaStyleString(radar->desc.pulseCapacity));
                RKLog("%s Warning. Waveform not changed.\n", transceiver->name);
                if (response != NULL) {
                    sprintf(response, "NAK. Waveform '%s' with %s samples not allowed (capacity = %s)." RKEOL, string,
                            RKIntegerToCommaStyleString(waveform->depth), RKIntegerToCommaStyleString(radar->desc.pulseCapacity));
                }
                return RKResultFailedToSetWaveform;
            }
            // Next cache index
            j = RKNextModuloS(transceiver->waveformCacheIndex, RKTestWaveformCacheCount);
            if (transceiver->waveformCache[j]) {
                RKLog("%s Freeing cache %d ...\n", transceiver->name, j);
                RKWaveformFree(transceiver->waveformCache[j]);
            }
            transceiver->waveformCache[j] = waveform;
            transceiver->waveformCacheIndex = j;
            RKWaveformSummary(waveform);
            if (response != NULL) {
                sprintf(response, "ACK. Waveform '%s' loaded." RKEOL, c);
            }
            RKSetWaveform(radar, waveform);
            transceiver->transmitting = true;
            break;
        case 'x':
            // Simulate critical temperature
            transceiver->simFault = !transceiver->simFault;
            sprintf(response, "ACK. simFault -> %d" RKEOL, transceiver->simFault);
            break;
        case 'z':
            transceiver->transmitting = false;
            break;
        default:
            if (response != NULL) {
                sprintf(response, "NAK. Command not understood." RKEOL);
            }
            break;
    }
    return RKResultSuccess;
}

int RKTestTransceiverFree(RKTransceiver transceiverReference) {
    int k;
    RKTestTransceiver *transceiver = (RKTestTransceiver *)transceiverReference;
    for (k = 0; k < 2; k++) {
        if (transceiver->waveformCache[k]) {
            if (transceiver->verbose > 1) {
                RKLog("%s Freeing waveform cache %d '%s' ...\n", transceiver->name, k, transceiver->waveformCache[k]->name);
            }
            RKWaveformFree(transceiver->waveformCache[k]);
        }
    }
    free(transceiver);
    return RKResultSuccess;
}

#pragma mark - Pedestal Emulator

void *RKTestPedestalRunLoop(void *input) {
    RKTestPedestal *pedestal = (RKTestPedestal *)input;
    RKRadar *radar = pedestal->radar;
    
    float azimuth = 0.0f;
    float elevation = 3.0f;
    double dt = 0.0;
    struct timeval t0, t1;
    unsigned long tic = 19760520;
    bool scanStartEndPPI = true;
    bool scanStartRHI = true;
    bool scanEndRHI = true;
    bool elTransition = false;

    pedestal->state |= RKEngineStateWantActive;
    pedestal->state &= ~RKEngineStateActivating;

    gettimeofday(&t1, NULL);

    RKLog("%s Started.   mem = %s B\n", pedestal->name, RKUIntegerToCommaStyleString(pedestal->memoryUsage));

    if (radar->desc.initFlags & RKInitFlagVerbose) {
        RKLog("%s fs = %s Hz\n", pedestal->name, RKIntegerToCommaStyleString((long)(1.0 / PEDESTAL_SAMPLING_TIME)));
    }

    // Use a counter that mimics microsecond increments
    RKSetPositionTicsPerSeconds(radar, 1.0 / PEDESTAL_SAMPLING_TIME);
    int healthTicCount = (int)(0.1 / PEDESTAL_SAMPLING_TIME);
    
    pedestal->state |= RKEngineStateActive;

    int commandCount = pedestal->commandCount;
    
    while (pedestal->state & RKEngineStateWantActive) {
        if (commandCount != pedestal->commandCount) {
            commandCount = pedestal->commandCount;
            elevation = pedestal->scanElevation;
            azimuth = pedestal->scanAzimuth;
        }

        // Get a vacation position to fill it in with the latest reading
        RKPosition *position = RKGetVacantPosition(radar);
        if (position == NULL) {
            usleep(1000);
            continue;
        }
        position->tic = tic++;
        position->elevationDegrees = elevation;
        position->azimuthDegrees = azimuth;
        position->azimuthVelocityDegreesPerSecond = pedestal->speedAzimuth;
        position->elevationVelocityDegreesPerSecond = pedestal->speedElevation;
        position->flag |= RKPositionFlagScanActive | RKPositionFlagAzimuthEnabled | RKPositionFlagElevationEnabled;

        if (pedestal->scanMode == RKTestPedestalScanModePPI) {
            position->sweepElevationDegrees = pedestal->scanElevation;
            position->sweepAzimuthDegrees = 0.0f;
            position->flag |= RKPositionFlagAzimuthSweep | RKPositionFlagElevationPoint | RKPositionFlagScanActive | RKPositionFlagVCPActive;
        } else if (pedestal->scanMode == RKTestPedestalScanModeRHI) {
            position->sweepAzimuthDegrees = pedestal->scanAzimuth;
            position->sweepElevationDegrees = 0.0f;
            position->flag |= RKPositionFlagElevationSweep | RKPositionFlagAzimuthPoint | RKPositionFlagScanActive | RKPositionFlagVCPActive;
        }
        if (scanStartEndPPI) {
            scanStartEndPPI = false;
            position->flag |= RKPositionFlagAzimuthComplete;
            if (pedestal->verbose > 1) {
                RKLog("%s scanStartEndPPI\n", pedestal->name);
            }
        }
        if (scanStartRHI) {
            scanStartRHI = false;
            position->flag |= RKPositionFlagElevationComplete;
        } else if (scanEndRHI) {
            scanEndRHI = false;
        }
        RKSetPositionReady(radar, position);
        
        // Report health
        if (tic % healthTicCount == 0) {
            RKHealth *health = RKGetVacantHealth(radar, RKHealthNodePedestal);
            if (health) {
                sprintf(health->string, "{"
                        "\"Pedestal AZ\":{\"Value\":\"%.2f deg\",\"Enum\":%d}, "
                        "\"Pedestal EL\":{\"Value\":\"%.2f deg\",\"Enum\":%d}, "
                        "\"Pedestal AZ Safety\":{\"Value\":true,\"Enum\":%d}, "
                        "\"Pedestal EL Safety\":{\"Value\":true,\"Enum\":%d}, "
                        "\"VCP Active\":{\"Value\":true,\"Enum\":%d}, "
                        "\"Pedestal Operate\":{\"Value\":true,\"Enum\":%d}"
                        "}",
                        position->azimuthDegrees, RKStatusEnumNormal,
                        position->elevationDegrees, RKStatusEnumNormal,
                        RKStatusEnumNormal,
                        RKStatusEnumNormal,
                        position->elevationVelocityDegreesPerSecond > 0.1f || position->azimuthVelocityDegreesPerSecond > 0.1f ? RKStatusEnumNormal : RKStatusEnumStandby,
                        position->elevationVelocityDegreesPerSecond > 0.1f || position->azimuthVelocityDegreesPerSecond > 0.1f ? RKStatusEnumNormal : RKStatusEnumStandby);
                RKSetHealthReady(radar, health);
            }
        }
        
        // Posiiton change
        if (pedestal->scanMode == RKTestPedestalScanModePPI) {
            azimuth += pedestal->speedAzimuth * PEDESTAL_SAMPLING_TIME;
            if (azimuth >= 360.0f) {
                azimuth -= 360.0f;
            } else if (azimuth < 0.0f) {
                azimuth += 360.0f;
            }
            // Transition over 0 degree
            if (pedestal->speedAzimuth > 0.0f && azimuth < 5.0f && position->azimuthDegrees > 355.0f) {
                scanStartEndPPI = true;
            } else if (pedestal->speedAzimuth < 0.0f && azimuth > 355.0f && position->azimuthDegrees < 5.0f) {
                scanStartEndPPI = true;
            }
            if (scanStartEndPPI &&
                azimuth > pedestal->speedAzimuth * PEDESTAL_SAMPLING_TIME &&
                azimuth < 360.0f - pedestal->speedAzimuth * PEDESTAL_SAMPLING_TIME) {
                fprintf(stderr, "Unexpected. azimuth = %.2f   position->azimuthDegrees = %.2f   speed = %.2f\n",
                        azimuth, position->azimuthDegrees, pedestal->speedAzimuth);
            }
        } else if (pedestal->scanMode == RKTestPedestalScanModeRHI) {
            if (elTransition) {
                elevation -= 2.0f * pedestal->speedElevation * PEDESTAL_SAMPLING_TIME;
            } else {
                elevation += pedestal->speedElevation * PEDESTAL_SAMPLING_TIME;
            }
            if (elevation > 180.0f) {
                elevation -= 360.0f;
            } else if (elevation < -180.0f) {
                elevation += 360.0f;
            }
            if (pedestal->speedElevation > 0.0f) {
                if (elevation > pedestal->rhiElevationEnd) {
                    scanEndRHI = true;
                    elTransition = true;
                    position->flag &= ~RKPositionFlagScanActive;
                } else if (elevation < pedestal->rhiElevationStart) {
                    scanStartRHI = true;
                    elTransition = false;
                    position->flag |= RKPositionFlagScanActive;
                }
            }
        } else if (pedestal->scanMode == RKTestPedestalScanModeBadPedestal) {
            azimuth = (float)rand() * 360.0f / RAND_MAX;
        }
        
        // Wait to simulate sampling time
        do {
            gettimeofday(&t0, NULL);
            dt = RKTimevalDiff(t0, t1);
            usleep(1000);
        } while (pedestal->state & RKEngineStateWantActive && dt < PEDESTAL_SAMPLING_TIME);
        t1 = t0;
    }
    
    pedestal->state ^= RKEngineStateActive;
    return NULL;
}

RKPedestal RKTestPedestalInit(RKRadar *radar, void *input) {
    RKTestPedestal *pedestal = (RKTestPedestal *)malloc(sizeof(RKTestPedestal));
    if (pedestal == NULL) {
        RKLog("Error. Unable to allocate a test pedestal.\n");
        exit(EXIT_FAILURE);
    }
    memset(pedestal, 0, sizeof(RKTestPedestal));
    sprintf(pedestal->name, "%s<AimPedestalCast>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(RKEngineColorPedestalRelayPedzy) : "",
            rkGlobalParameters.showColor ? RKNoColor : "");
    pedestal->memoryUsage = sizeof(RKTestPedestal);
    pedestal->radar = radar;
    pedestal->state = RKEngineStateAllocated;

    // Parse input here if there is any

    pedestal->state |= RKEngineStateActivating;
    if (pthread_create(&pedestal->tidRunLoop, NULL, RKTestPedestalRunLoop, pedestal)) {
        RKLog("%s. Unable to create pedestal run loop.\n", pedestal->name);
    }
    //RKLog("Pedestal input = '%s'\n", input == NULL ? "(NULL)" : input);
    while (!(pedestal->state & RKEngineStateActive)) {
        usleep(10000);
    }

    return (RKPedestal)pedestal;
}

int RKTestPedestalExec(RKPedestal pedestalReference, const char *command, char *response) {
    RKTestPedestal *pedestal = (RKTestPedestal *)pedestalReference;
    RKRadar *radar = pedestal->radar;
    
    int k;
    char sval[4][64];
    
    if (!strcmp(command, "disconnect")) {
        if (pedestal->state & RKEngineStateWantActive) {
            pedestal->state ^= RKEngineStateWantActive;
            if (radar->desc.initFlags & RKInitFlagVerbose) {
                RKLog("%s Disconnecting ...", pedestal->name);
            }
        } else {
            if (radar->desc.initFlags & RKInitFlagVerbose) {
                RKLog("%s Deactivate multiple times\n", pedestal->name);
            }
            return RKResultEngineDeactivatedMultipleTimes;
        }
        pedestal->state |= RKEngineStateDeactivating;
        pthread_join(pedestal->tidRunLoop, NULL);
        pedestal->state ^= RKEngineStateDeactivating;
        if (response != NULL) {
            sprintf(response, "ACK. Pedestal stopped." RKEOL);
        }
        if (radar->desc.initFlags & RKInitFlagVerbose) {
            RKLog("%s Stopped.\n", pedestal->name);
        }
    } else if (!strncmp(command, "state", 5)) {
        if (fabsf(pedestal->speedAzimuth) > 0.1f || fabsf(pedestal->speedElevation) > 0.1f) {
            sprintf(response, "1" RKEOL);
        } else {
            sprintf(response, "0" RKEOL);
        }
    } else if (!strncmp(command, "stop", 4)) {
        pedestal->scanMode = RKTestPedestalScanModeNull;
        pedestal->scanElevation = 0.0f;
        pedestal->scanAzimuth = 0.0f;
        pedestal->speedElevation = 0.0f;
        pedestal->speedAzimuth = 0.0f;
        if (response != NULL) {
            sprintf(response, "ACK. Pedestal stopped." RKEOL);
        }
    } else if (!strncmp(command, "ppi", 3)) {
        k = sscanf(command, "%s %s %s", sval[0], sval[1], sval[2]);
        if (k == 3) {
            pedestal->scanMode = RKTestPedestalScanModePPI;
            pedestal->commandCount++;
            pedestal->scanElevation = atof(sval[1]);
            pedestal->speedAzimuth = atof(sval[2]);
        }
        if (response != NULL) {
            sprintf(response, "ACK. PPI mode set at EL %.2f @ %.2f deg/sec" RKEOL, pedestal->scanElevation, pedestal->speedAzimuth);
        }
    } else if (!strncmp(command, "rhi", 3)) {
        k = sscanf(command, "%s %s %s %s", sval[0], sval[1], sval[2], sval[3]);
        if (k == 4) {
            pedestal->scanMode = RKTestPedestalScanModeRHI;
            pedestal->commandCount++;
            pedestal->scanAzimuth = atof(sval[1]);
            pedestal->speedElevation = atof(sval[3]);
            sscanf(sval[2], "%f,%f", &pedestal->rhiElevationStart, &pedestal->rhiElevationEnd);
        }
        if (response != NULL) {
            sprintf(response, "ACK. RHI mode set at AZ %.2f over %.2f-%.2f deg @ %.2f deg/sec" RKEOL,
                    pedestal->scanAzimuth, pedestal->rhiElevationStart, pedestal->rhiElevationEnd, pedestal->speedElevation);
        }
    } else if (!strncmp(command, "bad", 3)) {
        pedestal->scanMode = RKTestPedestalScanModeBadPedestal;
        pedestal->commandCount++;
        if (response != NULL) {
            sprintf(response, "ACK. Simulating bad pedestal" RKEOL);
        }
    } else if (!strcmp(command, "help")) {
        sprintf(response,
                "Commands:\n"
                UNDERLINE("help") " - Help list\n"
                UNDERLINE("ppi") " [EL] [AZ_RATE] - PPI scan at elevation EL at AZ_RATE deg/s.\n"
                UNDERLINE("rhi") " [AZ] [EL_START,EL_END] [EL_RATE] - RHI at AZ over EL_START to EL_END.\n"
                );
    } else if (response != NULL) {
        sprintf(response, "NAK. Command not understood." RKEOL);
    }
    
    return RKResultSuccess;
}

int RKTestPedestalFree(RKPedestal pedestalReference) {
    RKTestPedestal *pedestal = (RKTestPedestal *)pedestalReference;
    free(pedestal);
    return RKResultSuccess;
}

#pragma mark - Health Relay Emulator

void *RKTestHealthRelayRunLoop(void *input) {
    RKTestHealthRelay *healthRelay = (RKTestHealthRelay *)input;
    RKRadar *radar = healthRelay->radar;
    
    int n;
    float powerH, powerV;
    double latitude, longitude, heading;
    double dt = 0.0;
    struct timeval t1, t0;

    healthRelay->state |= RKEngineStateWantActive;
    healthRelay->state &= ~RKEngineStateActivating;

    gettimeofday(&t1, NULL);
    
    RKLog("%s Started.   mem = %s B\n", healthRelay->name, RKUIntegerToCommaStyleString(healthRelay->memoryUsage));
    
    if (radar->desc.initFlags & RKInitFlagVerbose) {
        RKLog("%s fs = %s Hz\n", healthRelay->name, RKIntegerToCommaStyleString((long)(1.0 / HEALTH_RELAY_SAMPLING_TIME)));
    }

    healthRelay->state |= RKEngineStateActive;

    while (healthRelay->state & RKEngineStateWantActive) {
        powerH = (float)rand() / RAND_MAX - 0.5f;
        powerV = (float)rand() / RAND_MAX - 0.5f;
        latitude = (double)rand() * 8.0e-6f / RAND_MAX + 35.181251;
        longitude = (double)rand() * 8.0e-6f / RAND_MAX - 97.4349928;
        heading = (double)rand() * 0.2 / RAND_MAX + 45.0;
        RKHealth *health = RKGetVacantHealth(radar, RKHealthNodeTweeta);
        if (health) {
            sprintf(health->string, "{"
                    "\"PSU H\":{\"Value\":true, \"Enum\":%d}, "
                    "\"PSU V\":{\"Value\":true, \"Enum\":%d}, "
                    "\"GPS Valid\":{\"Value\":true, \"Enum\":0}, "
                    "\"GPS Latitude\":{\"Value\":\"%.7f\",\"Enum\":0}, "
                    "\"GPS Longitude\":{\"Value\":\"%.7f\",\"Enum\":0}, "
                    "\"GPS Heading\":{\"Value\":\"%.1f deg\",\"Enum\":0}, "
                    "\"Platform Pitch\":{\"Value\":\"%.2f deg\",\"Enum\":%d}, "
                    "\"Platform Roll\":{\"Value\":\"%.2f deg\",\"Enum\":%d}"
                    "}",
                    RKStatusEnumNormal,
                    RKStatusEnumNormal,
                    latitude,
                    longitude,
                    heading,
                    powerH, RKStatusEnumNormal,
                    powerV, RKStatusEnumNormal);
            RKSetHealthReady(radar, health);
        }

        // Wait to simulate sampling time
        n = 0;
        do {
            gettimeofday(&t0, NULL);
            dt = RKTimevalDiff(t0, t1);
            usleep(10000);
            n++;
        } while (healthRelay->state & RKEngineStateWantActive && dt < HEALTH_RELAY_SAMPLING_TIME);
        t1 = t0;
    }

    healthRelay->state ^= RKEngineStateActive;
    return NULL;
}

RKHealthRelay RKTestHealthRelayInit(RKRadar *radar, void *input) {
    RKTestHealthRelay *healthRelay = (RKTestHealthRelay *)malloc(sizeof(RKTestHealthRelay));
    if (healthRelay == NULL) {
        RKLog("Error. Unable to allocate a test pedestal.\n");
        exit(EXIT_FAILURE);
    }
    memset(healthRelay, 0, sizeof(RKHealthRelay));
    sprintf(healthRelay->name, "%s<HealthRelayCast>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(RKEngineColorHealthRelayTweeta) : "",
            rkGlobalParameters.showColor ? RKNoColor : "");
    healthRelay->memoryUsage = sizeof(RKTestPedestal);
    healthRelay->radar = radar;
    healthRelay->state = RKEngineStateAllocated;
    
    // Parse input here if there is any
    
    healthRelay->state |= RKEngineStateActivating;
    if (pthread_create(&healthRelay->tidRunLoop, NULL, RKTestHealthRelayRunLoop, healthRelay)) {
        RKLog("%s. Unable to create pedestal run loop.\n", healthRelay->name);
    }
    //RKLog("Pedestal input = '%s'\n", input == NULL ? "(NULL)" : input);
    while (!(healthRelay->state & RKEngineStateActive)) {
        usleep(10000);
    }
    
    return (RKHealthRelay)healthRelay;
}

int RKTestHealthRelayExec(RKHealthRelay healthRelayReference, const char *command, char *response) {
    RKTestHealthRelay *healthRelay = (RKTestHealthRelay *)healthRelayReference;
    RKRadar *radar = healthRelay->radar;

    if (!strcmp(command, "disconnect")) {
        if (healthRelay->state & RKEngineStateWantActive) {
            healthRelay->state ^= RKEngineStateWantActive;
            if (radar->desc.initFlags & RKInitFlagVerbose) {
                RKLog("%s Disconnecting ...", healthRelay->name);
            }
        } else {
            if (radar->desc.initFlags & RKInitFlagVerbose) {
                RKLog("%s Deactivate multiple times\n", healthRelay->name);
            }
            return RKResultEngineDeactivatedMultipleTimes;
        }
        healthRelay->state |= RKEngineStateDeactivating;
        pthread_join(healthRelay->tidRunLoop, NULL);
        healthRelay->state ^= RKEngineStateDeactivating;
        if (response != NULL) {
            sprintf(response, "ACK. Transceiver stopped." RKEOL);
        }
        if (radar->desc.initFlags & RKInitFlagVerbose) {
            RKLog("%s Stopped.\n", healthRelay->name);
        }
    } else if (!strcmp(command, "help")) {
        sprintf(response,
                "Commands:\n"
                UNDERLINE("help") " - Help list\n"
                );
    } else if (response != NULL) {
        sprintf(response, "NAK. Command not understood." RKEOL);
    }
    
    return RKResultSuccess;
}

int RKTestHealthRelayFree(RKHealthRelay healthRelayReference) {
    RKTestHealthRelay *healthRelay = (RKTestHealthRelay *)healthRelayReference;
    free(healthRelay);
    return RKResultSuccess;
}

#pragma mark - Others

void RKTestSingleCommand(void) {
    SHOW_FUNCTION_NAME
}

void RKTestExperiment(void) {
    SHOW_FUNCTION_NAME
    // For Bird bath ZCal
    // o  A pedestal command to scan 360-deg at 90-deg EL
    // o  Wait for a sweep to complete
    // o  Stop the pedestal
    // o  Get the sweep, derive average ZDR
    // o  Derive ZCal
    
    // For SunCal
    // o  Get time of day --> elevation of the Sun
    // o  Pedestal command to scan 360-deg at Sun's EL
    // o  Wait for a sweep to complete
    // o  Find the peak of Sh + Sv
    // o  Derive heading
    
    // RadarKit should not know what is the pedestal command
    // One option: Have user fill in the task variables:
    // - Pedestal command for RKPedestalExec()
    // - Transceiver command for RKTransceiverExec()
    // - HealthRelay command for RKHealthRelayExec()
    // - Sweep end flag is defined here so waiting for a sweep complete is okay
    //   - Have the task sub-routine detect if the sweep is progressing as expected
    // - Stop command for RKPedestalExec()
    // - Stop command for RKTransceiverExec()
    // - Stop command for RKHealthRelayExec()
    // - Task function to modify pref.conf or user definied config file
    
//    fftwf_complex *in, *out;
//    uint32_t planSize = 1 << 8;
//    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&in, RKSIMDAlignSize, planSize * sizeof(fftwf_complex)))
//    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&out, RKSIMDAlignSize, planSize * sizeof(fftwf_complex)))
//
//    fftwf_plan fwd = fftwf_plan_dft_1d(planSize, in, in, FFTW_FORWARD, FFTW_MEASURE);
//
//    fftwf_plan plan = fwd;
//
//    printf("sizeof(fftwf_plan) = %d\n", (int)sizeof(fftwf_plan));
//    printf("%p == %p\n", fwd, plan);

//    char filename[] = "/Users/boonleng/Documents/iRadar/data/PX-20170220-050706-E2.4-Z.nc";
    //char symbol[8];
    //RKGetSymbolFromFilename(filename, symbol);
    
    //printf("symbol = %s\n", symbol);
//    printf("%s\n", RKLastNPartsOfPath(filename, 3));
    
//    const char filename[] = "PX1000/PX-20180724-212617-E2.0-Z.nc";
    const char filename[] = "P";
//    char symbol[8];
//    bool found = RKGetSymbolFromFilename(filename, symbol);
//    printf("%s\n", filename);
//    printf("%s\n", RKVariableInString("found", &found, RKValueTypeBool));
    char list[16][RKMaximumPathLength];
    RKProductCollection *productCollection = (RKProductCollection *)malloc(sizeof(RKProductCollection));
    productCollection->count = RKListFilesWithSamePrefix(filename, list);
    free(productCollection);
}

#pragma mark -
