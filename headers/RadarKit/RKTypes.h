//
//  RKTypes.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/17/15.
//

#ifndef __RadarKit_Types__
#define __RadarKit_Types__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <limits.h>
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

#define RKVersionString "1.2.10b"

//
// Memory Blocks
// Defines the number of slots and gates of each pulse of the RKRadar structure
//
// RKBufferCSlotCount The number of slots for config
// RKBufferHSlotCount The number of slots for health JSON string
// RKBufferSSlotCount The number of slots for status string
// RKBufferPSlotCount The number of slots for position buffer
// RKBuffer0SlotCount The number of slots for level-0 pulse storage in the host memory
// RKBuffer1SlotCount The number of slots for level-1 pulse storage in the host memory
// RKBuffer2SlotCount The number of slots for level-2 pulse storage in the host memory
// RKMaximumControlCount The number of controls (buttons)
// RKMaximumCalibrationCount The number of waveform calibration set
// RKGateCount The maximum number of gates allocated for each pulse
// RKSIMDAlignSize The minimum alignment size. AVX requires 256 bits = 32 bytes. AVX-512 is on the horizon now.
//

#pragma mark - Constants

#define RKBufferCSlotCount                   25                                // Config
#define RKBufferHSlotCount                   25                                // Health
#define RKBufferSSlotCount                   90                                // Status strings
#define RKBufferPSlotCount                   1000                              // Positions
#define RKBuffer0SlotCount                   20000                             // Raw I/Q
#define RKBuffer2SlotCount                   36000                             // Ray
#define RKMaximumControlCount                128                               // Controls
#define RKMaximumWaveformCalibrationCount    128                               // Waveform calibration
#define RKGateCount                          262144                            // Must be a multiple of RKSIMDAlignSize
#define RKLagCount                           5                                 // Number lags of ACF / CCF lag = +/-4 and 0
#define RKBaseMomentCount                    10                                // 16 to be the absolute max since productList enum is 32-bit (product + display)
#define RKSIMDAlignSize                      64                                // SSE 16, AVX 32, AVX-512 64
#define RKMaxFilterCount                     8                                 // Maximum filter count within each group. Check RKPulseParameters
#define RKMaxFilterGroups                    22                                // Maximum filter group count
#define RKWorkerDutyCycleBufferDepth         1000                              //
#define RKMaximumPulsesPerRay                2000                              //
#define RKMaximumRaysPerSweep                1500                              // 1440 is 0.25-deg. This should be plenty
#define RKMaximumPacketSize                  16 * 1024 * 1024                  // Maximum network packet size
#define RKNetworkTimeoutSeconds              20                                //
#define RKNetworkReconnectSeconds            3                                 //
#define RKLagRedThreshold                    0.5                               //
#define RKLagOrangeThreshold                 0.7                               //
#define RKDutyCyleRedThreshold               0.95                              //
#define RKDutyCyleOrangeThreshold            0.90                              //
#define RKStatusBarWidth                     10                                //
#define RKPulseCountForNoiseMeasurement      200                               //
#define RKProcessorStatusPulseCoreCount      16                                //
#define RKProcessorStatusRingCoreCount       16                                //
#define RKProcessorStatusRayCoreCount        16                                //
#define RKHostMonitorPingInterval            5                                 //
#define RKMaximumProductCount                64                                //

#define RKDefaultDataPath                    "data"
#define RKDataFolderIQ                       "iq"
#define RKDataFolderMoment                   "moment"
#define RKDataFolderHealth                   "health"
#define RKLogFolder                          "log"
#define RKWaveformFolder                     "waveforms"
#define RKFFTWisdomFile                      "radarkit-fft-wisdom"

#define RKNoColor                            "\033[0m"
#define RKRedColor                           "\033[38;5;196m"
#define RKOrangeColor                        "\033[38;5;214m"
#define RKYellowColor                        "\033[38;5;226m"
#define RKLimeColor                          "\033[38;5;118m"
#define RKGreenColor                         "\033[38;5;46m"
#define RKTealColor                          "\033[38;5;49m"
#define RKIceBlueColor                       "\033[38;5;51m"
#define RKSkyBlueColor                       "\033[38;5;45m"
#define RKBlueColor                          "\033[38;5;27m"
#define RKPurpleColor                        "\033[38;5;99m"
#define RKIndigoColor                        "\033[38;5;201m"
#define RKHotPinkColor                       "\033[38;5;199m"
#define RKDeepPinkColor                      "\033[38;5;197m"
#define RKPinkColor                          "\033[38;5;213m"
#define RKSalmonColor                        "\033[38;5;210m"
#define RKPythonColor                        "\033[38;5;226;48;5;24m"
#define RKRadarKitColor                      "\033[38;5;15;48;5;124m"
#define RKMaximumStringLength                4096
#define RKMaximumPathLength                  1024
#define RKMaximumFolderPathLength            768
#define RKMaximumCommandLength               512
#define RKNameLength                         128
#define RKPulseHeaderPaddedSize              256                               // Change this to higher number for post-AVX2 intrinsics
#define RKRayHeaderPaddedSize                128                               // Change this to higher number for post-AVX2 intrinsics

#define RKColorDutyCycle(x)  (x > RKDutyCyleRedThreshold ? "\033[91m" : (x > RKDutyCyleOrangeThreshold ? "\033[93m" : "\033[92m"))
#define RKColorLag(x)        (x > RKLagRedThreshold      ? "\033[91m" : (x > RKLagOrangeThreshold      ? "\033[93m" : "\033[92m"))

#define ITALIC(x)            "\033[3m" x "\033[23m"
#define UNDERLINE(x)         "\033[4m" x "\033[24m"
#define HIGHLIGHT(x)         "\033[38;5;82;48;5;238m" x "\033[0m"
#define UNDERLINE_ITALIC(x)  "\033[3;4m" x "\033[23;24m"

#define RKFilterAnchorDefault                           {.length = (1), .maxDataLength = RKGateCount, .filterGain = 1.0f}
#define RKFilterAnchorDefaultWithMaxDataLength(x)       {.length = (1), .maxDataLength = (x), .filterGain = 1.0f}
#define RKFilterAnchorOfLengthAndMaxDataLength(x, y)    {.length = (x), .maxDataLength = (y), .filterGain = 1.0f}

#define RKMarkerScanTypeString(x) \
(((x) & RKMarkerScanTypeMask) == RKMarkerScanTypePPI ? "PPI" : \
(((x) & RKMarkerScanTypeMask) == RKMarkerScanTypeRHI ? "RHI" : \
(((x) & RKMarkerScanTypeMask) == RKMarkerScanTytpePoint ? "SPT" : "UNK")))

#define RKMarkerScanTypeShortString(x) \
(((x) & RKMarkerScanTypeMask) == RKMarkerScanTypePPI ? "P" : \
(((x) & RKMarkerScanTypeMask) == RKMarkerScanTypeRHI ? "R" : \
(((x) & RKMarkerScanTypeMask) == RKMarkerScanTytpePoint ? "S" : "U")))

#pragma mark - Fundamental Types

typedef uint8_t       RKByte;                                                  //
typedef float         RKFloat;                                                 // We can change this to double if we decided one day
typedef ssize_t       RKResult;                                                // Generic return from functions, 0 for no errors and !0 for others.
typedef void *        RKBuffer;                                                //
typedef void *        RKTransceiver;                                           //
typedef void *        RKPedestal;                                              //
typedef void *        RKHealthRelay;                                           //
typedef void *        RKMasterController;                                      //
typedef char          RKName[RKNameLength];                                    // RKName x = char x[RKNameLength]
typedef char          RKCommand[RKMaximumCommandLength];                       // RKCommand x = char x[RKCommandLength]
typedef uint8_t       RKProductId;                                             // Product identifier
typedef uint64_t      RKIdentifier;                                            // Pulse identifier, ray identifier, config identifier, etc.
typedef const float   RKConst;

#pragma pack(push, 1)

//
// Fundamental unit of a (16-bit) + (16-bit) raw complex IQ sample
//
typedef struct rk_int16c {
    int16_t i;
    int16_t q;
} RKInt16C;

//
// Interleaved complex format. Fundamental unit of a (float) + (float) raw complex IQ sample
//
typedef struct rk_complex {
    RKFloat i;
    RKFloat q;
} RKComplex;

//
// Deinterleaved complex format for vector library
//
typedef struct rk_iqz {
    RKFloat *i;
    RKFloat *q;
} RKIQZ;

//
// A convenient way to convert bytes into several other types
//
typedef union rk_four_byte {
    struct { uint8_t byte[4]; };
    struct { uint16_t u16, u16_2; };
    struct { int16_t i16, i16_2; };
    struct { uint32_t u32; };
    struct { int32_t i32; };
    struct { float f; };
} RKFourByte;

typedef union rk_filter_anchor {
    struct {
        uint32_t      name;                                                    // Just an internal name id
        uint32_t      origin;                                                  // Filter origin to be used with RKWaveform
        uint32_t      length;                                                  // Filter length to be used with RKWaveform
        uint32_t      inputOrigin;                                             // Origin of input
        uint32_t      outputOrigin;                                            // Origin of output
        uint32_t      maxDataLength;                                           // Maximum data to decode
        RKFloat       subCarrierFrequency;                                     // For house keeping only, use the waveform->fc for DDC
        RKFloat       sensitivityGain;                                         // Sensitivity gain due to longer/efficient waveforms (dB)
        RKFloat       filterGain;                                              // Filter gain from the filter coefficients, should be 0.0 (dB)
        RKFloat       fullScale;                                               // Scaling factor to get to full scale
    };
    char bytes[64];
} RKFilterAnchor;

typedef struct rk_iir_filter {
    uint32_t      name;
    uint32_t      bLength;
    uint32_t      aLength;
    RKComplex     *B;
    RKComplex     *A;
} RKIIRFilter;

typedef struct rk_modulo_path {
    uint32_t      origin;
    uint32_t      length;
    uint32_t      modulo;
} RKModuloPath;

#pragma mark - Enums

#define RKResultNames \
N(RKResultSuccess) \
N(RKResultTooBig) \
N(RKResultTimeout) \
N(RKResultNullInput) \
N(RKResultEngineNotWired) \
N(RKResultEngineNotActive) \
N(RKResultIncompleteSend) \
N(RKResultIncompleteReceive) \
N(RKResultIncompleteTransceiver) \
N(RKResultIncompletePedestal) \
N(RKResultIncompleteHealthRelay) \
N(RKResultIncompleteControl) \
N(RKResultIncompleteWaveformCalibration) \
N(RKResultIncompleteProductDescription) \
N(RKResultErrorCreatingOperatorRoutine) \
N(RKResultErrorCreatingOperatorCommandRoutine) \
N(RKResultErrorCreatingClientRoutine) \
N(RKResultSDToFDError) \
N(RKResultNoPulseBuffer) \
N(RKResultNoRayBuffer) \
N(RKResultNoPulseCompressionEngine) \
N(RKResultNoMomentEngine) \
N(RKResultFailedToStartCompressionCore) \
N(RKResultFailedToStartPulseWatcher) \
N(RKResultFailedToStartRingPulseWatcher) \
N(RKResultFailedToInitiateSemaphore) \
N(RKResultFailedToRetrieveSemaphore) \
N(RKResultFailedToAllocateFFTSpace) \
N(RKResultFailedToAllocateFilter) \
N(RKResultFailedToAllocateDutyCycleBuffer) \
N(RKResultFailedToAllocateScratchSpace) \
N(RKResultFailedToSetWaveform) \
N(RKResultFailedToSetFilter) \
N(RKResultEngineDeactivatedMultipleTimes) \
N(RKResultFailedToStartMomentCore) \
N(RKResultFailedToStartPulseGatherer) \
N(RKResultUnableToChangeCoreCounts) \
N(RKResultFailedToStartPedestalWorker) \
N(RKResultFailedToGetVacantPosition) \
N(RKResultFailedToGetVacantHealth) \
N(RKResultFailedToStartRayGatherer) \
N(RKResultFailedToStartHealthWorker) \
N(RKResultFailedToStartPulseRecorder) \
N(RKResultFailedToStartPedestalMonitor) \
N(RKResultFailedToStartFileManager) \
N(RKResultFailedToStartFileRemover) \
N(RKResultFailedToStartTransceiver) \
N(RKResultFailedToStartPedestal) \
N(RKResultFailedToStartHealthRelay) \
N(RKResultPreferenceFileNotFound) \
N(RKResultPreferenceKeywordNotFound) \
N(RKResultFailedToMeasureNoise) \
N(RKResultFailedToCreateFileRemover) \
N(RKResultFileManagerBufferNotResuable) \
N(RKResultInvalidMomentParameters) \
N(RKResultFailedToCreateUnitWorker) \
N(RKResultFailedToStartHostWatcher) \
N(RKResultFailedToStartHostPinger) \
N(RKResultFailedToExecuteCommand) \
N(RKResultFailedToAddHost) \
N(RKResultFailedToFindProductId) \
N(RKResultClientNotConnected) \
N(RKResultRadarNotLive) \
N(RKResultNoRadar)

#define N(x) x,
enum RKResult {
    RKResultNames
    RKResultCount,
};
#undef N

enum RKEngineColor {
    RKEngineColorCommandCenter = 10,
    RKEngineColorPulseCompressionEngine = 7,
    RKEngineColorPulseRingFilterEngine = 3,
    RKEngineColorPositionEngine = 4,
    RKEngineColorMomentEngine = 8,
    RKEngineColorHealthEngine = 1,
    RKEngineColorDataRecorder = 9,
    RKEngineColorSweepEngine = 14,
    RKEngineColorHealthLogger = 5,
    RKEngineColorFileManager = 2,
    RKEngineColorTransceiver = 13,
    RKEngineColorPedestalRelayPedzy = 11,
    RKEngineColorHealthRelayTweeta = 0,
    RKEngineColorRadarRelay = 13,
    RKEngineColorHostMonitor = 12,
    RKEngineColorClock = 15,
    RKEngineColorMisc = 16,
    RKEngineColorEngineMonitor = 15,
    RKEngineColorConfig = 6
};

typedef uint32_t RKPositionFlag;
enum RKPositionFlag {
    RKPositionFlagVacant             = 0,
    RKPositionFlagAzimuthEnabled     = 1,
    RKPositionFlagAzimuthSafety      = (1 << 1),
    RKPositionFlagAzimuthError       = (1 << 2),
    RKPositionFlagAzimuthSweep       = (1 << 8),
    RKPositionFlagAzimuthPoint       = (1 << 9),
    RKPositionFlagAzimuthMode        = (RKPositionFlagAzimuthSweep | RKPositionFlagAzimuthPoint),
    RKPositionFlagAzimuthComplete    = (1 << 10),
    RKPositionFlagElevationEnabled   = (1 << 16),
    RKPositionFlagElevationSafety    = (1 << 17),
    RKPositionFlagElevationError     = (1 << 18),
    RKPositionFlagElevationSweep     = (1 << 24),
    RKPositionFlagElevationPoint     = (1 << 25),
    RKPositionFlagElevationMode      = (RKPositionFlagElevationSweep | RKPositionFlagElevationPoint),
    RKPositionFlagElevationComplete  = (1 << 26),
    RKPositionFlagScanActive         = (1 << 28),
    RKPositionFlagScanMode           = (RKPositionFlagAzimuthMode | RKPositionFlagElevationMode),
    RKPositionFlagVCPActive          = (1 << 29),
    RKPositionFlagHardwareMask       = 0x3FFFFFFF,
    RKPositionFlagReady              = (1 << 31)
};

typedef uint32_t RKStatusFlag;
enum RKStatusFlag {
    RKStatusFlagVacant               = 0,
    RKStatusFlagReady                = 1
};

typedef uint32_t RKHealthFlag;
enum RKHealthFlag {
    RKHealthFlagVacant               = 0,
    RKHealthFlagReady                = 1
};

typedef uint32_t RKMarker;
enum RKMarker {
    RKMarkerNull                     = 0,
    RKMarkerSweepMiddle              = 1,                                      //
    RKMarkerSweepBegin               = (1 << 1),                               //  0000 0010
    RKMarkerSweepEnd                 = (1 << 2),                               //  0000 0100
    RKMarkerVolumeBegin              = (1 << 3),                               //  0000 1000
    RKMarkerVolumeEnd                = (1 << 4),                               //  0001 0000
    RKMarkerScanTypeMask             = 0x60,                                   //  0110 0000
    RKMarkerScanTypeUnknown          = (0 << 5),                               //  .00. ....
    RKMarkerScanTypePPI              = (1 << 5),                               //  .01. ....
    RKMarkerScanTypeRHI              = (2 << 5),                               //  .10. ....
    RKMarkerScanTytpePoint           = (3 << 5),                               //  .11. ....
    RKMarkerMemoryManagement         = (1 << 7)                                //  1000 0000
};

//
// Typical status progression:
//
// -> RKPulseStatusVacant
// -> RKPulseStatusHasIQData
// -> RKPulseStatusInspected                                (main thread)
// -> RKPulseStatusCompressed / RKPulseStatusSkipped        (core threads)
// -> RKPulseStatusDownSampled                              (core threads)
// -> RKPulseStatusProcessed                                (core threads)
// -> RKPulseStatusRingInspected                            (main thread)
// -> RKPulseStatusRingFiltered / RKPulseStatusRingSkipped  (main thread consolidates)
// -> RKPulseStatusRingProcessed                            (main thread)
// -> RKPulseStatusHasPosition
// -> RKPulseStatusReadyForMoments
//
typedef uint32_t RKPulseStatus;
enum RKPulseStatus {
    RKPulseStatusVacant              = 0,
    RKPulseStatusHasIQData           = 1,                                      // 0x01
    RKPulseStatusHasPosition         = (1 << 1),                               // 0x02
    RKPulseStatusInspected           = (1 << 2),                               // 0x04
    RKPulseStatusCompressed          = (1 << 3),                               // 0x08
    RKPulseStatusSkipped             = (1 << 4),                               // 0x10
    RKPulseStatusDownSampled         = (1 << 5),                               // 0x20
    RKPulseStatusProcessed           = (1 << 6),                               // 0x40
    RKPulseStatusRingInspected       = (1 << 7),                               // 0x80
    RKPulseStatusRingFiltered        = (1 << 8),
    RKPulseStatusRingSkipped         = (1 << 9),
    RKPulseStatusRingProcessed       = (1 << 10),
    RKPulseStatusReadyForMoments     = (RKPulseStatusProcessed | RKPulseStatusRingProcessed | RKPulseStatusHasPosition),
    RKPulseStatusUsedForMoments      = (1 << 11),
    RKPulseStatusRecorded            = (1 << 12)
};

typedef uint32_t RKRayStatus;
enum RKRayStatus {
    RKRayStatusVacant                = 0,
    RKRayStatusProcessing            = 1,
    RKRayStatusProcessed             = (1 << 1),
    RKRayStatusSkipped               = (1 << 2),
    RKRayStatusReady                 = (1 << 3),
    RKRayStatusStreamed              = (1 << 4),
    RKRayStatusBeingConsumed         = (1 << 5)
};

typedef uint32_t RKInitFlag;
enum RKInitFlag {
    RKInitFlagNone                   = 0,
    RKInitFlagVerbose                = 0x0001,
    RKInitFlagVeryVerbose            = 0x0002,
    RKInitFlagVeryVeryVerbose        = 0x0004,
    RKInitFlagShowClockOffset        = 0x0008,
    RKInitFlagManuallyAssignCPU      = 0x0010,
    RKInitFlagReserved2              = 0x0020,
    RKInitFlagReserved3              = 0x0040,
    RKInitFlagReserved4              = 0x0080,
    RKInitFlagAllocStatusBuffer      = 0x0100,                                 // 1 << 8
    RKInitFlagAllocConfigBuffer      = 0x0200,                                 // 1 << 9
    RKInitFlagAllocRawIQBuffer       = 0x0400,                                 // 1 << 10
    RKInitFlagAllocPositionBuffer    = 0x0800,                                 // 1 << 11
    RKInitFlagAllocMomentBuffer      = 0x1000,                                 // 1 << 12
    RKInitFlagAllocHealthBuffer      = 0x2000,                                 // 1 << 13
    RKInitFlagAllocHealthNodes       = 0x4000,                                 // 1 << 14
    RKInitFlagSignalProcessor        = 0x8000,                                 // 1 << 15
    RKInitFlagRelay                  = 0x7703,                                 // Everything = 0xFF00 - 0x8000(DSP) - 0x0800(Pos)
    RKInitFlagAllocEverything        = 0xFF01,
    RKInitFlagAllocEverythingQuiet   = 0xFF00,
};

typedef uint32_t RKBaseMomentList;
enum RKBaseMomentList {
    RKBaseMomentListNone                = 0,                                  // None
    RKBaseMomentListDisplayZ            = (1),                                // Display Z - Reflectivity dBZ
    RKBaseMomentListDisplayV            = (1 << 1),                           // Display V - Velocity
    RKBaseMomentListDisplayW            = (1 << 2),                           // Display W - Width
    RKBaseMomentListDisplayD            = (1 << 3),                           // Display D - Differential Reflectivity
    RKBaseMomentListDisplayP            = (1 << 4),                           // Display P - PhiDP
    RKBaseMomentListDisplayR            = (1 << 5),                           // Display R - RhoHV
    RKBaseMomentListDisplayK            = (1 << 6),                           // Display K - KDP
    RKBaseMomentListDisplaySh           = (1 << 7),                           // Display Sh - Signal
    RKBaseMomentListDisplaySv           = (1 << 8),                           // Display Sv - Signal
    RKBaseMomentListDisplayZVWDPRKS     = 0x000000FF,                         // Display All
    RKBaseMomentListProductZ            = (1 << 16),                          // Data of Z
    RKBaseMomentListProductV            = (1 << 17),                          // Data of V
    RKBaseMomentListProductW            = (1 << 18),                          // Data of W
    RKBaseMomentListProductD            = (1 << 19),                          // Data of D
    RKBaseMomentListProductP            = (1 << 20),                          // Data of P
    RKBaseMomentListProductR            = (1 << 21),                          // Data of R
    RKBaseMomentListProductK            = (1 << 22),                          // Data of K
    RKBaseMomentListProductSh           = (1 << 23),                          // Data of Sh
    RKBaseMomentListProductSv           = (1 << 24),                          // Data of Sv
    RKBaseMomentListProductZVWDPR       = 0x003F0000,                         // Base data, i.e., without K, and S
    RKBaseMomentListProductZVWDPRK      = 0x007F0000,                         // Base data + K
    RKBaseMomentListProductZVWDPRKS     = 0x01FF0000                          // All data
};

typedef uint32_t RKBaseMomentIndex;
enum RKBaseMomentIndex {
    RKBaseMomentIndexZ,
    RKBaseMomentIndexV,
    RKBaseMomentIndexW,
    RKBaseMomentIndexD,
    RKBaseMomentIndexP,
    RKBaseMomentIndexR,
    RKBaseMomentIndexK,
    RKBaseMomentIndexSh,
    RKBaseMomentIndexSv,
    RKBaseMomentIndexZv,
    RKBaseMomentIndexVv,
    RKBaseMomentIndexWv,
    RKBaseMomentIndexCount
};

typedef uint32_t RKProductType;
enum RKProductType {
    RKProductTypeUnknown             = 0,                                      // Unspecified
    RKProductTypeCellMatch           = (1),                                    //
    RKProductTypePPI                 = (1 << 1),                               //
    RKProductTypeCAPPI               = (1 << 2)                                //
};

typedef uint32_t RKConfigKey;
enum RKConfigKey {
    RKConfigKeyNull,
    RKConfigKeySweepElevation,
    RKConfigKeySweepAzimuth,
    RKConfigKeyPositionMarker,
    RKConfigKeyPRF,
    RKConfigKeyDualPRF,
    RKConfigKeyPulseGateCount,
    RKConfigKeyPulseGateSize,
    RKConfigKeyWaveform,
    RKConfigKeyWaveformId,
    RKConfigKeyWaveformName,
    RKConfigKeySystemNoise,
    RKConfigKeySystemZCal,
    RKConfigKeySystemDCal,
    RKConfigKeySystemPCal,
    RKConfigKeyZCal,                                                           // deprecating
    RKConfigKeyDCal,                                                           // deprecating
    RKConfigKeyPCal,                                                           // deprecating
    RKConfigKeyZCal2,                                                          // deprecating
    RKConfigKeyDCal2,                                                          // deprecating
    RKConfigKeyPCal2,                                                          // deprecating
    RKConfigKeyZCals,                                                          // deprecating
    RKConfigKeyDCals,                                                          // deprecating
    RKConfigKeyPCals,                                                          // deprecating
    RKConfigKeyWaveformCalibration,
    RKConfigKeySNRThreshold,
    RKConfigKeyVCPDefinition,
    RKConfigKeyTotalNumberOfKeys
};

typedef uint8_t RKHealthNode;
enum RKHealthNode {
    RKHealthNodeRadarKit,
    RKHealthNodeTransceiver,
    RKHealthNodePedestal,
    RKHealthNodeTweeta,
    RKHealthNodeUser1,
    RKHealthNodeUser2,
    RKHealthNodeUser3,
    RKHealthNodeUser4,
    RKHealthNodeUser5,
    RKHealthNodeUser6,
    RKHealthNodeUser7,
    RKHealthNodeUser8,
    RKHealthNodeCount,
    RKHealthNodeInvalid = (RKHealthNode) - 1
};

//
// Typical progression:
//
// EngineInit         RKEngineStateAllocated
// EngineSetXYZ       RKEngineStateProperlyWired
// EngineStart        RKEngineStateActivating
//                    RKEngineStateActive
//                    RKEngineStateChildAllocated
//                    RKEngineStateChildProperlyWired
//                    RKEngineStateChildActivating
//                    RKEngineStateChildActive
//
// EngineStop         RKEngineStateDeactivating
//                    RKEngineStateChildDeactivating
//                    RKEngineStateChildActive -
//                    RKEngineStateActive -
//                    (RKEngineStateChildAllocated | RKEngineStateChildProperlyWired | RKEngineStateAllocated | RKEngineStateProperlyWired)
//
typedef uint32_t RKEngineState;
enum RKEngineState {
    RKEngineStateNull                            = 0,                          //
    RKEngineStateSleep0                          = 1,                          // Usually for a wait just outside of the main while loop
    RKEngineStateSleep1                          = (1 << 1),                   // Stage 1 wait - usually waiting for pulse
    RKEngineStateSleep2                          = (1 << 2),                   // Stage 2 wait
    RKEngineStateSleep3                          = (1 << 3),                   // Stage 3 wait
    RKEngineStateSleepMask                       = 0x0000000F,                 //
    RKEngineStateWritingFile                     = (1 << 4),                   // Generating an output file
    RKEngineStateMemoryChange                    = (1 << 5),                   // Some required pointers are being changed
    RKEngineStateSuspended                       = (1 << 6),                   // All indices stop increasing
    RKEngineStateBusyMask                        = 0x000000F0,                 //
    RKEngineStateAllocated                       = (1 << 8),                   // Resources have been allocated
    RKEngineStateProperlyWired                   = (1 << 9),                   // All required pointers are properly wired up
    RKEngineStateActivating                      = (1 << 10),                  // The main run loop is being activated
    RKEngineStateDeactivating                    = (1 << 11),                  // The main run loop is being deactivated
    RKEngineStateActive                          = (1 << 12),                  // The engine is active
    RKEngineStateMainMask                        = 0x00001F00,                 //
    RKEngineStateChildAllocated                  = (1 << 16),                  // The child resources have been allocated
    RKEngineStateChildProperlyWired              = (1 << 17),                  // Probably not used
    RKEngineStateChildActivating                 = (1 << 18),                  // The children are being activated
    RKEngineStateChildDeactivating               = (1 << 19),                  // The children are being deactivated
    RKEngineStateChildActive                     = (1 << 20),                  // The children are active
    RKEngineStateChildMask                       = 0x001F0000                  //
};

typedef uint32_t RKStatusEnum;
enum RKStatusEnum {
    RKStatusEnumUnknown                          = -3,                         //
    RKStatusEnumOld                              = -3,                         //
    RKStatusEnumInvalid                          = -2,                         //
    RKStatusEnumTooLow                           = -2,                         //
    RKStatusEnumLow                              = -1,                         //
    RKStatusEnumNormal                           =  0,                         //
    RKStatusEnumActive                           =  0,                         //
    RKStatusEnumHigh                             =  1,                         //
    RKStatusEnumStandby                          =  1,                         //
    RKStatusEnumInactive                         =  1,                         //
    RKStatusEnumOutOfRange                       =  1,                         //
    RKStatusEnumTooHigh                          =  2,                         //
    RKStatusEnumNotOperational                   =  2,                         //
    RKStatusEnumOff                              =  2,                         //
    RKStatusEnumFault                            =  2,                         //
    RKStatusEnumCritical                         =  4                          // This would the status we may shutdown the radar. Co-incidently, red = 0x4
};

typedef uint32_t RKFileType;
enum RKFileType {
    RKFileTypeIQ,
    RKFileTypeMoment,
    RKFileTypeHealth,
    RKFileTypeLog,
    RKFileTypeCount
};

typedef uint64_t RKStream;
enum RKStream {
    RKStreamNull                                 = 0,                          //
    RKStreamStatusMask                           = 0x07,                       // Values 0-8 in the lowest 4 bits (exclusive mode)
    RKStreamStatusPositions                      = 1,                          //
    RKStreamStatusPulses                         = 2,                          //
    RKStreamStatusRays                           = 3,                          //
    RKStreamStatusIngest                         = 4,                          // Ingest up keep
    RKStreamStatusEngines                        = 5,                          // State of Engines
    RKStreamStatusBuffers                        = 6,                          // Buffer overview
    RKStreamControl                              = (1 << 3),                   // Controls
    RKStreamStatusAll                            = 0xF7,                       //
    RKStreamHealthInJSON                         = (1 << 5),                   // Health in JSON
    RKStreamStatusEngineBinary                   = (1 << 6),                   //
    RKStreamStatusProcessorStatus                = (1 << 7),                   // Consolidated binary from of the system status
    RKStreamDisplayIQ                            = (1 << 8),                   // Low rate IQ (sub-smpled)
    RKStreamDisplayIQFiltered                    = (1 << 9),                   // Filtered IQ (usually matched filter is applied)
    RKStreamProductIQ                            = (1 << 10),                  // Full rate IQ
    RKStreamProductIQFiltered                    = (1 << 11),                  // Full rate filtered IQ
    RKStreamScopeStuff                           = 0x0000000000000300ULL,      //
    RKStreamDisplayZ                             = (1 << 16),                  // Display: Z = 0x00010000
    RKStreamDisplayV                             = (1 << 17),                  //
    RKStreamDisplayW                             = (1 << 18),                  //
    RKStreamDisplayD                             = (1 << 19),                  //
    RKStreamDisplayP                             = (1 << 20),                  //
    RKStreamDisplayR                             = (1 << 21),                  //
    RKStreamDisplayK                             = (1 << 22),                  //
    RKStreamDisplaySh                            = (1 << 23),                  //
    RKStreamDisplaySv                            = (1 << 24),                  //
    RKStreamDisplayZVWDPRKS                      = 0x0000000001FF0000ULL,      //
    RKStreamProductZ                             = (1ULL << 32),               // Products by ray
    RKStreamProductV                             = (1ULL << 33),               //
    RKStreamProductW                             = (1ULL << 34),               //
    RKStreamProductD                             = (1ULL << 35),               //
    RKStreamProductP                             = (1ULL << 36),               //
    RKStreamProductR                             = (1ULL << 37),               //
    RKStreamProductK                             = (1ULL << 38),               //
    RKStreamProductSh                            = (1ULL << 39),               //
    RKStreamProductSv                            = (1ULL << 40),               //
    RKStreamProductZVWDPRKS                      = 0x000001FF00000000ULL,      //
    RKStreamSweepZ                               = (1ULL << 48),               // Products by sweep
    RKStreamSweepV                               = (1ULL << 49),               //
    RKStreamSweepW                               = (1ULL << 50),               //
    RKStreamSweepD                               = (1ULL << 51),               //
    RKStreamSweepP                               = (1ULL << 52),               //
    RKStreamSweepR                               = (1ULL << 53),               //
    RKStreamSweepK                               = (1ULL << 54),               //
    RKStreamSweepSh                              = (1ULL << 55),               //
    RKStreamSweepSv                              = (1ULL << 56),               //
    RKStreamSweepZVWDPRKS                        = 0x01FF000000000000ULL,      //
    RKStreamEverything                           = 0x01FF01FF01FFFFFFULL       // (Don't use this)
};

typedef uint8_t RKHostStatus;
enum RKHostStatus {
    RKHostStatusUnknown,
    RKHostStatusUnreachable,
    RKHostStatusPartiallyReachable,
    RKHostStatusReachableUnusual,
    RKHostStatusReachable
};

typedef uint32_t RKProductStatus;
enum RKProductStatus {
    RKProductStatusVacant                        = 0,                          //
    RKProductStatusSleep0                        = (1 << 0),                   // Sleep stage 0 -
    RKProductStatusSleep1                        = (1 << 1),                   // Sleep stage 1 -
    RKProductStatusSleep2                        = (1 << 2),                   // Sleep stage 2 -
    RKProductStatusSleep3                        = (1 << 3),                   // Sleep stage 3 -
    RKProductStatusSkipped                       = (1 << 5),                   //
    RKProductStatusBusy                          = (1 << 6),                   // Waiting for processing node
    RKProductStatusActive                        = (1 << 7),                   //
};

typedef uint8_t RKOverviewFlag;
enum RKOverviewFlag {
    RKOverviewFlagNone                           = 0,                          //
    RKOverviewFlagShowColor                      = 1,                          // Use escape sequence for colors
    RKOverviewFlagDrawBackground                 = (1 << 1)                    // Repaint the background
};

typedef uint32_t RKWaveformType;
enum RKWaveformType {
    RKWaveformTypeNone                           = 0,                          //
    RKWaveformTypeIsComplex                      = 1,                          // Complex form usually represents baseband
    RKWaveformTypeSingleTone                     = (1 << 1),                   // The traditional single frequency waveform
    RKWaveformTypeFrequencyHopping               = (1 << 2),                   //
    RKWaveformTypeLinearFrequencyModulation      = (1 << 3),                   //
    RKWaveformTypeTimeFrequencyMultiplexing      = (1 << 4),                   //
    RKWaveformTypeFromFile                       = (1 << 5),                   //
    RKWaveformTypeFlatAnchors                    = (1 << 6)                    // Frequency hopping has multiple waveforms but the anchors are identical
};

#pragma mark - Structure Definitions

//
// A general description of a radar.
// Most parameters are used for initialization.
// Some may be overriden even when the radar is live.
//
typedef struct rk_radar_desc {
    RKInitFlag           initFlags;                                            //
    uint32_t             pulseCapacity;                                        //
    uint16_t             pulseToRayRatio;                                      //
    uint16_t             doNotUse;                                             //
    uint32_t             healthNodeCount;                                      //
    uint32_t             healthBufferDepth;                                    //
    uint32_t             statusBufferDepth;                                    //
    uint32_t             configBufferDepth;                                    //
    uint32_t             positionBufferDepth;                                  //
    uint32_t             pulseBufferDepth;                                     //
    uint32_t             rayBufferDepth;                                       //
    uint32_t             controlCapacity;                                      // Number of control buttons
    uint32_t             waveformCalibrationCapacity;                          //
    size_t               healthNodeBufferSize;                                 // Buffer size (B)
    size_t               healthBufferSize;                                     // Buffer size (B)
    size_t               statusBufferSize;                                     // Buffer size (B)
    size_t               configBufferSize;                                     // Buffer size (B)
    size_t               positionBufferSize;                                   // Buffer size (B)
    size_t               pulseBufferSize;                                      //
    size_t               rayBufferSize;                                        //
    uint32_t             pulseSmoothFactor;                                    // Pulse rate (Hz)
    uint32_t             pulseTicsPerSecond;                                   // Pulse tics per second (normally 10e6)
    uint32_t             positionSmoothFactor;                                 // Position rate (Hz)
    uint32_t             positionTicsPerSecond;                                // Position tics per second
    double               positionLatency;                                      // Position latency (s)
    double               latitude;                                             // Latitude (degrees)
    double               longitude;                                            // Longitude (degrees)
    float                heading;                                              // Radar heading
    float                radarHeight;                                          // Radar height from ground (m)
    float                wavelength;                                           // Radar wavelength (m)
    RKName               name;                                                 // Radar name
    RKName               filePrefix;                                           // Prefix of output files
    char                 dataPath[RKMaximumFolderPathLength];                  // Root path for the data files
} RKRadarDesc;

//
// A running configuration buffer
//
typedef struct rk_config {
    RKIdentifier         i;                                                    // Identity counter
    float                sweepElevation;                                       // Sweep elevation angle (degrees)
    float                sweepAzimuth;                                         // Sweep azimuth angle (degrees)
    RKMarker             startMarker;                                          // Marker of the start ray
    uint8_t              filterCount;                                          // Number of filters
    RKFilterAnchor       filterAnchors[RKMaxFilterCount];                      // Filter anchors
    uint32_t             pw[RKMaxFilterCount];                                 // Pulse width (ns)
    uint32_t             prf[RKMaxFilterCount];                                // Pulse repetition frequency (Hz)
    uint32_t             pulseGateCount;                                       // Number of range gates
    RKFloat              pulseGateSize;                                        // Size of range gate (m)
    uint32_t             waveformId[RKMaxFilterCount];                         // Transmit waveform
    RKFloat              noise[2];                                             // Noise floor (ADU)
    RKFloat              systemZCal[2];                                        // System-wide Z calibration (dB)
    RKFloat              systemDCal;                                           // System-wide ZDR calibration (dB)
    RKFloat              systemPCal;                                           // System-wide phase calibration (rad)
    RKFloat              ZCal[RKMaxFilterCount][2];                            // Waveform Z calibration (dB)
    RKFloat              DCal[RKMaxFilterCount];                               // Waveform ZDR calibration (dB)
    RKFloat              PCal[RKMaxFilterCount];                               // Waveform phase calibration (rad)
    RKFloat              SNRThreshold;                                         // Censor SNR (dB)
    RKName               waveform;                                             // Waveform name
    char                 vcpDefinition[RKMaximumCommandLength];                // Volume coverage pattern
} RKConfig;

//
// Consolidated health buffer
//
typedef union rk_heath {
    struct {
        RKIdentifier        i;                                                 // Identity counter
        RKHealthFlag        flag;                                              // Health flag
        struct timeval      time;                                              // Time in struct timeval
        double              timeDouble;                                        // Time in double
        char                string[RKMaximumStringLength];                     // Health string
    };
    RKByte               *bytes;
} RKHealth;

//
// Individual health buffer
//
typedef struct rk_nodal_health {
    RKHealth             *healths;                                             // Pointer (8 byte for 64-bit systems)
    uint32_t             index;                                                // Index (4 byte)
    bool                 active;                                               // Active flag (1 byte)
} RKNodalHealth;

//
// Raw position reported from a pedestal
//
typedef union rk_position {
    struct {
        RKIdentifier        i;                                                // Counter
        uint64_t            tic;                                              // Time tic
        RKFourByte          rawElevation;                                     // Raw elevation readout
        RKFourByte          rawAzimuth;                                       // Raw azimuth readout
        RKFourByte          rawElevationVelocity;                             // Raw velocity of elevation readout
        RKFourByte          rawAzimuthVelocity;                               // Raw velocity of azimuth readout
        RKFourByte          rawElevationStatus;                               // Raw status of elevation readout
        RKFourByte          rawAzimuthStatus;                                 // Raw status of azimuth readout
        uint8_t             queueSize;                                        // Queue size of the readout buffer
        uint8_t             elevationMode;                                    // Positioning mode of elevation
        uint8_t             azimuthMode;                                      // Positioning mode of azimuth
        uint8_t             sequence;                                         // DEBUG command sequence
        RKPositionFlag      flag;                                             // Position flag
        float               elevationDegrees;                                 // Decoded elevation
        float               azimuthDegrees;                                   // Decoded elevation
        float               elevationVelocityDegreesPerSecond;                // Decoded velocity of elevation
        float               azimuthVelocityDegreesPerSecond;                  // Decoded velocity of azimuth
        float               elevationCounter;                                 // Progress counter (of target) of the elevation
        float               elevationTarget;                                  // Targeted progress counter of the elevation
        float               azimuthCounter;                                   // Progress counter (of target) of the azimuth
        float               azimuthTarget;                                    // Targeted progress counter of the azimuth
        float               sweepElevationDegrees;                            // Set elevation for current sweep
        float               sweepAzimuthDegrees;                              // Set azimuth for current sweep
        struct timeval      time;                                             // Time in struct timeval
        double              timeDouble;                                       // Time in double;
    };
    RKByte               bytes[128];
} RKPosition;

//
// Pulse header
//
typedef struct rk_pulse_header {
    RKIdentifier        i;                                                    // Identity counter
    RKIdentifier        n;                                                    // Network counter, may be useful to indicate packet loss
    uint64_t            t;                                                    // A clean clock-related tic count
    RKPulseStatus       s;                                                    // Status flag
    uint32_t            capacity;                                             // Allocated capacity
    uint32_t            gateCount;                                            // Number of range gates
    RKMarker            marker;                                               // Position Marker
    uint32_t            pulseWidthSampleCount;                                // Pulsewidth
    struct timeval      time;                                                 // UNIX time in seconds since 1970/1/1 12:00am
    double              timeDouble;                                           // Time in double representation
    RKFourByte          rawAzimuth;                                           // Raw azimuth reading, which may take up to 4 bytes
    RKFourByte          rawElevation;                                         // Raw elevation reading, which may take up to 4 bytes
    uint16_t            configIndex;                                          // Operating configuration index
    uint16_t            configSubIndex;                                       // Operating configuration sub-index
    uint16_t            azimuthBinIndex;                                      // Ray bin
    float               gateSizeMeters;                                       // Size of range gates
    float               elevationDegrees;                                     // Elevation in degrees
    float               azimuthDegrees;                                       // Azimuth in degrees
    float               elevationVelocityDegreesPerSecond;                    // Velocity of elevation in degrees / second
    float               azimuthVelocityDegreesPerSecond;                      // Velocity of azimuth in degrees / second
} RKPulseHeader;

//
// Pulse parameters for matched filters (pulseCompressionCore)
//
typedef struct rk_pulse_parameters {
    uint32_t             filterCounts[2];
    uint32_t             planIndices[2][RKMaxFilterCount];
    uint32_t             planSizes[2][RKMaxFilterCount];
} RKPulseParameters;

//
// Pulse
//
// - RKPulse struct is padded to a SIMD alignment
//
typedef struct rk_pulse {
    union {
        struct {
            RKPulseHeader        header;
            RKPulseParameters    parameters;
        };
        RKByte               headerBytes[RKPulseHeaderPaddedSize];
    };
    RKByte               data[0];
} RKPulse;

//
// Ray header
//
typedef struct rk_ray_header {
    uint32_t             capacity;                                             // Capacity
    RKRayStatus          s;                                                    // Ray status
    RKIdentifier         i;                                                    // Ray indentity
    RKIdentifier         n;                                                    // Ray network counter
    RKMarker             marker;                                               // Volume / sweep / radial marker
    RKBaseMomentList     baseMomentList;                                       // 16-bit MSB for products + 16-bit LSB for display
    uint16_t             configIndex;                                          // Operating configuration index
    uint16_t             configSubIndex;                                       // Operating configuration sub-index
    uint16_t             gateCount;                                            //
    uint16_t             pulseCount;                                           //
    float                gateSizeMeters;                                       // Size of range gates
    float                sweepElevation;                                       // Sweep elevation for PPI
    float                sweepAzimuth;                                         // Sweep azimuth for RHI
    struct timeval       startTime;                                            // Start time of the ray in UNIX time
    double               startTimeDouble;                                      // Start time in double representation
    float                startAzimuth;                                         // End time in double representation
    float                startElevation;                                       //
    struct timeval       endTime;                                              // End time of the ray in UNIX time
    double               endTimeDouble;                                        //
    float                endAzimuth;                                           //
    float                endElevation;                                         //
} RKRayHeader;

//
// Ray
//
// - RKRay struct is padded to a SIMD alignment
//
typedef struct rk_ray {
    union {
        RKRayHeader      header;
        RKByte           headerBytes[RKRayHeaderPaddedSize];
    };
    RKByte           data[0];
} RKRay;

//
// Sweep header
//
typedef struct rk_sweep_header {
    uint32_t             rayCount;                                             // Number of rays
    uint32_t             gateCount;                                            // Number of range gates
    time_t               startTime;                                            // Start time of the sweep
    time_t               endTime;                                              // End time of the sweep
    RKBaseMomentList     baseMomentList;                                       // List of available products
    float                gateSizeMeters;                                       // Gate size in meters
    bool                 isPPI;                                                //
    bool                 isRHI;                                                //
    bool                 external;                                             // Data is external buffer, reference by *rays[]
    RKRadarDesc          desc;                                                 //
    RKConfig             config;                                               //
    char                 filename[RKMaximumPathLength];                        // Propose filename without symbol and extension, XX-20180520-112233
} RKSweepHeader;

//
// Sweep
//
typedef struct rk_sweep {
    RKSweepHeader        header;
    RKBuffer             rayBuffer;
    RKRay                *rays[RKMaximumRaysPerSweep];
} RKSweep;

//
// A scratch space for moment processor
//
typedef struct rk_scratch {
    uint32_t             capacity;                                             // Capacity
    bool                 showNumbers;                                          // A flag for showing numbers
    uint8_t              lagCount;                                             // Number of lags of R & C
    uint8_t              userLagChoice;                                        // Number of lags in multi-lag estimator from user
    RKIQZ                mX[2];                                                // Mean of X, 2 for dual-pol
    RKIQZ                vX[2];                                                // Variance of X, i.e., E{X' * X} - E{X}' * E{X}
    RKIQZ                R[2][RKLagCount];                                     // ACF up to RKLagCount - 1 for each polarization
    RKIQZ                C[2 * RKLagCount - 1];                                // CCF in [ -RKLagCount + 1, ..., -1, 0, 1, ..., RKLagCount - 1 ]
    RKIQZ                sC;                                                   // Summation of Xh * Xv'
    RKIQZ                ts;                                                   // Temporary scratch space
    RKFloat              *aR[2][RKLagCount];                                   // abs(ACF)
    RKFloat              *aC[2 * RKLagCount - 1];                              // abs(CCF)
    RKFloat              *gC;                                                  // Gaussian fitted CCF(0)  NOTE: Need to extend this to multi-multilag
    RKFloat              noise[2];                                             // Noise floor of each channel
    RKFloat              velocityFactor;                                       // Velocity factor to multiply by atan2(R(1))
    RKFloat              widthFactor;                                          // Width factor to multiply by the ln(S/|R(1)|) : 
    RKFloat              KDPFactor;                                            // Normalization factor of 1.0 / gateWidth in kilometers
    RKFloat              *dcal;                                                // Calibration offset to D
    RKFloat              *pcal;                                                // Calibration offset to P (radians)
    RKFloat              SNRThreshold;                                         // SNR threshold for masking
    RKFloat              *rcor[2];                                             // Reflectivity range correction factor
    RKFloat              *S[2];                                                // Signal
    RKFloat              *Z[2];                                                // Reflectivity in dB
    RKFloat              *V[2];                                                // Velocity in same units as aliasing velocity
    RKFloat              *W[2];                                                // Spectrum width in same units as aliasing velocity
    RKFloat              *SNR[2];                                              // Signal-to-noise ratio
    RKFloat              *ZDR;                                                 // Differential reflectivity ZDR
    RKFloat              *PhiDP;                                               // Differential phase PhiDP
    RKFloat              *RhoHV;                                               // Cross-correlation coefficient RhoHV
    RKFloat              *KDP;                                                 // Specific phase KDP
    int8_t               *mask;                                                // Mask for censoring
} RKScratch;

//
// File header of raw I/Q data
//
typedef union rk_file_header {
    struct {
        RKName               preface;                                          //
        uint32_t             buildNo;                                          //
        RKRadarDesc          desc;                                             //
        RKConfig             config;                                           //
    };                                                                         //
    RKByte               bytes[4096];                                          //
} RKFileHeader;

//
// Preference entry
//
typedef struct rk_preferene_object {
    char                 keyword[RKNameLength];                                //
    char                 valueString[RKMaximumStringLength];                   //
    bool                 isNumeric;                                            //
    bool                 isValid;                                              //
    int                  numericCount;                                         //
    char                 subStrings[4][RKNameLength];                          //
    double               doubleValues[4];                                      //
    bool                 boolValues[4];                                        //
} RKPreferenceObject;

//
// Control
//
typedef struct rk_control {
    uint32_t             uid;                                                  // A unique identifier
    uint8_t              state;                                                // Some internal state for house keeping
    uint8_t              level;                                                // Root level controls are for top interface
    char                 label[RKNameLength];                                  // Label up to RKNameLength
    char                 command[RKMaximumStringLength];                       // Control command
} RKControl;

//
// Status
//
// - This can be a supported feature reported back from client
//
typedef struct rk_status {
    RKIdentifier         i;
    RKStatusFlag         flag;
    uint8_t              pulseMonitorLag;
    uint8_t              pulseSkipCount;
    uint8_t              pulseCoreLags[RKProcessorStatusPulseCoreCount];
    uint8_t              pulseCoreUsage[RKProcessorStatusPulseCoreCount];
    uint8_t              ringMonitorLag;
    uint8_t              ringSkipCount;
    uint8_t              ringCoreLags[RKProcessorStatusRingCoreCount];
    uint8_t              ringCoreUsage[RKProcessorStatusRingCoreCount];
    uint8_t              rayMonitorLag;
    uint8_t              raySkipCount;
    uint8_t              rayCoreLags[RKProcessorStatusRayCoreCount];
    uint8_t              rayCoreUsage[RKProcessorStatusRayCoreCount];
    uint8_t              recorderLag;
} RKStatus;

//
// Simple engine
//
// - File monitor, etc.
//
typedef struct rk_simple_engine {
    RKName               name;                                                 // Engine name
    uint8_t              verbose;                                              //
    pthread_t            tid;                                                  //
    RKEngineState        state;                                                //
    uint32_t             memoryUsage;                                          //
    void                 *userResource;                                        // User defined resource
} RKSimpleEngine;

//
// File monitor
//
typedef struct rk_file_monitor {                                               // Simple engine template
    RKName               name;                                                 // Engine name
    uint8_t              verbose;                                              //
    pthread_t            tid;                                                  //
    RKEngineState        state;                                                //
    uint32_t             memoryUsage;                                          //
    char                 filename[RKMaximumPathLength];                        // User defined file to monitor
    void                 (*callbackRoutine)(void *);                           // User defined callback function
    void                 *userResource;                                        // User defined resource
} RKFileMonitor;

//
// User product from other processing nodes
//

typedef union rk_product_desc {                                                // A 1-KB struct that describes a product
    struct {                                                                   //
        RKName           name;                                                 // Name of the product
        RKName           unit;                                                 // Unit of the product
        RKName           colormap;                                             // Colormap of the product for the UI
        char             symbol[8];                                            // Product symbol
        RKProductType    type;                                                 // RKProductType
        uint32_t         pieceCount;                                           // Count of piece-wise function that maps data to color index
        RKFloat          w[16];                                                // Data to color index weight (piece-wise function)
        RKFloat          b[16];                                                // Data to color index bias (piece-wise function)
        RKFloat          l[16];                                                // The lower bound of each piece
        RKFloat          mininimumValue;                                       // Minimum value
        RKFloat          maximumValue;                                         // Maximum value
    };
    RKByte bytes[1024];
} RKProductDesc;

typedef struct rk_product {                                                    // A description of user product
    RKIdentifier         i;                                                    // Product counter to be synchronized with RKConfig->i
    RKProductId          pid;                                                  // Product identifier from RKProductRegister()
    RKProductDesc        desc;                                                 // Description
    RKProductStatus      flag;                                                 // Various state
    uint32_t             depth;                                                // Number of arrays
    uint32_t             capacity;                                             // Number of RKFloat elements in blcok of array
    RKFloat              *array;                                               // Flattened array of user product
} RKProduct;

typedef struct rk_waveform {
    int                  count;                                                // Number of groups
    int                  depth;                                                // Maximum number of samples
    double               fc;                                                   // Carrier frequency (Hz)
    double               fs;                                                   // Sampling frequency (Hz)
    RKWaveformType       type;                                                 // Various type of waveforms
    RKName               name;                                                 // Waveform name in plain string
    RKComplex            *samples[RKMaxFilterGroups];                          // Samples up to amplitude of 1.0
    RKInt16C             *iSamples[RKMaxFilterGroups];                         // 16-bit full-scale equivalence of the waveforms
    uint32_t             filterCounts[RKMaxFilterGroups];                      // Number of filters to applied to each waveform, see filterAnchors
    RKFilterAnchor       filterAnchors[RKMaxFilterGroups][RKMaxFilterCount];   // Filter anchors of each sub-waveform for de-multiplexing
} RKWaveform;

typedef struct rk_waveform_cal {
    uint32_t             uid;                                                   // A unique identifier
    RKName               name;                                                  // A string description
    uint8_t              count;                                                 // The number of tones in this waveform
    RKFloat              ZCal[RKMaxFilterCount][2];                             // Calibration factor for individual tone
    RKFloat              DCal[RKMaxFilterCount];                                // Calibration factor for individual tone
    RKFloat              PCal[RKMaxFilterCount];                                // Calibration factor for individual tone
} RKWaveformCalibration;

#pragma pack(pop)

#endif /* defined(__RadarKit_Types__) */
