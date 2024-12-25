//
//  RKProductFile.c
//  RadarKit
//
//  Created by Boonleng Cheong on 6/24/18.
//  Copyright © Boonleng Cheong. All rights reserved.
//

#include <RadarKit/RKProductFile.h>

#if RKFloat == float
#define rk_nc_get_var_float   nc_get_var_float
#else
#define rk_nc_get_var_float   nc_get_var_double
#endif

#if !defined (float32_t)
typedef float float32_t;
#endif

#define FILL_VALUE  -32768

static int getGlobalTextAttribute(char *dst, const char *name, const int ncid) {
    int r;
    size_t n = 0;
    r = nc_get_att_text(ncid, NC_GLOBAL, name, dst);
    if (r != NC_NOERR) {
        return r;
    }
    nc_inq_attlen(ncid, NC_GLOBAL, name, &n);
    dst[n] = 0;
    return NC_NOERR;
}

static int put_global_text_att(const int ncid, const char *att, const char *text) {
    return nc_put_att_text(ncid, NC_GLOBAL, att, strlen(text), text);
}

static int get_global_float_att(const int ncid, const nc_type type, const char *att, void *dest) {
    if (type == NC_FLOAT) {
        return nc_get_att_float(ncid, NC_GLOBAL, att, (float *)dest);
    }
    return nc_get_att_double(ncid, NC_GLOBAL, att, (double *)dest);
}

static int put_variable_text_att(const int ncid, const int varid, const char *att, const char *text) {
    return nc_put_att_text(ncid, varid, att, strlen(text), text);
}

// static int put_variable_string_att(const int ncid, const int varid, const char *att, const char *value) {
//     return nc_put_att_string(ncid, varid, att, 1, &value);
// }

static void rkfloat_to_f32(const float32_t *dst, const RKFloat *src, const size_t count) {
    float32_t *y = (float32_t *)dst;
    RKFloat *x = (RKFloat *)src;
    for (int i = 0; i < count; i++) {
        *y++ = (float32_t)*x++;
    }
}

static void rkfloat_to_i16_masked_inv_scale_offset(const int16_t *dst, const RKFloat *src, const float scale, const float offset, const size_t count) {
    int16_t *y = (int16_t *)dst;
    RKFloat *x = (RKFloat *)src;
    for (float32_t *e = x + count; x < e; x++) {
        *y++ = isfinite(*x) ? (int16_t)((*x - offset) / scale) : FILL_VALUE;
    }
}

static void rkfloat_to_i16_masked_inv_scale(const int16_t *dst, const RKFloat *src, const float scale, const size_t count) {
    int16_t *y = (int16_t *)dst;
    RKFloat *x = (RKFloat *)src;
    for (float32_t *e = x + count; x < e; x++) {
        *y++ = isfinite(*x) ? (int16_t)(*x / scale) : FILL_VALUE;
    }
}

static void rkfloat_to_i16_inv_scale(const int16_t *dst, const RKFloat *src, const float scale, const size_t count) {
    int16_t *y = (int16_t *)dst;
    RKFloat *x = (RKFloat *)src;
    for (int i = 0; i < count; i++) {
        *y++ = (int16_t)(*x++ / scale);
    }
}

static void rkfloat_to_u16_inv_scale(const uint16_t *dst, const RKFloat *src, const float scale, const size_t count) {
    uint16_t *y = (uint16_t *)dst;
    RKFloat *x = (RKFloat *)src;
    for (int i = 0; i < count; i++) {
        *y++ = (uint16_t)(*x++ / scale);
    }
}

// static void rkfloat_scale(const RKFloat *dst, const RKFloat *src, const float scale, const size_t count) {
//     RKFloat *y = (RKFloat *)dst;
//     RKFloat *x = (RKFloat *)src;
//     for (int i = 0; i < count; i++) {
//         *y++ = *x++ * scale;
//     }
// }

static void u16_to_rkfloat_scale(const RKFloat *dst, const uint16_t *src, const float scale, const size_t count) {
    RKFloat *y = (RKFloat *)dst;
    uint16_t *x = (uint16_t *)src;
    for (int i = 0; i < count; i++) {
        *y++ = (RKFloat)(*x++ * scale);
    }
}

static void u16_to_rkfloat_scale_offset(const RKFloat *dst, const uint16_t *src, const float scale, const float offset, const size_t count) {
    RKFloat *y = (RKFloat *)dst;
    uint16_t *x = (uint16_t *)src;
    for (int i = 0; i < count; i++) {
        *y++ = (RKFloat)(*x++ * scale + offset);
    }
}

static void i16_masked_to_rkfloat_scale(const RKFloat *dst, const int16_t *src, const float scale, const size_t count) {
    RKFloat *y = (RKFloat *)dst;
    int16_t *x = (int16_t *)src;
    for (int i = 0; i < count; i++) {
        *y++ = *x == FILL_VALUE ? NAN : (RKFloat)(*x * scale);
        x++;
    }
    // x = (int16_t *)src;
    // y = (RKFloat *)dst;
    // const int idx = 1330 + 685;
    // printf("selected value: %d -> %.4f\n", *(x + idx), *(y + idx));
}

static void i16_to_rkfloat_scale(const RKFloat *dst, const int16_t *src, const float scale, const size_t count) {
    RKFloat *y = (RKFloat *)dst;
    int16_t *x = (int16_t *)src;
    for (int i = 0; i < count; i++) {
        *y++ = (RKFloat)(*x++ * scale);
    }
}

static void i16_to_rkfloat_scale_offset(const RKFloat *dst, const int16_t *src, const float scale, const float offset, const size_t count) {
    RKFloat *y = (RKFloat *)dst;
    int16_t *x = (int16_t *)src;
    for (int i = 0; i < count; i++) {
        *y++ = (RKFloat)(*x++ * scale + offset);
    }
}

static void i16_masked_to_rkfloat_scale_offset(const RKFloat *dst, const int16_t *src, const float scale, const float offset, const size_t count) {
    RKFloat *y = (RKFloat *)dst;
    int16_t *x = (int16_t *)src;
    for (int i = 0; i < count; i++) {
        *y++ = *x == FILL_VALUE ? NAN : (RKFloat)(*x * scale + offset);
        x++;
    }
}

int RKProductFileWriterNC(RKProduct *product, const char *filename) {
    int j;
    int ncid;
    int dimensionIds[2];
    int variableIdAzimuth;
    int variableIdElevation;
    int variableIdBeamwidth;
    int variableIdGateWidth;
    int variableIdData;

    int tmpi;
    float tmpf;
    float *x;

    // Check for basic requirements
    if (strlen(product->desc.name) == 0) {
        RKLog("Error. Product name is empty.\n");
        return RKResultProductDescriptionNotSet;
    }
    if (product->header.rayCount == 0 || product->header.gateCount == 0) {
        RKLog("Error. Product dimensions are zero.\n");
        return RKResultProductDimensionsNotSet;
    }
    if (product->header.startTime == 0) {
        RKLog("Error. Product start time is zero.\n");
        return RKResultProductStartTimeNotSet;
    }
    if (product->header.gateSizeMeters == 0.0f) {
        RKLog("Error. Product gate size is zero.\n");
        return RKResultProductGateSizeNotSet;
    }

    // Open a file
    RKPreparePath(filename);
    if ((j = nc_create(filename, NC_MODE, &ncid)) > 0) {
        RKLog("Error. Unable to create %s   nc_create returned %d\n", filename, j);
        return RKResultFailedToOpenFileForProduct;
    }

    // Some global attributes
    const float zf = 0.0f;
    const float va = 0.25f * product->header.wavelength / product->header.prt[0];
    const nc_type floatType = sizeof(RKFloat) == sizeof(double) ? NC_DOUBLE : NC_FLOAT;

    // Local memory
    RKFloat *array1D = (RKFloat *)malloc(MAX(product->header.rayCount, product->header.gateCount) * sizeof(RKFloat));

    // Scan type
    if (product->header.isPPI) {
        nc_def_dim(ncid, "Azimuth", product->header.rayCount, &dimensionIds[0]);
    } else if (product->header.isRHI) {
        nc_def_dim(ncid, "Elevation", product->header.rayCount, &dimensionIds[0]);
    } else {
        nc_def_dim(ncid, "Beam", product->header.rayCount, &dimensionIds[0]);
    }
    nc_def_dim(ncid, "Gate", product->header.gateCount, &dimensionIds[1]);

    // Define variables
    nc_def_var(ncid, "Azimuth", NC_FLOAT, 1, dimensionIds, &variableIdAzimuth);
    nc_def_var(ncid, "Elevation", NC_FLOAT, 1, dimensionIds, &variableIdElevation);
    nc_def_var(ncid, "Beamwidth", NC_FLOAT, 1, dimensionIds, &variableIdBeamwidth);
    nc_def_var(ncid, "GateWidth", NC_FLOAT, 1, dimensionIds, &variableIdGateWidth);
    nc_def_var(ncid, product->desc.name, NC_FLOAT, 2, dimensionIds, &variableIdData);

    nc_put_att_text(ncid, variableIdAzimuth, "Units", 7, "Degrees");
    nc_put_att_text(ncid, variableIdElevation, "Units", 7, "Degrees");
    nc_put_att_text(ncid, variableIdBeamwidth, "Units", 7, "Degrees");
    nc_put_att_text(ncid, variableIdGateWidth, "Units", 6, "Meters");
    nc_put_att_text(ncid, variableIdData, "Units", strlen(product->desc.unit), product->desc.unit);

#if defined (COMPRESSED_NETCDF)

    RKLog("Warning. Using NetCDF compression.\n");
    nc_def_var_deflate(ncid, variableIdAzimuth, 1, 1, 3);
    nc_def_var_deflate(ncid, variableIdElevation, 1, 1, 3);
    nc_def_var_deflate(ncid, variableIdBeamwidth, 1, 1, 3);
    nc_def_var_deflate(ncid, variableIdGateWidth, 1, 1, 3);
    nc_def_var_deflate(ncid, variableIdData, 1, 1, 3);

#endif

    // Global attributes - some are WDSS-II required
    nc_put_att_text(ncid, NC_GLOBAL, "TypeName", strlen(product->desc.name), product->desc.name);
    nc_put_att_text(ncid, NC_GLOBAL, "DataType", 9, "RadialSet");
    if (product->header.isPPI) {
        nc_put_att_text(ncid, NC_GLOBAL, "ScanType", 3, "PPI");
    } else if (product->header.isRHI) {
        nc_put_att_text(ncid, NC_GLOBAL, "ScanType", 3, "RHI");
    } else {
        nc_put_att_text(ncid, NC_GLOBAL, "ScanType", 7, "Unknown");
    }
    // tmpf = product->header.latitude;
    // nc_put_att_float(ncid, NC_GLOBAL, "Latitude", NC_FLOAT, 1, &tmpf);
    nc_put_att_double(ncid, NC_GLOBAL, "Latitude", NC_DOUBLE, 1, &product->header.latitude);
    // tmpf = product->header.longitude;
    // nc_put_att_float(ncid, NC_GLOBAL, "Longitude", NC_FLOAT, 1, &tmpf);
    nc_put_att_double(ncid, NC_GLOBAL, "Longitude", NC_DOUBLE, 1, &product->header.longitude);
    nc_put_att_float(ncid, NC_GLOBAL, "Heading", NC_FLOAT, 1, &product->header.heading);
    nc_put_att_float(ncid, NC_GLOBAL, "Height", NC_FLOAT, 1, &product->header.radarHeight);
    long startTime = (long)product->header.startTime;
    nc_put_att_long(ncid, NC_GLOBAL, "Time", NC_LONG, 1, &startTime);
    nc_put_att_float(ncid, NC_GLOBAL, "FractionalTime", NC_FLOAT, 1, &zf);
    put_global_text_att(ncid, "attributes", "Wavelength Nyquist_Vel Unit radarName vcp ColorMap");
    put_global_text_att(ncid, "Wavelength-unit", "Meters");
    nc_put_att_float(ncid, NC_GLOBAL, "Wavelength-value", NC_FLOAT, 1, &product->header.wavelength);
    put_global_text_att(ncid, "Nyquist_Vel-unit", "MetersPerSecond");
    nc_put_att_float(ncid, NC_GLOBAL, "Nyquist_Vel-value", NC_FLOAT, 1, &va);
    put_global_text_att(ncid, "Unit-unit", "dimensionless");
    put_global_text_att(ncid, "Unit-value", product->desc.unit);
    put_global_text_att(ncid, "radarName-unit", "dimensionless");
    put_global_text_att(ncid, "radarName-value", product->header.radarName);
    put_global_text_att(ncid, "vcp-unit", "dimensionless");
    put_global_text_att(ncid, "vcp-value", "1");
    put_global_text_att(ncid, "ColorMap-unit", "dimensionless");
    put_global_text_att(ncid, "ColorMap-value", product->desc.colormap);

    // Other housekeeping attributes
    if (product->header.isPPI) {
        nc_put_att_float(ncid, NC_GLOBAL, "Elevation", NC_FLOAT, 1, &product->header.sweepElevation);
    } else {
        tmpf = W2_MISSING_DATA;
        nc_put_att_float(ncid, NC_GLOBAL, "Elevation", NC_FLOAT, 1, &tmpf);
    }
    put_global_text_att(ncid, "ElevationUnits", "Degrees");
    if (product->header.isRHI) {
        nc_put_att_float(ncid, NC_GLOBAL, "Azimuth", NC_FLOAT, 1, &product->header.sweepAzimuth);
    } else {
        tmpf = W2_MISSING_DATA;
        nc_put_att_float(ncid, NC_GLOBAL, "Azimuth", NC_FLOAT, 1, &tmpf);
    }
    put_global_text_att(ncid, "AzimuthUnits", "Degrees");
    nc_put_att_float(ncid, NC_GLOBAL, "GateSize", NC_FLOAT, 1, &product->header.gateSizeMeters);
    tmpf = 0.0f;
    nc_put_att_float(ncid, NC_GLOBAL, "RangeToFirstGate", NC_FLOAT, 1, &tmpf);
    put_global_text_att(ncid, "RangeToFirstGateUnits", "Meters");
    tmpf = W2_MISSING_DATA;
    nc_put_att_float(ncid, NC_GLOBAL, "MissingData", NC_FLOAT, 1, &tmpf);
    tmpf = W2_RANGE_FOLDED;
    nc_put_att_float(ncid, NC_GLOBAL, "RangeFolded", NC_FLOAT, 1, &tmpf);
    put_global_text_att(ncid, "RadarParameters", "PRF PulseWidth MaximumRange");
    put_global_text_att(ncid, "PRF-unit", "Hertz");
    tmpi = (int)(1.0f / product->header.prt[0]);
    nc_put_att_int(ncid, NC_GLOBAL, "PRF-value", NC_INT, 1, &tmpi);
    put_global_text_att(ncid, "PulseWidth-unit", "MicroSeconds");
    tmpf = (float)product->header.pw[0] * 1000.0f;
    nc_put_att_float(ncid, NC_GLOBAL, "PulseWidth-value", NC_FLOAT, 1, &tmpf);
    put_global_text_att(ncid, "MaximumRange-unit", "KiloMeters");
    tmpf = 1.0e-3f * product->header.gateSizeMeters * product->header.gateCount;
    nc_put_att_float(ncid, NC_GLOBAL, "MaximumRange-value", NC_FLOAT, 1, &tmpf);
    put_global_text_att(ncid, "ProcessParameters", "Noise Calibration Censoring");
    nc_put_att_float(ncid, NC_GLOBAL, "NoiseH-ADU", floatType, 1, &product->header.noise[0]);
    nc_put_att_float(ncid, NC_GLOBAL, "NoiseV-ADU", floatType, 1, &product->header.noise[1]);
    nc_put_att_float(ncid, NC_GLOBAL, "SystemZCalH-dB", floatType, 1, &product->header.systemZCal[0]);
    nc_put_att_float(ncid, NC_GLOBAL, "SystemZCalV-dB", floatType, 1, &product->header.systemZCal[1]);
    nc_put_att_float(ncid, NC_GLOBAL, "SystemDCal-dB", floatType, 1, &product->header.systemDCal);
    nc_put_att_float(ncid, NC_GLOBAL, "SystemPCal-Radians", floatType, 1, &product->header.systemPCal);
    nc_put_att_float(ncid, NC_GLOBAL, "ZCalH1-dB", floatType, 1, &product->header.ZCal[0][0]);
    nc_put_att_float(ncid, NC_GLOBAL, "ZCalV1-dB", floatType, 1, &product->header.ZCal[1][0]);
    nc_put_att_float(ncid, NC_GLOBAL, "ZCalH2-dB", floatType, 1, &product->header.ZCal[0][1]);
    nc_put_att_float(ncid, NC_GLOBAL, "ZCalV2-dB", floatType, 1, &product->header.ZCal[1][1]);
    nc_put_att_float(ncid, NC_GLOBAL, "DCal1-dB", floatType, 1, &product->header.DCal[0]);
    nc_put_att_float(ncid, NC_GLOBAL, "DCal2-dB", floatType, 1, &product->header.DCal[1]);
    nc_put_att_float(ncid, NC_GLOBAL, "PCal1-Radians", floatType, 1, &product->header.PCal[0]);
    nc_put_att_float(ncid, NC_GLOBAL, "PCal2-Radians", floatType, 1, &product->header.PCal[1]);
    nc_put_att_float(ncid, NC_GLOBAL, "SNRThreshold-dB", floatType, 1, &product->header.SNRThreshold);
    nc_put_att_float(ncid, NC_GLOBAL, "SQIThreshold-dB", floatType, 1, &product->header.SQIThreshold);
    put_global_text_att(ncid, "RadarKit-VCP-Definition", product->header.vcpDefinition);
    put_global_text_att(ncid, "Waveform", product->header.waveformName);
    put_global_text_att(ncid, "CreatedBy", "RadarKit v" __RKVersion__);
    put_global_text_att(ncid, "ContactInformation", "https://arrc.ou.edu");

    // NetCDF definition ends here
    nc_enddef(ncid);

    // Data
#if RKFloat == float

    nc_put_var_float(ncid, variableIdAzimuth, product->startAzimuth);
    nc_put_var_float(ncid, variableIdElevation, product->startElevation);
    if (product->header.isPPI) {
        for (j = 0; j < product->header.rayCount; j++) {
            array1D[j] = RKUMinDiff(product->endAzimuth[j], product->startAzimuth[j]);
        }
    } else if (product->header.isRHI) {
        for (j = 0; j < product->header.rayCount; j++) {
            array1D[j] = RKUMinDiff(product->endElevation[j], product->startElevation[j]);
        }
    }
    nc_put_var_float(ncid, variableIdBeamwidth, array1D);
    for (j = 0; j < product->header.rayCount; j++) {
        array1D[j] = product->header.gateSizeMeters;
    }
    nc_put_var_float(ncid, variableIdGateWidth, array1D);

    x = product->data;
    for (j = 0; j < product->header.rayCount * product->header.gateCount; j++) {
        if (!isfinite(*x)) {
            *x = W2_MISSING_DATA;
        }
        x++;
    }
    nc_put_var_float(ncid, variableIdData, product->data);

#elif RKloat == double

    nc_put_var_double(ncid, variableIdAzimuth, product->startAzimuth);
    nc_put_var_double(ncid, variableIdElevation, product->startElevation);
    if (product->header.isPPI) {
        for (j = 0; j < product->header.rayCount; j++) {
            array1D[j] = RKUMinDiff(product->endAzimuth[j], product->startAzimuth[j]);
        }
    } else if (product->header.isRHI) {
        for (j = 0; j < product->header.rayCount; j++) {
            array1D[j] = RKUMinDiff(product->endElevation[j], product->startElevation[j]);
        }
    }
    nc_put_var_double(ncid, variableIdBeamwidth, array1D);
    for (j = 0; j < product->header.rayCount; j++) {
        array1D[j] = product->header.gateSizeMeters;
    }
    nc_put_var_double(ncid, variableIdGateWidth, array1D);

    x = product->data;
    for (j = 0; j < product->header.rayCount * product->header.gateCount; j++) {
        if (!isfinite(*x)) {
            *x = W2_MISSING_DATA;
        }
        x++;
    }
    nc_put_var_float(ncid, variableIdData, product->data);

#else

    RKLog("Error. Unexpected data float type.\n");

#endif

    ncclose(ncid);

    free(array1D);

    return RKResultSuccess;
}

void RKProductDimensionsFromFile(const char *filename, uint32_t *rayCount, uint32_t *gateCount) {
    int r;
    int ncid, tmpId;

    MAKE_FUNCTION_NAME(name);

    // Early return if the file does not exist
    if (!RKFilenameExists(filename)) {
        RKLog("Error. File %s does not exist.\n", filename);
        return;
    }

    // Read the first file
    if ((r = nc_open(filename, NC_NOWRITE, &ncid)) > 0) {
        RKLog("%s Error opening file %s (%s)\n", name, filename, nc_strerror(r));
        return;
    }

    // Dimensions
    size_t localRayCount = 0;
    size_t localGateCount = 0;
    if ((r = nc_inq_dimid(ncid, "Azimuth", &tmpId)) != NC_NOERR) {
        r = nc_inq_dimid(ncid, "azimuth", &tmpId);
    }
    if (r != NC_NOERR) {
        if ((r = nc_inq_dimid(ncid, "Beam", &tmpId)) != NC_NOERR) {
            r = nc_inq_dimid(ncid, "beam", &tmpId);
        }
    }
    if (r == NC_NOERR) {
        nc_inq_dimlen(ncid, tmpId, &localRayCount);
    } else {
        nc_close(ncid);
        RKLog("Warning. Early return (rayCount)\n");
        return;
    }
    if ((r = nc_inq_dimid(ncid, "Gate", &tmpId)) != NC_NOERR)
    r = nc_inq_dimid(ncid, "gate", &tmpId);
    if (r == NC_NOERR) {
        nc_inq_dimlen(ncid, tmpId, &localGateCount);
    } else {
        RKLog("Warning. Early return (gateCount)\n");
        nc_close(ncid);
        return;
    }
    if (localGateCount > RKMaximumGateCount) {
        RKLog("Info. gateCount = %d capped to %d\n", localGateCount, RKMaximumGateCount);
        localGateCount = RKMaximumGateCount;
    }
    *gateCount = (uint32_t)localGateCount;
    *rayCount = (uint32_t)localRayCount;
    nc_close(ncid);
}

RKProductCollection *RKProductCollectionInit(const int count, const uint32_t rayCount, const uint32_t gateCount) {
    RKProductCollection *collection = (RKProductCollection *)malloc(sizeof(RKProductCollection));
    if (collection == NULL) {
        fprintf(stderr, "RKProductCollectionInit() Failed to allocate memory.\n");
        return NULL;
    }
    collection->count = count;
    RKProductBufferAlloc(&collection->products, collection->count, rayCount, gateCount);
    return collection;
}

RKProductCollection *RKProductCollectionInitFromSingles(RKProductCollection *singles[], const uint32_t count) {
    const uint32_t rayCount = singles[0]->products[0].header.rayCount;
    const uint32_t gateCount = singles[0]->products[0].header.gateCount;
    RKLog("Combining %d singles ...   %s   %s\n", count,
        RKVariableInString("rayCount", &rayCount, RKValueTypeUInt32),
        RKVariableInString("gateCount", &gateCount, RKValueTypeUInt32));
    RKProductCollection *collection = RKProductCollectionInit(count, rayCount, gateCount);
    for (int k = 0; k < collection->count; k++) {
        RKProduct *src = singles[k]->products;
        RKProduct *dst = &collection->products[k];
        dst->i = src->i;
        dst->pid = src->pid;
        dst->desc = src->desc;
        dst->flag = src->flag;
        dst->header = src->header;
        memcpy(dst->startAzimuth, src->startAzimuth, rayCount * sizeof(RKFloat));
        memcpy(dst->endAzimuth, src->endAzimuth, rayCount * sizeof(RKFloat));
        memcpy(dst->startElevation, src->startElevation, rayCount * sizeof(RKFloat));
        memcpy(dst->endElevation, src->endElevation, rayCount * sizeof(RKFloat));
        memcpy(dst->startTime, src->startTime, rayCount * sizeof(double));
        memcpy(dst->endTime, src->endTime, rayCount * sizeof(double));
        memcpy(dst->data, src->data, rayCount * gateCount * sizeof(RKFloat));
    }
    return collection;
}

void RKProductCollectionFree(RKProductCollection *collection) {
    RKProductBufferFree(collection->products, collection->count);
    free(collection);
}

// NOTE: This is not quite right, don't use just yet
RKProductCollection *RKProductCollectionExpand(RKProductCollection *collection, const int count) {
    RKProductCollection *newCollection = (RKProductCollection *)malloc(sizeof(RKProductCollection));
    if (newCollection == NULL) {
        fprintf(stderr, "RKProductCollectionExpand() Failed to allocate memory.\n");
        return NULL;
    }
    newCollection->count = collection->count + count;
    RKProductBufferAlloc(&newCollection->products, newCollection->count, collection->products[0].header.rayCount, collection->products[0].header.gateCount);
    for (int k = 0; k < collection->count; k++) {
        memcpy(&newCollection->products[k], &collection->products[k], sizeof(RKProduct));
    }
    return newCollection;
}


static RKProductCollection *read_ncid_as_wds(const int ncid) {
    MAKE_FUNCTION_NAME(myname);
    int r;
    int tmpId;
    size_t rayCount = 0;
    size_t gateCount = 0;
    RKName stringValue;
    size_t size;

    // Some constants
    const nc_type floatType = sizeof(RKFloat) == sizeof(double) ? NC_DOUBLE : NC_FLOAT;
    const RKFloat folded = W2_RANGE_FOLDED;
    const RKFloat missing = W2_MISSING_DATA;

    // Get the filename
    char filename[RKMaximumPathLength];
    if ((r = nc_inq_path(ncid, &size, filename)) != NC_NOERR) {
        RKLog("Error. Failed to get the file name.\n");
        return NULL;
    }
    filename[size] = '\0';

    // Dimensions (for double check later)
    if ((r = nc_inq_dimid(ncid, "Azimuth", &tmpId)) != NC_NOERR) {
        r = nc_inq_dimid(ncid, "azimuth", &tmpId);
    }
    if (r != NC_NOERR) {
        if ((r = nc_inq_dimid(ncid, "Beam", &tmpId)) != NC_NOERR) {
            r = nc_inq_dimid(ncid, "beam", &tmpId);
        }
    }
    if (r == NC_NOERR) {
        nc_inq_dimlen(ncid, tmpId, &rayCount);
    } else {
        RKLog("Warning. Early return (rayCount)\n");
        return NULL;
    }
    if ((r = nc_inq_dimid(ncid, "Gate", &tmpId)) != NC_NOERR)
    r = nc_inq_dimid(ncid, "gate", &tmpId);
    if (r == NC_NOERR) {
        nc_inq_dimlen(ncid, tmpId, &gateCount);
    } else {
        RKLog("Warning. Early return (gateCount)\n");
        return NULL;
    }
    if (gateCount > RKMaximumGateCount) {
        RKLog("Info. gateCount = %d capped to %d\n", gateCount, RKMaximumGateCount);
        gateCount = RKMaximumGateCount;
    }

    // WDS is always a collection of one product. Will gather them and make a new collection later.
    RKProductCollection *collection = RKProductCollectionInit(1, (uint32_t)rayCount, (uint32_t)gateCount);
    RKProduct *product = &collection->products[0];

    RKGetSymbolFromFilename(filename, product->desc.symbol);

    // Global attributes
    getGlobalTextAttribute(product->desc.name, "TypeName", ncid);
    getGlobalTextAttribute(product->desc.unit, "Unit-value", ncid);
    getGlobalTextAttribute(product->desc.colormap, "ColorMap-value", ncid);
    getGlobalTextAttribute(stringValue, "ScanType", ncid);
    if (!strcmp(stringValue, "PPI")) {
        product->header.isPPI = true;
    } else if (!strcmp(stringValue, "RHI")) {
        product->header.isRHI = true;
    }
    getGlobalTextAttribute(product->header.radarName, "radarName-value", ncid);
    getGlobalTextAttribute(product->header.waveformName, "Waveform", ncid);
    getGlobalTextAttribute(product->header.vcpDefinition, "RadarKit-VCP-Definition", ncid);

    int iv;
    float fv, *fp;

    r = nc_get_att_double(ncid, NC_GLOBAL, "LatitudeDouble", &product->header.latitude);
    if (r != NC_NOERR) {
        r = nc_get_att_float(ncid, NC_GLOBAL, "Latitude", &fv);
        if (r == NC_NOERR) {
            product->header.latitude = (double)fv;
        }
    }
    r = nc_get_att(ncid, NC_GLOBAL, "LongitudeDouble", &product->header.longitude);
    if (r != NC_NOERR) {
        r = nc_get_att_float(ncid, NC_GLOBAL, "Longitude", &fv);
        if (r == NC_NOERR) {
            product->header.longitude = (double)fv;
        }
    }
    r = nc_get_att_float(ncid, NC_GLOBAL, "Heading", &product->header.heading);
    if (r != NC_NOERR) {
        RKLog("No radar heading found.\n");
    }
    r = nc_get_att_float(ncid, NC_GLOBAL, "Height", &product->header.radarHeight);
    if (r != NC_NOERR) {
        RKLog("No radar height found.\n");
    }
    r = nc_get_att_float(ncid, NC_GLOBAL, "Elevation", &product->header.sweepElevation);
    if (r != NC_NOERR && product->header.isPPI) {
        RKLog("Warning. No sweep elevation found.\n");
    }
    r = nc_get_att_float(ncid, NC_GLOBAL, "Azimuth", &product->header.sweepAzimuth);
    if (r != NC_NOERR && product->header.isRHI) {
        RKLog("Warning. No sweep azimuth found.\n");
    }
    r = nc_get_att_float(ncid, NC_GLOBAL, "GateSize", &product->header.gateSizeMeters);
    if (r != NC_NOERR) {
        product->header.gateSizeMeters = 0.0f;
    }
    long l;
    nc_get_att_long(ncid, NC_GLOBAL, "Time", &l);
    product->header.startTime = (double)l;
    float f;
    nc_get_att_float(ncid, NC_GLOBAL, "FractionalTime", &f);
    product->header.startTime += (double)f;
    r = nc_get_att_float(ncid, NC_GLOBAL, "Wavelength-unit", &product->header.wavelength);
    if (r != NC_NOERR) {
        product->header.wavelength = 0.0f;
    }
    r = nc_get_att_int(ncid, NC_GLOBAL, "PRF-value", &iv);
    if (r == NC_NOERR) {
        if (iv == 0) {
            RKLog("Warning. Recorded PRF = 0 Hz. Assuming 1000 Hz ...\n");
            product->header.prt[0] = 1000.0f;
        } else {
            product->header.prt[0] = 1.0f / (RKFloat)iv;
        }
    } else {
        RKLog("Warning. No PRF information found.\n");
    }
    r = nc_get_att_float(ncid, NC_GLOBAL, "Nyquist_Vel-value", &fv);
    if (r == NC_NOERR && product->header.wavelength == 0.0f && product->header.prt[0] > 0.0f) {
        product->header.wavelength = 4.0f * fv * (RKFloat)product->header.prt[0];
    }
    nc_get_att_float(ncid, NC_GLOBAL, "PulseWidth-value", &fv);
    r = nc_get_att_text(ncid, NC_GLOBAL, "PulseWidth-unit", stringValue);
    if (r == NC_NOERR && !strncasecmp("microseconds", stringValue, 12)) {
        product->header.pw[0] = 1.0e-3f * fv;
    } else {
        product->header.pw[0] = fv;
    }
    get_global_float_att(ncid, floatType, "NoiseH-ADU", &product->header.noise[0]);
    get_global_float_att(ncid, floatType, "NoiseV-ADU", &product->header.noise[1]);
    get_global_float_att(ncid, floatType, "SystemZCalH-dB", &product->header.systemZCal[0]);
    get_global_float_att(ncid, floatType, "SystemZCalV-dB", &product->header.systemZCal[1]);
    get_global_float_att(ncid, floatType, "SystemDCal-dB", &product->header.systemDCal);
    get_global_float_att(ncid, floatType, "SystemPCal-Radians", &product->header.systemPCal);
    get_global_float_att(ncid, floatType, "ZCalH1-dB", &product->header.ZCal[0][0]);
    get_global_float_att(ncid, floatType, "ZCalH2-dB", &product->header.ZCal[0][1]);
    get_global_float_att(ncid, floatType, "ZCalV1-dB", &product->header.ZCal[1][0]);
    get_global_float_att(ncid, floatType, "ZCalV2-dB", &product->header.ZCal[1][1]);
    get_global_float_att(ncid, floatType, "DCal1-dB", &product->header.DCal[0]);
    get_global_float_att(ncid, floatType, "DCal2-dB", &product->header.DCal[1]);
    get_global_float_att(ncid, floatType, "PCal1-Radians", &product->header.PCal[0]);
    get_global_float_att(ncid, floatType, "PCal2-Radians", &product->header.PCal[1]);
    get_global_float_att(ncid, floatType, "SNRThreshold-dB", &product->header.SNRThreshold);
    get_global_float_att(ncid, floatType, "SQIThreshold-dB", &product->header.SQIThreshold);

    if (!strcmp(product->desc.symbol, "Z")) {
        product->desc.index = RKProductIndexZ;
    } else if (!strcmp(product->desc.symbol, "V")) {
        product->desc.index = RKProductIndexV;
    } else if (!strcmp(product->desc.symbol, "W")) {
        product->desc.index = RKProductIndexW;
    } else if (!strcmp(product->desc.symbol, "D")) {
        product->desc.index = RKProductIndexD;
    } else if (!strcmp(product->desc.symbol, "P")) {
        product->desc.index = RKProductIndexP;
    } else if (!strcmp(product->desc.symbol, "R")) {
        product->desc.index = RKProductIndexR;
    } else if (!strcmp(product->desc.symbol, "K")) {
        product->desc.index = RKProductIndexK;
    } else if (!strcmp(product->desc.symbol, "Lh")) {
        product->desc.index = RKProductIndexLh;
    } else if (!strcmp(product->desc.symbol, "Lv")) {
        product->desc.index = RKProductIndexLv;
    } else if (!strcmp(product->desc.symbol, "PXh")) {
        product->desc.index = RKProductIndexPXh;
    } else if (!strcmp(product->desc.symbol, "PXv")) {
        product->desc.index = RKProductIndexPXv;
    } else if (!strcmp(product->desc.symbol, "RXh")) {
        product->desc.index = RKProductIndexRXh;
    } else if (!strcmp(product->desc.symbol, "RXv")) {
        product->desc.index = RKProductIndexRXv;
    } else if (!strcmp(product->desc.symbol, "S") || !strcmp(product->desc.symbol, "Sh")) {
        product->desc.index = RKProductIndexSh;
    } else if (!strcmp(product->desc.symbol, "Sv")) {
        product->desc.index = RKProductIndexSv;
    } else {
        product->desc.index = RKProductIndexQ;
    }

    #if defined(DEBUG_PRODUCT_READER)
    RKLog("%s (%s%s%s)\n",
            RKVariableInString("filename", filename, RKValueTypeString),
            rkGlobalParameters.showColor ? RKYellowColor : "",
            product->desc.symbol,
            rkGlobalParameters.showColor ? RKNoColor : "");
    RKLog("%s   %s   %s\n",
            RKVariableInString("productName", product->desc.name, RKValueTypeString),
            RKVariableInString("colormap", product->desc.colormap, RKValueTypeString),
            RKVariableInString("unit", product->desc.unit, RKValueTypeString));
    RKLog("%s m   %s us",
            RKVariableInString("wavelength", &product->header.wavelength, RKValueTypeFloat),
            RKVariableInString("pulsewidth", &product->header.pw[0], RKValueTYpeFloatMultipliedBy1M));
    RKLog("%s   %s   %s\n",
            RKVariableInString("rayCount", RKIntegerToCommaStyleString(product->header.rayCount), RKValueTypeNumericString),
            RKVariableInString("gateCount", RKIntegerToCommaStyleString(product->header.gateCount), RKValueTypeNumericString),
            RKVariableInString("capacity", RKIntegerToCommaStyleString(product->capacity), RKValueTypeNumericString));
    RKLog("%s   %s",
            RKVariableInString("noise[0]", &product->header.noise[0], RKValueTypeFloat),
            RKVariableInString("noise[1]", &product->header.noise[1], RKValueTypeFloat));
    RKLog("%s dB   %s dB",
            RKVariableInString("SystemZCalH", &product->header.systemZCal[0], RKValueTypeFloat),
            RKVariableInString("SystemZCalV", &product->header.systemZCal[1], RKValueTypeFloat));
    #endif

    // Elevation array
    if ((r = nc_inq_varid(ncid, "Elevation", &tmpId)) != NC_NOERR) {
        r = nc_inq_varid(ncid, "elevation", &tmpId);
    }
    if (r == NC_NOERR) {
        rk_nc_get_var_float(ncid, tmpId, product->startElevation);
    } else {
        RKLog("Warning. No elevation array.\n");
    }

    // Azimuth array
    if ((r = nc_inq_varid(ncid, "Azimuth", &tmpId)) != NC_NOERR) {
        r = nc_inq_varid(ncid, "azimuth", &tmpId);
    }
    if (r == NC_NOERR) {
        rk_nc_get_var_float(ncid, tmpId, product->startAzimuth);
    } else {
        RKLog("Warning. No azimuth array.\n");
    }

    // Use the first element of gatewidth array as sweep gate size, it really does not change
    if (product->header.gateSizeMeters == 0.0f) {
        if ((r = nc_inq_varid(ncid, "GateWidth", &tmpId)) != NC_NOERR) {
            if ((r = nc_inq_varid(ncid, "Gatewidth", &tmpId)) != NC_NOERR) {
                r = nc_inq_varid(ncid, "gatewidth", &tmpId);
            }
        }
        if (r == NC_NOERR) {
            nc_get_var_float(ncid, tmpId, product->data);
            product->header.gateSizeMeters = product->data[0];
        }
    }

    // Data array
    r = nc_inq_varid(ncid, product->desc.name, &tmpId);
    if (r == NC_NOERR) {
        rk_nc_get_var_float(ncid, tmpId, product->data);
        fp = product->data;
        for (r = 0; r < product->capacity; r++) {
            if (*fp == missing || *fp == folded) {
                *fp = NAN;
            }
            fp++;
        }
        #if defined(DEBUG_PRODUCT_READER)
        RKShowArray(product->data, product->desc.symbol, product->header.gateCount, product->header.rayCount);
        #endif
    }

    // Derive some other parameters that are not in the file
    product->header.endTime = product->header.startTime + 1.0;

    return collection;
}

static RKProductCollection *read_ncid_as_cf1(const int ncid) {
    MAKE_FUNCTION_NAME(myname);
    int r;
    int varid;
    int dimIds[2];
    char varName[NC_MAX_NAME + 1];
    size_t rayCount = 0;
    size_t gateCount = 0;
    float fv;

    // Dimensions
    if ((r = nc_inq_dimid(ncid, "time", &dimIds[0])) != NC_NOERR) {
        RKLog("%s Error. No time dimension found.   r = %d\n", myname, r);
        nc_close(ncid);
        return NULL;
    }
    nc_inq_dimlen(ncid, dimIds[0], &rayCount);
    if ((r = nc_inq_dimid(ncid, "range", &dimIds[1])) != NC_NOERR) {
        RKLog("%s Error. No range dimension found.   r = %d\n", myname, r);
        nc_close(ncid);
        return NULL;
    }
    nc_inq_dimlen(ncid, dimIds[1], &gateCount);

    // Data arrays
    int16_t *i16Array = (int16_t *)RKMalloc(rayCount * gateCount * sizeof(short));
    uint16_t *u16Array = (uint16_t *)RKMalloc(rayCount * gateCount * sizeof(unsigned short));
    double *f64Array = (double *)malloc(MAX(rayCount, gateCount) * sizeof(double));
    float *f32Array = (float *)RKMalloc(rayCount * gateCount * sizeof(float));
    if (i16Array == NULL || f32Array == NULL) {
        RKLog("Error. Unable to allocate short/float buffer.");
        exit(EXIT_FAILURE);
    }

    // Global attributes
    double latitude;
    if ((r = nc_inq_varid(ncid, "latitude", &varid)) == NC_NOERR) {
        nc_get_var_double(ncid, varid, &latitude);
    } else {
        RKLog("Warning. No latitude.   r = %d\n", r);
    }
    double longitude;
    if ((r = nc_inq_varid(ncid, "longitude", &varid)) == NC_NOERR) {
        nc_get_var_double(ncid, varid, &longitude);
    } else {
        RKLog("Warning. No longitude.   r = %d\n", r);
    }
    RKLog("%s %s   %s\n", myname,
        RKVariableInString("longitude", &longitude, RKValueTypeDoubleWithSixDecimals),
        RKVariableInString("latitude", &latitude, RKValueTypeDoubleWithSixDecimals));

    // List all product variables in the NetCDF file, collect symbols with the same 2D dimensions
    int nd;
    int varCount;
    int inqDimIds[NC_MAX_VAR_DIMS];
    char symbols[RKMaximumProductCount][RKMaximumSymbolLength];
    size_t count = 0;
    if ((r = nc_inq_nvars(ncid, &varCount)) != NC_NOERR) {
        RKLog("%s Error. Failed to get the number of variables.   r = %d\n", myname, r);
        return NULL;
    }
    for (varid = 0; varid < varCount; varid++) {
        nc_inq_varndims(ncid, varid, &nd);
        if (nd != 2) {
            continue;
        }
        nc_inq_vardimid(ncid, varid, inqDimIds);
        nc_inq_varname(ncid, varid, varName);
        // printf("varId %d: %s (%d x %d) ==? (%d x %d)\n", varid, varName, inqDimIds[0], inqDimIds[1], dimIds[0], dimIds[1]);
        if (inqDimIds[0] == dimIds[0] && inqDimIds[1] == dimIds[1] && count < RKMaximumProductCount) {
            snprintf(symbols[count++], RKMaximumSymbolLength - 1, "%s", varName);
        }
    }

    // Data collection
    RKProductCollection *collection = RKProductCollectionInit(count, (uint32_t)rayCount, (uint32_t)gateCount);

    RKLog("%s %s   %s   %s\n", myname,
        RKVariableInString("rayCount", &rayCount, RKValueTypeSize),
        RKVariableInString("gateCount", &gateCount, RKValueTypeSize),
        RKVariableInString("collection->count", &collection->count, RKValueTypeUInt32));

    // Get the first product to keep global parameters
    RKProduct *product = collection->products;
    memset(&product->desc, 0, sizeof(RKProductDesc));
    memset(&product->header, 0, sizeof(RKProductHeader));
    product->header.gateCount = gateCount;
    product->header.rayCount = rayCount;

    char tmpString[64];
    if (getGlobalTextAttribute(tmpString, "time_coverage_start", ncid) == NC_NOERR) {
        product->header.startTime = RKTimeStringISOToTimeDouble(tmpString);;
    } else {
        RKLog("%s Error. No time_coverage_start attribute found.\n", myname);
    }
    RKLog("time_coverage_start: %s -> %s UTC\n", tmpString, RKTimeDoubleToString(product->header.startTime, 860, true));
    if (getGlobalTextAttribute(tmpString, "time_coverage_end", ncid) == NC_NOERR) {
        product->header.endTime = RKTimeStringISOToTimeDouble(tmpString);;
    } else {
        RKLog("%s Error. No time_coverage_end attribute found.\n", myname);
    }
    RKLog("time_coverage_end: %s -> %s UTC\n", tmpString, RKTimeDoubleToString(product->header.endTime, 860, true));
    getGlobalTextAttribute(product->header.radarName, "instrument_name", ncid);
    int sweepModeVid;
    memset(tmpString, 0, sizeof(tmpString));
    nc_inq_varid(ncid, "sweep_mode", &sweepModeVid);
    nc_get_var_text(ncid, sweepModeVid, tmpString);
    RKLog("sweep_mode: %s\n", tmpString);
    if (nc_inq_varid(ncid, "fixed_angle", &varid) == NC_NOERR) {
        nc_get_var_float(ncid, varid, &fv);
    } else {
        RKLog("Warning. No fixed_angle found.\n");
    }
    if (!strcmp(tmpString, "azimuth_surveillance")) {
        product->header.isPPI = true;
        product->header.sweepElevation = fv;
    } else if (!strcmp(tmpString, "rhi")) {
        product->header.isPPI = true;
        product->header.sweepAzimuth = fv;
    }
    int rkGid;
    r = nc_inq_grp_ncid(ncid, "radarkit_parameters", &rkGid);
    if (r == NC_NOERR) {
        nc_get_att_text(rkGid, NC_GLOBAL, "waveform", product->header.waveformName);
        nc_get_att_text(rkGid, NC_GLOBAL, "moment_method", product->header.momentMethod);
        if (nc_inq_varid(rkGid, "prf", &varid) == NC_NOERR) {
            nc_get_var_float(rkGid, varid, &fv);
            product->header.prt[0] = 1.0f / fv;
        }
        if (nc_inq_varid(rkGid, "snr_threshold", &varid) == NC_NOERR) {
            nc_get_var_float(rkGid, varid, &product->header.SNRThreshold);
        } else {
            RKLog("Warning. No SNR threshold found.\n");
        }
        if (nc_inq_varid(rkGid, "sqi_threshold", &varid) == NC_NOERR) {
            nc_get_var_float(rkGid, varid, &product->header.SQIThreshold);
        } else {
            RKLog("Warning. No SQI threshold found.\n");
        }
        if (nc_inq_varid(rkGid, "noise_h", &varid) == NC_NOERR) {
            nc_get_var_float(rkGid, varid, &product->header.noise[0]);
        } else {
            RKLog("Warning. No noise_h found.\n");
        }
        if (nc_inq_varid(rkGid, "noise_v", &varid) == NC_NOERR) {
            nc_get_var_float(rkGid, varid, &product->header.noise[1]);
        } else {
            RKLog("Warning. No noise_v found.\n");
        }
        if (nc_inq_varid(rkGid, "system_z_cal_h", &varid) == NC_NOERR) {
            nc_get_var_float(rkGid, varid, &product->header.systemZCal[0]);
        } else {
            RKLog("Warning. No system_z_cal_h found.\n");
        }
        if (nc_inq_varid(rkGid, "system_z_cal_v", &varid) == NC_NOERR) {
            nc_get_var_float(rkGid, varid, &product->header.systemZCal[1]);
        } else {
            RKLog("Warning. No system_z_cal_v found.\n");
        }
        if (nc_inq_varid(rkGid, "system_d_cal", &varid) == NC_NOERR) {
            nc_get_var_float(rkGid, varid, &product->header.systemDCal);
        } else {
            RKLog("Warning. No system_d_cal found.\n");
        }
        if (nc_inq_varid(rkGid, "system_p_cal", &varid) == NC_NOERR) {
            nc_get_var_float(rkGid, varid, &product->header.systemPCal);
        } else {
            RKLog("Warning. No system_p_cal found.\n");
        }
    }

    // Time, azimuth, elevation, etc.
    float scale, offset;
    char scaleOffsetDesc[32];
    if (nc_inq_varid(ncid, "time", &varid) == NC_NOERR) {
        nc_get_var_double(ncid, varid, f64Array);
        for (int k = 0; k < rayCount; k++) {
            product->startTime[k] = f64Array[k] + product->header.startTime;
        }
    }
    if (nc_inq_varid(ncid, "azimuth", &varid) == NC_NOERR) {
        if (nc_get_att_float(ncid, varid, "scale_factor", &scale) == NC_NOERR) {
            nc_get_var_ushort(ncid, varid, u16Array);
            if (nc_get_att_float(ncid, varid, "add_offset", &offset) == NC_NOERR) {
                RKLog("Info. Azimuth in scale-offset packed mode.\n");
                u16_to_rkfloat_scale_offset(product->startAzimuth, u16Array, scale, offset, rayCount);
            } else {
                RKLog("Info. Azimuth in scale packed mode.\n");
                u16_to_rkfloat_scale(product->startAzimuth, u16Array, scale, rayCount);
            }
            u16_to_rkfloat_scale(product->startAzimuth, u16Array, scale, rayCount);
        } else {
            nc_get_var_float(ncid, varid, product->startAzimuth);
        }
    }
    if (nc_inq_varid(ncid, "elevation", &varid) == NC_NOERR) {
        if (nc_get_att_float(ncid, varid, "scale_factor", &scale) == NC_NOERR) {
            nc_get_var_short(ncid, varid, i16Array);
            if (nc_get_att_float(ncid, varid, "add_offset", &offset) == NC_NOERR) {
                RKLog("Info. Elevation in scale-offset packed mode.\n");
                i16_to_rkfloat_scale_offset(product->startElevation, i16Array, scale, offset, rayCount);
            } else {
                RKLog("Info. Elevation in scale packed mode.\n");
                i16_to_rkfloat_scale(product->startElevation, i16Array, scale, rayCount);
            }
        } else {
            nc_get_var_float(ncid, varid, product->startElevation);
        }
    }
    // float *rr = (float *)malloc(gateCount * sizeof(float));
    if (nc_inq_varid(ncid, "range", &varid) == NC_NOERR) {
        if (nc_get_att_float(ncid, varid, "scale_factor", &scale) == NC_NOERR) {
            nc_get_var_ushort(ncid, varid, u16Array);
            if (nc_get_att_float(ncid, varid, "add_offset", &offset) == NC_NOERR) {
                RKLog("Info. Range in scale-offset packed mode.\n");
                u16_to_rkfloat_scale_offset(f32Array, u16Array, scale, offset, gateCount);
            } else {
                RKLog("Info. Range in scale packed mode.\n");
                u16_to_rkfloat_scale(f32Array, u16Array, scale, gateCount);
            }
        } else {
            nc_get_var_float(ncid, varid, f32Array);
        }
        product->header.gateSizeMeters = f32Array[1] - f32Array[0];
    }
    // free(rr);
    // Product data
    // char *symbols[RKMaximumSymbolLength] = {"DBZ", "VEL", "WIDTH", "ZDR", "PHIDP", "RHOHV"};
    for (int k = 0; k < collection->count; k++) {
        RKProduct *product = &collection->products[k];
        r = nc_inq_varid(ncid, symbols[k], &varid);
        if (r != NC_NOERR) {
            RKLog("%s Error. No %s variable found.\n", myname, symbols[k]);
            continue;
        }
        if (k > 0) {
            memcpy(&product->header, &collection->products[0].header, sizeof(RKProductHeader));
        }
        memcpy(&product->desc, &collection->products[0].desc, sizeof(RKProductDesc));
        strcpy(product->desc.symbol, symbols[k]);
        nc_get_att_text(ncid, varid, "long_name", product->desc.name);
        nc_get_att_text(ncid, varid, "units", product->desc.unit);
        nc_get_att_float(ncid, varid, "scale_factor", &product->desc.cfScale);
        nc_get_att_float(ncid, varid, "add_offset", &product->desc.cfOffset);
        nc_get_var_short(ncid, varid, i16Array);
        if (product->desc.cfOffset != 0.0f) {
            sprintf(scaleOffsetDesc, "f = s * %.4f + %.4f", product->desc.cfScale, product->desc.cfOffset);
            i16_masked_to_rkfloat_scale_offset(product->data, i16Array, product->desc.cfScale, product->desc.cfOffset, rayCount * gateCount);
        } else {
            i16_masked_to_rkfloat_scale(product->data, i16Array, product->desc.cfScale, rayCount * gateCount);
            sprintf(scaleOffsetDesc, "f = s * %.4f", product->desc.cfScale);
        }
        RKLog("| %-5s | %32s | %9s | %20s |\n", product->desc.symbol, product->desc.name, product->desc.unit, scaleOffsetDesc);
    }
    #if defined(DEBUG_PRODUCT_READER)
    for (int k = 0; k < 6; k++) {
        RKProduct *product = &collection->products[k];
        RKShowArray(product->data, product->desc.symbol, gateCount, rayCount);
        printf("\n");
    }
    #endif
    free(i16Array);
    free(u16Array);
    free(f32Array);
    free(f64Array);

    return collection;
}

static RKProductCollection *read_ncid_as_cf2(const int ncid) {
    RKLog("Error. Reading CF2 is not availble until version 6.1.\n");
    return NULL;
}

static RKProductCollection *read_ncid(const int ncid) {
    MAKE_FUNCTION_NAME(myname);

    int r;
    RKProductCollection *collection = NULL;

    // Global attributes. Use these to determine which kind of NetCDF file it is.
    char datatype[32] = "";      // WDS, RadialSet, etc.
    char convention[32] = "";    // CF-1.6, CF-1.7, etc.
    char version[32] = "";       // CF-Radial-1.4, CF-Radial-2.0, etc.
    char comment[32] = "";       // RadarKit, processing info, etc.
    r = getGlobalTextAttribute(datatype, "DataType", ncid);
    if (r != NC_NOERR) {
        r = getGlobalTextAttribute(convention, "Conventions", ncid);
        if (r != NC_NOERR) {
            RKLog("%s Error. Neither WDS, CF1, or CF2\n", myname);
            return NULL;
        }
        getGlobalTextAttribute(version, "version", ncid);
        if (r != NC_NOERR) {
            RKLog("%s Error. Expected version attribute for CF1 or CF2\n", myname);
            return NULL;
        }
        getGlobalTextAttribute(comment, "comment", ncid);
        #if defined(DEBUG_PRODUCT_READER)
        RKLog("%s %s   %s   %s\n", myname,
            RKVariableInString("convention", convention, RKValueTypeString),
            RKVariableInString("version", version, RKValueTypeString),
            RKVariableInString("comment", comment, RKValueTypeString));
        #endif
        if (strcmp(convention, "CF-1.6") && strcmp(convention, "CF-1.7")) {
            RKLog("%s Error. Convention is not CF-1.6 or CF-1.7.\n", myname);
            return NULL;
        }
        // Check if it is cf-radial
        if (!strncasecmp(version, "cf", 2)) {
            char *lastDash = strrchr(version, '-');
            if (lastDash != NULL) {
                float f;
                if ((r = sscanf(lastDash + 1, "%f", &f)) > 0) {
                    // RKLog("%s Reading as %s ...\n", myname, f < 2.0 ? "CF1" : "CF2");
                    if (f < 2.0) {
                        collection = read_ncid_as_cf1(ncid);
                    } else {
                        collection = read_ncid_as_cf2(ncid);
                    }
                } else {
                    RKLog("%s Error. Could not determe CF-Radial version from %s.\n", myname, version);
                    return NULL;
                }
            } else {
                RKLog("%s Error. Version does not contain CF*-\n", myname);
                return NULL;
            }
        } else {
            RKLog("%s Error. Version does not start with CF (%s).\n", myname, version);
            return NULL;
        }
        // This could be a soft requirement
        if (strstr(comment, "RadarKit") == NULL) {
            RKLog("%s Warning. Comment does not contain RadarKit.\n", myname);
        }
    } else if (!strcmp(datatype, "RadialSet")) {
        // RKLog("%s Reading as WDS ...\n", myname);
        collection = read_ncid_as_wds(ncid);
    }

    return collection;
}

static RKProductCollection *read_nc(const char *filename) {
    MAKE_FUNCTION_NAME(myname);

    // Early return if the file does not exist
    if (!RKFilenameExists(filename)) {
        RKLog("%s Error. File %s does not exist.\n", myname, filename);
        return NULL;
    }

    int r, k;
    int ncid;
    int count;
    char list[RKMaximumProductCount][RKMaximumPathLength];

    r = RKIsFilenameStandard(filename);
    if (r == RKResultFilenameHasNoProduct) {
        // If the filename is does not contain a product symbol, then it is a single file (CF1 or CF2)
        strcpy(list[0], filename);
        count = 1;
    } else if (r == true) {
        // If the filename is a standard filename, then it is a collection of files
        count = MIN(RKMaximumProductCount, RKListFilesWithSamePrefix(filename, list));
    } else {
        RKLog("%s Error. Unsupported NC file naming.\n", myname);
        return NULL;
    }
    // Now we go through the list of files
    RKProductCollection *singles[count];
    for (k = 0; k < count; k++) {
        char *currentFile = list[k];
        // Open the file
        if ((r = nc_open(currentFile, NC_NOWRITE, &ncid)) > 0) {
            RKLog("%s Error opening file %s (%s)\n", myname, currentFile, nc_strerror(r));
            return NULL;
        }
        singles[k] = read_ncid(ncid);
        // Close the file
        if ((r = nc_close(ncid))) {
            RKLog("Error: %s\n", nc_strerror(r));
            return NULL;
        }
    }
    // Now we combine the singles into one collection and free the singles
    RKProductCollection *collection = NULL;
    if (count > 1) {
        collection = RKProductCollectionInitFromSingles(singles, count);
        for (k = 0; k < count; k++) {
            RKProductCollectionFree(singles[k]);
        }
    } else {
        collection = singles[0];
    }
    // RKProductCollectionStandardizeForCFRadial(collection);
    // for (int k = 0; k < collection->count; k++) {
    //     RKProduct *product = &collection->products[k];
    //     RKLog("%d: %s (%s)\n", k, product->desc.name, product->desc.unit);
    //     RKShowArray(product->data, product->desc.symbol, product->header.gateCount, product->header.rayCount);
    // }
    // RKProductCollectionFileWriterCF(collection, "/Users/boonleng/Downloads/data/test.nc", RKWriterOptionNone);
    return collection;
}

#if defined(_HAS_NETCDF_MEM_H)

static RKProductCollection *read_tar(const char *filename) {
    MAKE_FUNCTION_NAME(myname);

    int r, k;
    int ncid;
    struct archive *a;
    struct archive_entry *entry;
    size_t buffer_size = 0;
    size_t capacity = 8 * 1024 * 1024;
    void *buffer = NULL;
    char hint[64];

    a = archive_read_new();
    archive_read_support_filter_gzip(a);
    archive_read_support_format_gnutar(a);
    archive_read_support_format_tar(a);
    archive_read_support_filter_xz(a);
    r = archive_read_open_filename(a, filename, 10240);
    if (r != ARCHIVE_OK) {
        RKLog("%s Error. %s\n", myname, archive_error_string(a));
        return NULL;
    }
    k = 0;
    RKProductCollection *singles[RKMaximumProductCount];
    buffer = malloc(capacity);
    if (buffer == NULL) {
        RKLog("%s Error. Unable to allocate memory\n", myname);
        return NULL;
    }
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        char *filename = (char *)archive_entry_pathname(entry);
        char *basename = RKLastPartOfPath(filename);
        if (!((basename[0] >= 'a' && basename[0] <= 'z') ||
              (basename[0] >= 'A' && basename[0] <= 'Z') ||
              (basename[0] >= '0' && basename[0] <= '9'))) {
            RKLog("%s Skipping %s ...\n", myname, filename);
            archive_read_data_skip(a);
            continue;
        }
        buffer_size = archive_entry_size(entry);
        if (buffer_size > capacity) {
            RKLog("%s Reallocating buffer to %s B\n", myname, RKIntegerToCommaStyleString(buffer_size));
            buffer = realloc(buffer, buffer_size);
            if (buffer == NULL) {
                RKLog("%s Error. Unable to allocate memory\n", myname);
                return NULL;
            }
            capacity = buffer_size;
        }
        RKLog("%s %s (%s B)\n", myname, filename, RKIntegerToCommaStyleString(buffer_size));
        r = archive_read_data(a, buffer, buffer_size);
        if (r < 0) {
            RKLog("%s Error. %s\n", myname, archive_error_string(a));
            free(buffer);
            return NULL;
        }
        // Open the file, pass a hint in the forms of MEMORY-YYYYMMDD-HHMMSS-P.nc for RKGetSymbolFromFilename()
        if (RKIsFilenameStandard(basename)) {
            sprintf(hint, "MEMORY%s", strchr(basename, '-'));
        } else {
            sprintf(hint, "MEMORY");
        }
        if ((r = nc_open_mem(hint, NC_NOWRITE, buffer_size, buffer, &ncid))) {
            RKLog("%s Error. %s\n", myname, nc_strerror(r));
            free(buffer);
            return NULL;
        }
        // Pass the file ncid to the reader
        singles[k++] = read_ncid(ncid);
        // Close the file
        if ((r = nc_close(ncid))) {
            RKLog("%s Error. %s\n", myname, nc_strerror(r));
            return NULL;
        }
        archive_read_data_skip(a);
    }
    r = archive_read_free(a);
    if (r != ARCHIVE_OK) {
        RKLog("%s Error. %s\n", myname, archive_error_string(a));
    }
    free(buffer);
    // Now we combine the singles into one collection and free the singles
    if (k == 1) {
        return singles[0];
    }
    RKProductCollection *collection = RKProductCollectionInitFromSingles(singles, k);
    while (k > 0) {
        RKProductCollectionFree(singles[--k]);
    }
    return collection;
}

#endif

RKProductCollection *RKProductCollectionInitWithFilename(const char *filename) {
    MAKE_FUNCTION_NAME(myname);
    RKLog("%s %s\n", myname, filename);
    if (filename == NULL) {
        RKLog("%s No filename given.\n", myname);
        exit(EXIT_FAILURE);
    }
    if (!RKFilenameExists(filename)) {
        RKLog("%s Error. File %s does not exist.\n", myname, filename);
        return NULL;
    }
    const char *ext = RKFileExtension(filename);
    if (ext == NULL) {
        RKLog("Error. Unable to determine the file extension.\n");
        return NULL;
    }
    #if defined(DEBUG_PRODUCT_READER)
    RKLog("File extension: %s\n", ext);
    #endif
    RKProductCollection *collection = NULL;
    if (!strcasecmp(ext, ".nc")) {
        RKLog("Reading %s ...\n", filename);
        collection = read_nc(filename);
    } else if (!strcasecmp(ext, ".txz") ||
               !strcasecmp(ext, ".tgz") ||
               !strcasecmp(ext, ".tar.xz") ||
               !strcasecmp(ext, ".tar.gz")) {
        #if defined(_HAS_NETCDF_MEM_H)
        RKLog("Reading %s using read_tar() ...\n", filename);
        collection = read_tar(filename);
        #else
        RKLog("Error. In-memory tar file reader is not supported.\n");
        #endif
    } else {
        RKLog("Error. Unsupported file extension: %s\n", ext);
    }
    for (int k = 0; k < collection->count; k++) {
        RKProduct *product = &collection->products[k];
        RKLog("%d: %s (%s)\n", k, product->desc.name, product->desc.unit);
        RKShowArray(product->data, product->desc.symbol, product->header.gateCount, product->header.rayCount);
    }
    return collection;
}

int RKProductCollectionFileWriterCF(RKProductCollection *collection, const char *filename, const RKWriterOption options) {
    MAKE_FUNCTION_NAME(name);

    RKProduct *product = collection->products;

    const size_t rayCount = product->header.rayCount;
    const size_t gateCount = product->header.gateCount;
    const size_t cellCount = rayCount * gateCount;
    const int16_t fillValue = FILL_VALUE;
    const time_t timeCreated = time(NULL);
    const float positionScale = 360.0f / 65536.0f;
    const int stringLength = 32;

    int k;
    int ncid;

    char *strArray = (char *)malloc(stringLength * sizeof(char));
    int16_t *i16Array = (int16_t *)malloc(cellCount * sizeof(int16_t));
    uint16_t *u16Array = (uint16_t *)malloc(MAX(rayCount, gateCount) * sizeof(uint16_t));
    float32_t *f32Array = (float32_t *)malloc(MAX(rayCount, gateCount) * sizeof(float32_t));
    double *f64Array = (double *)malloc(MAX(rayCount, gateCount) * sizeof(double));

    RKName stringValue;
    char startTimeString[24];
    char endTimeString[24];
    char refTimeString[40];

    float f;

    if (strlen(product->desc.name) == 0) {
        RKLog("Error. Product name is empty.\n");
        return RKResultProductDescriptionNotSet;
    }
    if (rayCount == 0 || gateCount == 0) {
        RKLog("Error. Product dimensions are zero.\n");
        return RKResultProductDimensionsNotSet;
    }
    if (product->header.startTime == 0) {
        RKLog("Error. Product start time is zero.\n");
        return RKResultProductStartTimeNotSet;
    }
    if (product->header.gateSizeMeters == 0.0f) {
        RKLog("Error. Product gate size is zero.\n");
        return RKResultProductGateSizeNotSet;
    }
    if (strcasecmp(product->desc.description, "equivalent_reflectivity_factor") || strcasecmp(product->desc.symbol, "dbz")) {
        RKProductCollectionStandardizeForCFRadial(collection);
    }
    // Open a file
    if ((k = nc_create(filename, NC_NETCDF4 | NC_CLOBBER, &ncid)) > 0) {
        RKLog("Error. Unable to create %s   nc_create() returned %d\n", filename, k);
        RKLog("Error. %s\n", nc_strerror(k));
        return RKResultFailedToOpenFileForProduct;
    }
    // Define the dimensions
    strftime(stringValue, RKNameLength, "%FT%X", gmtime(&timeCreated));
    sprintf(startTimeString, "%sZ", RKTimeDoubleToString(product->header.startTime, 1180, false));
    sprintf(endTimeString, "%sZ", RKTimeDoubleToString(product->header.endTime, 1180, false));
    sprintf(refTimeString, "seconds since %s", startTimeString);
    // Define the global attributes
    put_global_text_att(ncid, "Conventions", "CF-1.7");
    put_global_text_att(ncid, "Sub_conventions", "CF-Radial instrument_parameters radar_calibration");
    put_global_text_att(ncid, "version", "CF-Radial-1.4");
    put_global_text_att(ncid, "title", "Radar products");
    put_global_text_att(ncid, "institution", "University of Oklahoma");
    put_global_text_att(ncid, "source", "radar observation");
    put_global_text_att(ncid, "history", "");
    put_global_text_att(ncid, "references", "");
    put_global_text_att(ncid, "comment", "RadarKit " __RKVersion__);
    put_global_text_att(ncid, "instrument_name", product->header.radarName);
    put_global_text_att(ncid, "time_coverage_start", startTimeString);
    put_global_text_att(ncid, "time_coverage_end", endTimeString);
    put_global_text_att(ncid, "start_datetime", startTimeString);
    put_global_text_att(ncid, "end_datetime", endTimeString);
    put_global_text_att(ncid, "created", stringValue);
    put_global_text_att(ncid, "platform_is_mobile", "false");
    put_global_text_att(ncid, "ray_times_increase", "true");
    // Define the dimensions
    int timeDid;
    nc_def_dim(ncid, "time", rayCount, &timeDid);
    int rangeDid;
    nc_def_dim(ncid, "range", gateCount, &rangeDid);
    int sweepDimId;
    nc_def_dim(ncid, "sweep", 1, &sweepDimId);
    int stringDimId;
    nc_def_dim(ncid, "string_length", stringLength, &stringDimId);
    int calibrationDimId;
    nc_def_dim(ncid, "r_calib", 1, &calibrationDimId);
    int timeRangeDimId[] = {timeDid, rangeDid};
    int sweepStringDimId[] = {sweepDimId, stringDimId};
    // Define the variables
    int volumeNumberVid;
    nc_def_var(ncid, "volume_number", NC_INT, 0, NULL, &volumeNumberVid);
    put_variable_text_att(ncid, volumeNumberVid, "long_name", "volume_index_number_0_based");
    int timeCoverageStartVid;
    nc_def_var(ncid, "time_coverage_start", NC_CHAR, 1, &stringDimId, &timeCoverageStartVid);
    put_variable_text_att(ncid, timeCoverageStartVid, "standard_name", "data_volume_start_time_utc");
    put_variable_text_att(ncid, timeCoverageStartVid, "comments", "ray times are relative to start time in secs");
    int timeCoverageEndVid;
    nc_def_var(ncid, "time_coverage_end", NC_CHAR, 1, &stringDimId, &timeCoverageEndVid);
    put_variable_text_att(ncid, timeCoverageEndVid, "standard_name", "data_volume_end_time_utc");
    put_variable_text_att(ncid, timeCoverageEndVid, "comments", "ray times are relative to start time in secs");
    int latitudeVid;
    nc_def_var(ncid, "latitude", NC_DOUBLE, 0, NULL, &latitudeVid);
    put_variable_text_att(ncid, latitudeVid, "units", "degrees_north");
    int longitudeVid;
    nc_def_var(ncid, "longitude", NC_DOUBLE, 0, NULL, &longitudeVid);
    put_variable_text_att(ncid, longitudeVid, "units", "degrees_east");
    int altitudeVid;
    nc_def_var(ncid, "altitude", NC_DOUBLE, 0, NULL, &altitudeVid);
    put_variable_text_att(ncid, altitudeVid, "units", "meters");
    int sweepNumberVid;
    nc_def_var(ncid, "sweep_number", NC_INT, 1, &sweepDimId, &sweepNumberVid);
    put_variable_text_att(ncid, sweepNumberVid, "long_name", "sweep_index_number_0_based");
    int sweepModeVid;
    nc_def_var(ncid, "sweep_mode", NC_CHAR, 2, sweepStringDimId, &sweepModeVid);
    put_variable_text_att(ncid, sweepModeVid, "long_name", "scan_mode_for_sweep");
    int fixedAngleVid;
    nc_def_var(ncid, "fixed_angle", NC_FLOAT, 1, &sweepDimId, &fixedAngleVid);
    put_variable_text_att(ncid, fixedAngleVid, "long_name", "ray_target_fixed_angle");
    put_variable_text_att(ncid, fixedAngleVid, "unit", "degrees");
    int sweepStartVid;
    nc_def_var(ncid, "sweep_start_ray_index", NC_INT, 1, &sweepDimId, &sweepStartVid);
    put_variable_text_att(ncid, sweepStartVid, "long_name", "index_of_first_ray_in_sweep");
    int sweepEndVid;
    nc_def_var(ncid, "sweep_end_ray_index", NC_INT, 1, &sweepDimId, &sweepEndVid);
    put_variable_text_att(ncid, sweepEndVid, "long_name", "index_of_last_ray_in_sweep");
    // Define the coordinate variables
    int timeVid;
    nc_def_var(ncid, "time", NC_DOUBLE, 1, &timeDid, &timeVid);
    put_variable_text_att(ncid, timeVid, "standard_name", "time");
    put_variable_text_att(ncid, timeVid, "long_name", "time_in_seconds_since_volume_start");
    put_variable_text_att(ncid, timeVid, "units", refTimeString);
    put_variable_text_att(ncid, timeVid, "calendar", "gregorian");
    int rangeVid;
    nc_def_var(ncid, "range", options & RKWriterOptionPackPosition ? NC_USHORT : NC_FLOAT, 1, &rangeDid, &rangeVid);
    put_variable_text_att(ncid, rangeVid, "standard_name", "projection_range_coordinate");
    put_variable_text_att(ncid, rangeVid, "long_name", "range_to_measurement_volume");
    put_variable_text_att(ncid, rangeVid, "units", "meters");
    put_variable_text_att(ncid, rangeVid, "spacing_is_constant", "true");
    f = 0.5f * product->header.gateSizeMeters;
    nc_put_att_float(ncid, rangeVid, "meters_to_center_of_first_gate", NC_FLOAT, 1, &f);
    nc_put_att_float(ncid, rangeVid, "meters_between_gates", NC_FLOAT, 1, &product->header.gateSizeMeters);
    put_variable_text_att(ncid, rangeVid, "axis", "radial_range_coordinate");
    if (options & RKWriterOptionPackPosition) {
        nc_put_att_float(ncid, rangeVid, "scale_factor", NC_FLOAT, 1, &product->header.gateSizeMeters);
    }
    int azimuthVid;
    nc_def_var(ncid, "azimuth", options & RKWriterOptionPackPosition ? NC_USHORT : NC_FLOAT, 1, &timeDid, &azimuthVid);
    put_variable_text_att(ncid, azimuthVid, "standard_name", "ray_azimuth_angle");
    put_variable_text_att(ncid, azimuthVid, "long_name", "azimuth_angle_from_true_north");
    put_variable_text_att(ncid, azimuthVid, "units", "degrees");
    put_variable_text_att(ncid, azimuthVid, "axis", "radial_azimuth_coordinate");
    if (options & RKWriterOptionPackPosition) {
        nc_put_att_float(ncid, azimuthVid, "scale_factor", NC_FLOAT, 1, &positionScale);
    }
    int elevationVid;
    nc_def_var(ncid, "elevation", options & RKWriterOptionPackPosition ? NC_SHORT : NC_FLOAT, 1, &timeDid, &elevationVid);
    put_variable_text_att(ncid, elevationVid, "standard_name", "ray_elevation_angle");
    put_variable_text_att(ncid, elevationVid, "long_name", "elevation_angle_from_horizontal_plane");
    put_variable_text_att(ncid, elevationVid, "units", "degrees");
    put_variable_text_att(ncid, elevationVid, "axis", "radial_elevation_coordinate");
    if (options & RKWriterOptionPackPosition) {
        nc_put_att_float(ncid, elevationVid, "scale_factor", NC_FLOAT, 1, &positionScale);
    }
    int pwVid;
    nc_def_var(ncid, "pulse_width", NC_FLOAT, 1, &timeDid, &pwVid);
    put_variable_text_att(ncid, pwVid, "long_name", "transmitter_pulse_width");
    put_variable_text_att(ncid, pwVid, "units", "seconds");
    put_variable_text_att(ncid, pwVid, "meta_group", "instrument_parameters");
    int prtVid;
    nc_def_var(ncid, "prt", NC_FLOAT, 1, &timeDid, &prtVid);
    put_variable_text_att(ncid, prtVid, "long_name", "pulse repetition time");
    put_variable_text_att(ncid, prtVid, "units", "seconds");
    put_variable_text_att(ncid, prtVid, "meta_group", "instrument_parameters");
    // Define the products
    int *productVid, productVids[collection->count];
    memset(productVids, 0, collection->count * sizeof(int));
    for (k = 0; k < collection->count; k++) {
        product = collection->products + k;
        productVid = productVids + k;
        nc_def_var(ncid, product->desc.symbol, NC_SHORT, 2, timeRangeDimId, productVid);
        put_variable_text_att(ncid, *productVid, "standard_name", product->desc.description);
        put_variable_text_att(ncid, *productVid, "long_name", product->desc.name);
        put_variable_text_att(ncid, *productVid, "units", product->desc.unit);
        if (options & RKWriterOptionDeflateFields) {
            nc_def_var_deflate(ncid, *productVid, true, true, 3);
        }
        nc_put_att_short(ncid, *productVid, "_FillValue", NC_SHORT, 1, &fillValue);
        nc_put_att_float(ncid, *productVid, "scale_factor", NC_FLOAT, 1, &product->desc.cfScale);
        nc_put_att_float(ncid, *productVid, "add_offset", NC_FLOAT, 1, &product->desc.cfOffset);
        put_variable_text_att(ncid, *productVid, "coordinates", "time range");
    }
    // radar_calibration
    int zCalVid;
    nc_def_var(ncid, "r_calib_dbz_correction", NC_FLOAT, 1, &calibrationDimId, &zCalVid);
    put_variable_text_att(ncid, zCalVid, "long_name", "calibrated_radar_dbz_correction");
    put_variable_text_att(ncid, zCalVid, "units", "dB");
    put_variable_text_att(ncid, zCalVid, "meta_group", "radar_calibration");
    int dCalVid;
    nc_def_var(ncid, "r_calib_zdr_correction", NC_FLOAT, 1, &calibrationDimId, &dCalVid);
    put_variable_text_att(ncid, dCalVid, "long_name", "calibrated_radar_zdr_correction");
    put_variable_text_att(ncid, dCalVid, "units", "dB");
    put_variable_text_att(ncid, dCalVid, "meta_group", "radar_calibration");
    int pCalVid;
    nc_def_var(ncid, "r_calib_system_phidp", NC_FLOAT, 1, &calibrationDimId, &pCalVid);
    put_variable_text_att(ncid, pCalVid, "long_name", "calibrated_radar_system_phidp");
    put_variable_text_att(ncid, pCalVid, "units", "degrees");
    put_variable_text_att(ncid, pCalVid, "meta_group", "radar_calibration");
    // Populate global variables from the first product
    product = collection->products;
    // Define a group for RadarKit house-keeping parameters
    int radarkitGid;
    k = nc_def_grp(ncid, "radarkit_parameters", &radarkitGid);
    if (k != NC_NOERR) {
        RKLog("Error. Unable to create a group for RadarKit parameters.\n");
        RKLog("Error. %s\n", nc_strerror(k));
        return k;
    }
    put_global_text_att(radarkitGid, "waveform", product->header.waveformName);
    put_global_text_att(radarkitGid, "moment_method", product->header.momentMethod);
    int prfVid;
    nc_def_var(radarkitGid, "prf", NC_FLOAT, 0, NULL, &prfVid);
    put_variable_text_att(radarkitGid, prfVid, "long_name", "pulse_repetition_frequency");
    put_variable_text_att(radarkitGid, prfVid, "units", "Hz");
    int snrThresholdVid;
    nc_def_var(radarkitGid, "snr_threshold", NC_FLOAT, 0, NULL, &snrThresholdVid);
    put_variable_text_att(radarkitGid, snrThresholdVid, "long_name", "signal_to_noise_threshold");
    put_variable_text_att(radarkitGid, snrThresholdVid, "units", "dB");
    int sqiThresholdVid;
    nc_def_var(radarkitGid, "sqi_threshold", NC_FLOAT, 0, NULL, &sqiThresholdVid);
    put_variable_text_att(radarkitGid, sqiThresholdVid, "long_name", "signal_quality_index_threshold");
    put_variable_text_att(radarkitGid, sqiThresholdVid, "units", "unitless");
    int noiseHVid;
    nc_def_var(radarkitGid, "noise_h", NC_FLOAT, 0, NULL, &noiseHVid);
    put_variable_text_att(radarkitGid, noiseHVid, "long_name", "noise_power_h");
    put_variable_text_att(radarkitGid, noiseHVid, "units", "ADU");
    int noiseVVid;
    nc_def_var(radarkitGid, "noise_v", NC_FLOAT, 0, NULL, &noiseVVid);
    put_variable_text_att(radarkitGid, noiseVVid, "long_name", "noise_power_v");
    put_variable_text_att(radarkitGid, noiseVVid, "units", "ADU");
    int systemZCalHVid;
    nc_def_var(radarkitGid, "system_z_cal_h", NC_FLOAT, 0, NULL, &systemZCalHVid);
    put_variable_text_att(radarkitGid, systemZCalHVid, "long_name", "system_z_cal_h");
    put_variable_text_att(radarkitGid, systemZCalHVid, "units", "dB");
    int systemZCalVVid;
    nc_def_var(radarkitGid, "system_z_cal_v", NC_FLOAT, 0, NULL, &systemZCalVVid);
    put_variable_text_att(radarkitGid, systemZCalVVid, "long_name", "system_z_cal_v");
    put_variable_text_att(radarkitGid, systemZCalVVid, "units", "dB");
    int systemDCalVid;
    nc_def_var(radarkitGid, "system_d_cal", NC_FLOAT, 0, NULL, &systemDCalVid);
    put_variable_text_att(radarkitGid, systemDCalVid, "long_name", "system_d_cal");
    put_variable_text_att(radarkitGid, systemDCalVid, "units", "dB");
    int systemPCalVid;
    nc_def_var(radarkitGid, "system_p_cal", NC_FLOAT, 0, NULL, &systemPCalVid);
    put_variable_text_att(radarkitGid, systemPCalVid, "long_name", "system_p_cal");
    put_variable_text_att(radarkitGid, systemPCalVid, "units", "radians");
    // Conclude the RadarKit house-keeping parameters group
    nc_enddef(radarkitGid);
    // Conclude all definitions
    nc_enddef(ncid);
    // Populate global variables
    k = (int)product->header.volumeIndex;
    nc_put_var_int(ncid, volumeNumberVid, &k);
    memset(strArray, ' ', stringLength);
    memcpy(strArray, startTimeString, strlen(startTimeString));
    nc_put_var_text(ncid, timeCoverageStartVid, strArray);
    memset(strArray, ' ', stringLength);
    memcpy(strArray, endTimeString, strlen(endTimeString));
    nc_put_var_text(ncid, timeCoverageEndVid, strArray);
    nc_put_var_double(ncid, latitudeVid, &product->header.latitude);
    nc_put_var_double(ncid, longitudeVid, &product->header.longitude);
    nc_put_var_double(ncid, altitudeVid, &product->header.altitude);
    memset(strArray, ' ', stringLength);
    if (product->header.isPPI) {
        const char mode[] = "azimuth_surveillance";
        memcpy(strArray, mode, strlen(mode));
        nc_put_var_float(ncid, fixedAngleVid, &product->header.sweepElevation);
        nc_put_var_text(ncid, sweepModeVid, strArray);
    } else if (product->header.isRHI) {
        const char mode[] = "rhi";
        memcpy(strArray, mode, strlen(mode));
        nc_put_var_float(ncid, fixedAngleVid, &product->header.sweepAzimuth);
        nc_put_var_text(ncid, sweepModeVid, strArray);
    } else {
        const float f = 0.0f;
        const char mode[] = "manual";
        memcpy(strArray, mode, strlen(mode));
        nc_put_var_float(ncid, fixedAngleVid, &f);
        nc_put_var_text(ncid, sweepModeVid, strArray);
    }
    k = 0;
    nc_put_var_int(ncid, sweepStartVid, &k);
    k = (int)rayCount - 1;
    nc_put_var_int(ncid, sweepEndVid, &k);
    k = (int)product->header.sweepIndex;
    nc_put_var_int(ncid, sweepNumberVid, &k);
    memcpy(f64Array, product->startTime, rayCount * sizeof(double));
    for (k = 0; k < rayCount; k++) {
        f64Array[k] -= product->startTime[0];
    }
    nc_put_var_double(ncid, timeVid, f64Array);
    if (options & RKWriterOptionPackPosition) {
        for (k = 0; k < gateCount; k++) {
            u16Array[k] = k;
        }
        nc_put_var_ushort(ncid, rangeVid, u16Array);
        rkfloat_to_u16_inv_scale(u16Array, product->startAzimuth, positionScale, gateCount);
        nc_put_var_ushort(ncid, azimuthVid, u16Array);
        rkfloat_to_i16_inv_scale(i16Array, product->startElevation, positionScale, rayCount);
        nc_put_var_short(ncid, elevationVid, i16Array);
    } else {
        for (k = 0; k < gateCount; k++) {
            f32Array[k] = (float)(k + 0.5f) * product->header.gateSizeMeters;
        }
        nc_put_var_float(ncid, rangeVid, f32Array);
        rkfloat_to_f32(f32Array, product->startAzimuth, gateCount);
        nc_put_var_float(ncid, azimuthVid, f32Array);
        rkfloat_to_f32(f32Array, product->startElevation, rayCount);
        nc_put_var_float(ncid, elevationVid, f32Array);
    }
    f = product->header.pw[0];
    RKSetArrayToFloat32(f32Array, f, rayCount);
    nc_put_var_float(ncid, pwVid, f32Array);
    f = product->header.prt[0];
    RKSetArrayToFloat32(f32Array, f, rayCount);
    nc_put_var_float(ncid, prtVid, f32Array);
    // All the field data
    for (k = 0; k < collection->count; k++) {
        product = collection->products + k;
        productVid = productVids + k;
        if (product->desc.cfOffset == 0.0f) {
            rkfloat_to_i16_masked_inv_scale(i16Array, product->data, product->desc.cfScale, cellCount);
        } else {
            rkfloat_to_i16_masked_inv_scale_offset(i16Array, product->data, product->desc.cfScale, product->desc.cfOffset, cellCount);
        }
        nc_put_var_short(ncid, *productVid, i16Array);
    }
    nc_put_var_float(ncid, zCalVid, &product->header.systemZCal[0]);
    nc_put_var_float(ncid, dCalVid, &product->header.systemDCal);
    nc_put_var_float(ncid, pCalVid, &product->header.systemPCal);
    // Variables in RadarKit parameters group
    f = 1.0f / product->header.prt[0];
    nc_put_var_float(radarkitGid, prfVid, &f);
    nc_put_var_float(radarkitGid, snrThresholdVid, &product->header.SNRThreshold);
    nc_put_var_float(radarkitGid, sqiThresholdVid, &product->header.SQIThreshold);
    nc_put_var_float(radarkitGid, noiseHVid, &product->header.noise[0]);
    nc_put_var_float(radarkitGid, noiseVVid, &product->header.noise[1]);
    nc_put_var_float(radarkitGid, systemZCalHVid, &product->header.systemZCal[0]);
    nc_put_var_float(radarkitGid, systemZCalVVid, &product->header.systemZCal[1]);
    nc_put_var_float(radarkitGid, systemDCalVid, &product->header.systemDCal);
    nc_put_var_float(radarkitGid, systemPCalVid, &product->header.systemPCal);
    // Close the file
    ncclose(ncid);
    // Cleanup
    free(f64Array);
    free(f32Array);
    free(i16Array);
    free(u16Array);
    free(strArray);
    return RKResultSuccess;
}

int RKProductCollectionStandardizeForWDSSII(RKProductCollection *collection) {
    RKLog("Warning. This method has not been implemented yet.\n");
    return RKResultSuccess;
}

int RKProductCollectionStandardizeForCFRadial(RKProductCollection *collection) {
    RKProduct *product = collection->products;
    const uint32_t rayCount = product->header.rayCount;
    const uint32_t gateCount = product->header.gateCount;
    char summary[1024];
    int s;
    for (int p = 0; p < collection->count; p++) {
        s = sprintf(summary, "Standardizing %s%s%s (%s%s%s) -> ",
            rkGlobalParameters.showColor ? RKYellowColor : "",
            product->desc.symbol,
            rkGlobalParameters.showColor ? RKNoColor : "",
            rkGlobalParameters.showColor ? RKSalmonColor : "",
            product->desc.name,
            rkGlobalParameters.showColor ? RKNoColor : "");
        switch (product->desc.index) {
            case RKProductIndexZ:
                sprintf(product->desc.description, "equivalent_reflectivity_factor");
                sprintf(product->desc.name, "reflectivity");
                sprintf(product->desc.unit, "dBZ");
                sprintf(product->desc.symbol, "DBZ");
                product->desc.cfScale = 0.01f;
                product->desc.cfOffset = 0.0f;
                break;
            case RKProductIndexV:
                sprintf(product->desc.description, "radial_velocity_of_scatterers_away_from_instrument");
                sprintf(product->desc.name, "doppler_velocity");
                sprintf(product->desc.unit, "m/s");
                sprintf(product->desc.symbol, "VEL");
                product->desc.cfScale = 0.01f;
                product->desc.cfOffset = 0.0f;
                break;
            case RKProductIndexW:
                sprintf(product->desc.description, "doppler_spectrum_width");
                sprintf(product->desc.name, "spectrum_width");
                sprintf(product->desc.unit, "m/s");
                sprintf(product->desc.symbol, "WIDTH");
                product->desc.cfScale = 0.01f;
                product->desc.cfOffset = 0.0f;
                break;
            case RKProductIndexD:
                sprintf(product->desc.description, "log_differential_reflectivity_hv");
                sprintf(product->desc.name, "differential_reflectivity");
                sprintf(product->desc.unit, "dB");
                sprintf(product->desc.symbol, "ZDR");
                product->desc.cfScale = 0.01f;
                product->desc.cfOffset = 0.0f;
                break;
            case RKProductIndexP:
                if (!strncasecmp(product->desc.unit, "radian", 6)) {
                    float *x = product->data;
                    #ifdef DEBUG_SHOW_MIN_MAX_PHIDP
                    float minValue = 180.0f, maxValue = -180.0f;
                    for (int j = 0; j < rayCount * gateCount; j++) {
                        minValue = isfinite(product->data[j]) ? fminf(minValue, product->data[j]) : minValue;
                        maxValue = isfinite(product->data[j]) ? fmaxf(maxValue, product->data[j]) : maxValue;
                    }
                    RKLog("RKProductCollectionStandardizeForCFRadial() %s   %s\n",
                        RKVariableInString("minValue", &minValue, RKValueTypeFloat),
                        RKVariableInString("maxValue", &maxValue, RKValueTypeFloat));
                    #endif
                    RKSIMD_iscl(x, 180.0f / M_PI, rayCount * gateCount);
                }
                sprintf(product->desc.description, "differential_phase_hv");
                sprintf(product->desc.name, "differential_phase");
                sprintf(product->desc.unit, "degrees");
                sprintf(product->desc.symbol, "PHIDP");
                product->desc.cfScale = 0.01f;
                product->desc.cfOffset = 0.0f;
                break;
            case RKProductIndexR:
                sprintf(product->desc.description, "cross_correlation_ratio_hv");
                sprintf(product->desc.name, "cross_correlation_ratio");
                sprintf(product->desc.unit, "unitless");
                sprintf(product->desc.symbol, "RHOHV");
                product->desc.cfScale = 0.001f;
                product->desc.cfOffset = 0.0f;
                break;
            case RKProductIndexLh:
                sprintf(product->desc.description, "linear_depolarization_ratio_h");
                sprintf(product->desc.name, "linear_depolarization_ratio_h");
                sprintf(product->desc.unit, "dB");
                sprintf(product->desc.symbol, "LDRH");
                product->desc.cfScale = 0.01f;
                product->desc.cfOffset = 0.0f;
                break;
            case RKProductIndexLv:
                sprintf(product->desc.description, "linear_depolarization_ratio_v");
                sprintf(product->desc.name, "linear_depolarization_ratio_v");
                sprintf(product->desc.unit, "dB");
                sprintf(product->desc.symbol, "LDRV");
                product->desc.cfScale = 0.01f;
                product->desc.cfOffset = 0.0f;
                break;
            case RKProductIndexPXh:
                sprintf(product->desc.description, "differential_phase_copolar_h_crosspolar_v");
                sprintf(product->desc.name, "differential_phase_copolar_h_crosspolar_v");
                sprintf(product->desc.unit, "degrees");
                sprintf(product->desc.symbol, "PHIXH");
                product->desc.cfScale = 0.01f;
                product->desc.cfOffset = 0.0f;
                break;
            case RKProductIndexPXv:
                sprintf(product->desc.description, "differential_phase_copolar_v_crosspolar_h");
                sprintf(product->desc.name, "differential_phase_copolar_v_crosspolar_h");
                sprintf(product->desc.unit, "degrees");
                sprintf(product->desc.symbol, "PHIXV");
                product->desc.cfScale = 0.01f;
                product->desc.cfOffset = 0.0f;
                break;
            case RKProductIndexRXh:
                sprintf(product->desc.description, "correlation_coefficient_copolar_h_crospolar_v");
                sprintf(product->desc.name, "correlation_coefficient_copolar_h_crospolar_v");
                sprintf(product->desc.unit, "unitless");
                sprintf(product->desc.symbol, "RHOXH");
                product->desc.cfScale = 0.001f;
                product->desc.cfOffset = 0.0f;
                break;
            case RKProductIndexRXv:
                sprintf(product->desc.description, "correlation_coefficient_copolar_v_crospolar_h");
                sprintf(product->desc.name, "correlation_coefficient_copolar_v_crospolar_h");
                sprintf(product->desc.unit, "unitless");
                sprintf(product->desc.symbol, "RHOXV");
                product->desc.cfScale = 0.001f;
                product->desc.cfOffset = 0.0f;
                break;
            default:
                RKLog("RKProductCollectionStandardizeForCFRadial() Unknown product index %d\n", product->desc.index);
                return RKResultFailedToStandardizeProduct;
                break;
        }
        s += sprintf(summary + s, "%s%s%s (%s%s%s)\n",
            rkGlobalParameters.showColor ? RKYellowColor : "",
            product->desc.symbol,
            rkGlobalParameters.showColor ? RKNoColor : "",
            rkGlobalParameters.showColor ? RKSalmonColor : "",
            product->desc.name,
            rkGlobalParameters.showColor ? RKNoColor : "");
        RKLog(summary);
        product++;
    }
    return RKResultSuccess;
}
