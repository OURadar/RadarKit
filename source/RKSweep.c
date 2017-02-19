//
//  RKSweep.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 1/15/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKSweep.h>

#define W2_MISSING_DATA       -99900.0
#define W2_RANGE_FOLDED       -99901.0

#pragma mark - Helper Functions

static int put_global_text_att(const int ncid, const char *att, const char *text) {
    return nc_put_att_text(ncid, NC_GLOBAL, att, strlen(text), text);
}

void *sweepWriter(void *in) {
    RKSweepEngine *engine = (RKSweepEngine *)in;
    
    int i, j, k, p;
    uint32_t n = engine->sweep.rayCount;
    RKRay **rays = engine->sweep.rays;
    //RKLog(">%s %p %p %p ... %p\n", engine->name, rays[0], rays[1], rays[2], rays[n - 1]);

    RKRay *S = rays[0];
    RKRay *T = rays[1];
    RKRay *E = rays[n - 1];
    RKConfig *config = &engine->configBuffer[S->header.configIndex];
    RKRadarDesc *desc = engine->radarDescription;
    
    RKLog("%s C%02d-%02d-%02d  E%5.2f/%5.2f-%5.2f   A%6.2f-%6.2f   M%03x-%03x-%03x   (%d)\n",
          engine->name,
          S->header.configIndex    , T->header.configIndex    , E->header.configIndex,
          config->sweepElevation   ,
          S->header.startElevation , E->header.endElevation   ,
          S->header.startAzimuth   , E->header.endAzimuth     ,
          S->header.marker & 0xFFFF, T->header.marker & 0xFFFF, E->header.marker & 0xFFFF,
          n);

    // Mark the state
    engine->state |= RKEngineStateWritingFile;

    k = 0;
    if (n > 360) {
        if (S->header.marker & RKMarkerSweepBegin) {
            // 361 beams and start at 0, we discard the extra last beam
            n = 360;
            k = 0;
        } else if (T->header.marker & RKMarkerSweepBegin) {
            // 361 beams but start at 1, we discard the extra first beam
            n = 360;
            k = 1;
        }
    }
    S = rays[k];
    T = rays[k + 1];
    E = rays[k + n - 1];
    config = &engine->configBuffer[T->header.configIndex];
    RKLog(">%s    C%02d     E%5.2f/%5.2f-%5.2f   A%6.2f-%6.2f   M%03x-%03x       (%s%d%s)\n",
          engine->name,
          T->header.configIndex,
          config->sweepElevation,
          S->header.startElevation , E->header.endElevation,
          S->header.startAzimuth   , E->header.endAzimuth,
          S->header.marker & 0xFFFF, E->header.marker & 0xFFFF,
          n != 360 ? RKGetColorOfIndex(1) : "",
          n,
          RKNoColor);

//    RKRadarDesc *radar = engine->radarDescription;
//    RKLog("%s %s @ %.7f %.7f\n", engine->name,
//          radar->name, radar->latitude, radar->longitude);
    
    char symbol = 'U';
    char filename[RKMaximumStringLength];
    char productName[RKMaximumStringLength];
    char productUnit[RKMaximumStringLength];
    char productColormap[RKMaximumStringLength];
    
    int ncid;
    int dimensionIds[2];
    int variableIdAzimuth;
    int variableIdElevation;
    int variableIdBeamwidth;
    int variableIdGateWidth;
    int variableIdData;
    
    int tmpi;
    float tmpf;
    float *array1D = engine->array1D;
    float *array2D = engine->array2D;
    float *x;
    float *y;

    // Some global attributes
    time_t startTime = (time_t)S->header.startTime.tv_sec;
    float va = 0.25f * desc->wavelength * config->prf[0];

    // Go through all moments
    RKProductIndex productIndex = RKProductIndexS;
    uint32_t productList = S->header.productList;
    int productCount = __builtin_popcount(productList);
    for (p = 0; p < productCount; p++) {
        if (productList & RKProductListProductZ) {
            symbol = 'Z';
            sprintf(productName, "Corrected_Intensity");
            sprintf(productUnit, "dBZ");
            sprintf(productColormap, "Reflectivity");
            productList ^= RKProductListProductZ;
            productIndex = RKProductIndexZ;
        } else if (productList & RKProductListProductV) {
            symbol = 'V';
            sprintf(productName, "Radial_Velocity");
            sprintf(productUnit, "MetersPerSecond");
            sprintf(productColormap, "Velocity");
            productList ^= RKProductListProductV;
            productIndex = RKProductIndexV;
        } else if (productList & RKProductListProductW) {
            symbol = 'W';
            sprintf(productName, "SpectrumWidth");
            sprintf(productUnit, "MetersPerSecond");
            sprintf(productColormap, "Width");
            productList ^= RKProductListProductW;
            productIndex = RKProductIndexW;
        } else if (productList & RKProductListProductD) {
            symbol = 'D';
            sprintf(productName, "Differential_Reflectivity");
            sprintf(productUnit, "dB");
            sprintf(productColormap, "Differential_Reflectivity");
            productList ^= RKProductListProductD;
            productIndex = RKProductIndexD;
        } else if (productList & RKProductListProductP) {
            symbol = 'P';
            sprintf(productName, "PhiDP");
            sprintf(productUnit, "Degrees");
            sprintf(productColormap, "PhiDP");
            productList ^= RKProductListProductP;
            productIndex = RKProductIndexP;
        } else if (productList & RKProductListProductR) {
            symbol = 'R';
            sprintf(productName, "RhoHV");
            sprintf(productUnit, "Unitless");
            sprintf(productColormap, "RhoHV");
            productList ^= RKProductListProductR;
            productIndex = RKProductIndexR;
        } else if (productList & RKProductListProductS) {
            symbol = 'S';
            sprintf(productName, "Signal");
            sprintf(productUnit, "dBm");
            sprintf(productColormap, "Signal");
            productList ^= RKProductListProductS;
            productIndex = RKProductIndexS;
        } else if (productList & RKProductListProductK) {
            symbol = 'K';
            sprintf(productName, "KDP");
            sprintf(productUnit, "DegreesPerKilometer");
            sprintf(productColormap, "KDP");
            productList ^= RKProductListProductK;
            productIndex = RKProductIndexK;
        }

        // Make the filename as ../20170119/PX10k-20170119-012345-E1.0-Z.nc
        j = sprintf(filename, "moment/");
        j += strftime(filename + j, 16, "%Y%m%d", gmtime(&startTime));
        j += sprintf(filename + j, "/%s-", engine->radarDescription->filePrefix);
        j += strftime(filename + j, 16, "%Y%m%d-%H%M%S", gmtime(&startTime));
        if (engine->configBuffer[T->header.configIndex].startMarker & RKMarkerPPIScan) {
            j += sprintf(filename + j, "-E%.1f-%c", T->header.sweepElevation, symbol);
        } else if (engine->configBuffer[S->header.configIndex].startMarker & RKMarkerRHIScan) {
            j += sprintf(filename + j, "-A%.1f-%c", T->header.sweepAzimuth, symbol);
        } else {
            j += sprintf(filename + j, "-N%03d-%c", n, symbol);
        }
        sprintf(filename + j, ".nc");
        
        RKPreparePath(filename);
        
        if (engine->verbose) {
            RKLog("%s %s %s ...\n", engine->name, engine->doNotWrite ? "Skip generating" : "Generating", filename);
        }

        if (engine->doNotWrite) {
            continue;
        }
        
        if ((j = nc_create(filename, NC_CLOBBER, &ncid)) > 0) {
            RKLog("%s Error creating %s\n", engine->name, filename);
            return NULL;
        }
        
        if (S->header.marker & RKMarkerPPIScan) {
            nc_def_dim(ncid, "Azimuth", n, &dimensionIds[0]);
        } else if (S->header.marker & RKMarkerRHIScan) {
            nc_def_dim(ncid, "Elevation", n, &dimensionIds[0]);
        } else {
            nc_def_dim(ncid, "Beam", n, &dimensionIds[0]);
        }
        nc_def_dim(ncid, "Gate", S->header.gateCount, &dimensionIds[1]);
        
        // Define variables
        nc_def_var(ncid, "Azimuth", NC_FLOAT, 1, dimensionIds, &variableIdAzimuth);
        nc_put_att_text(ncid, variableIdAzimuth, "Units", 7, "Degrees");
        nc_def_var(ncid, "Elevation", NC_FLOAT, 1, dimensionIds, &variableIdElevation);
        nc_put_att_text(ncid, variableIdElevation, "Units", 7, "Degrees");
        nc_def_var(ncid, "Beamwidth", NC_FLOAT, 1, dimensionIds, &variableIdBeamwidth);
        nc_put_att_text(ncid, variableIdBeamwidth, "Units", 7, "Degrees");
        nc_def_var(ncid, "GateWidth", NC_FLOAT, 1, dimensionIds, &variableIdGateWidth);
        nc_put_att_text(ncid, variableIdGateWidth, "Units", 6, "Meters");
        nc_def_var(ncid, productName, NC_FLOAT, 2, dimensionIds, &variableIdData);
        nc_put_att_text(ncid, variableIdData, "Units", strlen(productUnit), productUnit);

        // Global attributes - some are WDSS-II required
        nc_put_att_text(ncid, NC_GLOBAL, "TypeName", strlen(productName), productName);
        nc_put_att_text(ncid, NC_GLOBAL, "DataType", 9, "RadialSet");
        if (S->header.marker & RKMarkerPPIScan) {
            nc_put_att_text(ncid, NC_GLOBAL, "ScanType", 3, "PPI");
        } else if (S->header.marker & RKMarkerRHIScan) {
            nc_put_att_text(ncid, NC_GLOBAL, "ScanType", 10, "RHI");
        }
        tmpf = engine->radarDescription->latitude;
        nc_put_att_float(ncid, NC_GLOBAL, "Latitude", NC_FLOAT, 1, &tmpf);
        tmpf = engine->radarDescription->longitude;
        nc_put_att_float(ncid, NC_GLOBAL, "Longitude", NC_FLOAT, 1, &tmpf);
        nc_put_att_float(ncid, NC_GLOBAL, "Height", NC_FLOAT, 1, &engine->radarDescription->radarHeight);
        nc_put_att_long(ncid, NC_GLOBAL, "Time", NC_LONG, 1, &S->header.startTime.tv_sec);
        tmpf = (float)S->header.startTime.tv_usec * 1.0e-6;
        nc_put_att_float(ncid, NC_GLOBAL, "FractionalTime", NC_FLOAT, 1, &tmpf);
        put_global_text_att(ncid, "attributes", "Nyquist_Vel Unit radarName vcp ColorMap");
        put_global_text_att(ncid, "Nyquist_Vel-unit", "MetersPerSecond");
        nc_put_att_float(ncid, NC_GLOBAL, "Nyquist_Vel-value", NC_FLOAT, 1, &va);
        put_global_text_att(ncid, "Unit-unit", "dimensionless");
        put_global_text_att(ncid, "Unit-value", productUnit);
        put_global_text_att(ncid, "radarName-unit", "dimensionless");
        put_global_text_att(ncid, "radarName-value", engine->radarDescription->name);
        put_global_text_att(ncid, "vcp-unit", "dimensionless");
        put_global_text_att(ncid, "vcp-value", "1");
        
        // WDSS-II auxiliary
        put_global_text_att(ncid, "ColorMap-unit", "dimensionless");
        put_global_text_att(ncid, "ColorMap-value", productColormap);
        
        // Other housekeeping attributes
        nc_put_att_float(ncid, NC_GLOBAL, "Elevation", NC_FLOAT, 1, &T->header.sweepElevation);
        put_global_text_att(ncid, "ElevationUnits", "Degrees");
        nc_put_att_float(ncid, NC_GLOBAL, "Azimuth", NC_FLOAT, 1, &T->header.sweepAzimuth);
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
        tmpi = config->prf[0];
        nc_put_att_int(ncid, NC_GLOBAL, "PRF-value", NC_INT, 1, &tmpi);
        put_global_text_att(ncid, "PulseWidth-unit", "MicroSeconds");
        tmpf = (float)config->pw[0] * 0.001f;
        nc_put_att_float(ncid, NC_GLOBAL, "PulseWidth-value", NC_FLOAT, 1, &tmpf);
        put_global_text_att(ncid, "MaximumRange-unit", "KiloMeters");
        tmpf = 1.0e-3f * T->header.gateSizeMeters * T->header.gateCount;
        nc_put_att_float(ncid, NC_GLOBAL, "MaximumRange-value", NC_FLOAT, 1, &tmpf);
        put_global_text_att(ncid, "ProcessParameters", "Noise Calib Censor");
        tmpf = 20.0f * log10f(config->noise[0]);
        put_global_text_att(ncid, "NoiseH-unit", "dB-ADU");
        nc_put_att_float(ncid, NC_GLOBAL, "NoiseH-value", NC_FLOAT, 1, &tmpf);
        tmpf = 20.0f * log10f(config->noise[1]);
        put_global_text_att(ncid, "NoiseV-unit", "dB-ADU");
        nc_put_att_float(ncid, NC_GLOBAL, "NoiseV-value", NC_FLOAT, 1, &tmpf);
        put_global_text_att(ncid, "CalibH-unit", "dB");
        nc_put_att_float(ncid, NC_GLOBAL, "CalibH-value", NC_FLOAT, 1, &config->ZCal[0]);
        put_global_text_att(ncid, "CalibV-unit", "dB");
        nc_put_att_float(ncid, NC_GLOBAL, "CalibV-value", NC_FLOAT, 1, &config->ZCal[1]);
        put_global_text_att(ncid, "CalibD1-unit", "dB");
        nc_put_att_float(ncid, NC_GLOBAL, "CalibD1-value", NC_FLOAT, 1, &config->DCal[0]);
        put_global_text_att(ncid, "CalibD2-unit", "dB");
        nc_put_att_float(ncid, NC_GLOBAL, "CalibD2-value", NC_FLOAT, 1, &config->DCal[1]);
        put_global_text_att(ncid, "CalibP1-unit", "Degrees");
        nc_put_att_float(ncid, NC_GLOBAL, "CalibP1-value", NC_FLOAT, 1, &config->PCal[0]);
        put_global_text_att(ncid, "CalibP2-unit", "Degrees");
        nc_put_att_float(ncid, NC_GLOBAL, "CalibP2-value", NC_FLOAT, 1, &config->PCal[1]);
        put_global_text_att(ncid, "CensorThreshold-unit", "dB");
        nc_put_att_float(ncid, NC_GLOBAL, "CensorThreshold-value", NC_FLOAT, 1, &config->censorSNR);
        put_global_text_att(ncid, "Waveform", "hop");
        put_global_text_att(ncid, "CreatedBy", "RadarKit");
        put_global_text_att(ncid, "ContactInformation", "http://arrc.ou.edu");

        // NetCDF definition ends here
        nc_enddef(ncid);

        // Data
        for (j = 0; j < n; j++) {
            T = rays[k + j];
            array1D[j] = T->header.startAzimuth;
        }
        nc_put_var_float(ncid, variableIdAzimuth, array1D);
        for (j = 0; j < n; j++) {
            T = rays[k + j];
            array1D[j] = T->header.startElevation;
        }
        nc_put_var_float(ncid, variableIdElevation, array1D);
        for (j = 0; j < n; j++) {
            T = rays[k + j];
            array1D[j] = RKUMinDiff(T->header.endAzimuth, T->header.startAzimuth);
        }
        nc_put_var_float(ncid, variableIdBeamwidth, array1D);
        for (j = 0; j < n; j++) {
            array1D[j] = T->header.gateSizeMeters;
        }
        nc_put_var_float(ncid, variableIdGateWidth, array1D);
        
        y = array2D;
        for (j = 0; j < n; j++) {
            x = RKGetFloatDataFromRay(rays[k + j], productIndex);
            for (i = 0; i < T->header.gateCount; i++) {
                if (isfinite(*x)) {
                    *y++ = *x;
                } else {
                    *y++ = W2_MISSING_DATA;
                }
                x++;
            }
        }
        nc_put_var_float(ncid, variableIdData, array2D);

        ncclose(ncid);
    }
    
    engine->state ^= RKEngineStateWritingFile;

    return NULL;
}

#pragma mark - Threads

void *rayGatherer(void *in) {
    RKSweepEngine *engine = (RKSweepEngine *)in;
    
    int k, s, n;
    
    RKRay *ray;
    RKRay **rays = engine->sweep.rays;

    // Start and end indices of the input rays
    uint32_t is = 0;
    pthread_t tidSweepWriter = NULL;
    
    RKLog("%s Started.   mem = %s B   rayIndex = %d\n", engine->name, RKIntegerToCommaStyleString(engine->memoryUsage), *engine->rayIndex);
    
    engine->state |= RKEngineStateActive;
    engine->state ^= RKEngineStateActivating;
    
    k = 0;   // ray index
    while (engine->state & RKEngineStateActive) {
        // The ray
        ray = RKGetRay(engine->rayBuffer, k);
        // Wait until the buffer is advanced
        s = 0;
        while (k == *engine->rayIndex && engine->state & RKEngineStateActive) {
            usleep(10000);
            if (++s % 100 == 0 && engine->verbose > 1) {
                RKLog("%s sleep 1/%.1f s   k = %d   rayIndex = %d   header.s = 0x%02x\n",
                      engine->name, (float)s * 0.01f, k, *engine->rayIndex, ray->header.s);
            }
        }
        // Wait until the ray is ready
        s = 0;
        while (!(ray->header.s & RKRayStatusReady) && engine->state & RKEngineStateActive) {
            usleep(10000);
            if (++s % 100 == 0 && engine->verbose > 1) {
                RKLog("%s sleep 2/%.1f s   k = %d   rayIndex = %d   header.s = 0x%02x\n",
                      engine->name, (float)s * 0.01f, k, *engine->rayIndex, ray->header.s);
            }
        }
        if (!(engine->state & RKEngineStateActive)) {
            break;
        }
        
        // Lag of the engine
        engine->lag = fmodf(((float)*engine->rayIndex + engine->rayBufferDepth - k) / engine->rayBufferDepth, 1.0f);
        if (ray->header.marker & RKMarkerSweepEnd) {
            n = 0;
            do {
                ray = RKGetRay(engine->rayBuffer, is);
                ray->header.n = is;
                rays[n++] = ray;
                is = RKNextModuloS(is, engine->rayBufferDepth);
            } while (is != k && n < RKMaxRaysPerSweep && n < engine->rayBufferDepth);
            ray = RKGetRay(engine->rayBuffer, is);
            ray->header.n = is;
            rays[n++] = ray;
            engine->sweep.rayCount = n;
            is = RKPreviousNModuloS(is, n + 1, engine->rayBufferDepth);
            
            //RKLog(">%s %p %p %p ... %p\n", engine->name, engine->sweep.rays[0], engine->sweep.rays[1], engine->sweep.rays[2], engine->sweep.rays[n - 1]);
            
            if (tidSweepWriter) {
                pthread_join(tidSweepWriter, NULL);
            }
            if (pthread_create(&tidSweepWriter, NULL, sweepWriter, engine)) {
                RKLog("%s Error. Unable to launch a sweep writer.\n", engine->name);
            }

            is = k;
        }

        // Update k to catch up for the next watch
        k = RKNextModuloS(k, engine->rayBufferDepth);
    }
    return NULL;
}

#pragma mark - Life Cycle

RKSweepEngine *RKSweepEngineInit(void) {
    RKSweepEngine *engine = (RKSweepEngine *)malloc(sizeof(RKSweepEngine));
    if (engine == NULL) {
        RKLog("Error. Unable to allocate a sweep engine.\n");
        return NULL;
    }
    memset(engine, 0, sizeof(RKSweepEngine));
    sprintf(engine->name, "%s<ProductRecorder>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColor() : "", rkGlobalParameters.showColor ? RKNoColor : "");
    engine->array1D = (float *)malloc(RKMaxRaysPerSweep * sizeof(float));
    if (engine->array1D == NULL) {
        RKLog("%s Error. Unable to allocate memory.\n", engine->name);
        exit(EXIT_FAILURE);
    }
    engine->array2D = (float *)malloc(RKMaxRaysPerSweep * RKGateCount * sizeof(float));
    if (engine->array2D == NULL) {
        RKLog("%s Error. Unable to allocate memory.\n", engine->name);
        exit(EXIT_FAILURE);
    }
    engine->state = RKEngineStateAllocated;
    engine->memoryUsage = sizeof(RKSweepEngine) + RKMaxRaysPerSweep * (RKGateCount + 1) * sizeof(float);
    engine->doNotWrite = false;
    return engine;
}

void RKSweepEngineFree(RKSweepEngine *engine) {
    if (engine->state & RKEngineStateActive) {
        RKSweepEngineStop(engine);
    }
    free(engine->array1D);
    free(engine->array2D);
    free(engine);
}

#pragma mark - Properties

void RKSweepEngineSetVerbose(RKSweepEngine *engine, const int verbose) {
    engine->verbose = verbose;
}

void RKSweepEngineSetInputOutputBuffer(RKSweepEngine *engine, RKRadarDesc *desc,
                                       RKConfig *configBuffer, uint32_t *configIndex, const uint32_t configBufferDepth,
                                       RKBuffer rayBuffer, uint32_t *rayIndex, const uint32_t rayBufferDepth) {
    engine->radarDescription  = desc;
    engine->configBuffer      = configBuffer;
    engine->configIndex       = configIndex;
    engine->configBufferDepth = configBufferDepth;
    engine->rayBuffer         = rayBuffer;
    engine->rayIndex          = rayIndex;
    engine->rayBufferDepth    = rayBufferDepth;
}

void RKSweepEngineSetDoNotWrite(RKSweepEngine *engine, const bool value) {
    engine->doNotWrite = value;
}

#pragma mark - Interactions

int RKSweepEngineStart(RKSweepEngine *engine) {
    if (engine->verbose) {
        RKLog("%s Starting ...\n", engine->name);
    }
    engine->state |= RKEngineStateActivating;
    if (pthread_create(&engine->tidRayGatherer, NULL, rayGatherer, engine) != 0) {
        RKLog("Error. Failed to start a ray gatherer.\n");
        return RKResultFailedToStartRayGatherer;
    }
    while (!(engine->state & RKEngineStateActive)) {
        usleep(10000);
    }
    return RKResultSuccess;
}

int RKSweepEngineStop(RKSweepEngine *engine) {
    if (engine->state & RKEngineStateDeactivating) {
        if (engine->verbose > 1) {
            RKLog("%s Info. Engine is being or has been deactivated.\n", engine->name);
        }
        return RKResultEngineDeactivatedMultipleTimes;
    }
    if (engine->verbose) {
        RKLog("%s Stopping ...\n", engine->name);
    }
    engine->state |= RKEngineStateDeactivating;
    engine->state ^= RKEngineStateActive;
    pthread_join(engine->tidRayGatherer, NULL);
    if (engine->verbose) {
        RKLog("%s Stopped.\n", engine->name);
    }
    engine->state = RKEngineStateAllocated;
    return RKResultSuccess;
}
