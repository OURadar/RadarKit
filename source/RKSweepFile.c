//
//  RKSweepFile.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 6/27/18.
//  Copyright © Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKSweepFile.h>

static void getGlobalTextAttribute(char *dst, const char *name, const int ncid) {
    size_t n = 0;
    nc_get_att_text(ncid, NC_GLOBAL, name, dst);
    nc_inq_attlen(ncid, NC_GLOBAL, name, &n);
    dst[n] = 0;
}

RKSweep *RKSweepFileRead(const char *inputFile) {
    int j, k, r;
    int ncid, tmpId;
    float *fp, fv;
    int iv;

    MAKE_FUNCTION_NAME(name)

    RKName typeName;
    RKName scanType;
    char filename[RKMaximumPathLength];
    memset(filename, 0, RKMaximumPathLength);

    uint32_t firstPartLength = 0;

    uint32_t productList = 0;

    size_t rayCount = 0;
    size_t gateCount = 0;
    uint32_t capacity = 0;
    RKSweep *sweep = NULL;
    RKRay *ray = NULL;

    // A scratch space for netcdf API
    void *scratch = NULL;

    // Try to go through all the sublings to gather a productLiset
    // For now, we only try Z, V, W, D, P, R, K
    // Filename in conventions of RADAR-20180101-010203-EL10.2-Z.nc

    // Find the last '.'
    char *e = NULL;
    e = strstr(inputFile, ".");
    if (e == NULL) {
        e = (char *)inputFile + strlen(inputFile) - 1;
    }
    while (*(e + 1) >= '0' && *(e + 1) <= '9') {
        e = strstr(e + 1, ".");
    }
    // Find the previous '-'
    char *b = e;
    while (b != inputFile && *b != '-') {
        b--;
    }
    if (b == inputFile) {
        RKLog("%s Unable to find product symbol.\n", name);
        return NULL;
    }

    b++;
    firstPartLength = (uint32_t)(b - inputFile);
    char symbol[RKMaximumSymbolLength];
    memset(symbol, 0, RKMaximumSymbolLength);
    strncpy(symbol, b, MIN(RKMaximumSymbolLength, e - b));
    // Substitute symbol with the ones I know
    char symbols[][RKNameLength] = {"Z", "V", "W", "D", "P", "R", "K"};
    RKName productNames[] = {
        "Intensity",
        "Radial_Velocity",
        "Width",
        "Differential_Reflectivity",
        "PhiDP",
        "RhoHV",
        "KDP"
    };
    uint32_t products[] = {
        RKBaseMomentListProductZ,
        RKBaseMomentListProductV,
        RKBaseMomentListProductW,
        RKBaseMomentListProductD,
        RKBaseMomentListProductP,
        RKBaseMomentListProductR,
        RKBaseMomentListProductK
    };
    uint32_t productIndices[] = {
        RKBaseMomentIndexZ,
        RKBaseMomentIndexV,
        RKBaseMomentIndexW,
        RKBaseMomentIndexD,
        RKBaseMomentIndexP,
        RKBaseMomentIndexR,
        RKBaseMomentIndexK
    };

    // First part: go through all the symbols I know of, get the very first filename
    for (k = 0; k < sizeof(symbols) / RKNameLength; k++) {
        b = symbols[k];
        strncpy(filename, inputFile, firstPartLength);
        snprintf(filename + firstPartLength, RKMaximumPathLength - firstPartLength, "%s%s", b, e);
        // Read in the header from the very first file that exists
        if (RKFilenameExists(filename)) {
            // Read the first file
            if ((r = nc_open(filename, NC_NOWRITE, &ncid)) > 0) {
                RKLog("%s Error opening file %s (%s)\n", name, inputFile, nc_strerror(r));
                return NULL;
            }
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

            if (gateCount > RKMaximumGateCount) {
                RKLog("Info. gateCount = %d capped to %d\n", gateCount, RKMaximumGateCount);
                gateCount = RKMaximumGateCount;
            }

            // Derive the RKSIMDAlignSize compliant capacity
            capacity = (uint32_t)ceilf((float)gateCount / RKSIMDAlignSize) * RKSIMDAlignSize;

            RKLog("rayCount = %s   gateCount = %s   capacity = %s\n",
                  RKIntegerToCommaStyleString(rayCount), RKIntegerToCommaStyleString(gateCount), RKIntegerToCommaStyleString(capacity));

            // A scratch space for netcdf API
            scratch = (void *)malloc(rayCount * capacity * sizeof(float));

            // Allocate the return object
            sweep = (RKSweep *)malloc(sizeof(RKSweep));
            if (sweep == NULL) {
                RKLog("Error. Unable to allocate memory.\n");
                return NULL;
            }
            memset(sweep, 0, sizeof(RKSweep));
            RKRayBufferAlloc(&sweep->rayBuffer, (uint32_t)capacity, (uint32_t)rayCount);
            for (j = 0; j < rayCount; j++) {
                sweep->rays[j] = RKGetRayFromBuffer(sweep->rayBuffer, j);
            }
            ray = (RKRay *)sweep->rayBuffer;

            // Global attributes
            getGlobalTextAttribute(typeName, "TypeName", ncid);
            getGlobalTextAttribute(scanType, "ScanType", ncid);
            getGlobalTextAttribute(sweep->header.desc.name, "radarName-value", ncid);
            r = nc_get_att_double(ncid, NC_GLOBAL, "LatitudeDouble", &sweep->header.desc.latitude);
            if (r != NC_NOERR) {
                r = nc_get_att_float(ncid, NC_GLOBAL, "Latitude", &fv);
                if (r == NC_NOERR) {
                    sweep->header.desc.latitude = (double)fv;
                }
            }
            r = nc_get_att(ncid, NC_GLOBAL, "LongitudeDouble", &sweep->header.desc.longitude);
            if (r != NC_NOERR) {
                r = nc_get_att_float(ncid, NC_GLOBAL, "Longitude", &fv);
                if (r == NC_NOERR) {
                    sweep->header.desc.longitude = (double)fv;
                }
            }
            r = nc_get_att_float(ncid, NC_GLOBAL, "Heading", &sweep->header.desc.heading);
            if (r != NC_NOERR) {
                RKLog("No radar heading found.\n");
            }
            r = nc_get_att_float(ncid, NC_GLOBAL, "Height", &sweep->header.desc.radarHeight);
            if (r != NC_NOERR) {
                RKLog("No radar height found.\n");
            }
            r = nc_get_att_float(ncid, NC_GLOBAL, "Elevation", &ray->header.sweepElevation);
            if (r != NC_NOERR && sweep->header.config.startMarker & RKMarkerScanTypePPI) {
                RKLog("Warning. No sweep elevation found.\n");
            }
            r = nc_get_att_float(ncid, NC_GLOBAL, "Azimuth", &ray->header.sweepAzimuth);
            if (r != NC_NOERR && sweep->header.config.startMarker & RKMarkerScanTypeRHI) {
                RKLog("Warning. No sweep azimuth found.\n");
            }
            if (!strcmp(scanType, "PPI")) {
                sweep->header.config.sweepElevation = ray->header.sweepElevation;
                sweep->header.config.startMarker |= RKMarkerScanTypePPI;
            } else if (!strcmp(scanType, "RHI")) {
                sweep->header.config.sweepAzimuth = ray->header.sweepAzimuth;
                sweep->header.config.startMarker |= RKMarkerScanTypeRHI;
            }
            r = nc_get_att_int(ncid, NC_GLOBAL, "PRF-value", &iv);
            if (r == NC_NOERR) {
                sweep->header.config.prf[0] = iv;
                if (sweep->header.config.prf[0] == 0) {
                    RKLog("Warning. Recorded PRF = 0 Hz.\n");
                }
            } else {
                RKLog("Warning. No PRF information found.\n");
            }
            r = nc_get_att_float(ncid, NC_GLOBAL, "Nyquist_Vel-value", &fv);
            if (r == NC_NOERR) {
                sweep->header.desc.wavelength = 4.0f * fv / (RKFloat)sweep->header.config.prf[0];
                RKLog("Radar wavelength = %.4f m\n", sweep->header.desc.wavelength);
            }

            // Elevation array
            if ((r = nc_inq_varid(ncid, "Elevation", &tmpId)) != NC_NOERR) {
                r = nc_inq_varid(ncid, "elevation", &tmpId);
            }
            if (r == NC_NOERR) {
                nc_get_var_float(ncid, tmpId, scratch);
                fp = (float *)scratch;
                for (j = 0; j < rayCount; j++) {
                    ray = RKGetRayFromBuffer(sweep->rayBuffer, j);
                    ray->header.startElevation = *fp++;
                    ray->header.endElevation = ray->header.startElevation;
                }
            } else {
                RKLog("Warning. No elevation array.\n");
            }

            // Azimuth array
            if ((r = nc_inq_varid(ncid, "Azimuth", &tmpId)) != NC_NOERR) {
                r = nc_inq_varid(ncid, "azimuth", &tmpId);
            }
            if (r == NC_NOERR) {
                nc_get_var_float(ncid, tmpId, scratch);
                fp = (float *)scratch;
                for (j = 0; j < rayCount; j++) {
                    ray = RKGetRayFromBuffer(sweep->rayBuffer, j);
                    ray->header.startAzimuth = *fp++;
                }
            } else {
                RKLog("Warning. No azimuth array.\n");
            }

            // Gatewidth array (this is here for historical reasons)
            if ((r = nc_inq_varid(ncid, "GateWidth", &tmpId)) != NC_NOERR) {
                if ((r = nc_inq_varid(ncid, "Gatewidth", &tmpId)) != NC_NOERR) {
                    r = nc_inq_varid(ncid, "gatewidth", &tmpId);
                }
            }
            if (r == NC_NOERR) {
                nc_get_var_float(ncid, tmpId, scratch);
                fp = (float *)scratch;
                for (j = 0; j < rayCount; j++) {
                    ray = RKGetRayFromBuffer(sweep->rayBuffer, j);
                    ray->header.gateCount = gateCount;
                    ray->header.gateSizeMeters = *fp++;
                }
            } else {
                RKLog("Warning. No gatewidth array.\n");
            }

            // Beamwidth array (this is here for historical reasons)
            if ((r = nc_inq_varid(ncid, "BeamWidth", &tmpId)) != NC_NOERR) {
                if ((r = nc_inq_varid(ncid, "Beamwidth", &tmpId)) != NC_NOERR) {
                    r = nc_inq_varid(ncid, "beamwidth", &tmpId);
                }
            }
            if (r == NC_NOERR) {
                nc_get_var_float(ncid, tmpId, scratch);
                fp = (float *)scratch;
                for (j = 0; j < rayCount; j++) {
                    ray = RKGetRayFromBuffer(sweep->rayBuffer, j);
                    ray->header.endAzimuth = ray->header.startAzimuth + *fp;
                    ray->header.endElevation = ray->header.endElevation + *fp++;
                }
            } else {
                RKLog("Warning. No beamwidth array.\n");
            }

            // Be a good citizen, close the file
            nc_close(ncid);
            break;
        }
    }

    // If none of the files exist, sweep is NULL. There is no point continuing
    if (sweep == NULL) {
        RKLog("%s Inconsistent state.\n", name);
        return NULL;
    }

    // Second pass: go through all the symbols I know of and actually read in the data
    const float w2_missing_data = W2_MISSING_DATA;
    const float w2_range_folded = W2_RANGE_FOLDED;
    for (k = 0; k < sizeof(symbols) / RKNameLength; k++) {
        b = symbols[k];
        strncpy(filename, inputFile, firstPartLength);
        snprintf(filename + firstPartLength, RKMaximumPathLength - firstPartLength, "%s%s", b, e);
        if (!RKFilenameExists(filename)) {
            RKLog("%s not found.\n", filename);
            continue;
        }
        productList |= products[k];

        // Open the file
        if ((r = nc_open(filename, NC_NOWRITE, &ncid)) > 0) {
            RKLog("%s Error opening file %s (%s)\n", name, inputFile, nc_strerror(r));
            free(scratch);
            return NULL;
        }

        RKLog("%s %s (*)\n", name, filename);

        // Product
        r = nc_inq_varid(ncid, productNames[k], &tmpId);
        if (r != NC_NOERR) {
            RKLog("Base moment %s not found.\n", productNames[k]);
            continue;
        }
        nc_get_var_float(ncid, tmpId, scratch);
        fp = (float *)scratch;
        for (j = 0; j < rayCount * gateCount; j++) {
            if (*fp == w2_missing_data || *fp == w2_range_folded) {
                *fp = NAN;
            }
            fp++;
        }
        for (j = 0; j < rayCount; j++) {
            ray = RKGetRayFromBuffer(sweep->rayBuffer, j);
            fp = RKGetFloatDataFromRay(ray, productIndices[k]);
            memcpy(fp, scratch + j * gateCount * sizeof(float), gateCount * sizeof(float));
        }
        nc_close(ncid);
    }

    // We are done with the scratch space at this point, we can free it
    free(scratch);

    // This really should not happen
    if (sweep == NULL) {
        RKLog("%s Inconsistent state towards the end.\n", name);
        return NULL;
    }

    ray = RKGetRayFromBuffer(sweep->rayBuffer, 0);

    sweep->header.rayCount = (uint32_t)rayCount;
    sweep->header.gateCount = (uint32_t)gateCount;
    sweep->header.gateSizeMeters = ray->header.gateSizeMeters;
    sweep->header.baseMomentList = productList;

    for (j = 0; j < rayCount; j++) {
        ray = RKGetRayFromBuffer(sweep->rayBuffer, j);
        ray->header.i += sweep->header.rayCount;
        ray->header.s = RKRayStatusReady;
        ray->header.baseMomentList = productList;
    }

    /*
     RKLog("  -> %s%s%s%s%s%s%s\n",
     productList & RKBaseMomentListProductZ ? "Z" : "",
     productList & RKBaseMomentListProductV ? "V" : "",
     productList & RKBaseMomentListProductW ? "W" : "",
     productList & RKBaseMomentListProductD ? "D" : "",
     productList & RKBaseMomentListProductP ? "P" : "",
     productList & RKBaseMomentListProductR ? "R" : "",
     productList & RKBaseMomentListProductK ? "K" : ""
     );
     */

    return sweep;
}
