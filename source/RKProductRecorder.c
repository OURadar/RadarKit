//
//  RKProductRecorder.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 6/24/18.
//  Copyright © Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKProductRecorder.h>

#define W2_MISSING_DATA       -99900.0
#define W2_RANGE_FOLDED       -99901.0

#if defined (COMPRESSED_NETCDF)
#define NC_MODE  NC_NETCDF4
#else
#define NC_MODE  NC_CLOBBER
#endif

static int put_global_text_att(const int ncid, const char *att, const char *text) {
    return nc_put_att_text(ncid, NC_GLOBAL, att, strlen(text), text);
}

int RKProductRecorderNCWriter(RKProduct *product, char *filename) {
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
   
    // Local memory
    float *array1D = (float *)malloc(MAX(product->header.rayCount, product->header.gateCount));

    // Some global attributes
    const float zf = 0.0f;
    const float va = 0.25f * product->header.wavelength * product->header.prf[0];
    const float radianToDegree = 180.0f / M_PI;
    const bool convertRadiansToDegrees = !strcasecmp(product->desc.unit, "radians");
    const nc_type floatType = sizeof(RKFloat) == sizeof(double) ? NC_DOUBLE : NC_FLOAT;
    if (convertRadiansToDegrees) {
        x =  product->data;
        for (j = 0; j < product->header.gateCount * product->header.rayCount; j++) {
            *x = *x * radianToDegree;
            x++;
        }
        sprintf(product->desc.unit, "Degrees");
    }
    
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
    put_global_text_att(ncid, "attributes", "Nyquist_Vel Unit radarName vcp ColorMap");
    put_global_text_att(ncid, "Nyquist_Vel-unit", "MetersPerSecond");
    nc_put_att_float(ncid, NC_GLOBAL, "Nyquist_Vel-value", NC_FLOAT, 1, &va);
    put_global_text_att(ncid, "Unit-unit", "dimensionless");
    put_global_text_att(ncid, "Unit-value", product->desc.unit);
    put_global_text_att(ncid, "radarName-unit", "dimensionless");
    put_global_text_att(ncid, "radarName-value", product->header.radarName);
    put_global_text_att(ncid, "vcp-unit", "dimensionless");
    put_global_text_att(ncid, "vcp-value", "1");

    // WDSS-II auxiliary
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
    put_global_text_att(ncid, "ProcessParameters", "Noise Calib Censor");
    tmpf = 20.0f * log10f(product->header.noise[0]);
    nc_put_att_float(ncid, NC_GLOBAL, "NoiseH-dB-ADU", NC_FLOAT, 1, &tmpf);
    tmpf = 20.0f * log10f(product->header.noise[1]);
    nc_put_att_float(ncid, NC_GLOBAL, "NoiseV-dB-ADU", NC_FLOAT, 1, &tmpf);
    nc_put_att_float(ncid, NC_GLOBAL, "SystemZCalH-dB", floatType, 1, &product->header.systemZCal[0]);
    nc_put_att_float(ncid, NC_GLOBAL, "SystemZCalV-dB", floatType, 1, &product->header.systemZCal[1]);
    nc_put_att_float(ncid, NC_GLOBAL, "SystemDCal-Degrees", floatType, 1, &product->header.systemDCal);
    nc_put_att_float(ncid, NC_GLOBAL, "SystemPCal-Degrees", floatType, 1, &product->header.systemPCal);
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
    put_global_text_att(ncid, "Radar", product->header.radarName);
    put_global_text_att(ncid, "CreatedBy", "RadarKit v" RKVersionString);
    put_global_text_att(ncid, "ContactInformation", "https://arrc.ou.edu");

    // NetCDF definition ends here
    nc_enddef(ncid);

    // Data
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
    
    ncclose(ncid);
    
    free(array1D);

    return RKResultSuccess;
}
