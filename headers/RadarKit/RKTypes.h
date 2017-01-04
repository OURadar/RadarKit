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
#define RKBufferCSlotCount               16           // Config
#define RKBuffer0SlotCount               5000         // Raw I/Q
#define RKBuffer1SlotCount               200          //
#define RKBuffer2SlotCount               4000         // Ray
#define RKGateCount                      32768        // Must power of 2!
#define RKLagCount                       5            // Number lags of ACF / CCF lag = +/-4 and 0
#define RKSIMDAlignSize                  64           // SSE 16, AVX 32, AVX-512 64
#define RKMaxMatchedFilterCount          4            // Maximum filter count within each filter group. Check RKPulseParameters
#define RKMaxMatchedFilterGroupCount     8            // Maximum filter group count
#define RKWorkerDutyCycleBufferSize      1000
#define RKMaxPulsesPerRay                2000
#define RKMaxProductCount                6
#define RKMaxPacketSize                  1024 * 1024
#define RKClientDefaultTimeoutSeconds    10

/*! @/definedblock */

#define RKMaximumStringLength            1024

typedef uint8_t   RKBoolean;
typedef int8_t    RKByte;
typedef uint64_t  RKEnum;
typedef float     RKFloat;   // We can change this to double if we decided one day
typedef ssize_t   RKResult;  // Generic return from functions, 0 for no errors and !0 for others.

#pragma pack(push, 1)

/// Fundamental unit of a (16-bit) + (16-bit) raw complex IQ sample
typedef struct RKInt16C {
    int16_t i;
    int16_t q;
} RKInt16C;

/// Fundamental unit of a (float) + (float) raw complex IQ sample
typedef struct RKComplex {
    RKFloat i;
    RKFloat q;
} RKComplex;

//! Deinterleaved complex format for vector library
typedef struct RKIQZ {
    RKFloat *i;
    RKFloat *q;
} RKIQZ;

// A convenient way to convert bytes into several other types
typedef union rk_user_data {
    struct { uint8_t byte[4]; };
    struct { uint16_t u16, u16_2; };
    struct { int16_t i16, i16_2; };
    struct { uint32_t u32; };
    struct { float f; };
} RKFourByte;

// A running configuration buffer
typedef struct RKOperatingParameters {
    uint32_t                  n;
    uint32_t                  prf[RKMaxMatchedFilterCount];
    uint32_t                  gateCount[RKMaxMatchedFilterCount];
    uint32_t                  waveformId[RKMaxMatchedFilterCount];
    char                      vcpDefinition[RKMaximumStringLength];
} RKOperatingParameters;

/*!
 @typedef RKPulseHeader
 @brief Fundamental unit of the pulse header that is designed to be SIMD alignment friendly.
 Changing this structure would have a major impacts on the workingness of the entiire framework.
 It is imperative to maintain the SIMD alignment whenever new fields are added.
 
 12/29/2016 - I think I have made this immunie to non-SIMD size header.
            - As long as  sizeof(RKPulseHeader) + sizeof(RKPulseParameters) is multiples of 
              SIMD align size, we should be okay the way the buffer is designed.
 
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
 @param configIndex            Configuration index
 @param configSubIndex         Configuration sub-index
 @param gateCount              Number of usable gates in this pulse
 @param azimuthBinIndex        Azimuth bin index
 @param gateSizeMeters         Gate size in meters
 @param azimuthDegrees         Azimuth in degrees
 @param elevationDegrees       Elevation in degrees
 @param vazDps                 Velocity of azimuth in degrees per second
 @param velDps                 Velocity of azimuth in degrees per second
 */
typedef struct rk_pulse_header {
    uint32_t    capacity;
    uint32_t    s;
    uint32_t    i;
    uint32_t    n;
    uint32_t    marker;
    uint32_t    pulseWidthSampleCount;
    uint32_t    reserved1;
    uint32_t    reserved2;
    uint32_t    timeSec;
    uint32_t    timeUSec;
    double      timeDouble;
    uint16_t    az;
    uint16_t    el;
    uint16_t    configIndex;
    uint16_t    configSubIndex;
    uint16_t    azimuthBinIndex;
    uint16_t    gateCount;
    float       gateSizeMeters;
    float       azimuthDegrees;
    float       elevationDegrees;
    float       vazDps;
    float       velDps;
} RKPulseHeader;

// Make sure the size (bytes) can cover all the struct elements and still conform to SIMD alignemt
typedef struct rk_pulse_parameters {
    uint32_t    filterCounts[2];
    uint32_t    planIndices[2][RKMaxMatchedFilterCount];
    uint32_t    planSizes[2][RKMaxMatchedFilterCount];
} RKPulseParameters;

// RKPulse struct is carefully designed to obey the SIMD alignment
typedef struct rk_pulse {
    union {
        struct {
            RKPulseHeader      header;
            RKPulseParameters  parameters;
        };
        RKByte                 headerBytes[256];
    };
    RKByte                     data[0];
} RKPulse;


typedef uint32_t RKPulseStatus;
enum RKPulseStatus {
    RKPulseStatusVacant       = 0,
    RKPulseStatusHasIQData    = 1,                                                    // 0x01
    RKPulseStatusHasPosition  = 1 << 1,                                               // 0x02
    RKPulseStatusReady        = RKPulseStatusHasIQData | RKPulseStatusHasPosition,    // 0x03
    RKPulseStatusInspected    = 1 << 2,
    RKPulseStatusCompressed   = 1 << 3,
    RKPulseStatusSkipped      = 1 << 4,
    RKPulseStatusProcessed    = 1 << 5
};

typedef uint32_t RKRayStatus;
enum RKRayStatus {
    RKRayStatusVacant        = 0,
    RKRayStatusProcessing    = 1,
    RKRayStatusProcessed     = 1 << 1,
    RKRayStatusSkipped       = 1 << 2,
    RKRayStatusReady         = 1 << 3,
    RKRayStatusUsedOnce      = 1 << 4
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
    uint32_t       capacity;
    RKRayStatus    s;
    uint32_t       i;
    uint32_t       n;
    uint32_t       marker;
    uint32_t       reserved1;
    uint16_t       configIndex;
    uint16_t       configSubIndex;
    uint16_t       gateCount;
    uint16_t       reserved2;
    uint32_t       startTimeSec;
    uint32_t       startTimeUSec;
    double         startTimeD;
    uint32_t       endTimeSec;
    uint32_t       endTimeUSec;
    double         endTimeD;
    float          startAzimuth;
    float          endAzimuth;
    float          startElevation;
    float          endElevation;
} RKRayHeader;

typedef struct RKRay {
    union {
        RKRayHeader  header;
        RKByte       headerBytes[128];
    };
    RKByte           data[0];
} RKRay;

enum RKResult {
    RKResultTimeout = -1000,
    RKResultIncompleteSend,
    RKResultIncompleteReceive,
    RKResultErrorCreatingOperatorRoutine,
    RKResultErrorCreatingClientRoutine,
    RKResultSDToFDError,
    RKResultNoPulseBuffer,
    RKResultNoRayBuffer,
    RKResultNoPulseCompressionEngine,
    RKResultNoMomentEngine,
    RKResultFailedToStartCompressionCore,
    RKResultFailedToStartPulseWatcher,
    RKResultFailedToInitiateSemaphore,
    RKResultFailedToRetrieveSemaphore,
    RKResultTooBig,
    RKResultFailedToAllocateFFTSpace,
    RKResultFailedToAllocateFilter,
    RKResultFailedToAllocateDutyCycleBuffer,
    RKResultFailedToAllocateScratchSpace,
    RKResultFailedToAddFilter,
    RKResultEngineDeactivatedMultipleTimes,
    RKResultFailedToStartMomentCore,
    RKResultFailedToStartPulseGatherer,
    RKResultUnableToChangeCoreCounts,
    RKResultSuccess = 0,
    RKResultNoError = 0
};

typedef struct RKModuloPath {
    uint32_t      origin;
    uint32_t      length;
    uint32_t      modulo;
} RKModuloPath;

typedef void * RKTransceiver;
typedef void * RKPedestal;

typedef struct rk_scratch {
    bool             showNumbers;
    uint8_t          lagCount;
    RKIQZ            mX[2];                                // Mean of X, 2 for dual-pol
    RKIQZ            vX[2];                                // Variance of X, i.e., E{X' * X} - E{X}' * E{X}
    RKIQZ            R[2][RKLagCount];                     // ACF up to RKLagCount - 1 for each polarization
    RKIQZ            C[2 * RKLagCount - 1];                // CCF in [ -RKLagCount + 1, ..., -1, 0, 1, ..., RKLagCount - 1 ]
    RKIQZ            sC;                                   // Summation of Xh * Xv'
    RKIQZ            ts;                                   // Temporary scratch space
    RKFloat          *aR[2][RKLagCount];                   // abs(ACF)
    RKFloat          *aC[2 * RKLagCount - 1];              // abs(CCF)
    RKFloat          *gC;                                  // Gaussian fitted CCF(0)  NOTE: Need to extend this to multi-multilag
} RKScratch;

typedef struct rk_position {
    uint64_t         c;                                    // Counter
    uint32_t         tic;                                  // Time tic
    RKFourByte       rawElevation;                         // Raw elevation readout
    RKFourByte       rawAzimuth;                           // Raw azimuth readout
    RKFourByte       rawElevationVelocity;                 // Raw velocity of elevation readout
    RKFourByte       rawAzimuthVelocity;                   // Raw velocity of azimuth readout
    RKFourByte       rawElevationStatus;                   // Raw status of elevation readout
    RKFourByte       rawAzimuthStatus;                     // Raw status of azimuth readout
    uint8_t          queueSize;                            // Queue size of the readout buffer
    uint8_t          elevationMode;                        // Positioning mode of elevation
    uint8_t          azimuthMode;                          // Positioning mode of azimuth
    uint8_t          sequence;                             // DEBUG command sequence
    uint32_t         flag;                                 // Position flag
    float            elevationDegrees;                     // Decoded elevation
    float            azimuthDegrees;                       // Decoded elevation
    float            elevationVelocityDegreesPerSecond;    // Decoded velocity of elevation
    float            azimuthVelocityDegreesPerSecond;      // Decoded velocity of azimuth
    float            elevationCounter;                     // Progress counter (of target) of the elevation
    float            elevationTarget;                      // Targeted progress counter of the elevation
    float            azimuthCounter;                       // Progress counter (of target) of the azimuth
    float            azimuthTarget;                        // Targeted progress counter of the azimuth
    float            sweepElevationDegrees;                // Set elevation for current sweep
    float            sweepAzimuthDegrees;                  // Set azimuth for current sweep
} RKPosition;

typedef union rk_block_header {
    struct {
        uint16_t     type;                                 // Type
        uint16_t     subtype;                              // Sub-type
        uint32_t     size;                                 // Raw size in bytes to read / skip ahead
        uint32_t     decodedSize;                          // Decided size if this is a compressed block
    };
    RKByte bytes[32];                                      // Make this struct always 32 bytes
} RKBlockHeader;

#pragma pack(pop)

#endif /* defined(__RadarKit_RKTypes__) */
