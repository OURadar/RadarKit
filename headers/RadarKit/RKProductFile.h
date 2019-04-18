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
#include <RadarKit/RKProduct.h>
#include <netcdf.h>

#define W2_MISSING_DATA       -99900.0
#define W2_RANGE_FOLDED       -99901.0

#if defined (COMPRESSED_NETCDF)
#define NC_MODE  NC_NETCDF4
#else
#define NC_MODE  NC_CLOBBER
#endif

void RKProductDimensionsFromFile(const char *, uint32_t *rayCount, uint32_t *gateCount);

int RKProductFileWriterNC(RKProduct *, const char *);
RKProduct *RKProductFileReaderNC(const char *, const bool);
void RKProductReadFileIntoBuffer(RKProduct *buffer, const char *, const bool);

RKProductCollection *RKProductCollectionInitWithFilename(const char *);
void RKProductCollectionFree(RKProductCollection *);

#endif
