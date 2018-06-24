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

//static int put_global_text_att(const int ncid, const char *att, const char *text) {
//    return nc_put_att_text(ncid, NC_GLOBAL, att, strlen(text), text);
//}

int RKProductRecorderNetCDF(const RKProduct *product, char *filename) {
    int j;
    int ncid;
    
    if ((j = nc_create(product->header.suggestedFilename, NC_MODE, &ncid)) > 0) {
        return RKResultFailedToOpenFileForProduct;
    }
    
//    if (sweep->header.isPPI) {
//        nc_def_dim(ncid, "Azimuth", sweep->header.rayCount, &dimensionIds[0]);
//    } else if (sweep->header.isRHI) {
//        nc_def_dim(ncid, "Elevation", sweep->header.rayCount, &dimensionIds[0]);
//    } else {
//        nc_def_dim(ncid, "Beam", sweep->header.rayCount, &dimensionIds[0]);
//    }
//    nc_def_dim(ncid, "Gate", sweep->header.gateCount, &dimensionIds[1]);
//
//    // Define variables
//    nc_def_var(ncid, "Azimuth", NC_FLOAT, 1, dimensionIds, &variableIdAzimuth);
//    nc_def_var(ncid, "Elevation", NC_FLOAT, 1, dimensionIds, &variableIdElevation);
//    nc_def_var(ncid, "Beamwidth", NC_FLOAT, 1, dimensionIds, &variableIdBeamwidth);
//    nc_def_var(ncid, "GateWidth", NC_FLOAT, 1, dimensionIds, &variableIdGateWidth);
//    nc_def_var(ncid, name, NC_FLOAT, 2, dimensionIds, &variableIdData);
//
//    nc_put_att_text(ncid, variableIdAzimuth, "Units", 7, "Degrees");
//    nc_put_att_text(ncid, variableIdElevation, "Units", 7, "Degrees");
//    nc_put_att_text(ncid, variableIdBeamwidth, "Units", 7, "Degrees");
//    nc_put_att_text(ncid, variableIdGateWidth, "Units", 6, "Meters");
//    nc_put_att_text(ncid, variableIdData, "Units", strlen(unit), unit);
//
//#if defined (COMPRESSED_NETCDF)
//
//    nc_def_var_deflate(ncid, variableIdAzimuth, 1, 1, 3);
//    nc_def_var_deflate(ncid, variableIdElevation, 1, 1, 3);
//    nc_def_var_deflate(ncid, variableIdBeamwidth, 1, 1, 3);
//    nc_def_var_deflate(ncid, variableIdGateWidth, 1, 1, 3);
//    nc_def_var_deflate(ncid, variableIdData, 1, 1, 3);
//
//#endif
//
//    // Global attributes - some are WDSS-II required
//    nc_put_att_text(ncid, NC_GLOBAL, "TypeName", strlen(name), name);
//    nc_put_att_text(ncid, NC_GLOBAL, "DataType", 9, "RadialSet");
//    if (sweep->header.isPPI) {
//        nc_put_att_text(ncid, NC_GLOBAL, "ScanType", 3, "PPI");
//    } else if (sweep->header.isRHI) {
//        nc_put_att_text(ncid, NC_GLOBAL, "ScanType", 3, "RHI");
//    } else {
//        nc_put_att_text(ncid, NC_GLOBAL, "ScanType", 3, "Unknown");
//    }
//    tmpf = engine->radarDescription->latitude;
//    nc_put_att_float(ncid, NC_GLOBAL, "Latitude", NC_FLOAT, 1, &tmpf);
//    nc_put_att_double(ncid, NC_GLOBAL, "LatitudeDouble", NC_DOUBLE, 1, &engine->radarDescription->latitude);
//    tmpf = engine->radarDescription->longitude;
//    nc_put_att_float(ncid, NC_GLOBAL, "Longitude", NC_FLOAT, 1, &tmpf);
//    nc_put_att_double(ncid, NC_GLOBAL, "LongitudeDouble", NC_DOUBLE, 1, &engine->radarDescription->longitude);
//    nc_put_att_float(ncid, NC_GLOBAL, "Heading", NC_FLOAT, 1, &engine->radarDescription->heading);
//    nc_put_att_float(ncid, NC_GLOBAL, "Height", NC_FLOAT, 1, &engine->radarDescription->radarHeight);
//    nc_put_att_long(ncid, NC_GLOBAL, "Time", NC_LONG, 1, &sweep->header.startTime);
//    tmpf = (float)sweep->rays[0]->header.startTime.tv_usec * 1.0e-6;
//    nc_put_att_float(ncid, NC_GLOBAL, "FractionalTime", NC_FLOAT, 1, &tmpf);
//    put_global_text_att(ncid, "attributes", "Nyquist_Vel Unit radarName vcp ColorMap");
//    put_global_text_att(ncid, "Nyquist_Vel-unit", "MetersPerSecond");
//    nc_put_att_float(ncid, NC_GLOBAL, "Nyquist_Vel-value", NC_FLOAT, 1, &va);
//    put_global_text_att(ncid, "Unit-unit", "dimensionless");
//    put_global_text_att(ncid, "Unit-value", unit);
//    put_global_text_att(ncid, "radarName-unit", "dimensionless");
//    put_global_text_att(ncid, "radarName-value", engine->radarDescription->name);
//    put_global_text_att(ncid, "vcp-unit", "dimensionless");
//    put_global_text_att(ncid, "vcp-value", "1");
//
//    // WDSS-II auxiliary
//    put_global_text_att(ncid, "ColorMap-unit", "dimensionless");
//    put_global_text_att(ncid, "ColorMap-value", colormap);
//
//    // Other housekeeping attributes
//    if (sweep->header.isPPI) {
//        nc_put_att_float(ncid, NC_GLOBAL, "Elevation", NC_FLOAT, 1, &sweep->header.config.sweepElevation);
//    } else {
//        tmpf = W2_MISSING_DATA;
//        nc_put_att_float(ncid, NC_GLOBAL, "Elevation", NC_FLOAT, 1, &tmpf);
//    }
//    put_global_text_att(ncid, "ElevationUnits", "Degrees");
//    if (sweep->header.isRHI) {
//        nc_put_att_float(ncid, NC_GLOBAL, "Azimuth", NC_FLOAT, 1, &sweep->header.config.sweepAzimuth);
//    } else {
//        tmpf = W2_MISSING_DATA;
//        nc_put_att_float(ncid, NC_GLOBAL, "Azimuth", NC_FLOAT, 1, &tmpf);
//    }
//    put_global_text_att(ncid, "AzimuthUnits", "Degrees");
//    tmpf = 0.0f;
//    nc_put_att_float(ncid, NC_GLOBAL, "RangeToFirstGate", NC_FLOAT, 1, &tmpf);
//    put_global_text_att(ncid, "RangeToFirstGateUnits", "Meters");
//    tmpf = W2_MISSING_DATA;
//    nc_put_att_float(ncid, NC_GLOBAL, "MissingData", NC_FLOAT, 1, &tmpf);
//    tmpf = W2_RANGE_FOLDED;
//    nc_put_att_float(ncid, NC_GLOBAL, "RangeFolded", NC_FLOAT, 1, &tmpf);
//    put_global_text_att(ncid, "RadarParameters", "PRF PulseWidth MaximumRange");
//    put_global_text_att(ncid, "PRF-unit", "Hertz");
//    tmpi = sweep->header.config.prf[0];
//    nc_put_att_int(ncid, NC_GLOBAL, "PRF-value", NC_INT, 1, &tmpi);
//    put_global_text_att(ncid, "PulseWidth-unit", "MicroSeconds");
//    tmpf = (float)sweep->header.config.pw[0] * 0.001f;
//    nc_put_att_float(ncid, NC_GLOBAL, "PulseWidth-value", NC_FLOAT, 1, &tmpf);
//    put_global_text_att(ncid, "MaximumRange-unit", "KiloMeters");
//    tmpf = 1.0e-3f * sweep->rays[0]->header.gateSizeMeters * sweep->header.gateCount;
//    nc_put_att_float(ncid, NC_GLOBAL, "MaximumRange-value", NC_FLOAT, 1, &tmpf);
//    put_global_text_att(ncid, "ProcessParameters", "Noise Calib Censor");
//    tmpf = 20.0f * log10f(sweep->header.config.noise[0]);
//    put_global_text_att(ncid, "NoiseH-unit", "dB-ADU");
//    nc_put_att_float(ncid, NC_GLOBAL, "NoiseH-value", NC_FLOAT, 1, &tmpf);
//    tmpf = 20.0f * log10f(sweep->header.config.noise[1]);
//    put_global_text_att(ncid, "NoiseV-unit", "dB-ADU");
//    nc_put_att_float(ncid, NC_GLOBAL, "NoiseV-value", NC_FLOAT, 1, &tmpf);
//    put_global_text_att(ncid, "CalibH-unit", "dB");
//    nc_put_att_float(ncid, NC_GLOBAL, "CalibH-value", NC_FLOAT, 1, &sweep->header.config.ZCal[0][0]);
//    put_global_text_att(ncid, "CalibV-unit", "dB");
//    nc_put_att_float(ncid, NC_GLOBAL, "CalibV-value", NC_FLOAT, 1, &sweep->header.config.ZCal[1][0]);
//    put_global_text_att(ncid, "CalibD1-unit", "dB");
//    nc_put_att_float(ncid, NC_GLOBAL, "CalibD1-value", NC_FLOAT, 1, &sweep->header.config.DCal[0]);
//    put_global_text_att(ncid, "CalibD2-unit", "dB");
//    nc_put_att_float(ncid, NC_GLOBAL, "CalibD2-value", NC_FLOAT, 1, &sweep->header.config.DCal[1]);
//    put_global_text_att(ncid, "CalibP1-unit", "Degrees");
//    nc_put_att_float(ncid, NC_GLOBAL, "CalibP1-value", NC_FLOAT, 1, &sweep->header.config.PCal[0]);
//    put_global_text_att(ncid, "CalibP2-unit", "Degrees");
//    nc_put_att_float(ncid, NC_GLOBAL, "CalibP2-value", NC_FLOAT, 1, &sweep->header.config.PCal[1]);
//    put_global_text_att(ncid, "CensorThreshold-unit", "dB");
//    nc_put_att_float(ncid, NC_GLOBAL, "CensorThreshold-value", NC_FLOAT, 1, &sweep->header.config.SNRThreshold);
//    put_global_text_att(ncid, "Waveform", sweep->header.config.waveform);
//    put_global_text_att(ncid, "CreatedBy", "RadarKit");
//    put_global_text_att(ncid, "ContactInformation", "http://arrc.ou.edu");
//
//    // NetCDF definition ends here
//    nc_enddef(ncid);
//
//    // Data
//    for (j = 0; j < sweep->header.rayCount; j++) {
//        array1D[j] = sweep->rays[j]->header.startAzimuth;
//    }
//    nc_put_var_float(ncid, variableIdAzimuth, array1D);
//    for (j = 0; j < sweep->header.rayCount; j++) {
//        array1D[j] = sweep->rays[j]->header.startElevation;
//    }
//    nc_put_var_float(ncid, variableIdElevation, array1D);
//    for (j = 0; j < sweep->header.rayCount; j++) {
//        array1D[j] = RKUMinDiff(sweep->rays[j]->header.endAzimuth, sweep->rays[j]->header.startAzimuth);
//    }
//    nc_put_var_float(ncid, variableIdBeamwidth, array1D);
//    for (j = 0; j < sweep->header.rayCount; j++) {
//        array1D[j] = sweep->rays[j]->header.gateSizeMeters;
//    }
//    nc_put_var_float(ncid, variableIdGateWidth, array1D);
//
//    y = array2D;
//    // Should AND it with a user preference
//    convertRadiansToDegrees = momentIndex == RKBaseMomentIndexP || momentIndex == RKBaseMomentIndexK;
//    for (j = 0; j < sweep->header.rayCount; j++) {
//        x = RKGetFloatDataFromRay(sweep->rays[j], momentIndex);
//        if (convertRadiansToDegrees) {
//            for (i = 0; i < sweep->rays[0]->header.gateCount; i++) {
//                if (isfinite(*x)) {
//                    *y++ = *x * radianToDegree;
//                } else {
//                    *y++ = W2_MISSING_DATA;
//                }
//                x++;
//            }
//        } else {
//            for (i = 0; i < sweep->rays[0]->header.gateCount; i++) {
//                if (isfinite(*x)) {
//                    *y++ = *x;
//                } else {
//                    *y++ = W2_MISSING_DATA;
//                }
//                x++;
//            }
//        }
//    }
//    nc_put_var_float(ncid, variableIdData, array2D);
    
    ncclose(ncid);
    
    return RKResultSuccess;
}
