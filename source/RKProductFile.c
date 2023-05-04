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

    // Open a file
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
    put_global_text_att(ncid, "CreatedBy", "RadarKit v" __RKVersionString__);
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

void RKProductReadFileIntoBuffer(RKProduct *product, const char *filename, const bool showInfo) {
    int r;
    int ncid, tmpId;
    float fv, *fp;
    int iv;

    MAKE_FUNCTION_NAME(name);

    size_t rayCount = 0;
    size_t gateCount = 0;
    RKName stringValue;

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

    // Some constants
    const nc_type floatType = sizeof(RKFloat) == sizeof(double) ? NC_DOUBLE : NC_FLOAT;
    const RKFloat folded = W2_RANGE_FOLDED;
    const RKFloat missing = W2_MISSING_DATA;

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
        nc_close(ncid);
        RKLog("Warning. Early return (rayCount)\n");
        return;
    }
    if ((r = nc_inq_dimid(ncid, "Gate", &tmpId)) != NC_NOERR)
    r = nc_inq_dimid(ncid, "gate", &tmpId);
    if (r == NC_NOERR) {
        nc_inq_dimlen(ncid, tmpId, &gateCount);
    } else {
        RKLog("Warning. Early return (gateCount)\n");
        nc_close(ncid);
        return;
    }
    if (gateCount > RKMaximumGateCount) {
        RKLog("Info. gateCount = %d capped to %d\n", gateCount, RKMaximumGateCount);
        gateCount = RKMaximumGateCount;
    }

    // Double check the dimensions
    if (product->header.rayCount != (uint32_t)rayCount || product->header.gateCount != (uint32_t)gateCount) {
        RKLog("Warning. Inconsistent dimentions: %u x %u  vs  %u x %u\n",
              (uint32_t)rayCount, (uint32_t)gateCount, product->header.rayCount, product->header.gateCount);
    }
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
    if (r != NC_NOERR && showInfo) {
        RKLog("No radar heading found.\n");
    }
    r = nc_get_att_float(ncid, NC_GLOBAL, "Height", &product->header.radarHeight);
    if (r != NC_NOERR && showInfo) {
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
    nc_get_att_int(ncid, NC_GLOBAL, "PulseWidth-value", &iv);
    r = nc_get_att_text(ncid, NC_GLOBAL, "PulseWidth-unit", stringValue);
    if (r == NC_NOERR && !strcasecmp("microseconds", stringValue)) {
        product->header.pw[0] = 1.0e-3 * (float)iv;
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

    if (showInfo) {
        RKLog("%s   (%s%s%s)\n",
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
    }

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
        if (showInfo) {
            RKShowArray(product->data, product->desc.symbol, product->header.gateCount, product->header.rayCount);
        }
    }

    nc_close(ncid);
}

RKProduct *RKProductFileReaderNC(const char *inputFile, const bool showInfo) {
    uint32_t rayCount = 0;
    uint32_t gateCount = 0;
    RKProduct *product = NULL;

    RKProductDimensionsFromFile(inputFile, &rayCount, &gateCount);

    RKProductBufferAlloc(&product, 1, rayCount, gateCount);

    RKProductReadFileIntoBuffer(product, inputFile, showInfo);

    return product;
}

RKProductCollection *RKProductCollectionInitWithFilename(const char *firstFilename) {
    int k;
    char list[16][RKMaximumPathLength];

    // Collect a list of product files
    RKProductCollection *productCollection = (RKProductCollection *)malloc(sizeof(RKProductCollection));
    if (productCollection == NULL) {
        fprintf(stderr, "RKProductCollectionInitWithFilename() Failed to allocate memory.\n");
        return NULL;
    }
    productCollection->count = RKListFilesWithSamePrefix(firstFilename, list);

    uint32_t rayCount = 0;
    uint32_t gateCount = 0;

    if (productCollection->count == 0 && RKFilenameExists(firstFilename)) {
        productCollection->count = 1;
        RKProductDimensionsFromFile(firstFilename, &rayCount, &gateCount);
        RKLog("Found 1 product   rayCount = %u   gateCount = %u\n", rayCount, gateCount);
    } else if (productCollection->count > 0) {
        RKProductDimensionsFromFile(firstFilename, &rayCount, &gateCount);
        RKLog("Found %d products   rayCount = %u   gateCount = %u\n", productCollection->count, rayCount, gateCount);
    } else {
        RKLog("RKProductCollectionInitWithFilename() Inconsistent state. Returning ...\n");
        free(productCollection);
        return NULL;
    }

    RKProductBufferAlloc(&productCollection->products, productCollection->count, rayCount, gateCount);
    for (k = 0; k < productCollection->count; k++) {
        if (strlen(list[k]) > 40) {
            RKLog("RKProductCollectionInitWithFilename() ...%s\n", RKLastNPartsOfPath(list[k], 3));
        } else {
            RKLog("RKProductCollectionInitWithFilename() %s\n", list[k]);
        }
        RKProductReadFileIntoBuffer(&productCollection->products[k], list[k], false);
    }

    return productCollection;
}

void RKProductCollectionFree(RKProductCollection *collection) {
    RKProductBufferFree(collection->products, collection->count);
    free(collection);
}
