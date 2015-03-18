//
//  RadarKit.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/17/15.
//
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
#include <RadarKit/RKServer.h>
#include <RadarKit/RKClient.h>
#include <RadarKit/RKPulseCompression.h>

#endif /* defined(__RadarKit__) */
