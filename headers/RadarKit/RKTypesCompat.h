//
//  RKTypes.h
//  RadarKit
//
//  These types are being phased out. Don't use them in new codes.
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
// A running configuration buffer (version 1, see below for updated version)
//
typedef struct rk_config_v1 {
    RKIdentifier         i;                                                    // Identity counter
    float                sweepElevation;                                       // Sweep elevation angle (degrees)
    float                sweepAzimuth;                                         // Sweep azimuth angle (degrees)
    RKMarker             startMarker;                                          // Marker of the latest start ray
    uint8_t              filterCount;                                          // Number of filters
    RKFilterAnchor       filterAnchors[RKMaximumFilterCount];                  // Filter anchors at ray level
    RKFloat              prt[RKMaximumFilterCount];                            // Pulse repetition time (s)
    RKFloat              pw[RKMaximumFilterCount];                             // Pulse width (s)
    uint32_t             pulseGateCount;                                       // Number of range gates
    RKFloat              pulseGateSize;                                        // Size of range gate (m)
    uint32_t             pulseRingFilterGateCount;                             // Number of range gates to apply ring filter
    uint32_t             waveformId[RKMaximumFilterCount];                     // Transmit waveform
    RKFloat              noise[2];                                             // Noise floor (ADU)
    RKFloat              systemZCal[2];                                        // System-wide Z calibration (dB)
    RKFloat              systemDCal;                                           // System-wide ZDR calibration (dB)
    RKFloat              systemPCal;                                           // System-wide phase calibration (rad)
    RKFloat              ZCal[RKMaximumFilterCount][2];                        // Waveform Z calibration (dB)
    RKFloat              DCal[RKMaximumFilterCount];                           // Waveform ZDR calibration (dB)
    RKFloat              PCal[RKMaximumFilterCount];                           // Waveform phase calibration (rad)
    RKFloat              SNRThreshold;                                         // Censor SNR (dB)
    RKFloat              SQIThreshold;                                         // Censor SQI
    RKName               waveform;                                             // Waveform name
    char                 vcpDefinition[RKMaximumCommandLength];                // Volume coverage pattern
} RKConfigV1;

//
// Ray header
//
typedef struct rk_ray_header_v1 {
    uint32_t             capacity;                                             // Capacity
    RKRayStatus          s;                                                    // Ray status
    RKIdentifier         i;                                                    // Ray indentity
    RKIdentifier         n;                                                    // Ray network counter
    RKMarker             marker;                                               // Volume / sweep / radial marker
    RKBaseProductList    baseProductList;                                      // List of calculated moments
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
} RKRayHeaderV1;

//
// File header of raw I/Q data
//
typedef union rk_file_header_v1 {
    struct {                                                                   // Up to version (buildNo) 5
        RKName               preface;                                          //
        uint32_t             buildNo;                                          //
        RKRadarDesc          desc;                                             //
        RKConfigV1           config;                                           //
        RKRawDataType        dataType;                                         //
    };                                                                         //
    RKByte               bytes[4096];                                          //
} RKFileHeaderV1;

#pragma pack(pop)

#endif
