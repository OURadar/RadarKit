//
//  RKProductFile.h
//  RadarKit
//
//  Created by Boonleng Cheong on 6/24/18.
//  Copyright © Boonleng Cheong. All rights reserved.
//

#ifndef __RadarKit_ProductFile__
#define __RadarKit_ProductFile__

#include <RadarKit/RKFoundation.h>
#include <RadarKit/RKProduct.h>
#include <archive_entry.h>
#include <archive.h>
#include <netcdf.h>

#if defined(_HAS_NETCDF_MEM_H)
#include <netcdf_mem.h>
#endif

#define W2_MISSING_DATA       -99900.0
#define W2_RANGE_FOLDED       -99901.0

#if defined (COMPRESSED_NETCDF)
#define NC_MODE  NC_NETCDF4
#else
#define NC_MODE  NC_CLOBBER
#endif

void RKProductDimensionsFromFile(const char *, uint32_t *rayCount, uint32_t *gateCount);

int RKProductFileWriterNC(RKProduct *, const char *);
// RKProduct *RKProductFileReaderNC(const char *, const bool);
// void RKProductReadWDSFileIntoBuffer(RKProduct *buffer, const char *, const bool);

RKProductCollection *RKProductCollectionInit(const int count, const uint32_t rayCount, const uint32_t gateCount);
RKProductCollection *RKProductCollectionInitFromSingles(RKProductCollection *singles[], const uint32_t count);
void RKProductCollectionFree(RKProductCollection *);

RKProductCollection *RKProductCollectionInitWithFilename(const char *);

// int RKProductCollectionStandardizeForWDSSII(RKProductCollection *);
int RKProductCollectionStandardizeForCFRadial(RKProductCollection *);
int RKProductCollectionFileWriterCF(RKProductCollection *, const char *, const RKWriterOption);

#endif
