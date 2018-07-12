//
//  RKProductFile.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 6/24/18.
//  Copyright © Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKProductFile.h>

#if RKfloat == float
#define rk_nc_get_var_float   nc_get_var_float
#else
#define rk_nc_get_var_float   nc_get_var_double
#endif

static void getGlobalTextAttribute(char *dst, const char *name, const int ncid) {
    size_t n = 0;
    nc_get_att_text(ncid, NC_GLOBAL, name, dst);
    nc_inq_attlen(ncid, NC_GLOBAL, name, &n);
    dst[n] = 0;
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

int RKProductFileWriterNC(RKProduct *product, char *filename) {
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

    // Open a file
    if ((j = nc_create(product->header.suggestedFilename, NC_MODE, &ncid)) > 0) {
        return RKResultFailedToOpenFileForProduct;
    }
   
    // Some global attributes
    const float zf = 0.0f;
    const float va = 0.25f * product->header.wavelength * product->header.prf[0];
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
        nc_put_att_text(ncid, NC_GLOBAL, "ScanType", 3, "Unknown");
    }
    tmpf = product->header.latitude;
    nc_put_att_float(ncid, NC_GLOBAL, "Latitude", NC_FLOAT, 1, &tmpf);
    nc_put_att_double(ncid, NC_GLOBAL, "LatitudeDouble", NC_DOUBLE, 1, &product->header.latitude);
    tmpf = product->header.longitude;
    nc_put_att_float(ncid, NC_GLOBAL, "Longitude", NC_FLOAT, 1, &tmpf);
    nc_put_att_double(ncid, NC_GLOBAL, "LongitudeDouble", NC_DOUBLE, 1, &product->header.longitude);
    nc_put_att_float(ncid, NC_GLOBAL, "Heading", NC_FLOAT, 1, &product->header.heading);
    nc_put_att_float(ncid, NC_GLOBAL, "Height", NC_FLOAT, 1, &product->header.radarHeight);
    nc_put_att_long(ncid, NC_GLOBAL, "Time", NC_LONG, 1, &product->header.startTime);
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
    tmpi = product->header.prf[0];
    nc_put_att_int(ncid, NC_GLOBAL, "PRF-value", NC_INT, 1, &tmpi);
    put_global_text_att(ncid, "PulseWidth-unit", "MicroSeconds");
    tmpf = (float)product->header.pw[0] * 0.001f;
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
    nc_put_att_float(ncid, NC_GLOBAL, "PCal1-Degrees", floatType, 1, &product->header.PCal[0]);
    nc_put_att_float(ncid, NC_GLOBAL, "PCal2-Degrees", floatType, 1, &product->header.PCal[1]);
    nc_put_att_float(ncid, NC_GLOBAL, "CensorThreshold-dB", floatType, 1, &product->header.SNRThreshold);
    put_global_text_att(ncid, "RadarKit-VCP-Definition", product->header.vcpDefinition);
    put_global_text_att(ncid, "Waveform", product->header.waveform);
    put_global_text_att(ncid, "CreatedBy", "RadarKit v" RKVersionString);
    put_global_text_att(ncid, "ContactInformation", "https://arrc.ou.edu");

    // NetCDF definition ends here
    nc_enddef(ncid);

    // Data
#if RKFloat == float

    nc_put_var_float(ncid, variableIdAzimuth, product->startAzimuth);
    nc_put_var_float(ncid, variableIdElevation, product->startElevation);
    for (j = 0; j < product->header.rayCount; j++) {
        array1D[j] = RKUMinDiff(product->endAzimuth[j], product->startAzimuth[j]);
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
    for (j = 0; j < product->header.rayCount; j++) {
        array1D[j] = RKUMinDiff(product->endAzimuth[j], product->startAzimuth[j]);
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

RKProduct *RKProductFileReaderNC(const char *inputFile) {
    int r;
    int ncid, tmpId;
    float fv, *fp;
    int iv;

    MAKE_FUNCTION_NAME(name);

    RKProduct *product = NULL;
    
    RKName stringValue;

    size_t rayCount = 0;
    size_t gateCount = 0;

    // Early return if the file does not exist
    if (!RKFilenameExists(inputFile)) {
        RKLog("Error. File %s does not exist.\n", inputFile);
        return NULL;
    }


    // Read the first file
    if ((r = nc_open(inputFile, NC_NOWRITE, &ncid)) > 0) {
        RKLog("%s Error opening file %s (%s)\n", name, inputFile, nc_strerror(r));
        return NULL;
    }

    // Some constants
    const nc_type floatType = sizeof(RKFloat) == sizeof(double) ? NC_DOUBLE : NC_FLOAT;
    const RKFloat folded = W2_RANGE_FOLDED;
    const RKFloat missing = W2_MISSING_DATA;

    // Dimensions
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
        nc_close(ncid);
        RKLog("Warning. Early return (rayCount)\n");
        return NULL;
    }
    if ((r = nc_inq_dimid(ncid, "Gate", &tmpId)) != NC_NOERR)
        r = nc_inq_dimid(ncid, "gate", &tmpId);
    if (r == NC_NOERR) {
        nc_inq_dimlen(ncid, tmpId, &gateCount);
    } else {
        RKLog("Warning. Early return (gateCount)\n");
        nc_close(ncid);
        return NULL;
    }

    if (gateCount > RKGateCount) {
        RKLog("Info. gateCount = %d capped to %d\n", gateCount, RKGateCount);
        gateCount = RKGateCount;
    }

    // Now we allocate a product buffer
    RKProductBufferAlloc(&product, 1, (uint32_t)rayCount, (uint32_t)gateCount);
    RKGetSymbolFromFilename(inputFile, product->desc.symbol);

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
    getGlobalTextAttribute(product->header.waveform, "Waveform", ncid);
    getGlobalTextAttribute(product->header.vcpDefinition, "RadarKit-VCP-Definition", ncid);

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
    nc_get_att_long(ncid, NC_GLOBAL, "Time", &product->header.startTime);
    r = nc_get_att_float(ncid, NC_GLOBAL, "Wavelength-unit", &product->header.wavelength);
    if (r != NC_NOERR) {
        product->header.wavelength = 0.0f;
    }
    r = nc_get_att_int(ncid, NC_GLOBAL, "PRF-value", &iv);
    if (r == NC_NOERR) {
        product->header.prf[0] = iv;
        if (product->header.prf[0] == 0) {
            RKLog("Warning. Recorded PRF = 0 Hz.\n");
        }
    } else {
        RKLog("Warning. No PRF information found.\n");
    }
    r = nc_get_att_float(ncid, NC_GLOBAL, "Nyquist_Vel-value", &fv);
    if (r == NC_NOERR && product->header.wavelength == 0.0f && product->header.prf[0] > 0.0f) {
        product->header.wavelength = 4.0f * fv / (RKFloat)product->header.prf[0];
    }
    nc_get_att_int(ncid, NC_GLOBAL, "PulseWidth-value", &iv);
    r = nc_get_att_text(ncid, NC_GLOBAL, "PulseWidth-unit", stringValue);
    if (r == NC_NOERR && !strcasecmp("microseconds", stringValue)) {
        product->header.pw[0] = 1000 * iv;
    }
    get_global_float_att(ncid, floatType, "NoiseH-ADU", &product->header.noise[0]);
    get_global_float_att(ncid, floatType, "NoiseV-ADU", &product->header.noise[1]);
    get_global_float_att(ncid, floatType, "SystemZCalH-dB", &product->header.systemZCal[0]);
    get_global_float_att(ncid, floatType, "SystemZCalV-dB", &product->header.systemZCal[1]);
    get_global_float_att(ncid, floatType, "SystemDCal-Degrees", &product->header.systemDCal);
    get_global_float_att(ncid, floatType, "SystemPCal-Degrees", &product->header.systemPCal);
    get_global_float_att(ncid, floatType, "ZCalH1-dB", &product->header.ZCal[0][0]);
    get_global_float_att(ncid, floatType, "ZCalV1-dB", &product->header.ZCal[1][0]);
    get_global_float_att(ncid, floatType, "ZCalH2-dB", &product->header.ZCal[0][1]);
    get_global_float_att(ncid, floatType, "ZCalV2-dB", &product->header.ZCal[1][1]);
    get_global_float_att(ncid, floatType, "DCal1-dB", &product->header.DCal[0]);
    get_global_float_att(ncid, floatType, "DCal2-dB", &product->header.DCal[1]);
    get_global_float_att(ncid, floatType, "PCal1-Degrees", &product->header.PCal[0]);
    get_global_float_att(ncid, floatType, "PCal2-Degrees", &product->header.PCal[1]);
    get_global_float_att(ncid, floatType, "CensorThreshold-dB", &product->header.SNRThreshold);

    RKLog("%s   (%s%s%s)\n",
          RKVariableInString("filename", inputFile, RKValueTypeString),
          rkGlobalParameters.showColor ? RKYellowColor : "",
          product->desc.symbol,
          rkGlobalParameters.showColor ? RKNoColor : "");
    RKLog("%s   %s   %s\n",
          RKVariableInString("productName", product->desc.name, RKValueTypeString),
          RKVariableInString("colormap", product->desc.colormap, RKValueTypeString),
          RKVariableInString("unit", product->desc.unit, RKValueTypeString));
    RKLog("%s m   %s us",
          RKVariableInString("wavelength", &product->header.wavelength, RKValueTypeFloat),
          RKVariableInString("pulsewidth", &product->header.pw[0], RKValueTypeUInt32));
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
        RKShowArray(product->data, product->desc.symbol, product->header.gateCount, product->header.rayCount);
    }

    nc_close(ncid);
    return product;
}
