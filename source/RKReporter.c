//
//  RKReporter.c
//  RadarKit
//
//  Created by Boonleng Cheong on 2/9/22.
//  Copyright (c) 2017-2023 Boonleng Cheong. All rights reserved.
//

#include <RadarKit/RKReporter.h>

// Private declarations

#define PAYLOAD_CAPACITY    (1024 * 1024)

const char healthString[][2400] = {
    "{\"Transceiver\":{\"Value\":true,\"Enum\":0}, \"Pedestal\":{\"Value\":true,\"Enum\":0}, \"Health Relay\":{\"Value\":false,\"Enum\":2}, \"Internet\":{\"Value\":true,\"Enum\":0}, \"Recorder\":{\"Value\":false,\"Enum\":1}, \"Ring Filter\":{\"Value\":false,\"Enum\":1}, \"Processors\":{\"Value\":true,\"Enum\":0}, \"Measured PRF\":{\"Value\":\"1,475 Hz\",\"Enum\":0}, \"Noise\":[50.274,33.654], \"Position Rate\":{\"Value\":\"249 Hz\",\"Enum\":0}, \"rayRate\":6.010, \"10-MHz Clock\":{\"Value\":true,\"Enum\":0}, \"DAC PLL\":{\"Value\":true,\"Enum\":0}, \"FPGA Temp\":{\"Value\":\"61.5degC\",\"Enum\":0}, \"Core Volt\":{\"Value\":\"1.00 V\",\"Enum\":0}, \"Aux. Volt\":{\"Value\":\"2.466 V\",\"Enum\":0}, \"XMC Volt\":{\"Value\":\"12.223 V\",\"Enum\":0}, \"XMC 3p3\":{\"Value\":\"3.222 V\",\"Enum\":0}, \"Transmit H\":{\"Value\":\"57.625 dBm\",\"Enum\":0,\"MaxIndex\":0,\"Max\":\"4.282 dBm\",\"Min\":\"3.827 dBm\"}, \"Transmit V\":{\"Value\":\"53.309 dBm\",\"Enum\":0,\"MaxIndex\":1,\"Max\":\"-0.042 dBm\",\"Min\":\"-0.485 dBm\"}, \"DAC QI\":{\"Value\":\"0.237\",\"Enum\":1}, \"Waveform\":{\"Value\":\"x706\",\"Enum\":0}, \"UnderOver\":[0,233116], \"Lags\":[-1167035501,-1167035501,-1185559874], \"NULL\":[1602822], \"Pedestal AZ Interlock\":{\"Value\":true,\"Enum\":0}, \"Pedestal EL Interlock\":{\"Value\":true,\"Enum\":0}, \"VCP Active\":{\"Value\":true,\"Enum\":0}, \"Pedestal AZ\":{\"Value\":\"337.89 deg\",\"Enum\":0}, \"Pedestal EL\":{\"Value\":\"3.18 deg\",\"Enum\":0}, \"Pedestal Update\":\"251.021 Hz\", \"PedestalHealthEnd\":0, \"GPS Valid\":{\"Value\":true,\"Enum\":3}, \"GPS Latitude\":{\"Value\":\"35.2369167\",\"Enum\":3}, \"GPS Longitude\":{\"Value\":\"-97.4638233\",\"Enum\":3}, \"T-Box Bearing\":{\"Value\":\"-448.9 deg\", \"Enum\":0}, \"T-Box Temp\":{\"Value\":\"19.6 degC\", \"Enum\":0}, \"T-Box Pressure\":{\"Value\":\"985.7 hPa\", \"Enum\":0}, \"RF TRX Health\":{\"Value\":\"0x70\", \"Enum\":2}, \"RF Over Temp H\":{\"Value\":true, \"Enum\":0}, \"RF Over Temp V\":{\"Value\":false, \"Enum\":2}, \"RF VSWR H\":{\"Value\":true, \"Enum\":1}, \"RF VSWR V\":{\"Value\":true, \"Enum\":0}, \"STALO\":{\"Value\":true, \"Enum\":0}, \"tic\":\"31008358\", \"Heading Override\":{\"Value\":true,\"Enum\":0}, \"Sys Heading\":{\"Value\":\"181.00 deg\",\"Enum\":0}, \"GPS Override\":{\"Value\":true,\"Enum\":0}, \"Sys Latitude\":{\"Value\":\"35.2369467\",\"Enum\":0}, \"Sys Longitude\":{\"Value\":\"-97.4638167\",\"Enum\":0}, \"Log Time\":1570804516}",
    "{\"Transceiver\":{\"Value\":true,\"Enum\":0}, \"Pedestal\":{\"Value\":true,\"Enum\":0}, \"Health Relay\":{\"Value\":false,\"Enum\":2}, \"Internet\":{\"Value\":true,\"Enum\":1}, \"Recorder\":{\"Value\":false,\"Enum\":2}, \"Ring Filter\":{\"Value\":false,\"Enum\":1}, \"Processors\":{\"Value\":true,\"Enum\":0}, \"Measured PRF\":{\"Value\":\"1,476 Hz\",\"Enum\":0}, \"Noise\":[50.274,33.654], \"Position Rate\":{\"Value\":\"251 Hz\",\"Enum\":0}, \"rayRate\":6.001, \"10-MHz Clock\":{\"Value\":true,\"Enum\":0}, \"DAC PLL\":{\"Value\":true,\"Enum\":0}, \"FPGA Temp\":{\"Value\":\"61.4degC\",\"Enum\":0}, \"Core Volt\":{\"Value\":\"1.01 V\",\"Enum\":0}, \"Aux. Volt\":{\"Value\":\"2.464 V\",\"Enum\":0}, \"XMC Volt\":{\"Value\":\"12.225 V\",\"Enum\":0}, \"XMC 3p3\":{\"Value\":\"3.220 V\",\"Enum\":0}, \"Transmit H\":{\"Value\":\"57.511 dBm\",\"Enum\":0,\"MaxIndex\":0,\"Max\":\"4.282 dBm\",\"Min\":\"3.827 dBm\"}, \"Transmit V\":{\"Value\":\"53.309 dBm\",\"Enum\":0,\"MaxIndex\":1,\"Max\":\"-0.042 dBm\",\"Min\":\"-0.485 dBm\"}, \"DAC QI\":{\"Value\":\"0.881\",\"Enum\":0}, \"Waveform\":{\"Value\":\"x706\",\"Enum\":0}, \"UnderOver\":[0,233116], \"Lags\":[-1167035501,-1167035501,-1185559874], \"NULL\":[1602822], \"Pedestal AZ Interlock\":{\"Value\":true,\"Enum\":0}, \"Pedestal EL Interlock\":{\"Value\":true,\"Enum\":0}, \"VCP Active\":{\"Value\":true,\"Enum\":0}, \"Pedestal AZ\":{\"Value\":\"337.89 deg\",\"Enum\":0}, \"Pedestal EL\":{\"Value\":\"3.18 deg\",\"Enum\":0}, \"Pedestal Update\":\"251.021 Hz\", \"PedestalHealthEnd\":0, \"GPS Valid\":{\"Value\":true,\"Enum\":3}, \"GPS Latitude\":{\"Value\":\"35.2369165\",\"Enum\":3}, \"GPS Longitude\":{\"Value\":\"-97.4638230\",\"Enum\":3}, \"T-Box Bearing\":{\"Value\":\"-448.9 deg\", \"Enum\":0}, \"T-Box Temp\":{\"Value\":\"19.6 degC\", \"Enum\":0}, \"T-Box Pressure\":{\"Value\":\"984.3 hPa\", \"Enum\":0}, \"RF TRX Health\":{\"Value\":\"0x71\", \"Enum\":0}, \"RF Over Temp H\":{\"Value\":true, \"Enum\":0}, \"RF Over Temp V\":{\"Value\":false, \"Enum\":2}, \"RF VSWR H\":{\"Value\":true, \"Enum\":0}, \"RF VSWR V\":{\"Value\":true, \"Enum\":0}, \"STALO\":{\"Value\":true, \"Enum\":0}, \"tic\":\"31008358\", \"Heading Override\":{\"Value\":true,\"Enum\":1}, \"Sys Heading\":{\"Value\":\"180.00 deg\",\"Enum\":0}, \"GPS Override\":{\"Value\":true,\"Enum\":0}, \"Sys Latitude\":{\"Value\":\"35.2369467\",\"Enum\":0}, \"Sys Longitude\":{\"Value\":\"-97.4638167\",\"Enum\":0}, \"Log Time\":1570804517}",
    "{\"Transceiver\":{\"Value\":true,\"Enum\":0}, \"Pedestal\":{\"Value\":true,\"Enum\":0}, \"Health Relay\":{\"Value\":false,\"Enum\":2}, \"Internet\":{\"Value\":true,\"Enum\":2}, \"Recorder\":{\"Value\":false,\"Enum\":0}, \"Ring Filter\":{\"Value\":false,\"Enum\":1}, \"Processors\":{\"Value\":true,\"Enum\":0}, \"Measured PRF\":{\"Value\":\"1,475 Hz\",\"Enum\":0}, \"Noise\":[50.274,33.654], \"Position Rate\":{\"Value\":\"250 Hz\",\"Enum\":0}, \"rayRate\":6.005, \"10-MHz Clock\":{\"Value\":true,\"Enum\":0}, \"DAC PLL\":{\"Value\":true,\"Enum\":1}, \"FPGA Temp\":{\"Value\":\"61.4degC\",\"Enum\":0}, \"Core Volt\":{\"Value\":\"1.02 V\",\"Enum\":0}, \"Aux. Volt\":{\"Value\":\"2.468 V\",\"Enum\":0}, \"XMC Volt\":{\"Value\":\"12.224 V\",\"Enum\":0}, \"XMC 3p3\":{\"Value\":\"3.181 V\",\"Enum\":0}, \"Transmit H\":{\"Value\":\"57.326 dBm\",\"Enum\":0,\"MaxIndex\":0,\"Max\":\"4.282 dBm\",\"Min\":\"3.827 dBm\"}, \"Transmit V\":{\"Value\":\"53.309 dBm\",\"Enum\":0,\"MaxIndex\":1,\"Max\":\"-0.042 dBm\",\"Min\":\"-0.485 dBm\"}, \"DAC QI\":{\"Value\":\"0.881\",\"Enum\":0}, \"Waveform\":{\"Value\":\"x706\",\"Enum\":0}, \"UnderOver\":[0,233116], \"Lags\":[-1167035501,-1167035501,-1185559874], \"NULL\":[1602822], \"Pedestal AZ Interlock\":{\"Value\":true,\"Enum\":0}, \"Pedestal EL Interlock\":{\"Value\":true,\"Enum\":0}, \"VCP Active\":{\"Value\":true,\"Enum\":0}, \"Pedestal AZ\":{\"Value\":\"337.89 deg\",\"Enum\":0}, \"Pedestal EL\":{\"Value\":\"3.18 deg\",\"Enum\":0}, \"Pedestal Update\":\"251.021 Hz\", \"PedestalHealthEnd\":0, \"GPS Valid\":{\"Value\":true,\"Enum\":3}, \"GPS Latitude\":{\"Value\":\"35.2369165\",\"Enum\":3}, \"GPS Longitude\":{\"Value\":\"-97.4638230\",\"Enum\":3}, \"T-Box Bearing\":{\"Value\":\"-448.9 deg\", \"Enum\":0}, \"T-Box Temp\":{\"Value\":\"19.6 degC\", \"Enum\":0}, \"T-Box Pressure\":{\"Value\":\"985.0 hPa\", \"Enum\":0}, \"RF TRX Health\":{\"Value\":\"0x71\", \"Enum\":0}, \"RF Over Temp H\":{\"Value\":true, \"Enum\":0}, \"RF Over Temp V\":{\"Value\":false, \"Enum\":2}, \"RF VSWR H\":{\"Value\":true, \"Enum\":0}, \"RF VSWR V\":{\"Value\":true, \"Enum\":0}, \"STALO\":{\"Value\":true, \"Enum\":1}, \"tic\":\"31008358\", \"Heading Override\":{\"Value\":true,\"Enum\":0}, \"Sys Heading\":{\"Value\":\"180.50 deg\",\"Enum\":0}, \"GPS Override\":{\"Value\":true,\"Enum\":0}, \"Sys Latitude\":{\"Value\":\"35.2369467\",\"Enum\":0}, \"Sys Longitude\":{\"Value\":\"-97.4638167\",\"Enum\":0}, \"Log Time\":1570804518}",
    "{\"Transceiver\":{\"Value\":true,\"Enum\":0}, \"Pedestal\":{\"Value\":true,\"Enum\":0}, \"Health Relay\":{\"Value\":false,\"Enum\":2}, \"Internet\":{\"Value\":true,\"Enum\":1}, \"Recorder\":{\"Value\":false,\"Enum\":2}, \"Ring Filter\":{\"Value\":false,\"Enum\":1}, \"Processors\":{\"Value\":true,\"Enum\":0}, \"Measured PRF\":{\"Value\":\"1,476 Hz\",\"Enum\":0}, \"Noise\":[50.274,33.654], \"Position Rate\":{\"Value\":\"250 Hz\",\"Enum\":0}, \"rayRate\":6.005, \"10-MHz Clock\":{\"Value\":true,\"Enum\":0}, \"DAC PLL\":{\"Value\":true,\"Enum\":1}, \"FPGA Temp\":{\"Value\":\"61.4degC\",\"Enum\":0}, \"Core Volt\":{\"Value\":\"1.01 V\",\"Enum\":0}, \"Aux. Volt\":{\"Value\":\"2.463 V\",\"Enum\":0}, \"XMC Volt\":{\"Value\":\"12.222 V\",\"Enum\":0}, \"XMC 3p3\":{\"Value\":\"3.196 V\",\"Enum\":0}, \"Transmit H\":{\"Value\":\"57.326 dBm\",\"Enum\":0,\"MaxIndex\":0,\"Max\":\"4.282 dBm\",\"Min\":\"3.827 dBm\"}, \"Transmit V\":{\"Value\":\"53.309 dBm\",\"Enum\":0,\"MaxIndex\":1,\"Max\":\"-0.042 dBm\",\"Min\":\"-0.485 dBm\"}, \"DAC QI\":{\"Value\":\"0.881\",\"Enum\":0}, \"Waveform\":{\"Value\":\"x706\",\"Enum\":0}, \"UnderOver\":[0,233116], \"Lags\":[-1167035501,-1167035501,-1185559874], \"NULL\":[1602822], \"Pedestal AZ Interlock\":{\"Value\":true,\"Enum\":0}, \"Pedestal EL Interlock\":{\"Value\":true,\"Enum\":0}, \"VCP Active\":{\"Value\":true,\"Enum\":0}, \"Pedestal AZ\":{\"Value\":\"337.89 deg\",\"Enum\":0}, \"Pedestal EL\":{\"Value\":\"3.18 deg\",\"Enum\":0}, \"Pedestal Update\":\"251.021 Hz\", \"PedestalHealthEnd\":0, \"GPS Valid\":{\"Value\":true,\"Enum\":3}, \"GPS Latitude\":{\"Value\":\"35.2369165\",\"Enum\":3}, \"GPS Longitude\":{\"Value\":\"-97.4638230\",\"Enum\":3}, \"T-Box Bearing\":{\"Value\":\"-448.9 deg\", \"Enum\":0}, \"T-Box Temp\":{\"Value\":\"19.6 degC\", \"Enum\":0}, \"T-Box Pressure\":{\"Value\":\"985.0 hPa\", \"Enum\":0}, \"RF TRX Health\":{\"Value\":\"0x71\", \"Enum\":0}, \"RF Over Temp H\":{\"Value\":true, \"Enum\":0}, \"RF Over Temp V\":{\"Value\":false, \"Enum\":2}, \"RF VSWR H\":{\"Value\":true, \"Enum\":0}, \"RF VSWR V\":{\"Value\":true, \"Enum\":1}, \"STALO\":{\"Value\":true, \"Enum\":0}, \"tic\":\"31008358\", \"Heading Override\":{\"Value\":true,\"Enum\":0}, \"Sys Heading\":{\"Value\":\"180.75 deg\",\"Enum\":0}, \"GPS Override\":{\"Value\":true,\"Enum\":0}, \"Sys Latitude\":{\"Value\":\"35.2369467\",\"Enum\":0}, \"Sys Longitude\":{\"Value\":\"-97.4638167\",\"Enum\":0}, \"Log Time\":1570804519}"
};

#pragma mark - Busy Loop

void *reporter(void *in) {
    RKReporter *engine = (RKReporter *)in;
    RKRadar *radar = engine->radar;

    int j;
    int k = 0;
    int s = 0;
    char *c;
    uint32_t index;

    void *payload = malloc(PAYLOAD_CAPACITY);
    size_t payload_size = 0;

    RKLog("%s Started.  mem = %s B\n", engine->name, RKIntegerToCommaStyleString(engine->memoryUsage));

    engine->state |= RKEngineStateActive;

    while (engine->state & RKEngineStateActive) {
        RKLog("%s Busy %d\n", engine->name, k);
        
        // Health Status
        #pragma mark Health Status
        
        if (engine->streams & RKStreamHealthInJSON) {
            index = RKPreviousModuloS(radar->healthIndex, radar->desc.healthBufferDepth);

            if (!(engine->streamsInProgress & RKStreamHealthInJSON)) {
                engine->streamsInProgress |= RKStreamHealthInJSON;
                if (engine->verbose) {
                    RKLog("%s Begin streaming RKHealth -> %d (0x%x %s).\n", engine->name, index,
                          radar->healths[index].flag,
                          radar->healths[index].flag == RKStatusFlagVacant ? "vacant" : "ready");
                }
                engine->healthIndex = index;
            }

            s = 0;
            while (radar->healths[engine->healthIndex].flag != RKHealthFlagReady && engine->ws->connected && s++ < 20) {
                if (s % 10 == 0 && engine->verbose > 1) {
                    RKLog("%s sleep 0/%.1f s  RKHealth\n", engine->name, s * 0.1f);
                }
                usleep(50000);
            }

            if (radar->healths[engine->healthIndex].flag == RKHealthFlagReady && engine->ws->connected) {
                j = 0;
                k = 0;
//                while (engine->healthIndex != endIndex && k < RKMaximumStringLength - 200) {
//                    c = radar->healths[engine->healthIndex].string;
//                    k += sprintf(engine->string + k, "%s\n", c);
//                    engine->healthIndex = RKNextModuloS(engine->healthIndex, radar->desc.healthBufferDepth);
//                    j++;
//                }
//                if (j) {
//                    // Take out the last '\n', replace it with somethign else + EOL
//                    snprintf(engine->string + k - 1, RKMaximumStringLength - k - 1, "" RKEOL);
//                    engine->delimTx.type = RKNetworkPacketTypeHealth;
//                    engine->delimTx.size = k;
//                    RKOperatorSendPackets(O, &O->delimTx, sizeof(RKNetDelimiter), user->string, O->delimTx.size, NULL);
//                }
                while (engine->healthIndex != index && k < RKMaximumStringLength - 200) {
                    payload_size = snprintf(payload, PAYLOAD_CAPACITY, "%c%s", RKRadarHubTypeHealth, radar->healths[engine->healthIndex].string);
                    RKLog("%s Sending health packet %z\n", engine->name, payload_size);
                    RKWebSocketSend(engine->ws, payload, payload_size + 1);
                    engine->healthIndex = RKNextModuloS(engine->healthIndex, radar->desc.healthBufferDepth);
                }
            } else {
                RKLog("%s No Health / Deactivated.   healthIndex = %d / %d\n", engine->name, engine->healthIndex, radar->healthIndex);
            }

        }
        
        s = 0;
        do {
            usleep(100000);
        } while (engine->state & RKEngineStateActive && s++ < 10);
    }

    return NULL;
}

#pragma mark - Helper Functions

#pragma mark - Delegate Workers

void handleOpen(RKWebSocket *w) {
    RKReporter *R = (RKReporter *)w->parent;
    if (R->verbose) {
        printf("RKReporter: ONOPEN\n");
    }
    int r;
    r = sprintf(R->welcome,
        "%c{"
            "\"name\":\"%s\", "
            "\"command\":\"radarConnect\""
        "}",
        RadarHubTypeHandshake, R->radar->desc.name);
    RKLog("%s Sending open packet ...\n", R->name);
    printf("%s\n", R->welcome);
    RKWebSocketSend(w, R->welcome, r);
//    sendControl(w);
}

void handleClose(RKWebSocket *W) {
//    R->connected = false;
//    if (R->verbose) {
//        printf("ONCLOSE\n");
//    }
    RKReporter *reporter = (RKReporter *)W->parent;
    if (reporter->state & RKEngineStateActive) {
        reporter->state ^= RKEngineStateActive;
    }
    RKLog("%s WebSocket handleClose()\n", reporter->name);
}

void handleMessage(RKWebSocket *W, void *payload, size_t size) {
    RKReporter *reporter = (RKReporter *)W->parent;
    RKRadar *radar = reporter->radar;
}


#pragma mark - Life Cycle

RKReporter *RKReporterInitWithHost(const char *host) {
    RKReporter *engine = (RKReporter *)malloc(sizeof(RKReporter));
    if (engine == NULL) {
        RKLog("Error. Unable to allocate an RKReporter.\n");
        return NULL;
    }
    memset(engine, 0, sizeof(RKReporter));
    sprintf(engine->name, "%s<RadarHubConnect>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(RKEngineColorRadarHubReporter) : "",
            rkGlobalParameters.showColor ? RKNoColor : "");
    engine->memoryUsage = sizeof(RKReporter);
    strncpy(engine->host, host, RKNameLength);
    RKLog("%s host = %s\n", engine->name, engine->host);
    if (strstr(engine->host, "https") != NULL) {
        engine->flag = RKWebSocketFlagSSLOn;
    } else {
        engine->flag = RKWebSocketFlagSSLOff;
    }
    char *c = strstr(engine->host, "://");
    if (c) {
        size_t l = strlen(engine->host) - (size_t)(c - engine->host) - 3;
        memmove(engine->host, c + 3, l);
        engine->host[l] = '\0';
    }
    if (engine->flag & RKWebSocketFlagSSLOn && strstr(engine->host, ":") == NULL) {
        strcat(engine->host, ":443");
    }
    if (strstr(engine->host, ":443")) {
        engine->flag = RKWebSocketFlagSSLOn;
    }
    RKLog("%s revised host = %s (SSL %s)\n", engine->name, engine->host, engine->flag & RKWebSocketFlagSSLOn ? "On" : "Off");
    // Default properties
    engine->streams = RKStreamHealthInJSON;
    return engine;
}

RKReporter *RKReporterInitForRadarHub(void) {
//    return RKReporterInitWithHost("https://radarhub.arrc.ou.edu");
//    return RKReporterInitWithHost("radarhub.arrc.ou.edu:443");
    return RKReporterInitWithHost("10.197.14.52:8001");
}

RKReporter *RKReporterInitForLocal(void) {
    return RKReporterInitWithHost("http://localhost:8001");
}

RKReporter *RKReporterInit(void) {
    return RKReporterInitForRadarHub();
}

void RKReporterFree(RKReporter *engine) {
    free(engine);
}

#pragma mark - Properties

void RKReporterSetVerbose(RKReporter *engine, const int verbose) {
    engine->verbose = verbose;
    if (engine->ws) {
        RKWebSocketSetVerbose(engine->ws, engine->verbose);
    }
}

void RKReporterSetRadar(RKReporter *engine, RKRadar *radar) {
    engine->radar = radar;
    RKName node;
    strcpy(node, radar->desc.name);
    RKStringLower(node);
    sprintf(engine->address, "/ws/radar/%s/", node);
    RKLog("%s Setting up radar %s @ %s\n", engine->name, radar->desc.name, engine->address);
    engine->ws = RKWebSocketInit(engine->host, engine->address, engine->flag);
    RKWebSocketSetOpenHandler(engine->ws, &handleOpen);
    RKWebSocketSetCloseHandler(engine->ws, &handleClose);
    RKWebSocketSetMessageHandler(engine->ws, &handleMessage);
    RKWebSocketSetVerbose(engine->ws, engine->verbose);
    RKWebSocketSetParent(engine->ws, engine);
}

#pragma mark - Interactions

void RKReporterStart(RKReporter *engine) {
    RKLog("%s Starting %s:%s ...\n", engine->name, engine->ws->host, engine->ws->path);
    RKWebSocketStart(engine->ws);
    if (pthread_create(&engine->ticWorker, NULL, reporter, engine)) {
        RKLog("%s Error. Failed to create reporter().\n", engine->name);
        return;
    }
}

void RKReporterStop(RKReporter *engine) {
    if (engine->verbose) {
        RKLog("%s Stopping ...\n", engine->name);
    }
    RKLog("%s Disconnecting %s:%s ...\n", engine->name, engine->ws->host, engine->ws->path);
    RKWebSocketStop(engine->ws);
    RKLog("%s Stopped.\n", engine->name);
}
