//
//  RKTypes.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/17/15.
//
//

#ifndef __RadarKit_RKTypes__
#define __RadarKit_RKTypes__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sched.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

/*!
 @definedblock Memory Blocks
 @abstract Defines the number of slots and gates of each pulse of the RKRadar structure
 @define RKBuffer0SlotCount The number of slots for level-0 pulse storage in the host memory
 @define RKBuffer1SlotCount The number of slots for level-1 pulse storage in the host memory
 @define RKBuffer2SlotCount The number of slots for level-2 pulse storage in the host memory
 @define RKGateCount The maximum number of gates allocated for each pulse
 @define RKSIMDAlignSize The minimum alignment size. AVX requires 256 bits = 32 bytes. AVX-512 is on the horizon now.
 */
#define RKBuffer0SlotCount               5000
#define RKBuffer1SlotCount               200
#define RKBuffer2SlotCount               4000
#define RKGateCount                      32768        // Must power of 2!
#define RKSIMDAlignSize                  64           // SSE 16, AVX 32, AVX-512 64
#define RKMaxMatchedFilterCount          4            // Maximum filter count within each filter group. Check RKPulseParameters
#define RKMaxMatchedFilterGroupCount     8            // Maximum filter group count
#define RKWorkerDutyCycleBufferSize      1000
#define RKMaxPulsesPerRay                1000

/*! @/definedblock */

#define RKMaximumStringLength  1024

typedef uint8_t   RKBoolean;
typedef int8_t    RKByte;
typedef uint64_t  RKEnum;
typedef float     RKFloat;   // We can change this to double if we decided one day
typedef ssize_t   RKResult;  // Generic return from functions, 0 for no errors and !0 for others.

#pragma pack(push, 1)

/// Fundamental unit of a (16-bit) + (16-bit) raw complex IQ sample
typedef struct RKInt16 {
    int16_t i;
    int16_t q;
} RKInt16;

/// Fundamental unit of a (float) + (float) raw complex IQ sample
typedef struct RKComplex {
    RKFloat i;
    RKFloat q;
} RKComplex;

//! Deinterleaved complex format for vector library
typedef struct RKIQZ {
    RKFloat i[RKGateCount];
    RKFloat q[RKGateCount];
} RKIQZ;


/*!
 @typedef RKPulseHeader
 @brief Fundamental unit of the pulse header that is designed to be SIMD alignment friendly.
 Changing this structure would have a major impacts on the workingness of the entiire framework.
 It is imperative to maintain the SIMD alignment whenever new fields are added.
 @param s                      Pulse Status
 @param i                      Pulse Identity
 @param n                      Pulse Network Counter
 @param marker                 Volume / sweep / radial marker
 @param pulseWidthSampleCount  Pulse width in number of samples
 @param timeSec                UNIX time, seconds since Epoch 1970
 @param timeUSec               Microsecond part of the time
 @param timeDouble             Double representation of the time
 @param az                     Azimuth raw reading
 @param el                     Elevation raw reading
 @param prfHz                  PRF (Pulse Repetition Frequency) in Hz since last pulse
 @param prfIdx                 PRF index for multi-PRF operations
 @param gateCount              Number of usable gates in this pulse
 @param azimuthBinIndex        Azimuth bin index
 @param gateSizeMeters         Gate size in meters
 @param azimuthDegrees         Azimuth in degrees
 @param elevationDegrees       Elevation in degrees
 @param vazDps                 Velocity of azimuth in degrees per second
 @param velDps                 Velocity of azimuth in degrees per second
 */
typedef struct rk_pulse_header {
    // First 128-bit chunk
    uint32_t    s;
    uint32_t    i;
    uint32_t    n;
    uint16_t    marker;
    uint16_t    pulseWidthSampleCount;
    
    // Second 128-bit chunk
    uint32_t    timeSec;
    uint32_t    timeUSec;
    double      timeDouble;
    
    // Third 128-bit chunk
    uint16_t    az;
    uint16_t    el;
    uint16_t    prfHz;
    uint16_t    prfIdx;
    uint16_t    gateCount;
    uint16_t    azimuthBinIndex;
    float       gateSizeMeters;
    
    // Fourth 128-bit chunk
    float       azimuthDegrees;
    float       elevationDegrees;
    float       vazDps;
    float       velDps;
} RKPulseHeader;

// Make sure the size (bytes) can cover all the struct elements and still conform to SIMD alignemt
typedef union rk_pulse_parameters {
    struct {
        uint32_t    filterCounts[2];
        uint32_t    planIndices[2][RKMaxMatchedFilterCount];
        uint32_t    planSizes[2][RKMaxMatchedFilterCount];
    };
    char bytes[RKSIMDAlignSize * 2];
} RKPulseParameters;

// RKPulse struct is carefully designed to obey the SIMD alignment
typedef struct rk_pulse {
    RKPulseHeader      header;
    RKPulseParameters  parameters;
    RKInt16            X[2][RKGateCount];
    RKComplex          Y[2][RKGateCount];
    RKIQZ              Z[2];
} RKPulse;


typedef uint32_t RKPulseStatus;
enum RKPulseStatus {
    RKPulseStatusVacant      = 0,
    RKPulseStatusHasIQData   = 1,
    RKPulseStatusHasPosition = 1 << 1,
    RKPulseStatusReady       = RKPulseStatusHasIQData | RKPulseStatusHasPosition,
    RKPulseStatusCompressed  = 1 << 2
};

typedef uint32_t RKRayStatus;
enum RKRayStatus {
    RKRayStatusVacant        = 0,
    RKRayStatusReady         = 1,
    RKRayStatusUsedOnce      = 1 << 1
};

/*!
 @typedef RKRayHeader
 @brief Fundamental unit of the ray header.
 @param s                      Pulse Status
 @param i                      Pulse Identity
 @param n                      Pulse Network Counter
 @param marker                 Volume / sweep / radial marker
 @param reserved1              For future...
 @param startTimeSec           Start time of the ray in UNIX time
 @param startTimeUSec          Start time's microsecond portion
 @param startTimeD             Start time in double representation
 @param endTimeSec             End time of the ray in UNIX time
 @param endTimeUSec            End time's microsecond portion
 @param endTimeD               End time in double representation
 */
typedef struct RKRayHeader {
    RKRayStatus    s;
    uint32_t       i;
    uint32_t       n;
    uint16_t       marker;
    uint16_t       reserved1;
    
    uint32_t       startTimeSec;
    uint32_t       startTimeUSec;
    double         startTimeD;
    
    uint32_t       endTimeSec;
    uint32_t       endTimeUSec;
    double         endTimeD;
} RKRayHeader;

typedef struct RKInt16Ray {
    RKRayHeader    header;
    int16_t        data[RKGateCount];
} RKInt16Ray;

typedef struct RKFloatRay {
    RKRayHeader    header;
    float          data[RKGateCount];
} RKFloatRay;

enum RKResult {
    RKResultTimeout = -1000,
    RKResultIncompleteSend,
    RKResultIncompleteReceive,
    RKResultErrorCreatingOperatorRoutine,
    RKResultSDToFDError,
    RKResultFailedToStartCompressionCore,
    RKResultFailedToStartPulseWatcher,
    RKResultFailedToInitiateSemaphore,
    RKResultFailedToRetrieveSemaphore,
    RKResultTooBig,
    RKResultFailedToAllocateFFTSpace,
    RKResultFailedToAllocateFilter,
    RKResultFailedToAllocateDutyCycleBuffer,
    RKResultFailedToAddFilter,
    RKResultEngineDeactivatedMultipleTimes,
    RKResultFailedToStartPulseGatherer,
    RKResultNoError = 0
};

#pragma pack(pop)

#endif /* defined(__RadarKit_RKTypes__) */
