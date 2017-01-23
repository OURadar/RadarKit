//
//  RKSweep.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 1/15/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKSweep.h>

#pragma mark - Helper Functions

static int put_global_text_att(const int ncid, const char *att, const char *text) {
    return nc_put_att_text(ncid, NC_GLOBAL, att, strlen(text), text);
}

void *sweepWriter(void *in) {
    RKSweepEngine *engine = (RKSweepEngine *)in;
    
    uint32_t n = engine->sweep.rayCount;
    RKRay **rays = engine->sweep.rays;
    //RKLog(">%s %p %p %p ... %p\n", engine->name, rays[0], rays[1], rays[2], rays[n - 1]);

    RKRay *S = rays[0];
    RKRay *T = rays[1];
    RKRay *E = rays[n - 1];
    RKConfig *config = &engine->configBuffer[S->header.configIndex];
    
    RKLog("%s Sweep   C%02d-%02d  E%4.2f-%4.2f-%.2f   A%6.2f-%6.2f   %sM%s%04x-%04x-%04x   %05lu...%05lu (%s%d%s)\n",
          engine->name,
          S->header.configIndex    , E->header.configIndex,
          config->sweepElevation,
          S->header.startElevation , E->header.endElevation,
          S->header.startAzimuth   , E->header.endAzimuth,
          RKGetColorOfIndex(2)     , RKNoColor,
          S->header.marker & 0xFFFF, T->header.marker & 0xFFFF, E->header.marker & 0xFFFF,
          S->header.n              , E->header.n,
          n < 360 ? RKGetColorOfIndex(0) : (n > 361 ? RKGetColorOfIndex(1) : ""),
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
    
    int k, p;
    int ncid;
    int dimensionIds[2];
    int variableIdAzimuth;
    int variableIdElevation;
    int variableIdBeamwidth;
    int variableIdGateWidth;
    int variableIdData;
    
    float tmp;

    time_t startTime = (time_t)S->header.startTime.tv_sec;

    // Go through all moments
    uint32_t productList = S->header.productList;
    int productCount = __builtin_popcount(productList);
    for (p = 0; p < productCount; p++) {
        if (productList & RKProductListProductZ) {
            symbol = 'Z';
            sprintf(productName, "Corrected_Intensity");
            sprintf(productUnit, "dBZ");
            sprintf(productColormap, "Reflectivity");
            productList ^= RKProductListProductZ;
        } else if (productList & RKProductListProductV) {
            symbol = 'V';
            sprintf(productName, "Radial_Velocity");
            sprintf(productUnit, "MetersPerSecond");
            sprintf(productColormap, "Velocity");
            productList ^= RKProductListProductV;
        } else if (productList & RKProductListProductW) {
            symbol = 'W';
            sprintf(productName, "SpectrumWidth");
            sprintf(productUnit, "MetersPerSecond");
            sprintf(productColormap, "Width");
            productList ^= RKProductListProductW;
        } else if (productList & RKProductListProductD) {
            symbol = 'D';
            sprintf(productName, "Differential_Reflectivity");
            sprintf(productUnit, "dB");
            sprintf(productColormap, "Differential_Reflectivity");
            productList ^= RKProductListProductD;
        } else if (productList & RKProductListProductP) {
            symbol = 'P';
            sprintf(productName, "PhiDP");
            sprintf(productUnit, "Degrees");
            sprintf(productColormap, "PhiDP");
            productList ^= RKProductListProductP;
        } else if (productList & RKProductListProductR) {
            symbol = 'P';
            sprintf(productName, "RhoHV");
            sprintf(productUnit, "Unitless");
            sprintf(productColormap, "RhoHV");
            productList ^= RKProductListProductR;
        } else if (productList & RKProductListProductS) {
            symbol = 'S';
            sprintf(productName, "Signal");
            sprintf(productUnit, "dBm");
            sprintf(productColormap, "Signal");
            productList ^= RKProductListProductS;
        } else if (productList & RKProductListProductK) {
            symbol = 'K';
            sprintf(productName, "KDP");
            sprintf(productUnit, "DegreesPerKilometer");
            sprintf(productColormap, "KDP");
            productList ^= RKProductListProductK;
        }

        // Make the filename as ../20170119/PX10k-20170119-012345-E1.0-Z.nc
        k = sprintf(filename, "moment/");
        k += strftime(filename + k, 16, "%Y%m%d", gmtime(&startTime));
        k += sprintf(filename + k, "/%s-", engine->radarDescription->filePrefix);
        k += strftime(filename + k, 16, "%Y%m%d-%H%M%S", gmtime(&startTime));
        if (engine->configBuffer[S->header.configIndex].startMarker & RKMarkerPPIScan) {
            k += sprintf(filename + k, "-E%.1f-%c", S->header.sweepElevation, symbol);
        } else if (engine->configBuffer[S->header.configIndex].startMarker & RKMarkerRHIScan) {
            k += sprintf(filename + k, "-A%.1f-%c", S->header.sweepAzimuth, symbol);
        } else {
            k += sprintf(filename + k, "-N%03d-%c", n, symbol);
        }
        sprintf(filename + k, ".nc");
        
        RKPreparePath(filename);
        
        if ((k = nc_create(filename, NC_CLOBBER, &ncid)) > 0) {
            RKLog("%s Error creating %s\n", engine->name, filename);
            return NULL;
        } else {
            RKLog("%s Generating %s ...\n", engine->name, filename);
        }
        
        if (engine->doNotWrite) {
            continue;
        }
        
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
        tmp = engine->radarDescription->latitude;
        nc_put_att_float(ncid, NC_GLOBAL, "Latitude", NC_FLOAT, 1, &tmp);
        tmp = engine->radarDescription->longitude;
        nc_put_att_float(ncid, NC_GLOBAL, "Longitude", NC_FLOAT, 1, &tmp);
        nc_put_att_float(ncid, NC_GLOBAL, "Height", NC_FLOAT, 1, &engine->radarDescription->radarHeight);
        nc_put_att_long(ncid, NC_GLOBAL, "Time", NC_LONG, 1, &S->header.startTime.tv_sec);
        tmp = (float)S->header.startTime.tv_usec * 1.0e-6;
        nc_put_att_float(ncid, NC_GLOBAL, "FractionalTime", NC_FLOAT, 1, &tmp);
        put_global_text_att(ncid, "attributes", "Nyquist_Vel Unit radarName vcp ColorMap");
        put_global_text_att(ncid, "Nyquist_Vel-unit", "MetersPerSecond");
//        nc_put_att_float(ncid, NC_GLOBAL, "Nyquist_Vel-value", NC_FLOAT, 1, &vol->va);
        put_global_text_att(ncid, "Unit-unit", "dimensionless");
        put_global_text_att(ncid, "Unit-value", productUnit);
        put_global_text_att(ncid, "radarName-unit", "dimensionless");
        put_global_text_att(ncid, "radarName-value", engine->radarDescription->name);
        put_global_text_att(ncid, "vcp-unit", "dimensionless");
        put_global_text_att(ncid, "vcp-value", "NA");

        ncclose(ncid);
    }
    return NULL;
}

#pragma mark - Threads

void *rayGatherer(void *in) {
    RKSweepEngine *engine = (RKSweepEngine *)in;
    
    int k, s, n;
    
    RKRay *ray, *S, *E;
    RKRay **rays = engine->sweep.rays;

    // Start and end indices of the input rays
    uint32_t is = 0;
    pthread_t tidSweepWriter = NULL;
    
    RKLog("%s started.   mem = %s B   rayIndex = %d\n", engine->name, RKIntegerToCommaStyleString(engine->memoryUsage), *engine->rayIndex);
    
    engine->state |= RKSweepEngineStateActive;
    
    k = 0;   // ray index
    while (engine->state & RKSweepEngineStateActive) {
        // The ray
        ray = RKGetRay(engine->rayBuffer, k);
        // Wait until the buffer is advanced
        s = 0;
        while (k == *engine->rayIndex && engine->state & RKSweepEngineStateActive) {
            usleep(10000);
            if (++s % 100 == 0 && engine->verbose > 1) {
                RKLog("%s sleep 1/%.1f s   k = %d   rayIndex = %d   header.s = 0x%02x\n",
                      engine->name, (float)s * 0.01f, k, *engine->rayIndex, ray->header.s);
            }
        }
        // Wait until the ray is ready
        s = 0;
        while (!(ray->header.s & RKRayStatusReady) && engine->state & RKSweepEngineStateActive) {
            usleep(10000);
            if (++s % 100 == 0 && engine->verbose > 1) {
                RKLog("%s sleep 2/%.1f s   k = %d   rayIndex = %d   header.s = 0x%02x\n",
                      engine->name, (float)s * 0.01f, k, *engine->rayIndex, ray->header.s);
            }
        }
        if (engine->state & RKSweepEngineStateActive) {
            // Lag of the engine
            engine->lag = fmodf(((float)*engine->rayIndex + engine->rayBufferDepth - k) / engine->rayBufferDepth, 1.0f);
            if (ray->header.marker & RKMarkerSweepEnd) {
                S = RKGetRay(engine->rayBuffer, is);
                E = ray;
                n = 0;
                do {
                    ray = RKGetRay(engine->rayBuffer, is);
                    ray->header.n = is;
                    rays[n++] = ray;
                    is = RKNextModuloS(is, engine->rayBufferDepth);
                } while (is != k && n < RKMaxRaysPerSweep);
                ray = RKGetRay(engine->rayBuffer, is);
                ray->header.n = is;
                rays[n++] = ray;
                engine->sweep.rayCount = n;
                is = RKPreviousNModuloS(is, n + 1, engine->rayBufferDepth);

//                RKLog("%s Sweep   E%4.2f-%.2f   A%6.2f-%6.2f   \033[32mM\033[0m%04x-%04x   %05lu...%05lu (%d)\n",
//                      engine->name,
//                      S->header.startElevation , E->header.endElevation,
//                      S->header.startAzimuth   , E->header.endAzimuth,
//                      S->header.marker & 0xFFFF, E->header.marker & 0xFFFF,
//                      is, k, n);
                
                //RKLog(">%s %p %p %p ... %p\n", engine->name, engine->sweep.rays[0], engine->sweep.rays[1], engine->sweep.rays[2], engine->sweep.rays[n - 1]);
                
                if (tidSweepWriter) {
                    pthread_join(tidSweepWriter, NULL);
                }
                if (pthread_create(&tidSweepWriter, NULL, sweepWriter, engine)) {
                    RKLog("%s Error. Unable to launch a sweep writer.\n", engine->name);
                }

                is = k;
            }
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
    sprintf(engine->name, "%s<SweepProducer>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColor() : "", rkGlobalParameters.showColor ? RKNoColor : "");
    engine->state = RKSweepEngineStateAllocated;
    engine->memoryUsage = sizeof(RKSweepEngine);
    engine->doNotWrite = true;
    return engine;
}

void RKSweepEngineFree(RKSweepEngine *engine) {
    if (engine->state & RKSweepEngineStateActive) {
        engine->state ^= RKSweepEngineStateActive;
    }
    
}

#pragma mark - Properties

void RKSweepEngineSetVerbose(RKSweepEngine *engine, const int verbose) {
    engine->verbose = verbose;
}

void RKSweepEngineSetInputBuffer(RKSweepEngine *engine, RKRadarDesc *desc,
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

#pragma mark - Interactions

int RKSweepEngineStart(RKSweepEngine *engine) {
    RKLog("%s starting ...\n", engine->name);
    if (pthread_create(&engine->tidRayGatherer, NULL, rayGatherer, engine) != 0) {
        RKLog("Error. Failed to start a ray gatherer.\n");
        return RKResultFailedToStartRayGatherer;
    }
    while (!(engine->state & RKSweepEngineStateActive)) {
        usleep(10000);
    }
    return RKResultSuccess;
}

int RKSweepEngineStop(RKSweepEngine *engine) {
    if (engine->state & RKSweepEngineStateActive) {
        engine->state ^= RKSweepEngineStateActive;
    }
    pthread_join(engine->tidRayGatherer, NULL);
    return RKResultSuccess;
}

