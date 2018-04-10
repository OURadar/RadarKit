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

#define RKVersionString "1.2.8b"

/*
  Memory Blocks
  Defines the number of slots and gates of each pulse of the RKRadar structure
 
  RKBufferCSlotCount The number of slots for config
  RKBufferHSlotCount The number of slots for health JSON string
  RKBufferSSlotCount The number of slots for status string
  RKBufferPSlotCount The number of slots for position buffer
  RKBuffer0SlotCount The number of slots for level-0 pulse storage in the host memory
  RKBuffer1SlotCount The number of slots for level-1 pulse storage in the host memory
  RKBuffer2SlotCount The number of slots for level-2 pulse storage in the host memory
  RKcontrolCapacity The number of controls (buttons)
  RKGateCount The maximum number of gates allocated for each pulse
  RKSIMDAlignSize The minimum alignment size. AVX requires 256 bits = 32 bytes. AVX-512 is on the horizon now.
 
 */
#define RKBufferCSlotCount               25                          // Config
#define RKBufferHSlotCount               25                          // Health
#define RKBufferSSlotCount               90                          // Status strings
#define RKBufferPSlotCount               1000                        // Positions
#define RKBuffer0SlotCount               20000                       // Raw I/Q
#define RKBuffer2SlotCount               36000                       // Ray
#define RKMaxControlCount                128                         // Controls
#define RKGateCount                      262144                      // Must be a multiple of RKSIMDAlignSize
#define RKLagCount                       5                           // Number lags of ACF / CCF lag = +/-4 and 0
#define RKSIMDAlignSize                  64                          // SSE 16, AVX 32, AVX-512 64
#define RKMaxFilterCount                 8                           // Maximum filter count within each group. Check RKPulseParameters
#define RKMaxFilterGroups                22                          // Maximum filter group count
#define RKWorkerDutyCycleBufferDepth     1000
#define RKMaxPulsesPerRay                2000
#define RKMaxProductCount                10                          // 16 to be the absolute max since productList enum is 32-bit (product + display)
#define RKMaxRaysPerSweep                1500                        // 1440 is 0.25-deg. This should be plenty
#define RKMaxPacketSize                  1024 * 1024
#define RKNetworkTimeoutSeconds          20
#define RKNetworkReconnectSeconds        3
#define RKLagRedThreshold                0.5
#define RKLagOrangeThreshold             0.7
#define RKDutyCyleRedThreshold           0.95
#define RKDutyCyleOrangeThreshold        0.90
#define RKStatusBarWidth                 10
#define RKPulseCountForNoiseMeasurement  200
#define RKProcessorStatusPulseCoreCount  16
#define RKProcessorStatusRingCoreCount   16
#define RKProcessorStatusRayCoreCount    16
#define RKHostMonitorPingInterval        5

#define RKDefaultDataPath                "data"
#define RKDataFolderIQ                   "iq"
#define RKDataFolderMoment               "moment"
#define RKDataFolderHealth               "health"
#define RKLogFolder                      "log"
#define RKWaveformFolder                 "waveforms"
#define RKFFTWisdomFile                  "radarkit-fft-wisdom"

#define RKNoColor                        "\033[0m"
#define RKRedColor                       "\033[38;5;196m"
#define RKGreenColor                     "\033[38;5;82m"
#define RKOrangeColor                    "\033[38;5;214m"
#define RKMaximumStringLength            4096
#define RKMaximumPathLength              1024
#define RKNameLength                     256
#define RKPulseHeaderPaddedSize          256                         // Change this to higher number for post-AVX2 intrinsics
#define RKRayHeaderPaddedSize            128                         // Change this to higher number for post-AVX2 intrinsics

#define RKColorDutyCycle(x)  (x > RKDutyCyleRedThreshold ? "\033[91m" : (x > RKDutyCyleOrangeThreshold ? "\033[93m" : "\033[92m"))
#define RKColorLag(x)        (x > RKLagRedThreshold      ? "\033[91m" : (x > RKLagOrangeThreshold      ? "\033[93m" : "\033[92m"))

#define ITALIC(x)            "\033[3m" x "\033[23m"
#define UNDERLINE(x)         "\033[4m" x "\033[24m"
#define HIGHLIGHT(x)         "\033[38;5;82;48;5;238m" x "\033[0m"
#define UNDERLINE_ITALIC(x)  "\033[3;4m" x "\033[23;24m"

typedef uint8_t   RKByte;
typedef float     RKFloat;           // We can change this to double if we decided one day
typedef ssize_t   RKResult;          // Generic return from functions, 0 for no errors and !0 for others.
typedef void *    RKBuffer;
typedef void *    RKTransceiver;
typedef void *    RKPedestal;
typedef void *    RKHealthRelay;
typedef void *    RKMasterController;
typedef char      RKName[RKNameLength];

typedef const float RKConst;

#pragma pack(push, 1)

// Fundamental unit of a (16-bit) + (16-bit) raw complex IQ sample
typedef struct rk_int16c {
    int16_t i;
    int16_t q;
} RKInt16C;

// Interleaved complex format. Fundamental unit of a (float) + (float) raw complex IQ sample
typedef struct rk_complex {
    RKFloat i;
    RKFloat q;
} RKComplex;

// Deinterleaved complex format for vector library
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
    struct { int32_t i32; };
    struct { float f; };
} RKFourByte;

typedef union rk_filter_anchor {
	struct {
		uint32_t      name;                                              // Just an internal name id
		uint32_t      origin;                                            // Filter origin to be used with RKWaveform
		uint32_t      length;                                            // Filter length to be used with RKWaveform
		uint32_t      inputOrigin;                                       // Origin of input
		uint32_t      outputOrigin;                                      // Origin of output
		uint32_t      maxDataLength;                                     // Maximum data to decode
		RKFloat       subCarrierFrequency;                               // For house keeping only, use the waveform->fc for DDC
		RKFloat       sensitivityGain;                                   // Sensitivity gain due to longer/efficient waveforms (dB)
		RKFloat       filterGain;                                        // Filter gain from the filter coefficients, should be 0.0 (dB)
	};
	char bytes[64];
} RKFilterAnchor;

#define RKFilterAnchorDefault                           {{0, 0,  1 ,  0, 0, RKGateCount, 0.0f, 1.0f, 1.0f}}
#define RKFilterAnchorDefaultWithMaxDataLength(x)       {{0, 0,  1 ,  0, 0, (x) , 0.0f, 1.0f, 1.0f}}
#define RKFilterAnchorOfLengthAndMaxDataLength(x, y)    {{0, 0, (x),  0, 0, (y) , 0.0f, 1.0f, 1.0f}}

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

enum RKResult {
    RKResultTimeout = -99,
	RKResultNullInput,
    RKResultEngineNotWired,
    RKResultIncompleteSend,
    RKResultIncompleteReceive,
	RKResultIncompleteTransceiver,
	RKResultIncompletePedestal,
	RKResultIncompleteHealthRelay,
    RKResultErrorCreatingOperatorRoutine,
    RKResultErrorCreatingOperatorCommandRoutine,
    RKResultErrorCreatingClientRoutine,
    RKResultSDToFDError,
    RKResultNoPulseBuffer,
    RKResultNoRayBuffer,
    RKResultNoPulseCompressionEngine,
    RKResultNoMomentEngine,
    RKResultFailedToStartCompressionCore,
    RKResultFailedToStartPulseWatcher,
    RKResultFailedToStartRingPulseWatcher,
    RKResultFailedToInitiateSemaphore,
    RKResultFailedToRetrieveSemaphore,
    RKResultTooBig,
    RKResultFailedToAllocateFFTSpace,
    RKResultFailedToAllocateFilter,
    RKResultFailedToAllocateDutyCycleBuffer,
    RKResultFailedToAllocateScratchSpace,
	RKResultFailedToSetWaveform,
    RKResultFailedToSetFilter,
    RKResultEngineDeactivatedMultipleTimes,
    RKResultFailedToStartMomentCore,
    RKResultFailedToStartPulseGatherer,
    RKResultUnableToChangeCoreCounts,
    RKResultFailedToStartPedestalWorker,
    RKResultFailedToGetVacantPosition,
    RKResultFailedToGetVacantHealth,
    RKResultFailedToStartRayGatherer,
    RKResultFailedToStartHealthWorker,
    RKResultFailedToStartPulseRecorder,
    RKResultFailedToStartPedestalMonitor,
    RKResultFailedToStartFileManager,
    RKResultFailedToStartFileRemover,
	RKResultFailedToStartTransceiver,
	RKResultFailedToStartPedestal,
	RKResultFailedToStartHealthRelay,
    RKResultPreferenceFileNotFound,
    RKResultFailedToMeasureNoise,
    RKResultFailedToCreateFileRemover,
    RKResultFileManagerBufferNotResuable,
	RKResultInvalidMomentParameters,
    RKResultFailedToCreateUnitWorker,
    RKResultFailedToStartHostWatcher,
    RKResultFailedToStartHostPinger,
    RKResultFailedToExecuteCommand,
	RKResultNoRadar,
    RKResultSuccess = 0,
    RKResultNoError = 0
};

enum RKEngineColor {
    RKEngineColorCommandCenter = 9,
    RKEngineColorPulseCompressionEngine = 6,
    RKEngineColorPulseRingFilterEngine = 3,
    RKEngineColorPositionEngine = 4,
    RKEngineColorMomentEngine = 7,
    RKEngineColorHealthEngine = 1,
    RKEngineColorDataRecorder = 8,
    RKEngineColorSweepEngine = 13,
    RKEngineColorHealthLogger = 5,
    RKEngineColorFileManager = 2,
    RKEngineColorTransceiver = 12,
    RKEngineColorPedestalRelayPedzy = 10,
    RKEngineColorHealthRelayTweeta = 0,
    RKEngineColorRadarRelay = 12,
    RKEngineColorHostMonitor = 11,
    RKEngineColorClock = 14,
    RKEngineColorMisc = 15,
    RKEngineColorEngineMonitor = 14
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
    RKPositionFlagScanActive         = (1 << 28),
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
    RKMarkerSweepMiddle              = 1,
    RKMarkerSweepBegin               = (1 << 1),
    RKMarkerSweepEnd                 = (1 << 2),
    RKMarkerVolumeBegin              = (1 << 3),
    RKMarkerVolumeEnd                = (1 << 4),
    RKMarkerPPIScan                  = (1 << 8),
    RKMarkerRHIScan                  = (1 << 9),
    RKMarkerPointScan                = (1 << 10),
    RKMarkerMemoryManagement         = (1 << 15)
};

// Typical status progression:
// -> RKPulseStatusVacant
// -> RKPulseStatusHasIQData
// -> RKPulseStatusInspected                                (main thread)
// -> RKPulseStatusCompressed / RKPulseStatusSkipped        (core threads)
// -> RKPulseStatusDownSampled                              (core threads)
// -> RKPulseStatusProcessed                                (core thread)
// -> RKPulseStatusRingInspected                            (main thread)
// -> RKPulseStatusRingFiltered / RKPulseStatusRingSkipped  (main thread consolidates)
// -> RKPulseStatusRingProcessed                            (main thread)
// -> RKPulseStatusHasPosition
// -> RKPulseStatusReadyForMoment
typedef uint32_t RKPulseStatus;
enum RKPulseStatus {
    RKPulseStatusVacant              = 0,
    RKPulseStatusHasIQData           = 1,                            // 0x01
    RKPulseStatusHasPosition         = (1 << 1),                     // 0x02
    RKPulseStatusInspected           = (1 << 2),                     // 0x04
    RKPulseStatusCompressed          = (1 << 3),                     // 0x08
    RKPulseStatusSkipped             = (1 << 4),                     // 0x10
    RKPulseStatusDownSampled         = (1 << 5),                     // 0x20
	RKPulseStatusProcessed           = (1 << 6),                     // 0x40
	RKPulseStatusRingInspected       = (1 << 7),                     // 0x80
    RKPulseStatusRingFiltered        = (1 << 8),
    RKPulseStatusRingSkipped         = (1 << 9),
    RKPulseStatusRingProcessed       = (1 << 10),
    RKPulseStatusReadyForMoment      = (RKPulseStatusProcessed | RKPulseStatusRingProcessed | RKPulseStatusHasPosition)
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
    RKInitFlagVerbose                = 0x0001,
    RKInitFlagVeryVerbose            = 0x0002,
    RKInitFlagVeryVeryVerbose        = 0x0004,
	RKInitFlagShowClockOffset        = 0x0008,
	RKInitFlagReserved1              = 0x0020,
	RKInitFlagReserved2              = 0x0020,
	RKInitFlagReserved3              = 0x0040,
	RKInitFlagReserved4              = 0x0080,
    RKInitFlagAllocStatusBuffer      = 0x0100,                       // 1 << 8
    RKInitFlagAllocConfigBuffer      = 0x0200,                       // 1 << 9
    RKInitFlagAllocRawIQBuffer       = 0x0400,                       // 1 << 10
    RKInitFlagAllocPositionBuffer    = 0x0800,                       // 1 << 11
    RKInitFlagAllocMomentBuffer      = 0x1000,                       // 1 << 12
    RKInitFlagAllocHealthBuffer      = 0x2000,                       // 1 << 13
    RKInitFlagAllocHealthNodes       = 0x4000,                       // 1 << 14
    RKInitFlagSignalProcessor        = 0x8000,                       // 1 << 15
    RKInitFlagRelay                  = 0x7703,                       // Everything = 0xFF00 - 0x8000(DSP) - 0x0800(Pos)
    RKInitFlagAllocEverything        = 0xFF01,
    RKInitFlagAllocEverythingQuiet   = 0xFF00,
};

typedef uint32_t RKProductList;
enum RKProductList {
    RKProductListNone                = 0,                            // None
    RKProductListDisplayZ            = (1),                          // Display Z - Reflectivity dBZ
    RKProductListDisplayV            = (1 << 1),                     // Display V - Velocity
    RKProductListDisplayW            = (1 << 2),                     // Display W - Width
    RKProductListDisplayD            = (1 << 3),                     // Display D - Differential Reflectivity
    RKProductListDisplayP            = (1 << 4),                     // Display P - PhiDP
    RKProductListDisplayR            = (1 << 5),                     // Display R - RhoHV
    RKProductListDisplayK            = (1 << 6),                     // Display K - KDP
    RKProductListDisplaySh           = (1 << 7),                     // Display Sh - Signal
    RKProductListDisplaySv           = (1 << 8),                     // Display Sv - Signal
    RKProductListDisplayZVWDPRKS     = 0x000000FF,                   // Display All
    RKProductListProductZ            = (1 << 16),                    // Data of Z
    RKProductListProductV            = (1 << 17),                    // Data of V
    RKProductListProductW            = (1 << 18),                    // Data of W
    RKProductListProductD            = (1 << 19),                    // Data of D
    RKProductListProductP            = (1 << 20),                    // Data of P
    RKProductListProductR            = (1 << 21),                    // Data of R
    RKProductListProductK            = (1 << 22),                    // Data of K
    RKProductListProductSh           = (1 << 23),                    // Data of Sh
	RKProductListProductSv           = (1 << 24),                    // Data of Sv
    RKProductListProductZVWDPR       = 0x003F0000,                   // Base data, i.e., without K, and S
    RKProductListProductZVWDPRK      = 0x007F0000,                   // Base data + K
    RKProductListProductZVWDPRKS     = 0x01FF0000                    // All data
};

typedef uint32_t RKProductIndex;
enum RKProductIndex {
    RKProductIndexZ,
    RKProductIndexV,
    RKProductIndexW,
    RKProductIndexD,
    RKProductIndexP,
    RKProductIndexR,
	RKProductIndexK,
    RKProductIndexSh,
	RKProductIndexSv,
    RKProductIndexZv,
    RKProductIndexVv,
    RKProductIndexWv
};

typedef uint32_t RKConfigKey;
enum RKConfigKey {
    RKConfigKeyNull,
    RKConfigKeySweepElevation,
    RKConfigKeySweepAzimuth,
    RKConfigKeyPositionMarker,
    RKConfigKeyPRF,
    RKConfigKeyDualPRF,
    RKConfigKeyGateCount,
    RKConfigKeyWaveform,
    RKConfigKeyWaveformId,
    RKConfigKeyFilterCount,
    RKConfigKeyFilterAnchor,
    RKConfigKeyFilterAnchor2,
    RKConfigKeyFilterAnchors,
    RKConfigKeyNoise,
    RKConfigKeySystemZCal,
	RKConfigKeySystemDCal,
    RKConfigKeyZCal,
    RKConfigKeyDCal,
    RKConfigKeyPCal,
    RKConfigKeyZCal2,
    RKConfigKeyDCal2,
    RKConfigKeyPCal2,
    RKConfigKeyZCals,
    RKConfigKeyDCals,
    RKConfigKeyPCals,
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
    RKHealthNodeInvalid = (RKHealthNode)-1
};

typedef uint32_t RKEngineState;
enum RKEngineState {
    RKEngineStateNull                = 0,
    RKEngineStateSleep0              = 1,                            // Usually for a wait just outside of the main while loop
    RKEngineStateSleep1              = (1 << 1),                     // Stage 1 wait - usually waiting for pulse
    RKEngineStateSleep2              = (1 << 2),                     // Stage 2 wait
    RKEngineStateSleep3              = (1 << 3),                     // Stage 3 wait
    RKEngineStateSleepMask           = 0x0F,
    RKEngineStateWritingFile         = (1 << 4),                     // Generating an output file
	RKEngineStateMemoryChange        = (1 << 5),                     // Some required pointers are being changed
    RKEngineStateAllocated           = (1 << 8),                     // Resources have been allocated
    RKEngineStateProperlyWired       = (1 << 9),                     // All required pointers are properly wired up
    RKEngineStateActivating          = (1 << 10),                    // The main run loop is being activated
    RKEngineStateDeactivating        = (1 << 11),                    // The main run loop is being deactivated
    RKEngineStateActive              = (1 << 12)                     // The engine is active
};

typedef uint32_t RKStatusEnum;
enum RKStatusEnum {
    RKStatusEnumUnknown              = -3,
    RKStatusEnumOld                  = -3,
    RKStatusEnumInvalid              = -2,
    RKStatusEnumTooLow               = -2,
    RKStatusEnumLow                  = -1,
    RKStatusEnumNormal               =  0,
    RKStatusEnumActive               =  0,
    RKStatusEnumHigh                 =  1,
    RKStatusEnumStandby              =  1,
    RKStatusEnumInactive             =  1,
    RKStatusEnumOutOfRange           =  1,
    RKStatusEnumTooHigh              =  2,
    RKStatusEnumNotOperational       =  2,
    RKStatusEnumOff                  =  2,
    RKStatusEnumFault                =  2,
    RKStatusEnumCritical             =  4                            // This would the status we may shutdown the radar. Co-incidently, red = 0x4
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
    RKStreamNull                     = 0,                            //
    RKStreamControl                  = 1,                            // Controls
    RKStreamStatusHealth             = (1 << 1),                     //
    RKStreamStatusPulses             = (1 << 2),                     //
    RKStreamStatusRays               = (1 << 3),                     //
    RKStreamStatusPositions          = (1 << 4),                     //
    RKStreamStatusEngines            = (1 << 6),                     //
    RKStreamStatusProcessorStatus    = (1 << 7),                     // Consolidated binary from of the system status
    RKStreamStatusAll                = 0xFE,                         //
    RKStreamDisplayIQ                = (1 << 8),                     // Low rate IQ (sub-smpled)
    RKStreamDisplayIQFiltered        = (1 << 9),                     // Filtered IQ (usually matched filter is applied)
    RKStreamProductIQ                = (1 << 10),                    // Full rate IQ
    RKStreamProductIQFiltered        = (1 << 11),                    // Full rate filtered IQ
    RKStreamDisplayZ                 = (1 << 16),                    // Display: Z = 0x00010000
    RKStreamDisplayV                 = (1 << 17),                    //
    RKStreamDisplayW                 = (1 << 18),                    //
    RKStreamDisplayD                 = (1 << 19),                    //
    RKStreamDisplayP                 = (1 << 20),                    //
    RKStreamDisplayR                 = (1 << 21),                    //
    RKStreamDisplayK                 = (1 << 22),                    //
    RKStreamDisplaySh                = (1 << 23),                    //
    RKStreamDisplaySv                = (1 << 24),                    //
    RKStreamDisplayZVWDPRKS          = 0x00000001FF0000ULL,          //
    RKStreamProductZ                 = (1ULL << 32),                 // Products
    RKStreamProductV                 = (1ULL << 33),                 //
    RKStreamProductW                 = (1ULL << 34),                 //
    RKStreamProductD                 = (1ULL << 35),                 //
    RKStreamProductP                 = (1ULL << 36),                 //
    RKStreamProductR                 = (1ULL << 37),                 //
    RKStreamProductK                 = (1ULL << 38),                 //
    RKStreamProductSh                = (1ULL << 39),                 //
    RKStreamProductSv                = (1ULL << 40),                 //
    RKStreamProductZVWDPRKS          = 0x0001FF00000000ULL,          //
    RKStreamEverything               = 0x0001FFFFFFFFFFULL           //
};

typedef uint8_t RKHostState;
enum RKHostState {
    RKHostStateUnknown,
    RKHostStateUnreachable,
    RKHostStatePartiallyReachable,
    RKHostStateReachable
};


// A general description of a radar. Most parameters are used for initialization. Some may be
// overriden after the radar has gone live.
typedef struct rk_radar_desc {
    RKInitFlag       initFlags;
    uint32_t         pulseCapacity;
    uint32_t         pulseToRayRatio;
    uint32_t         healthNodeCount;
    uint32_t         healthBufferDepth;
    uint32_t         statusBufferDepth;
    uint32_t         configBufferDepth;
    uint32_t         positionBufferDepth;
    uint32_t         pulseBufferDepth;
    uint32_t         rayBufferDepth;
    uint32_t         controlCapacity;
    uint32_t         pulseSmoothFactor;                              // Pulse rate (Hz)
    uint32_t         pulseTicsPerSecond;                             // Pulse tics per second (normally 10e6)
    uint32_t         positionSmoothFactor;                           // Position rate (Hz)
    uint32_t         positionTicsPerSecond;                          // Position tics per second
    double           positionLatency;                                // Position latency (s)
    double           latitude;                                       // Latitude (degrees)
    double           longitude;                                      // Longitude (degrees)
    float            heading;                                        // Radar heading
    float            radarHeight;                                    // Radar height from ground (m)
    float            wavelength;                                     // Radar wavelength (m)
    char             name[RKNameLength];                             // Radar name
    char             filePrefix[RKNameLength];                       // Prefix of output files
    char             dataPath[RKMaximumPathLength];                  // Root path for the data files
} RKRadarDesc;

// A running configuration buffer
typedef struct rk_config {
    uint64_t         i;                                              // Identity counter
    float            sweepElevation;                                 // Sweep elevation angle (degrees)
    float            sweepAzimuth;                                   // Sweep azimuth angle (degrees)
    RKMarker         startMarker;                                    // Marker of the start ray
    uint8_t          filterCount;                                    // Number of filters
    RKFilterAnchor   filterAnchors[RKMaxFilterCount];                // Filter anchors
    uint32_t         pw[RKMaxFilterCount];                           // Pulse width (ns)
    uint32_t         prf[RKMaxFilterCount];                          // Pulse repetition frequency (Hz)
    uint32_t         gateCount[RKMaxFilterCount];                    // Number of range gates
    uint32_t         waveformId[RKMaxFilterCount];                   // Transmit waveform
    RKFloat          noise[2];                                       // Noise floor (ADU)
    RKFloat          systemZCal[2];                                  // System-wide reflectivity calibration (dB)
	RKFloat          systemDCal;                                     // System-wide differential reflectivity calibration (dB)
    RKFloat          ZCal[2][RKMaxFilterCount];                      // Waveform Z calibration (dB)
    RKFloat          DCal[RKMaxFilterCount];                         // Waveform ZDR calibration (dB)
    RKFloat          PCal[RKMaxFilterCount];                         // Waveform Phase calibration
    RKFloat          SNRThreshold;                                   // Censor SNR (dB)
    char             waveform[RKNameLength];                         // Waveform name
    char             vcpDefinition[RKNameLength];                    // Volume coverage pattern
} RKConfig;

typedef union rk_heath {
    struct {
        uint64_t         i;                                          // Identity counter
        RKHealthFlag     flag;                                       // Health flag
        struct timeval   time;                                       // Time in struct timeval
        double           timeDouble;                                 // Time in double
        char             string[RKMaximumStringLength];              // Health string
    };
    RKByte               *bytes;
} RKHealth;

typedef struct rk_nodal_health {
    RKHealth         *healths;                                       // Pointer (8 byte for 64-bit systems)
    uint32_t         index;                                          // Index (4 byte)
    bool             active;                                         // Active flag (1 byte)
} RKNodalHealth;

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
    uint32_t         planIndices[2][RKMaxFilterCount];
    uint32_t         planSizes[2][RKMaxFilterCount];
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
    uint64_t         i;                                              // Ray indentity
    uint64_t         n;                                              // Ray network counter
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

typedef struct rk_sweep {
	RKName           name;                                           // Name
	uint32_t         rayCount;
	uint32_t         gateCount;
	uint32_t         productList;
	RKRadarDesc      desc;
	RKConfig         config;
	RKBuffer         rayBuffer;
} RKSweep;

typedef struct rk_scratch {
    uint32_t         capacity;                                       // Capacity
    bool             showNumbers;                                    // A flag for showing numbers
    uint8_t          lagCount;                                       // Number of lags of R & C
    uint8_t          userLagChoice;                                  // Number of lags in multi-lag estimator from user
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
    RKFloat          velocityFactor;                                 // Velocity factor to multiply by atan2(R(1))
	RKFloat          widthFactor;                                    // Width factor to multiply by the ln(S/|R(1)|) : 
    RKFloat          KDPFactor;                                      // Normalization factor of 1.0 / gateWidth in kilometers
    RKFloat          dcal;                                           // Calibration offset to D
    RKFloat          pcal;                                           // Calibration offset to P (radians)
    RKFloat          SNRThreshold;                                   // SNR threshold for masking
    RKFloat          *rcor[2];                                       // Reflectivity range correction factor
    RKFloat          *S[2];                                          // Signal
    RKFloat          *Z[2];                                          // Reflectivity in dB
    RKFloat          *V[2];                                          // Velocity in same units as aliasing velocity
    RKFloat          *W[2];                                          // Spectrum width in same units as aliasing velocity
    RKFloat          *SNR[2];                                        // Signal-to-noise ratio
    RKFloat          *ZDR;                                           // Differential reflectivity ZDR
    RKFloat          *PhiDP;                                         // Differential phase PhiDP
    RKFloat          *RhoHV;                                         // Cross-correlation coefficient RhoHV
    RKFloat          *KDP;                                           // Specific phase KDP
    int8_t           *mask;                                          // Mask for censoring
} RKScratch;

typedef union rk_file_header {
    struct {
        char         preface[RKNameLength];
        uint32_t     buildNo;
        RKRadarDesc  desc;
        RKConfig     config;
    };
    RKByte bytes[4096];
} RKFileHeader;

typedef struct rk_preferene_object {
    char             keyword[RKNameLength];
    char             valueString[RKMaximumStringLength];
    bool             isNumeric;
    bool             isValid;
    int              numericCount;
	char             subStrings[4][RKNameLength];
    double           doubleValues[4];
	bool             boolValues[4];
} RKPreferenceObject;

typedef struct rk_control {
    uint8_t          uid;                                             // A unique identifier
    uint8_t          state;                                           // Some internal state for house keeping
    uint8_t          level;                                           // Root level controls are for top interface
    char             label[RKNameLength];                             // Label up to RKNameLength
    char             command[RKMaximumStringLength];                  // Control command
} RKControl;

// This can be a supported feature reported back from client
typedef struct rk_status {
    uint64_t         i;
    RKStatusFlag     flag;
    uint8_t          pulseMonitorLag;
    uint8_t          pulseSkipCount;
    uint8_t          pulseCoreLags[RKProcessorStatusPulseCoreCount];
    uint8_t          pulseCoreUsage[RKProcessorStatusPulseCoreCount];
    uint8_t          ringMonitorLag;
    uint8_t          ringSkipCount;
    uint8_t          ringCoreLags[RKProcessorStatusRingCoreCount];
    uint8_t          ringCoreUsage[RKProcessorStatusRingCoreCount];
    uint8_t          rayMonitorLag;
    uint8_t          raySkipCount;
    uint8_t          rayCoreLags[RKProcessorStatusRayCoreCount];
    uint8_t          rayCoreUsage[RKProcessorStatusRayCoreCount];
    uint8_t          recorderLag;
} RKStatus;

typedef struct rk_simple_engine {
    RKName           name;
    uint8_t          verbose;
    pthread_t        tid;
    RKEngineState    state;
    uint32_t         memoryUsage;
    void             *userResource;
} RKSimpleEngine;

typedef struct rk_file_monitor {
    // Simple engine template
    RKName           name;
    uint8_t          verbose;
    pthread_t        tid;
    RKEngineState    state;
    uint32_t         memoryUsage;
    // User defined variables
    char             filename[RKMaximumPathLength];
    void             (*callbackRoutine)(void *);
} RKFileMonitor;

#pragma pack(pop)

#endif /* defined(__RadarKit_RKTypes__) */
