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
#define RKVersionString                  "1.0"
#define RKBufferCSlotCount               16                          // Config
#define RKBufferHSlotCount               50                          // Health
#define RKBufferSSlotCount               90                          // Status strings
#define RKBufferPSlotCount               500                         // Positions
#define RKBuffer0SlotCount               5000                        // Raw I/Q
#define RKBuffer2SlotCount               1440                        // Ray
#define RKGateCount                      32768                       // Must power of 2!
#define RKLagCount                       5                           // Number lags of ACF / CCF lag = +/-4 and 0
#define RKSIMDAlignSize                  64                          // SSE 16, AVX 32, AVX-512 64
#define RKMaxMatchedFilterCount          4                           // Maximum filter count within each filter group. Check RKPulseParameters
#define RKMaxMatchedFilterGroupCount     8                           // Maximum filter group count
#define RKWorkerDutyCycleBufferDepth     1000
#define RKMaxPulsesPerRay                2000
#define RKMaxProductCount                10                          // 16 to be the absolute max since productList enum is 32-bit
#define RKMaxRaysPerSweep                1500                        // 1440 is 0.25-deg. This should be plenty
#define RKMaxPacketSize                  1024 * 1024
#define RKNetworkTimeoutSeconds          20
#define RKNetworkReconnectSeconds        3
#define RKLagRedThreshold                0.5
#define RKLagOrangeThreshold             0.7
#define RKDutyCyleRedThreshold           0.95
#define RKDutyCyleOrangeThreshold        0.90
#define RKStatusBarWidth                 10

#define RKNoColor                        "\033[0m"
#define RKMaximumStringLength            4096
#define RKNameLength                     64
#define RKPulseHeaderPaddedSize          256                         // Change this to higher number for post-AVX2 intrinsics
#define RKRayHeaderPaddedSize            128                         // Change this to higher number for post-AVX2 intrinsics

#define RKColorDutyCycle(x)  (x > RKDutyCyleRedThreshold ? "\033[91m" : (x > RKDutyCyleOrangeThreshold ? "\033[93m" : "\033[92m"))
#define RKColorLag(x)        (x > RKLagRedThreshold      ? "\033[91m" : (x > RKLagOrangeThreshold      ? "\033[93m" : "\033[92m"))

/*! @/definedblock */


typedef uint8_t   RKBoolean;
typedef int8_t    RKByte;
typedef float     RKFloat;           // We can change this to double if we decided one day
typedef ssize_t   RKResult;          // Generic return from functions, 0 for no errors and !0 for others.
typedef void *    RKBuffer;
typedef void *    RKTransceiver;
typedef void *    RKPedestal;
typedef void *    RKHealthRelay;

#pragma pack(push, 1)

/// Fundamental unit of a (16-bit) + (16-bit) raw complex IQ sample
typedef struct rk_int16c {
    int16_t i;
    int16_t q;
} RKInt16C;

/// Fundamental unit of a (float) + (float) raw complex IQ sample
typedef struct rk_complex {
    RKFloat i;
    RKFloat q;
} RKComplex;

//! Deinterleaved complex format for vector library
typedef struct rk_iqz {
    RKFloat *i;
    RKFloat *q;
} RKIQZ;

// A convenient way to convert bytes into several other types
typedef union rk_four_byte {
    struct { uint8_t byte[4]; };
    struct { uint16_t u16, u16_2; };
    struct { int16_t i16, i16_2; };
    struct { uint32_t u32; };
    struct { float f; };
} RKFourByte;

enum RKResult {
    RKResultTimeout = -99,
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
    RKResultFailedToStartPedestalWorker,
    RKResultFailedToGetVacantPosition,
    RKResultFailedToStartRayGatherer,
    RKResultFailedToStartHealthWorker,
    RKResultSuccess = 0,
    RKResultNoError = 0
};

typedef uint32_t RKPositionFlag;
enum RKPositionFlag {
    RKPositionFlagVacant             = 0,
    RKPositionFlagAzimuthEnabled     = 1,
    RKPositionFlagAzimuthSafety      = (1 << 1),
    RKPositionFlagAzimuthError       = (1 << 2),
    RKPositionFlagAzimuthSweep       = (1 << 8),
    RKPositionFlagAzimuthPoint       = (1 << 9),
    RKPositionFlagAzimuthComplete    = (1 << 10),
    RKPositionFlagElevationEnabled   = (1 << 16),
    RKPositionFlagElevationSafety    = (1 << 17),
    RKPositionFlagElevationError     = (1 << 18),
    RKPositionFlagElevationSweep     = (1 << 24),
    RKPositionFlagElevationPoint     = (1 << 25),
    RKPositionFlagElevationComplete  = (1 << 26),
    RKPositionFlagActive             = (1 << 28),
    RKPositionFlagHardwareMask       = 0x3FFFFFFF,
    RKPositionFlagReady              = (1 << 31)
};

typedef uint32_t RKHealthFlag;
enum RKHealthFlag {
    RKHealthFlagVacant               = 0,
    RKHealthFlagReady                = 1
};

typedef uint32_t RKMarker;
enum RKMarker {
    RKMarkerNull                     = 0,
    RKMarkerSweepMiddle              = 1,
    RKMarkerSweepBegin               = (1 << 1),
    RKMarkerSweepEnd                 = (1 << 2),
    RKMarkerVolumeBegin              = (1 << 3),
    RKMarkerVolumeEnd                = (1 << 4),
    RKMarkerPPIScan                  = (1 << 8),
    RKMarkerRHIScan                  = (1 << 9),
    RKMarkerPointScan                = (1 << 10)
};

typedef uint32_t RKPulseStatus;
enum RKPulseStatus {
    RKPulseStatusVacant              = 0,
    RKPulseStatusHasIQData           = 1,                            // 0x01
    RKPulseStatusHasPosition         = (1 << 1),                     // 0x02
    RKPulseStatusInspected           = (1 << 2),
    RKPulseStatusCompressed          = (1 << 3),
    RKPulseStatusSkipped             = (1 << 4),
    RKPulseStatusProcessed           = (1 << 5),
    RKPulseStatusReadyForMoment      = (RKPulseStatusProcessed | RKPulseStatusHasPosition)
};

typedef uint32_t RKRayStatus;
enum RKRayStatus {
    RKRayStatusVacant                = 0,
    RKRayStatusProcessing            = 1,
    RKRayStatusProcessed             = (1 << 1),
    RKRayStatusSkipped               = (1 << 2),
    RKRayStatusReady                 = (1 << 3),
    RKRayStatusUsedOnce              = (1 << 4)
};

typedef uint32_t RKInitFlag;
enum RKInitFlag {
    RKInitFlagNone                   = 0,
    RKInitFlagVerbose                = 1,
    RKInitFlagVeryVerbose            = (1 << 1),
    RKInitFlagVeryVeryVerbose        = (1 << 2),
    RKInitFlagAllocMomentBuffer      = (1 << 8),
    RKInitFlagAllocRawIQBuffer       = (1 << 9),
    RKInitFlagAllocEverything        = (RKInitFlagAllocMomentBuffer | RKInitFlagAllocRawIQBuffer | RKInitFlagVerbose),
    RKInitFlagAllocEverythingQuiet   = (RKInitFlagAllocMomentBuffer | RKInitFlagAllocRawIQBuffer),
};

typedef uint32_t RKProductList;
enum RKProductList {
    RKProductListDisplayZ            = (1),                          // Display Z - Reflectivity dBZ
    RKProductListDisplayV            = (1 << 1),                     // Display V - Velocity
    RKProductListDisplayW            = (1 << 2),                     // Display W - Width
    RKProductListDisplayD            = (1 << 3),                     // Display D - Differential Reflectivity
    RKProductListDisplayP            = (1 << 4),                     // Display P - PhiDP
    RKProductListDisplayR            = (1 << 5),                     // Display R - RhoHV
    RKProductListDisplayK            = (1 << 6),                     // Display K - KDP
    RKProductListDisplayS            = (1 << 7),                     // Display S - Signal
    RKProductListDisplayZVWDPRKS     = 0x000000FF,                   // Display All
    RKProductListProductZ            = (1 << 16),                    // Data of Z
    RKProductListProductV            = (1 << 17),                    // Data of V
    RKProductListProductW            = (1 << 18),                    // Data of W
    RKProductListProductD            = (1 << 19),                    // Data of D
    RKProductListProductP            = (1 << 20),                    // Data of P
    RKProductListProductR            = (1 << 21),                    // Data of R
    RKProductListProductK            = (1 << 22),                    // Data of K
    RKProductListProductS            = (1 << 23),                    // Data of S
    RKProductListProductZVWDPR       = 0x003F0000,                   // Base data, i.e., without K, and S
    RKProductListProductZVWDPRKS     = 0x00FF0000                    // All data
};

typedef uint32_t RKProductIndex;
enum RKProductIndex {
    RKProductIndexZ,
    RKProductIndexV,
    RKProductIndexW,
    RKProductIndexD,
    RKProductIndexP,
    RKProductIndexR,
    RKProductIndexS,
    RKProductIndexK,
    RKProductIndexZv,
    RKProductIndexVv,
    RKProductIndexWv
};

typedef uint32_t RKConfigKey;
enum RKConfigKey {
    RKConfigKeyNull,
    RKConfigKeySweepElevation,
    RKConfigKeySweepAzimuth,
    RKConfigPositionMarker,
    RKConfigKeyPRF,
    RKConfigKeyDualPRF,
    RKConfigKeyGateCount,
    RKConfigKeyWaveformId,
    RKConfigKeyVCPDefinition,
    RKConfigKeyZCal,
    RKConfigKeyDCal,
    RKConfigKeyPCal,
    RKConfigKeyEnd
};

// A general description of a radar. These should never change after the radar has gone live
typedef struct rk_radar_desc {
    RKInitFlag       initFlags;
    uint32_t         pulseCapacity;
    uint32_t         pulseToRayRatio;
    uint32_t         healthBufferDepth;
    uint32_t         configBufferDepth;
    uint32_t         positionBufferDepth;
    uint32_t         pulseBufferDepth;
    uint32_t         rayBufferDepth;
    double           latitude;
    double           longitude;
    float            heading;                                        // Radar heading
    float            radarHeight;                                    // Radar height from ground (m)
    float            wavelength;                                     // Radar wavelength (m)
    char             name[RKNameLength];                             // Radar name
    char             filePrefix[RKNameLength];                       // Prefix of output files
} RKRadarDesc;

// A running configuration buffer
typedef struct rk_config {
    uint32_t         i;                                              // Identity counter
    uint32_t         pw[RKMaxMatchedFilterCount];                    // Pulse width (ns)
    uint32_t         prf[RKMaxMatchedFilterCount];                   // Pulse repetition frequency (Hz)
    uint32_t         gateCount[RKMaxMatchedFilterCount];             // Number of range gates
    uint32_t         waveformId[RKMaxMatchedFilterCount];            // Transmit waveform
    char             vcpDefinition[RKMaximumStringLength];           // Volume coverage pattern
    RKFloat          noise[2];                                       // Noise floor (ADU)
    RKFloat          ZCal[2];                                        // Reflectivity calibration (dB)
    RKFloat          DCal[2];                                        // ZDR calibration (dB)
    RKFloat          PCal[2];                                        // Phase calibration
    RKFloat          censorSNR;                                      // Censor SNR (dB)
    float            sweepElevation;                                 // Sweep elevation angle (degrees)
    float            sweepAzimuth;                                   // Sweep azimuth angle (degrees)
    RKMarker         startMarker;                                    // Marker of the start ray
} RKConfig;

typedef union rk_heath {
    struct {
        uint64_t         i;                                          // Identity counter
        char             string[RKMaximumStringLength];              // Health string
        RKHealthFlag     flag;                                       // Health flag
        struct timeval   time;                                       // Time in struct timeval
        double           timeDouble;                                 // Time in double
    };
    RKByte               *bytes;
} RKHealth;

typedef union rk_position {
    struct {
        uint64_t         i;                                          // Counter
        uint64_t         tic;                                        // Time tic
        RKFourByte       rawElevation;                               // Raw elevation readout
        RKFourByte       rawAzimuth;                                 // Raw azimuth readout
        RKFourByte       rawElevationVelocity;                       // Raw velocity of elevation readout
        RKFourByte       rawAzimuthVelocity;                         // Raw velocity of azimuth readout
        RKFourByte       rawElevationStatus;                         // Raw status of elevation readout
        RKFourByte       rawAzimuthStatus;                           // Raw status of azimuth readout
        uint8_t          queueSize;                                  // Queue size of the readout buffer
        uint8_t          elevationMode;                              // Positioning mode of elevation
        uint8_t          azimuthMode;                                // Positioning mode of azimuth
        uint8_t          sequence;                                   // DEBUG command sequence
        RKPositionFlag   flag;                                       // Position flag
        float            elevationDegrees;                           // Decoded elevation
        float            azimuthDegrees;                             // Decoded elevation
        float            elevationVelocityDegreesPerSecond;          // Decoded velocity of elevation
        float            azimuthVelocityDegreesPerSecond;            // Decoded velocity of azimuth
        float            elevationCounter;                           // Progress counter (of target) of the elevation
        float            elevationTarget;                            // Targeted progress counter of the elevation
        float            azimuthCounter;                             // Progress counter (of target) of the azimuth
        float            azimuthTarget;                              // Targeted progress counter of the azimuth
        float            sweepElevationDegrees;                      // Set elevation for current sweep
        float            sweepAzimuthDegrees;                        // Set azimuth for current sweep
        struct timeval   time;                                       // Time in struct timeval
        double           timeDouble;                                 // Time in double;
    };
    RKByte               bytes[128];
} RKPosition;

typedef struct rk_pulse_header {
    uint64_t         i;                                              // Identity counter
    uint64_t         n;                                              // Network counter, may be useful to indicate packet loss
    uint64_t         t;                                              // A clean clock-related tic count
    RKPulseStatus    s;                                              // Status flag
    uint32_t         capacity;                                       // Allocated capacity
    uint32_t         gateCount;                                      // Number of range gates
    RKMarker         marker;                                         // Position Marker
    uint32_t         pulseWidthSampleCount;                          // Pulsewidth
    struct timeval   time;                                           // UNIX time in seconds since 1970/1/1 12:00am
    double           timeDouble;                                     // Time in double representation
    RKFourByte       rawAzimuth;                                     // Raw azimuth reading, which may take up to 4 bytes
    RKFourByte       rawElevation;                                   // Raw elevation reading, which may take up to 4 bytes
    uint16_t         configIndex;                                    // Operating configuration index
    uint16_t         configSubIndex;                                 // Operating configuration sub-index
    uint16_t         azimuthBinIndex;                                // Ray bin
    float            gateSizeMeters;                                 // Size of range gates
    float            elevationDegrees;                               // Elevation in degrees
    float            azimuthDegrees;                                 // Azimuth in degrees
    float            elevationVelocityDegreesPerSecond;              // Velocity of elevation in degrees / second
    float            azimuthVelocityDegreesPerSecond;                // Velocity of azimuth in degrees / second
} RKPulseHeader;

// Pulse parameters for matched filters (pulseCompressionCore)
typedef struct rk_pulse_parameters {
    uint32_t         filterCounts[2];
    uint32_t         planIndices[2][RKMaxMatchedFilterCount];
    uint32_t         planSizes[2][RKMaxMatchedFilterCount];
} RKPulseParameters;

// RKPulse struct is padded to a SIMD conformal size
typedef struct rk_pulse {
    union {
        struct {
            RKPulseHeader      header;
            RKPulseParameters  parameters;
        };
        RKByte                 headerBytes[RKPulseHeaderPaddedSize];
    };
    RKByte                     data[0];
} RKPulse;

typedef struct rk_ray_header {
    uint32_t         capacity;                                       // Capacity
    RKRayStatus      s;                                              // Ray status
    uint32_t         i;                                              // Ray indentity
    uint32_t         n;                                              // Ray network counter
    RKMarker         marker;                                         // Volume / sweep / radial marker
    RKProductList    productList;                                    // 16-bit MSB for products + 16-bit LSB for display
    uint16_t         configIndex;                                    // Operating configuration index
    uint16_t         configSubIndex;                                 // Operating configuration sub-index
    uint16_t         gateCount;                                      //
    uint16_t         reserved2;                                      //
    float            gateSizeMeters;                                 // Size of range gates
    float            sweepElevation;                                 // Sweep elevation for PPI
    float            sweepAzimuth;                                   // Sweep azimuth for RHI
    struct timeval   startTime;                                      // Start time of the ray in UNIX time
    double           startTimeDouble;                                // Start time in double representation
    float            startAzimuth;                                   // End time in double representation
    float            startElevation;                                 //
    struct timeval   endTime;                                        // End time of the ray in UNIX time
    double           endTimeDouble;                                  //
    float            endAzimuth;                                     //
    float            endElevation;                                   //
} RKRayHeader;

typedef struct rk_ray {
    union {
        RKRayHeader  header;
        RKByte       headerBytes[RKRayHeaderPaddedSize];
    };
    RKByte           data[0];
} RKRay;

typedef struct rk_modulo_path {
    uint32_t      origin;
    uint32_t      length;
    uint32_t      modulo;
} RKModuloPath;

typedef struct rk_scratch {
    bool             showNumbers;
    uint8_t          lagCount;                                       // Number of lags of R & C
    RKIQZ            mX[2];                                          // Mean of X, 2 for dual-pol
    RKIQZ            vX[2];                                          // Variance of X, i.e., E{X' * X} - E{X}' * E{X}
    RKIQZ            R[2][RKLagCount];                               // ACF up to RKLagCount - 1 for each polarization
    RKIQZ            C[2 * RKLagCount - 1];                          // CCF in [ -RKLagCount + 1, ..., -1, 0, 1, ..., RKLagCount - 1 ]
    RKIQZ            sC;                                             // Summation of Xh * Xv'
    RKIQZ            ts;                                             // Temporary scratch space
    RKFloat          *aR[2][RKLagCount];                             // abs(ACF)
    RKFloat          *aC[2 * RKLagCount - 1];                        // abs(CCF)
    RKFloat          *gC;                                            // Gaussian fitted CCF(0)  NOTE: Need to extend this to multi-multilag
    RKFloat          noise[2];                                       // Noise floor of each channel
    RKFloat          aliasingVelocity;                               // Aliasing velocity
    RKFloat          *rcor;                                          // Reflectivity range correction factor
    RKFloat          *S[2];                                          // Signal
    RKFloat          *Z[2];                                          // Reflectivity in dB
    RKFloat          *V[2];                                          // Velocity in same units as aliasing velocity
    RKFloat          *W[2];                                          // Spectrum width in same units as aliasing velocity
    RKFloat          *SNR[2];                                        // Signal-to-noise ratio
    RKFloat          *ZDR;                                           // Differential reflectivity ZDR
    RKFloat          *PhiDP;                                         // Differential phase PhiDP
    RKFloat          *RhoHV;                                         // Cross-correlation coefficient RhoHV
    RKFloat          *KDP;                                           // Specific phase KDP
} RKScratch;

#pragma pack(pop)

#endif /* defined(__RadarKit_RKTypes__) */
