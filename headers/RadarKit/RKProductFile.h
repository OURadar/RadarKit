//
//  RKProductFile.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 6/24/18.
//  Copyright © Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_ProductFile__
#define __RadarKit_ProductFile__

#include <RadarKit/RKFoundation.h>
#include <netcdf.h>

int RKProductFileWriterNC(RKProduct *, char *);
RKProduct *RKProductFileReaderNC(const char *);

#endif
