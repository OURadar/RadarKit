//
//  RadarKit.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/17/15.
//

//
// Updates
//
//  █
//
//  1.2.8b - 4/31/2018
//         - New stream types: RKSweep, RKSweepHeader and RKSweepRay
//         - Fixed buffer ID initialization
//         - Fixed a memory leak in RKHostMonitor
//         - Added RKSoftRestart() to restart DSP-related engines only
//         - Added RKSystemInspector() for gathering system status
//         - Migrated RKFileMonitor() to RKSimpleEngine design
//         - Added RKSimpleEngine for simple sus-systems
//
//  1.2.7  - 3/31/2018
//         - Added RKSweepRead()
//         - Added waveform sensitivity gain
//         - Added calibration adjustment with senstivity gain
//         - Added calibration adjustment with ADC sampling frequency
//         - Unity noise gain enforced for compression filters
//         - Carrier frequency is now in RKWaveform
//         - Added setSystemLevel() for various configurations
//         - Added ring filter engine
//         - Added waveform loading in RKTestTransceiver
//
//  1.2.6  - 2/27/2018
//         - Added RKHostMonitor for ICMP echo reqeust
//         - Default host is Google's 8.8.8.8
//         - Multiple-range calibration based filter anchors
//         - All waveforms are required to have unity noise gain
//         - Sampling frequency change is now in RKWaveform
//         - Senstivity calculation based on waveform
//         - Improved CPU core count
//         - IP address is logged alongside the command
//         - Improved efficiency of RKTestTransceiver
//
//  1.2.5  - 1/8/2018
//         - Added RKClearControls(), RKConcludeControls()
//         - Send updated controls
//
//  1.2.4  - Improved efficiency of RKPulseCompression
//         - Reordered RKTest modules
//
//  1.2.3  - 12/7/2017
//         - Default waveform and pedestal task for RKTestTransceiver
//         - RKTestPulseCompression() is now self contained
//         - Consolidated many RKTest modules
//         - GPS reading has been moved to RKTestHealthRelay
//         - Added handleRadarTgz.sh for LDM server
//         - Added a MATLAB ACF & CCF calculations for validation
//         - Added ring worker for FIR / IIR buffer
//         - Completed multilag estimator
//         - Status enum expanded
//         - All filters are now normlized to have unity noise gain
//         - Added LFM generation to RKWaveform
//
//  1.2.2  - Boolean value parsing in preference
//         - Waveform generation with fc
//
//  1.2.1  - Improved stream reset mechanism
//         - Multiple filters within a filter group
//
//  1.2    - Radar display stream relay with internal consolidation
//         - RadarKit system engine status
//         - On-demand raw data recording
//         - General bug fixes
//
//  1.1.1  - Command relay
//         - RKFileManager
//         - RKHealthEngine and RKHealthLogger detached
//         - Incorporated NetCDF-4
//         - General bug fixes
//
//  1.1    - Optmized sequence for frequency hop
//         - Raw data I/Q recording
//         - Health logger
//         - Reduced memory footprint
//
//  1.0    - First working version
//

#ifndef __RadarKit__
#define __RadarKit__

/*!
 @framework RadarKit Framework Reference
 @author Boon Leng Cheong
 @abstract
 @discussion The RadarKit Framework provides the APIs and support for fundamental
 radar time-series data processing. It defines the base structure for base level
 raw I/Q data, 16-bit straight from the analog-to-digital converters, fundamental
 processing such as auto-correlation calculations, pulse compressions, FIR (finite
 impulse response) and IIR (infinite impulse response) filtering, window functions,
 data transportation across network, etc.
 @frameworkuid RK001
 
 */

#include <RadarKit/RKRadar.h>
#include <RadarKit/RKTest.h>
#include <RadarKit/RKCommandCenter.h>
#include <RadarKit/RKPedestalPedzy.h>
#include <RadarKit/RKHealthRelayTweeta.h>
#include <RadarKit/RKRadarRelay.h>
#include <RadarKit/RKHostMonitor.h>

#endif /* defined(__RadarKit__) */
