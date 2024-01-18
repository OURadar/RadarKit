//
//  RKFoundation.h
//  RadarKit
//
//  Created by Boonleng Cheong on 3/17/15.
//
//

#ifndef __RadarKit_Foundation__
#define __RadarKit_Foundation__

#include <RadarKit/RKTypesCompat.h>
#include <RadarKit/RKTypes.h>
#include <RadarKit/RKMisc.h>
#include <RadarKit/RKSIMD.h>

#define RKDefaultLogfile                 "messages.log"
#define RKEOL                            "\r\n"

// Compute the next/previous N-stride location with size S
#define RKNextNModuloS(i, N, S)          ((i) >= (S) - (N) ? (i) + (N) - (S) : (i) + (N))
#define RKPreviousNModuloS(i, N, S)      ((i) < (N) ? (S) + (i) - (N) : (i) - (N))

// Compute the next/previous location with size S
#define RKNextModuloS(i, S)              ((i) == (S) - 1 ? 0 : (i) + 1)
#define RKPreviousModuloS(i, S)          ((i) == 0 ? (S) - 1 : (i) - 1)

// Compute the modulo lag, assume h is always ahead of t
#define RKModuloLag(h, t, S)             ((h) < (t) ? (h) + (S) - (t) : (h) - (t))

//
// Z in [-32.0    ... 95.5]             Zi = (Z) x 2 + 64
// V in [-16.0    ... 15.875]           Vi = (V) x 8 + 128
// W in [ -0.05   ... 12.7]             Wi = (W) x 20 + 1
// D in [-10.0    ... 15.5]             Di = (D) x 10 + 100
// P in [  -PI    ... PI*(N-1)/N]       Pi = (P) x 128 / M_PI + 129
// R in [  0.0    ... 1.079]
// K in [ -0.1*PI ... 0.1*PI*(N-1)/N]   Ki = (K) x 1280 / M_PI + 128
// Q in [  0.0    ... 1.0]              Ri = (Q) * 250
// L in [-40.0    ... 2.5]              Li = (L) x 6 + 240
//
// Z  in [CLR  -31.50   -31.00  ... 95.5]            Z = (Zi) * 0.5 - 32.0
// V  in [CLR  -15.875  -15.750 ... 15.875]          V = (Vi) * 0.125 - 16.0
// W  in [CLR    0.00     0.05  ... 12.70]           W = (Wi) * 0.05 - 0.05
// D  in [CLR   -9.90    -9.80  ... 15.50]           D = (Di) * 0.10 - 10.0
// P  in [CLR    -PI+    -PI++  ... +PI]             P = (Pi) * M_PI / 128.0 - M_PI
// R  in [CLR   0.019    0.038  ... 1.079]
// K  in [CLR    0.00   -0.1*PI ... +0.1*PI]         K = (Ki) * 0.1 * M_PI - 0.1 * M_PI
// Q  in [CLR    0.004   0.008  ... 1.0]             Q = (Qi) / 250
// VE in [CLR   -63.5   -63.0  ... 63.5]             V = (Vi) * 0.5 + 64.0
//

#define RKZLHMAC  { lhma[0] = -32.0f;     lhma[1] = 95.5f;       lhma[2] = 2.0f;      lhma[3] =  64.0f; }  //
#define RKVLHMAC  { lhma[0] = -16.0f;     lhma[1] = 15.875f;     lhma[2] = 8.0f;      lhma[3] = 128.0f; }  //
#define RKWLHMAC  { lhma[0] = -0.05f;     lhma[1] = 12.70f;      lhma[2] = 20.0f;     lhma[3] =   1.0f; }  //
#define RKDLHMAC  { lhma[0] = -10.0f;     lhma[1] = 15.5f;       lhma[2] = 10.0f;     lhma[3] = 100.0f; }  //
#define RKPLHMAC  { lhma[0] = -3.16623f;  lhma[1] = 3.11695f;    lhma[2] = 40.5845f;  lhma[3] = 128.5f; }  //  pi - 2*pi/255
#define RKKLHMAC  { lhma[0] = -0.558508f; lhma[1] = 0.55414249f; lhma[2] = 229.1831f; lhma[3] = 128.0f; }  //  -0.2*pi + 0.4*pi/255
#define RKRLHMAC  { lhma[0] = 0.0f;       lhma[1] = 1.079f;      lhma[2] = 1.0f;      lhma[3] =   0.0f; }  //
#define RKV2LHMAC { lhma[0] = -32.0f;     lhma[1] = 31.75f;      lhma[2] = 4.0f;      lhma[3] = 128.0f; }  //
#define RKV3LHMAC { lhma[0] = -64.0f;     lhma[1] = 63.5f;       lhma[2] = 2.0f;      lhma[3] = 128.0f; }  //
#define RKQLHMAC  { lhma[0] = 0.0f;       lhma[1] = 1.0f;        lhma[2] = 250.0f;    lhma[3] =   0.0f; }  //
#define RKSLHMAC  { lhma[0] = -120.0f;    lhma[1] = 0.0f;        lhma[2] = 250.0f;    lhma[3] = -96.3f; }  // Asuumed 16-bit, 96 dB, peak @ 0 dBm
#define RKLLHMAC  { lhma[0] = -40.0f;     lhma[1] = 2.5f;        lhma[2] = 6.0f;      lhma[3] = 240.0f; }  //

//#define RKRho2Uint8(r)    (r > 0.93f ? roundf((r - 0.93f) * 1000.0f) + 106.0f : (r > 0.7f ? roundf((r - 0.7f) * 300.0f) + 37.0f : roundf(r * 52.8571f)))
#define RKRho2Uint8(r)    roundf(r > 0.93f ? r * 1000.0f - 824.0f : (r > 0.7f ? r * 300.0f - 173.0f : r * 52.8571f))

#define RKSingleWrapTo2PI(x)   ((x) < -M_PI ? ((x) + 2.0f * M_PI) : ((x) >= M_PI ? ((x) - 2.0f * M_PI) : (x)))

#define RKInstructIsAzimuth(i)     ((i & RKPedestalInstructTypeAxisMask) == RKPedestalInstructTypeAxisAzimuth)
#define RKInstructIsElevation(i)   ((i & RKPedestalInstructTypeAxisMask) == RKPedestalInstructTypeAxisElevation)
#define RKInstructIsNone(i)        ((i & RKPedestalInstructTypeModeMask) == RKPedestalInstructTypeNone)
#define RKInstructIsTest(i)        ((i & RKPedestalInstructTypeModeMask) == RKPedestalInstructTypeModeTest)
#define RKInstructIsSlew(i)        ((i & RKPedestalInstructTypeModeMask) == RKPedestalInstructTypeModeSlew)
#define RKInstructIsPoint(i)       ((i & RKPedestalInstructTypeModeMask) == RKPedestalInstructTypeModePoint)
#define RKInstructIsStandby(i)     ((i & RKPedestalInstructTypeModeMask) == RKPedestalInstructTypeModeStandby)
#define RKInstructIsDisable(i)     ((i & RKPedestalInstructTypeModeMask) == RKPedestalInstructTypeModeDisable)
#define RKInstructIsEnable(i)      ((i & RKPedestalInstructTypeModeMask) == RKPedestalInstructTypeModeEnable)

typedef struct RKGlobalParameterStruct {
    char             program[32];                                                                  // Name of the program in log
    char             logfile[RKMaximumPathLength];                                                 // Name of the log file. This is ignored when dailyLog = true
    char             logFolder[256];                                                               // Log folder. This has priority, otherwise, logs are in {rootDataFolder}/log
    char             rootDataFolder[256];                                                          // Root folder where iq, moment health and log files are stored
    bool             logTimeOnly;                                                                  // Time stamp of log entries
    bool             dailyLog;                                                                     // Daily mode where log file is {logFolder}/YYYYMMDD.log
    bool             showColor;                                                                    // Show colors on screen
    bool             statusColor;                                                                  // Color terminal
    pthread_mutex_t  lock;                                                                         // Mutual exclusive access
    FILE             *stream;                                                                      // Secondary output stream, can be NULL
} RKGlobalParameters;

extern RKGlobalParameters rkGlobalParameters;
extern const char * const rkResultStrings[];

#pragma mark - Common Functions

// Basic
RKComplex RKComplexAdd(const RKComplex, const RKComplex);
RKComplex RKComplexSubtract(const RKComplex, const RKComplex);
RKComplex RKComplexMultiply(const RKComplex, const RKComplex);
RKComplex RKComplexConjugate(const RKComplex);
RKFloat RKComplexAbsSquare(const RKComplex);

// Array
void RKComplexArrayInConjugate(RKComplex *srcdst, const int);                                      // srcdst = conj(srcdst)
void RKComplexArrayInPlaceAdd(RKComplex *src, RKComplex *dst, const int);                          // dst = src + dst
void RKComplexArrayInPlaceSubtract(RKComplex *src, RKComplex *dst, const int);                     // dst = src - dst
void RKComplexArrayInPlaceMultiply(RKComplex *src, RKComplex *dst, const int);                     // dst = src * dst
void RKComplexArrayInPlaceConjugateMultiply(RKComplex *src, RKComplex *dst, const int);            // dst = src * conj(dst)

// Reduction
RKFloat RKFloatArraySum(RKFloat *src, const int);
RKComplex RKComplexArraySum(RKComplex *src, const int);

// Log
int RKLog(const char *, ...);
void RKExit(int);

// File operations
FILE *RKFileOpen(const char *, const char *);
int RKFileClose(FILE *);
long RKFileTell(FILE *);
size_t RKFileGetSize(FILE *);
int RKFileSeek(FILE *, long);

// Variables in rkGlobalVariable / Presentation
void RKSetStatusColor(const bool);
void RKSetWantColor(const bool);
void RKSetWantScreenOutput(const bool);
bool RKGetWantScreenOutput(void);
void RKSetUseDailyLog(const bool);
int RKSetProgramName(const char *);
int RKSetRootFolder(const char *) __attribute__ ((deprecated));
int RKSetRootDataFolder(const char *);
int RKSetLogfile(const char *);
int RKSetLogfileToDefault(void);

char *RKVersionString(void);
RKValueType RKGuessValueType(const char *);

// Filename / string
bool RKGetSymbolFromFilename(const char *filename, char *symbol);
bool RKGetPrefixFromFilename(const char *filename, char *prefix);
int RKListFilesWithSamePrefix(const char *filename, char list[][RKMaximumPathLength]);

// Common numeric output
void RKShowName(void);
void RKShowTypeSizes(void);
void RKShowVecFloatLowPrecision(const char *name, const float *p, const int n);
void RKShowVecFloat(const char *name, const float *p, const int n);
void RKShowVecIQZ(const char *name, const RKIQZ *p, const int n);
void RKShowVecComplex(const char *name, const RKComplex *p, const int n);
void RKShowArray(const RKFloat *data, const char *letter, const int width, const int height);
char *RKStringFromValue(const void *value, RKValueType type);
char *RKVariableInString(const char *name, const void *value, RKValueType type);
size_t RKPrettyStringSizeEstimate(const char *);
size_t RKPrettyStringFromKeyValueString(char *, const char *);

// Clearing buffer
void RKZeroOutFloat(RKFloat *data, const uint32_t capacity);
void RKZeroOutIQZ(RKIQZ *data, const uint32_t capacity);
void RKZeroTailFloat(RKFloat *data, const uint32_t capacity, const uint32_t origin);
void RKZeroTailIQZ(RKIQZ *data, const uint32_t capacity, const uint32_t origin);

// Pulse
size_t RKPulseBufferAlloc(RKBuffer *, const uint32_t capacity, const uint32_t count);
void RKPulseBufferFree(RKBuffer);
RKPulse *RKGetPulseFromBuffer(RKBuffer, const uint32_t pulseIndex);
RKInt16C *RKGetInt16CDataFromPulse(RKPulse *, const uint32_t);
RKComplex *RKGetComplexDataFromPulse(RKPulse *, const uint32_t);
RKIQZ RKGetSplitComplexDataFromPulse(RKPulse *, const uint32_t);
int RKClearPulseBuffer(RKBuffer, const uint32_t);
int RKReadPulseFromFileReference(RKPulse *, RKFileHeader *, FILE *);
RKPulse *RKGetVacantPulseFromBuffer(RKBuffer, uint32_t *, const uint32_t);
RKBuffer RKPulseBufferAllocCopyFromBuffer(RKBuffer pulses, const uint32_t start, const uint32_t count, const uint32_t depth);

// Ray
size_t RKRayBufferAlloc(RKBuffer *, const uint32_t capacity, const uint32_t count);
void RKRayBufferFree(RKBuffer);
RKRay *RKGetRayFromBuffer(RKBuffer, const uint32_t);
int16_t *RKGetInt16DataFromRay(RKRay *, const RKMomentIndex);
uint8_t *RKGetUInt8DataFromRay(RKRay *, const RKBaseProductIndex);
float *RKGetFloatDataFromRay(RKRay *, const RKBaseProductIndex);
int RKClearRayBuffer(RKBuffer buffer, const uint32_t);
RKRay *RKGetVacantRayFromBuffer(RKBuffer, uint32_t *, const uint32_t);

// Standalone file monitor (one file per thread)
RKFileMonitor *RKFileMonitorInit(const char *filename, void (*)(void *), void *);
int RKFileMonitorFree(RKFileMonitor *);

// Stream symbols / binary
RKStream RKStreamFromString(const char *);
char *RKStringOfStream(RKStream);
int RKStringFromStream(char *, RKStream);
//int RKGetNextProductDescription(char *symbol, char *name, char *unit, char *colormap, RKBaseProductIndex *, RKBaseProductList *);
RKProductDesc RKGetNextProductDescription(RKBaseProductList *);

// Parser, enum, strings
size_t RKParseCommaDelimitedValues(void *, RKValueType , const size_t, const char *);
size_t RKParseNumericArray(void *, RKValueType, const size_t, const char *);
void RKParseQuotedStrings(const char *source, ...);
void RKMakeJSONStringFromControls(char *, RKControl *, uint32_t count);
RKStatusEnum RKValueToEnum(RKConst value, RKConst tlo, RKConst lo, RKConst nlo, RKConst nhi, RKConst hi, RKConst thi);
RKStatusEnum RKStatusFromTemperatureForCE(RKConst value);
RKStatusEnum RKStatusFromTemperatureForIE(RKConst value);
RKStatusEnum  RKStatusFromTemperatureForComputers(RKConst value);
bool RKFindCondition(const char *, const RKStatusEnum, const bool, char *firstKey, char *firstValue);
bool RKAnyCritical(const char *, const bool, char *firstKey, char *firstValue);
int RKParseProductDescription(RKProductDesc *, const char *);
RKProductId RKProductIdFromString(const char *);
RKIdentifier RKIdentifierFromString(const char *);

// Simple engine
int RKSimpleEngineFree(RKSimpleEngine *);

// FIFO command queue
RKCommandQueue *RKCommandQueueInit(const uint16_t, const bool);
RKCommand *RKCommandQueuePop(RKCommandQueue *);
int RKCommandQueuePush(RKCommandQueue *, RKCommand *);
int RKCommandQueueFree(RKCommandQueue *);

// RKPedestalActionString
char *RKPedestalActionString(const RKScanAction *);

#endif /* defined(__RadarKit_RKFoundation__) */
