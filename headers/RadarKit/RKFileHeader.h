//
//  RKFileHeader.h
//  RadarKit
//
//  Created by Boonleng Cheong on 5/13/2023.
//  Copyright © Boonleng Cheong. All rights reserved.
//

#ifndef __RadarKit_File_Header__
#define __RadarKit_File_Header__

#include <RadarKit/RKFoundation.h>
#include <RadarKit/RKWaveform.h>

RKFileHeader *RKFileHeaderAlloc(void);
void RKFileHeaderFree(RKFileHeader *);

RKFileHeader *RKFileHeaderRead(FILE *);

void RKFileHeaderSummary(RKFileHeader *);

#endif /* defined(__RadarKit_File_Header__) */
