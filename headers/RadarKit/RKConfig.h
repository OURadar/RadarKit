//
//  RKConfig.h
//  RadarKit
//
//  Created by Boonleng Cheong on 1/16/17.
//  Copyright © 2017-2021 Boonleng Cheong. All rights reserved.
//

#ifndef __RadarKit_Config__
#define __RadarKit_Config__

#include <RadarKit/RKFoundation.h>

size_t RKConfigBufferAlloc(RKConfig **, const uint32_t configBufferDepth);
void RKConfigBufferFree(RKConfig *);

void RKConfigAdvanceEllipsis(RKConfig *configs, uint32_t *configIndex, uint32_t configBufferDepth, ...);
void RKConfigAdvance(RKConfig *configs, uint32_t *configIndex, uint32_t configBufferDepth, va_list args);

//void RKConfigAddSweep();
RKConfig *RKConfigWithId(RKConfig *configs, uint32_t configBufferDepth, uint64_t id);

#endif /* __RadarKit_RKConfig__ */
