//
//  RKTypes.h
//  RadarKit
//
//  These types are being phased out. Avoid them in new codes.
//
//  Created by Boonleng Cheong on 6/24/2022.
//

#ifndef __RadarKit_Types_Compat__
#define __RadarKit_Types_Compat__

#include "RKTypes.h"

#pragma pack(push, 1)

typedef uint32_t RKPositionFlagV1;
enum {
    RKPositionFlagV1Vacant                       = 0,
    RKPositionFlagV1AzimuthEnabled               = 1,                          //  0 - EN
    RKPositionFlagV1AzimuthSafety                = (1 << 1),                   //  1
    RKPositionFlagV1AzimuthError                 = (1 << 2),                   //  2
    RKPositionFlagV1AzimuthSweep                 = (1 << 8),                   //  8
    RKPositionFlagV1AzimuthPoint                 = (1 << 9),                   //  9
    RKPositionFlagV1AzimuthComplete              = (1 << 10),                  // 10
    RKPositionFlagV1ElevationEnabled             = (1 << 16),                  //  0 - EN
    RKPositionFlagV1ElevationSafety              = (1 << 17),                  //  1
    RKPositionFlagV1ElevationError               = (1 << 18),                  //  2
    RKPositionFlagV1ElevationSweep               = (1 << 24),                  //  8
    RKPositionFlagV1ElevationPoint               = (1 << 25),                  //  9
    RKPositionFlagV1ElevationComplete            = (1 << 26),                  // 10
    RKPositionFlagV1ScanActive                   = (1 << 28),
    RKPositionFlagV1VCPActive                    = (1 << 29),
    RKPositionFlagV1HardwareMask                 = 0x3FFFFFFF,
    RKPositionFlagV1Used                         = (1 << 30),
    RKPositionFlagV1Ready                        = (1 << 31),
    RKPositionFlagV1AzimuthModeMask              = (RKPositionFlagV1AzimuthSweep | RKPositionFlagV1AzimuthPoint),
    RKPositionFlagV1ElevationModeMask            = (RKPositionFlagV1ElevationSweep | RKPositionFlagV1ElevationPoint),
    RKPositionFlagV1ScanModeMask                 = (RKPositionFlagV1AzimuthModeMask | RKPositionFlagV1ElevationModeMask)
};

//
// A running configuration buffer (version 1)
//
typedef struct rk_config_f1 {
    RKIdentifier         i;                                                    // Identity counter
    float                sweepElevation;                                       // Sweep elevation angle (degrees)
    float                sweepAzimuth;                                         // Sweep azimuth angle (degrees)
    RKMarker             startMarker;                                          // Marker of the latest start ray
    uint8_t              filterCount;                                          // Number of filters
    RKFilterAnchor       filterAnchors[8];                                     // Filter anchors at ray level
    RKFloat              prt[8];                                               // Pulse repetition time (s)
    RKFloat              pw[8];                                                // Pulse width (s)
    uint32_t             pulseGateCount;                                       // Number of range gates
    RKFloat              pulseGateSize;                                        // Size of range gate (m)
    uint32_t             pulseRingFilterGateCount;                             // Number of range gates to apply ring filter
    uint32_t             waveformId[8];                                        // Transmit waveform
    RKFloat              noise[2];                                             // Noise floor (ADU)
    RKFloat              systemZCal[2];                                        // System-wide Z calibration (dB)
    RKFloat              systemDCal;                                           // System-wide ZDR calibration (dB)
    RKFloat              systemPCal;                                           // System-wide phase calibration (rad)
    RKFloat              ZCal[8][2];                                           // Waveform Z calibration (dB)
    RKFloat              DCal[8];                                              // Waveform ZDR calibration (dB)
    RKFloat              PCal[8];                                              // Waveform phase calibration (rad)
    RKFloat              SNRThreshold;                                         // Censor SNR (dB)
    RKFloat              SQIThreshold;                                         // Censor SQI
    RKName               waveform;                                             // Waveform name
    char                 vcpDefinition[512];                                   // Volume coverage pattern
} RKConfigF1;

typedef union rk_config_f5 {
    struct {
        RKIdentifier         i;                                                // Identity counter
        float                sweepElevation;                                   // Sweep elevation angle (degrees)
        float                sweepAzimuth;                                     // Sweep azimuth angle (degrees)
        RKMarker             startMarker;                                      // Marker of the latest start ray
        RKFloat              prt[8];                                           // Pulse repetition time (s)
        RKFloat              pw[8];                                            // Pulse width (s)
        uint32_t             pulseGateCount;                                   // Number of range gates
        RKFloat              pulseGateSize;                                    // Size of range gate (m)
        uint32_t             transitionGateCount;                              // Transition gate count
        uint32_t             ringFilterGateCount;                              // Number of range gates to apply ring filter
        uint32_t             waveformId[8];                                    // Transmit waveform
        RKFloat              noise[2];                                         // Noise floor (ADU)
        RKFloat              systemZCal[2];                                    // System-wide Z calibration (dB)
        RKFloat              systemDCal;                                       // System-wide ZDR calibration (dB)
        RKFloat              systemPCal;                                       // System-wide phase calibration (rad)
        RKFloat              ZCal[8][2];                                       // Waveform Z calibration (dB)
        RKFloat              DCal[8];                                          // Waveform ZDR calibration (dB)
        RKFloat              PCal[8];                                          // Waveform phase calibration (rad)
        RKFloat              SNRThreshold;                                     // Censor SNR (dB)
        RKFloat              SQIThreshold;                                     // Censor SQI
        RKName               waveformName;                                     // Waveform name
        RKWaveform           *waveform;                                        // Reference to the waveform storage
        RKWaveform           *waveformDecimate;                                // Reference to the waveform storage in Level-II sampling rate
        RKUserResource       userResource;                                     // User resource (not yet)
        uint32_t             userIntegerParameters[8];                         // User integer parameters (not yet)
        float                userFloatParameters[8];                           // User float parameters (not yet)
        char                 vcpDefinition[512];                               // Volume coverage pattern
    };
    RKByte               bytes[1024];
} RKConfigF5;

typedef struct rk_radar_desc_f1_to_f5 {
    RKInitFlag           initFlags;                                            // Initialization. See RKInitFlag enum.
    uint32_t             pulseCapacity;                                        // Capacity of a pulse, i.e., maximum number of range gates
    uint16_t             pulseToRayRatio;                                      // The down-sampling ratio of range gates from pulses to rays
    uint16_t             doNotUse;                                             //
    uint32_t             healthNodeCount;                                      // Number of user health node count
    uint32_t             healthBufferDepth;                                    // Depth of the cosolidated health buffer
    uint32_t             statusBufferDepth;                                    // Depth of the status buffer (RKStatus)
    uint32_t             configBufferDepth;                                    // Depth of the operational configuration parameters
    uint32_t             positionBufferDepth;                                  // Depth of the position readings
    uint32_t             pulseBufferDepth;                                     // Depth of the pulse buffer
    uint32_t             rayBufferDepth;                                       // Depth of the ray buffer
    uint32_t             productBufferDepth;                                   // Depth of the product buffer
    uint32_t             controlCapacity;                                      // Number of control buttons
    uint32_t             waveformCalibrationCapacity;                          // Number of waveform specific calibrations
    size_t               healthNodeBufferSize;                                 // Buffer size (B)
    size_t               healthBufferSize;                                     // Buffer size (B)
    size_t               statusBufferSize;                                     // Buffer size (B)
    size_t               configBufferSize;                                     // Buffer size (B)
    size_t               positionBufferSize;                                   // Buffer size (B)
    size_t               pulseBufferSize;                                      // Buffer size (B)
    size_t               rayBufferSize;                                        // Buffer size (B)
    size_t               productBufferSize;                                    // Buffer size (B)
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
    char                 filePrefix[8];                                        // Prefix of output files
    char                 dataPath[768];                                        // Root path for the data files
} RKRadarDescF1;

//
// Ray header
//
typedef struct rk_ray_header_f1 {
    uint32_t             capacity;                                             // Capacity
    RKRayStatus          s;                                                    // Ray status
    RKIdentifier         i;                                                    // Ray indentity
    RKIdentifier         n;                                                    // Ray network counter
    RKMarker             marker;                                               // Volume / sweep / radial marker
    RKProductList        productList;                                      // List of calculated moments
    uint16_t             configIndex;                                          // Operating configuration index
    uint16_t             configSubIndex;                                       // Operating configuration sub-index
    uint16_t             gateCount;                                            // Gate count of the ray
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
    uint8_t              fftOrder;                                             // The order of FFT (2^N) = plan index of FFTModule
    uint8_t              reserved1;                                            //
    uint8_t              reserved2;                                            //
    uint8_t              reserved3;                                            //
} RKRayHeaderF1;

//
// File header of raw I/Q data
//
typedef union rk_file_header_f5 {
    struct {                                                                   // Up to format (buildNo) 5
        RKName               preface;                                          //
        uint32_t             buildNo;                                          //
        RKRadarDesc          desc;                                             //
        RKConfigF1           config;                                           //
        RKRawDataType        dataType;                                         //
    };                                                                         //
    RKByte               bytes[4096];                                          //
} RKFileHeaderF5;

typedef union rk_file_header_f6_f7 {
    struct {
        RKName               preface;                                          // 128 B
        uint32_t             version;                                          //   4 B
        RKRawDataType        dataType;                                         //   1 B
        uint8_t              reserved[123];                                    // 123 B = 256 B
        RKRadarDesc          desc;                                             //         1072 B
        RKConfigF5           config;                                           //         1600 B
    };                                                                         //
    RKByte               bytes[4096];                                          //
} RKFileHeaderF6;

//
// Pulse header
//
typedef struct rk_pulse_header_f1 {
    RKIdentifier        i;                                                     // Identity counter
    RKIdentifier        n;                                                     // Network counter, may be useful to indicate packet loss
    uint64_t            t;                                                     // A clean clock-related tic count
    RKPulseStatus       s;                                                     // Status flag
    uint32_t            capacity;                                              // Allocated capacity
    uint32_t            gateCount;                                             // Number of range gates
    uint32_t            downSampledGateCount;                                  // Number of range gates after down-sampling
    RKMarker            marker;                                                // Position Marker
    uint32_t            pulseWidthSampleCount;                                 // Pulsewidth
    struct timeval      time;                                                  // UNIX time in seconds since 1970/1/1 12:00am
    double              timeDouble;                                            // Time in double representation
    RKFourByte          rawAzimuth;                                            // Raw azimuth reading, which may take up to 4 bytes
    RKFourByte          rawElevation;                                          // Raw elevation reading, which may take up to 4 bytes
    uint16_t            configIndex;                                           // Operating configuration index
    uint16_t            configSubIndex;                                        // Operating configuration sub-index
    uint16_t            azimuthBinIndex;                                       // Ray bin
    float               gateSizeMeters;                                        // Size of range gates
    float               elevationDegrees;                                      // Elevation in degrees
    float               azimuthDegrees;                                        // Azimuth in degrees
    float               elevationVelocityDegreesPerSecond;                     // Velocity of elevation in degrees / second
    float               azimuthVelocityDegreesPerSecond;                       // Velocity of azimuth in degrees / second
} RKPulseHeaderF1;

//
// Pulse
//
// - RKPulse struct is padded to a SIMD alignment
//
typedef struct rk_pulse_f1 {
    union {
        struct {
            RKPulseHeaderF1      header;                                       //
            RKPulseParameters    parameters;                                   //
        };                                                                     //
        RKByte               headerBytes[RKPulseHeaderPaddedSize];             //
    };                                                                         //
    RKByte               data[0];                                              //
} RKPulseF1;

typedef struct rk_waveform_f1 {
    RKName               name;                                                 // Waveform name in plain string
    RKWaveformType       type;                                                 // Various type of waveforms
    uint8_t              count;                                                // Number of groups
    int                  depth;                                                // Maximum number of samples
    double               fc;                                                   // Carrier frequency (Hz)
    double               fs;                                                   // Sampling frequency (Hz)
    uint8_t              filterCounts[RKMaximumWaveformCount];                 // Number of filters (see filterAnchors)
    RKFilterAnchorGroup  filterAnchors[RKMaximumWaveformCount];                // Filter anchors of each sub-waveform for de-multiplexing
    RKComplex            *samples[RKMaximumWaveformCount];                     // Samples up to amplitude of 1.0 (for compression, non-distorted)
    RKInt16C             *iSamples[RKMaximumWaveformCount];                    // 16-bit full-scale equivalence of the waveforms (pre-distorted)
} RKWaveformF1;

#pragma pack(pop)

#endif
