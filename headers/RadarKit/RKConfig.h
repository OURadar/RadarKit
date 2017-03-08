//
//  RKConfig.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 1/16/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_RKConfig__
#define __RadarKit_RKConfig__

#include <RadarKit/RKFoundation.h>

void RKConfigAdvanceEllipsis(RKConfig *configs, uint32_t *configIndex, uint32_t configBufferDepth, ...);
void RKConfigAdvance(RKConfig *configs, uint32_t *configIndex, uint32_t configBufferDepth, va_list arg);

void RKConfigAddSweep();

#endif /* __RadarKit_RKConfig__ */
