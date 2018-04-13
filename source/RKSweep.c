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

#if defined (COMPRESSED_NETCDF)
#define NC_MODE  NC_NETCDF4
#else
#define NC_MODE  NC_CLOBBER
#endif

// Internal Functions

static int put_global_text_att(const int ncid, const char *att, const char *text);
static void *sweepWriter(void *);
static void *rayGatherer(void *in);

#pragma mark - Helper Functions

static int put_global_text_att(const int ncid, const char *att, const char *text) {
    return nc_put_att_text(ncid, NC_GLOBAL, att, strlen(text), text);
}

static void *sweepWriter(void *in) {
    RKSweepEngine *engine = (RKSweepEngine *)in;

    int i, j, p;

	RKSweep *sweep = RKSweepCollect(engine);
	if (engine->verbose) {
		RKRay *S = sweep->rays[0];
		RKRay *E = sweep->rays[sweep->header.rayCount - 1];
		RKLog("%s C%02d   E%5.2f/%5.2f-%5.2f   A%6.2f-%6.2f   M%03x-%03x   (%s x %s%d%s, %.1f km)\n",
			  engine->name,
			  S->header.configIndex,
			  sweep->header.config.sweepElevation,
			  S->header.startElevation , E->header.endElevation,
			  S->header.startAzimuth   , E->header.endAzimuth,
			  S->header.marker & 0xFFFF, E->header.marker & 0xFFFF,
			  RKIntegerToCommaStyleString(sweep->header.gateCount),
			  sweep->header.rayCount != 360 ? RKGetColorOfIndex(1) : "",
			  sweep->header.rayCount,
			  RKNoColor,
			  1.0e-3f * S->header.gateCount * S->header.gateSizeMeters);
	}

    // Mark the state
    engine->state |= RKEngineStateWritingFile;
    
    char *filelist = engine->filelist;
    char *filename = engine->filename;

	char *symbol = engine->productSymbol;
	char *productName = engine->productName;
    char *productUnit = engine->productUnit;
    char *productColormap = engine->productColormap;
	RKProductIndex productIndex;

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
    bool convertRadiansToDegrees;
    
    // Some global attributes
    time_t startTime = (time_t)sweep->rays[0]->header.startTime.tv_sec;
    float va = 0.25f * sweep->header.desc.wavelength * sweep->header.config.prf[0];

    // Go through all moments
    if (engine->hasHandleFilesScript) {
        strncpy(filelist, engine->handleFilesScript, RKMaximumPathLength);
    }

    const float radianToDegree = 180.0f / M_PI;

    int summarySize = 0;
    uint32_t productList = sweep->header.productList;
    int productCount = __builtin_popcount(productList);

	for (p = 0; p < productCount; p++) {
		// Get the symbol, name, unit, colormap, etc. from the product list
		RKGetNextProductDescription(symbol, productName, productUnit, productColormap, &productIndex, &productList);

        // Make the filename as ../20170119/PX10k-20170119-012345-E1.0-Z.nc
        i = sprintf(filename, "%s%s%s/", engine->radarDescription->dataPath, engine->radarDescription->dataPath[0] == '\0' ? "" : "/", RKDataFolderMoment);
        i += strftime(filename + i, 10, "%Y%m%d", gmtime(&startTime));
        i += sprintf(filename + i, "/%s-", engine->radarDescription->filePrefix);
        i += strftime(filename + i, 16, "%Y%m%d-%H%M%S", gmtime(&startTime));
		if (sweep->header.config.startMarker & RKMarkerPPIScan) {
            i += sprintf(filename + i, "-E%.1f-%s", sweep->header.config.sweepElevation, symbol);
        } else if (sweep->header.config.startMarker & RKMarkerRHIScan) {
            i += sprintf(filename + i, "-A%.1f-%s", sweep->header.config.sweepAzimuth, symbol);
        } else {
            i += sprintf(filename + i, "-N%03d-%s", sweep->header.rayCount, symbol);
        }
        sprintf(filename + i, ".nc");
        
        if (engine->verbose > 1) {
            RKLog("%s %s %s ...\n", engine->name, engine->doNotWrite ? "Skipping" : "Creating", filename);
        } else if (engine->verbose) {
            if (p == 0) {
                // There are at least two '/'s in the filename: ...rootDataFolder/moment/YYYYMMDD/RK-YYYYMMDD-HHMMSS-Enn.n-Z.nc
                summarySize = sprintf(engine->summary, "%s ...%s", engine->doNotWrite ? "Skipped" : "Created", RKLastTwoPartsOfPath(filename));
            } else {
                summarySize += sprintf(engine->summary + summarySize, ", %s", symbol);
            }
        }

        if (engine->doNotWrite) {
            continue;
        }
        
        RKPreparePath(filename);

        if (engine->hasHandleFilesScript) {
            sprintf(filelist + strlen(filelist), " %s", filename);
        }

        if ((j = nc_create(filename, NC_MODE, &ncid)) > 0) {
            RKLog("%s Error creating %s\n", engine->name, filename);
            engine->state ^= RKEngineStateWritingFile;
            return NULL;
        }

        if (sweep->header.config.startMarker & RKMarkerPPIScan) {
            nc_def_dim(ncid, "Azimuth", sweep->header.rayCount, &dimensionIds[0]);
        } else if (sweep->header.config.startMarker & RKMarkerRHIScan) {
            nc_def_dim(ncid, "Elevation", sweep->header.rayCount, &dimensionIds[0]);
        } else {
            nc_def_dim(ncid, "Beam", sweep->header.rayCount, &dimensionIds[0]);
        }
        nc_def_dim(ncid, "Gate", sweep->header.gateCount, &dimensionIds[1]);
        
        // Define variables
        nc_def_var(ncid, "Azimuth", NC_FLOAT, 1, dimensionIds, &variableIdAzimuth);
        nc_def_var(ncid, "Elevation", NC_FLOAT, 1, dimensionIds, &variableIdElevation);
        nc_def_var(ncid, "Beamwidth", NC_FLOAT, 1, dimensionIds, &variableIdBeamwidth);
        nc_def_var(ncid, "GateWidth", NC_FLOAT, 1, dimensionIds, &variableIdGateWidth);
        nc_def_var(ncid, productName, NC_FLOAT, 2, dimensionIds, &variableIdData);

        nc_put_att_text(ncid, variableIdAzimuth, "Units", 7, "Degrees");
        nc_put_att_text(ncid, variableIdElevation, "Units", 7, "Degrees");
        nc_put_att_text(ncid, variableIdBeamwidth, "Units", 7, "Degrees");
        nc_put_att_text(ncid, variableIdGateWidth, "Units", 6, "Meters");
        nc_put_att_text(ncid, variableIdData, "Units", strlen(productUnit), productUnit);

#if defined (COMPRESSED_NETCDF)

        nc_def_var_deflate(ncid, variableIdAzimuth, 1, 1, 3);
        nc_def_var_deflate(ncid, variableIdElevation, 1, 1, 3);
        nc_def_var_deflate(ncid, variableIdBeamwidth, 1, 1, 3);
        nc_def_var_deflate(ncid, variableIdGateWidth, 1, 1, 3);
        nc_def_var_deflate(ncid, variableIdData, 1, 1, 3);

#endif
        
        // Global attributes - some are WDSS-II required
        nc_put_att_text(ncid, NC_GLOBAL, "TypeName", strlen(productName), productName);
        nc_put_att_text(ncid, NC_GLOBAL, "DataType", 9, "RadialSet");
        if (sweep->header.config.startMarker & RKMarkerPPIScan) {
            nc_put_att_text(ncid, NC_GLOBAL, "ScanType", 3, "PPI");
        } else if (sweep->header.config.startMarker & RKMarkerRHIScan) {
            nc_put_att_text(ncid, NC_GLOBAL, "ScanType", 3, "RHI");
        }
        tmpf = engine->radarDescription->latitude;
        nc_put_att_float(ncid, NC_GLOBAL, "Latitude", NC_FLOAT, 1, &tmpf);
        nc_put_att_double(ncid, NC_GLOBAL, "LatitudeDouble", NC_DOUBLE, 1, &engine->radarDescription->latitude);
        tmpf = engine->radarDescription->longitude;
        nc_put_att_float(ncid, NC_GLOBAL, "Longitude", NC_FLOAT, 1, &tmpf);
        nc_put_att_double(ncid, NC_GLOBAL, "LongitudeDouble", NC_DOUBLE, 1, &engine->radarDescription->longitude);
        nc_put_att_float(ncid, NC_GLOBAL, "Heading", NC_FLOAT, 1, &engine->radarDescription->heading);
        nc_put_att_float(ncid, NC_GLOBAL, "Height", NC_FLOAT, 1, &engine->radarDescription->radarHeight);
        nc_put_att_long(ncid, NC_GLOBAL, "Time", NC_LONG, 1, &startTime);
        tmpf = (float)sweep->rays[0]->header.startTime.tv_usec * 1.0e-6;
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
        if (sweep->header.config.startMarker & RKMarkerPPIScan) {
            nc_put_att_float(ncid, NC_GLOBAL, "Elevation", NC_FLOAT, 1, &sweep->header.config.sweepElevation);
        } else {
            tmpf = W2_MISSING_DATA;
            nc_put_att_float(ncid, NC_GLOBAL, "Elevation", NC_FLOAT, 1, &tmpf);
        }
        put_global_text_att(ncid, "ElevationUnits", "Degrees");
        if (sweep->header.config.startMarker & RKMarkerPPIScan) {
            nc_put_att_float(ncid, NC_GLOBAL, "Azimuth", NC_FLOAT, 1, &sweep->header.config.sweepAzimuth);
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
        tmpi = sweep->header.config.prf[0];
        nc_put_att_int(ncid, NC_GLOBAL, "PRF-value", NC_INT, 1, &tmpi);
        put_global_text_att(ncid, "PulseWidth-unit", "MicroSeconds");
        tmpf = (float)sweep->header.config.pw[0] * 0.001f;
        nc_put_att_float(ncid, NC_GLOBAL, "PulseWidth-value", NC_FLOAT, 1, &tmpf);
        put_global_text_att(ncid, "MaximumRange-unit", "KiloMeters");
		tmpf = 1.0e-3f * sweep->rays[0]->header.gateSizeMeters * sweep->header.gateCount;
        nc_put_att_float(ncid, NC_GLOBAL, "MaximumRange-value", NC_FLOAT, 1, &tmpf);
        put_global_text_att(ncid, "ProcessParameters", "Noise Calib Censor");
        tmpf = 20.0f * log10f(sweep->header.config.noise[0]);
        put_global_text_att(ncid, "NoiseH-unit", "dB-ADU");
        nc_put_att_float(ncid, NC_GLOBAL, "NoiseH-value", NC_FLOAT, 1, &tmpf);
        tmpf = 20.0f * log10f(sweep->header.config.noise[1]);
        put_global_text_att(ncid, "NoiseV-unit", "dB-ADU");
        nc_put_att_float(ncid, NC_GLOBAL, "NoiseV-value", NC_FLOAT, 1, &tmpf);
        put_global_text_att(ncid, "CalibH-unit", "dB");
        nc_put_att_float(ncid, NC_GLOBAL, "CalibH-value", NC_FLOAT, 1, &sweep->header.config.ZCal[0][0]);
        put_global_text_att(ncid, "CalibV-unit", "dB");
        nc_put_att_float(ncid, NC_GLOBAL, "CalibV-value", NC_FLOAT, 1, &sweep->header.config.ZCal[1][0]);
        put_global_text_att(ncid, "CalibD1-unit", "dB");
        nc_put_att_float(ncid, NC_GLOBAL, "CalibD1-value", NC_FLOAT, 1, &sweep->header.config.DCal[0]);
        put_global_text_att(ncid, "CalibD2-unit", "dB");
        nc_put_att_float(ncid, NC_GLOBAL, "CalibD2-value", NC_FLOAT, 1, &sweep->header.config.DCal[1]);
        put_global_text_att(ncid, "CalibP1-unit", "Degrees");
        nc_put_att_float(ncid, NC_GLOBAL, "CalibP1-value", NC_FLOAT, 1, &sweep->header.config.PCal[0]);
        put_global_text_att(ncid, "CalibP2-unit", "Degrees");
        nc_put_att_float(ncid, NC_GLOBAL, "CalibP2-value", NC_FLOAT, 1, &sweep->header.config.PCal[1]);
        put_global_text_att(ncid, "CensorThreshold-unit", "dB");
        nc_put_att_float(ncid, NC_GLOBAL, "CensorThreshold-value", NC_FLOAT, 1, &sweep->header.config.SNRThreshold);
        put_global_text_att(ncid, "Waveform", sweep->header.config.waveform);
        put_global_text_att(ncid, "CreatedBy", "RadarKit");
        put_global_text_att(ncid, "ContactInformation", "http://arrc.ou.edu");

        // NetCDF definition ends here
        nc_enddef(ncid);

        // Data
        for (j = 0; j < sweep->header.rayCount; j++) {
            array1D[j] = sweep->rays[j]->header.startAzimuth;
        }
        nc_put_var_float(ncid, variableIdAzimuth, array1D);
        for (j = 0; j < sweep->header.rayCount; j++) {
            array1D[j] = sweep->rays[j]->header.startElevation;
        }
        nc_put_var_float(ncid, variableIdElevation, array1D);
        for (j = 0; j < sweep->header.rayCount; j++) {
            array1D[j] = RKUMinDiff(sweep->rays[j]->header.endAzimuth, sweep->rays[j]->header.startAzimuth);
        }
        nc_put_var_float(ncid, variableIdBeamwidth, array1D);
        for (j = 0; j < sweep->header.rayCount; j++) {
            array1D[j] = sweep->rays[j]->header.gateSizeMeters;
        }
        nc_put_var_float(ncid, variableIdGateWidth, array1D);
        
        y = array2D;
        // Should AND it with a user preference
        convertRadiansToDegrees = productIndex == RKProductIndexP || productIndex == RKProductIndexK;
        for (j = 0; j < sweep->header.rayCount; j++) {
            x = RKGetFloatDataFromRay(sweep->rays[j], productIndex);
            if (convertRadiansToDegrees) {
                for (i = 0; i < sweep->rays[0]->header.gateCount; i++) {
                    if (isfinite(*x)) {
                        *y++ = *x * radianToDegree;
                    } else {
                        *y++ = W2_MISSING_DATA;
                    }
                    x++;
                }
            } else {
                for (i = 0; i < sweep->rays[0]->header.gateCount; i++) {
                    if (isfinite(*x)) {
                        *y++ = *x;
                    } else {
                        *y++ = W2_MISSING_DATA;
                    }
                    x++;
                }
            }
        }
        nc_put_var_float(ncid, variableIdData, array2D);

        ncclose(ncid);

        // Notify file manager of a new addition
        RKFileManagerAddFile(engine->fileManager, filename, RKFileTypeMoment);
    }

    // Show a summary of all the files created
    if (engine->verbose && summarySize > 0) {
        RKLog("%s %s", engine->name, engine->summary);
    }

    if (!engine->doNotWrite && engine->hasHandleFilesScript) {
        //printf("CMD: '%s'\n", filelist);
        system(filelist);
        // Potential filenames that may be generated by the custom command. Need to notify file manager about them.
        sprintf(productName, "-%s.nc", symbol);
        RKReplaceFileExtension(filename, productName, ".__");
        if (engine->handleFilesScriptProducesTgz) {
            RKReplaceFileExtension(filename, ".__", ".tgz");
            RKLog("%s %s", engine->name, filename);
            if (RKFilenameExists(filename)) {
                RKFileManagerAddFile(engine->fileManager, filename, RKFileTypeMoment);
            }
            RKReplaceFileExtension(filename, ".tgz", ".__");
        }
        if (engine->handleFilesScriptProducesZip) {
            RKReplaceFileExtension(filename, ".__", ".zip");
            RKLog("%s %s", engine->name, filename);
            if (RKFilenameExists(filename)) {
                RKFileManagerAddFile(engine->fileManager, filename, RKFileTypeMoment);
            }
            RKReplaceFileExtension(filename, "zip", ".__");
        }
    }

    engine->state ^= RKEngineStateWritingFile;

    return NULL;
}

#pragma mark - Delegate Workers

static void *rayGatherer(void *in) {
    RKSweepEngine *engine = (RKSweepEngine *)in;
    
    int j, n, s;
    
	// Start index
	uint32_t is = 0;
	pthread_t tidSweepWriter = NULL;

    RKRay *ray = RKGetRay(engine->rayBuffer, 0);
    RKRay **rays = engine->rayAnchors[engine->rayAnchorsIndex].rays;

    // Allocate if the arrays have not been allocated
    if (engine->array1D == NULL) {
        engine->array1D = (float *)malloc(RKMaxRaysPerSweep * sizeof(float));
        if (engine->array1D == NULL) {
            RKLog("%s Error. Unable to allocate memory.\n", engine->name);
            exit(EXIT_FAILURE);
        }
        engine->array2D = (float *)malloc(RKMaxRaysPerSweep * ray->header.capacity * sizeof(float));
        if (engine->array2D == NULL) {
            RKLog("%s Error. Unable to allocate memory.\n", engine->name);
            exit(EXIT_FAILURE);
        }
        engine->memoryUsage += RKMaxRaysPerSweep * (ray->header.capacity + 1) * sizeof(float);
    }
    
	// Update the engine state
	engine->state |= RKEngineStateActive;
	engine->state ^= RKEngineStateActivating;

	if (engine->verbose) {
        RKLog("%s Started.   mem = %s B   rayIndex = %d\n", engine->name, RKIntegerToCommaStyleString(engine->memoryUsage), *engine->rayIndex);
		RKLog(">%s Handle files using '%s'   expectTgz = %s\n", engine->name, engine->handleFilesScript, engine->handleFilesScriptProducesTgz ? "true" : "false");
    }

	// Increase the tic once to indicate the engine is ready
	engine->tic = 1;

    j = 0;   // ray index
    while (engine->state & RKEngineStateActive) {
        // The ray
        ray = RKGetRay(engine->rayBuffer, j);
        
        // Wait until the buffer is advanced
        engine->state |= RKEngineStateSleep1;
        s = 0;
        while (j == *engine->rayIndex && engine->state & RKEngineStateActive) {
            usleep(10000);
            if (++s % 100 == 0 && engine->verbose > 1) {
                RKLog("%s sleep 1/%.1f s   k = %d   rayIndex = %d   header.s = 0x%02x\n",
                      engine->name, (float)s * 0.01f, j, *engine->rayIndex, ray->header.s);
            }
        }
        engine->state ^= RKEngineStateSleep1;
        engine->state |= RKEngineStateSleep2;
        // Wait until the ray is ready
        s = 0;
        while (!(ray->header.s & RKRayStatusReady) && engine->state & RKEngineStateActive) {
            usleep(10000);
            if (++s % 100 == 0 && engine->verbose > 1) {
                RKLog("%s sleep 2/%.1f s   k = %d   rayIndex = %d   header.s = 0x%02x\n",
                      engine->name, (float)s * 0.01f, j, *engine->rayIndex, ray->header.s);
            }
        }
        engine->state ^= RKEngineStateSleep2;

        if (!(engine->state & RKEngineStateActive)) {
            break;
        }
        
        // Lag of the engine
        engine->lag = fmodf(((float)*engine->rayIndex + engine->radarDescription->rayBufferDepth - j) / engine->radarDescription->rayBufferDepth, 1.0f);
        if (ray->header.marker & RKMarkerSweepEnd) {
            n = 0;
            do {
                ray = RKGetRay(engine->rayBuffer, is);
                ray->header.n = is;
                rays[n++] = ray;
                is = RKNextModuloS(is, engine->radarDescription->rayBufferDepth);
            } while (is != j && n < RKMaxRaysPerSweep - 1 && n < engine->radarDescription->rayBufferDepth);
            ray = RKGetRay(engine->rayBuffer, is);
            ray->header.n = is;
            rays[n++] = ray;
			engine->rayAnchors[engine->rayAnchorsIndex].count = n;
            
            if (tidSweepWriter) {
                pthread_join(tidSweepWriter, NULL);
            }
			engine->rayAnchorsIndex = RKNextModuloS(engine->rayAnchorsIndex, RKRayAnchorsDepth);
			rays = engine->rayAnchors[engine->rayAnchorsIndex].rays;
            if (pthread_create(&tidSweepWriter, NULL, sweepWriter, engine)) {
                RKLog("%s Error. Unable to launch a sweep writer.\n", engine->name);
            }

            is = j;
        } else if (ray->header.marker & RKMarkerSweepBegin) {
            is = j;
        }

		engine->tic++;

        // Update k to catch up for the next watch
        j = RKNextModuloS(j, engine->radarDescription->rayBufferDepth);
    }
    if (tidSweepWriter) {
        pthread_join(tidSweepWriter, NULL);
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
            rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(RKEngineColorSweepEngine) : "",
            rkGlobalParameters.showColor ? RKNoColor : "");
    engine->state = RKEngineStateAllocated;
    engine->memoryUsage = sizeof(RKSweepEngine);
    engine->doNotWrite = false;
    return engine;
}

void RKSweepEngineFree(RKSweepEngine *engine) {
    if (engine->state & RKEngineStateActive) {
        RKSweepEngineStop(engine);
    }
    if (engine->array1D) {
        free(engine->array1D);
        free(engine->array2D);
    }
    free(engine);
}

#pragma mark - Properties

void RKSweepEngineSetVerbose(RKSweepEngine *engine, const int verbose) {
    engine->verbose = verbose;
}

void RKSweepEngineSetInputOutputBuffer(RKSweepEngine *engine, RKRadarDesc *desc, RKFileManager *fileManager,
                                       RKConfig *configBuffer, uint32_t *configIndex,
                                       RKBuffer rayBuffer, uint32_t *rayIndex) {
    engine->radarDescription  = desc;
    engine->fileManager       = fileManager;
    engine->configBuffer      = configBuffer;
    engine->configIndex       = configIndex;
    engine->rayBuffer         = rayBuffer;
    engine->rayIndex          = rayIndex;
    engine->state |= RKEngineStateProperlyWired;
}

void RKSweepEngineSetDoNotWrite(RKSweepEngine *engine, const bool value) {
    engine->doNotWrite = value;
}

void RKSweepEngineSetHandleFilesScript(RKSweepEngine *engine, const char *script, const bool expectTgz) {
    if (RKFilenameExists(script)) {
        strcpy(engine->handleFilesScript, script);
        engine->hasHandleFilesScript = true;
        engine->handleFilesScriptProducesTgz = expectTgz;
    } else {
        RKLog("%s Error. File handler script does not exist.\n", engine->name);
    }
}

#pragma mark - Interactions

int RKSweepEngineStart(RKSweepEngine *engine) {
    if (!(engine->state & RKEngineStateProperlyWired)) {
        RKLog("%s Error. Not properly wired.\n", engine->name);
        return RKResultEngineNotWired;
    }
    if (engine->verbose) {
        RKLog("%s Starting ...\n", engine->name);
    }
    engine->tic = 0;
    engine->state |= RKEngineStateActivating;
    if (pthread_create(&engine->tidRayGatherer, NULL, rayGatherer, engine) != 0) {
        RKLog("Error. Failed to start a ray gatherer.\n");
        return RKResultFailedToStartRayGatherer;
    }
    while (engine->tic == 0) {
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
    engine->state ^= RKEngineStateDeactivating;
    if (engine->verbose) {
        RKLog("%s Stopped.\n", engine->name);
    }
    if (engine->state != (RKEngineStateAllocated | RKEngineStateProperlyWired)) {
        RKLog("%s Inconsistent state 0x%04x\n", engine->name, engine->state);
    }
    return RKResultSuccess;
}

#pragma mark - Reader

void getGlobalTextAttribute(char *dst, const char *name, const int ncid) {
	size_t n = 0;
	nc_get_att_text(ncid, NC_GLOBAL, name, dst);
	nc_inq_attlen(ncid, NC_GLOBAL, name, &n);
	dst[n] = 0;
}

RKSweep *RKSweepCollect(RKSweepEngine *engine) {
	MAKE_FUNCTION_NAME(name)
	RKSweep *sweep = NULL;

	uint8_t anchorIndex = RKPreviousModuloS(engine->rayAnchorsIndex, RKRayAnchorsDepth);
	uint32_t n = engine->rayAnchors[anchorIndex].count;
	RKRay **rays = engine->rayAnchors[anchorIndex].rays;
	//RKLog(">%s %p %p %p ... %p\n", engine->name, rays[0], rays[1], rays[2], rays[n - 1]);

	RKRay *S = rays[0];
	RKRay *T = rays[1];
	RKRay *E = rays[n - 1];
	RKConfig *config = &engine->configBuffer[S->header.configIndex];

	if (engine->verbose > 1) {
		RKLog("%s C%02d-%02d-%02d   E%5.2f/%5.2f-%5.2f   A%6.2f-%6.2f   M%03x-%03x-%03x   (%s x %d, %.1f km)\n",
			  engine->name,
			  S->header.configIndex    , T->header.configIndex    , E->header.configIndex,
			  config->sweepElevation   ,
			  S->header.startElevation , E->header.endElevation   ,
			  S->header.startAzimuth   , E->header.endAzimuth     ,
			  S->header.marker & 0xFFFF, T->header.marker & 0xFFFF, E->header.marker & 0xFFFF,
			  RKIntegerToCommaStyleString(S->header.gateCount), n, 1.0e-3f * S->header.gateCount * S->header.gateSizeMeters);
	}

	int k = 0;
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

	// Allocate the return object
	sweep = (RKSweep *)malloc(sizeof(RKSweep));
	if (sweep == NULL) {
		RKLog("Error. Unable to allocate memory.\n");
		return NULL;
	}
	memset(sweep, 0, sizeof(RKSweep));

	// Populate the contents
	sweep->header.rayCount = n;
	sweep->header.gateCount = S->header.gateCount;
	sweep->header.productList = S->header.productList;
	sweep->header.external = true;
	memcpy(&sweep->header.desc, engine->radarDescription, sizeof(RKRadarDesc));
	memcpy(&sweep->header.config, &engine->configBuffer[S->header.configIndex], sizeof(RKConfig));
	memcpy(sweep->rays, rays + k, n * sizeof(RKRay *));

	return sweep;
}

RKSweep *RKSweepRead(const char *inputFile) {
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
	char symbol[8];
	memset(symbol, 0, 8);
	strncpy(symbol, b, MIN(8, e - b));
	// Substitute symbol with the ones I know
	char symbols[][RKNameLength] = {"Z", "V", "W", "D", "P", "R", "K"};
	RKName productNames[] = {
		"Corrected_Intensity",
		"Radial_Velocity",
		"Width",
		"Differential_Reflectivity",
		"PhiDP",
		"RhoHV",
		"KDP"
	};
	uint32_t products[] = {
		RKProductListProductZ,
		RKProductListProductV,
		RKProductListProductW,
		RKProductListProductD,
		RKProductListProductP,
		RKProductListProductR,
		RKProductListProductK
	};
	uint32_t productIndices[] = {
		RKProductIndexZ,
		RKProductIndexV,
		RKProductIndexW,
		RKProductIndexD,
		RKProductIndexP,
		RKProductIndexR,
		RKProductIndexK
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

			if (gateCount > RKGateCount) {
				RKLog("Info. gateCount = %d capped to %d\n", gateCount, RKGateCount);
				gateCount = RKGateCount;
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
				sweep->rays[j] = RKGetRay(sweep->rayBuffer, j);
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
				r = nc_get_att_float(ncid, NC_GLOBAL, "Latitude", &fv);
				if (r == NC_NOERR) {
					sweep->header.desc.longitude = (double)fv;
				}
			}
			if (!strcmp(scanType, "PPI")) {
				sweep->header.config.sweepElevation = ray->header.sweepElevation;
				sweep->header.config.startMarker |= RKMarkerPPIScan;
			} else if (!strcmp(scanType, "RHI")) {
				sweep->header.config.sweepAzimuth = ray->header.sweepAzimuth;
				sweep->header.config.startMarker |= RKMarkerRHIScan;
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
			if (r != NC_NOERR && sweep->header.config.startMarker & RKMarkerPPIScan) {
				RKLog("Warning. No sweep elevation found.\n");
			}
			r = nc_get_att_float(ncid, NC_GLOBAL, "Azimuth", &ray->header.sweepAzimuth);
			if (r != NC_NOERR && sweep->header.config.startMarker & RKMarkerRHIScan) {
				RKLog("Warning. No sweep azimuth found.\n");
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
					ray = RKGetRay(sweep->rayBuffer, j);
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
					ray = RKGetRay(sweep->rayBuffer, j);
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
					ray = RKGetRay(sweep->rayBuffer, j);
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
					ray = RKGetRay(sweep->rayBuffer, j);
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
	for (k = 0; k < sizeof(symbols) / RKNameLength; k++) {
		b = symbols[k];
		strncpy(filename, inputFile, firstPartLength);
		snprintf(filename + firstPartLength, RKMaximumPathLength - firstPartLength, "%s%s", b, e);
		if (RKFilenameExists(filename)) {
			productList |= products[k];

			// Open the file
			if ((r = nc_open(filename, NC_NOWRITE, &ncid)) > 0) {
				RKLog("%s Error opening file %s (%s)\n", name, inputFile, nc_strerror(r));
				return NULL;
			}

            RKLog("%s %s (*)\n", name, filename);

            // Product
			r = nc_inq_varid(ncid, productNames[k], &tmpId);
			if (r == NC_NOERR) {
				nc_get_var_float(ncid, tmpId, scratch);
				fp = (float *)scratch;
				for (j = 0; j < rayCount; j++) {
					ray = RKGetRay(sweep->rayBuffer, j);
					fp = RKGetFloatDataFromRay(ray, productIndices[k]);
					memcpy(fp, scratch + j * gateCount * sizeof(float), gateCount * sizeof(float));
				}
                nc_close(ncid);
			} else {
				RKLog("%s not found.\n", productNames[k]);
			}
		} else {
			RKLog("%s not found.\n", filename);
		}
	}

	// We are done with the scratch space at this point, we can free it
	free(scratch);

	// This really should not happen
    if (sweep == NULL) {
		RKLog("%s Inconsistent state towards the end.\n", name);
        return NULL;
    }
    
	sweep->header.rayCount = (uint32_t)rayCount;
	sweep->header.gateCount = (uint32_t)gateCount;
	sweep->header.productList = productList;

	for (j = 0; j < rayCount; j++) {
		ray = RKGetRay(sweep->rayBuffer, j);
		ray->header.i += sweep->header.rayCount;
		ray->header.s = RKRayStatusReady;
		ray->header.productList = productList;
	}

	/*
	RKLog("  -> %s%s%s%s%s%s%s\n",
		  productList & RKProductListProductZ ? "Z" : "",
		  productList & RKProductListProductV ? "V" : "",
		  productList & RKProductListProductW ? "W" : "",
		  productList & RKProductListProductD ? "D" : "",
		  productList & RKProductListProductP ? "P" : "",
		  productList & RKProductListProductR ? "R" : "",
		  productList & RKProductListProductK ? "K" : ""
		  );
	*/

	return sweep;
}

int RKSweepFree(RKSweep *sweep) {
	if (sweep == NULL) {
		RKLog("No es bueno, amigo!\n");
		return RKResultNullInput;
	}
	if (!sweep->header.external) {
		if (sweep->rayBuffer == NULL) {
			RKLog("Esto es malo!\n");
			return RKResultNullInput;
		}
		RKRayBufferFree(sweep->rayBuffer);
	}
	free(sweep);
	return RKResultSuccess;
}
