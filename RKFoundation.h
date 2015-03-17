//
//  RKFoundation.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/17/15.
//
//

#ifndef RadarKit_RKFoundation_h
#define RadarKit_RKFoundation_h

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/*!
 @definedblock Memory Blocks
 @abstract Defines the number of slots and gates of each pulse of the RKRadar structure
 @define RKBuffer0SlotCount The number of slots for level-0 pulse storage in the host memory
 @define RKBuffer1SlotCount The number of slots for level-1 pulse storage in the host memory
 @define RKBuffer2SlotCount The number of slots for level-2 pulse storage in the host memory
 @define RKGateCount The maximum number of gates allocated for each pulse
 @define RKSIMDAlignSize The minimum alignment size. AVX requires 256 bits = 32 bytes. AVX-512 is on the horizon now.
 */
#define RKBuffer0SlotCount    4000
#define RKBuffer1SlotCount    4000
#define RKBuffer2SlotCount    4000
#define RKGateCount           8192
#define RKSIMDAlignSize         64

/*! @/definedblock */

#endif
