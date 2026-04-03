//
//  RKHostMonitor.c
//  RadarKit
//
//  Created by Boonleng Cheong on 2/24/18.
//  Copyright © Boonleng Cheong. All rights reserved.
//

#include <RadarKit/RKHostMonitor.h>

static char *coloredBoolean(const bool value) {
    static char string[256];
    sprintf(string, "%s%s%s",
        rkGlobalParameters.showColor ? (value ? RKGreenColor : RKMonokaiRed) : "",
        value ? "True" : "False",
        rkGlobalParameters.showColor ? RKNoColor : "");
    return string;
}

//
// Try with port 443, which is commonly open for outbound traffic
// If failed, try with port 80
// If both failed, the host is not reachable
//
static void *hostWatcher(void *in) {
    RKHostMonitor *engine = (RKHostMonitor *)in;

    int j, k, s;
    int sock, result;
    struct sockaddr_in addr;
    struct hostent *hostname;
    struct timeval timeout;
    bool allKnown, allReachable, anyReachable;
    engine->state |= RKEngineStateWantActive | RKEngineStateActive;
    engine->state ^= RKEngineStateActivating;

    RKLog("%s Started.   mem = %s B\n", engine->name, RKUIntegerToCommaStyleString(engine->memoryUsage));

    // Increase the tic once to indicate the engine is ready
    engine->tic = 1;

    // Wait here while the engine should stay active
    while (engine->state & RKEngineStateWantActive) {

        j = rand() % engine->hostCount;

        for (k = 0; k < engine->hostCount; k++) {
            // Process each host
            RKHostTarget *host = &engine->hosts[j];
            hostname = gethostbyname(host->name);
            if (hostname == NULL) {
                RKLog("%s Error. Unable to resolve %s.", engine->name, host->name);
                host->known = false;
                continue;
            } else {
                host->known = true;
            }

            sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock < 0) {
                RKLog("%s Error. Unable to create a socket.\n", engine->name);
                continue;
            }

            timeout.tv_sec = 2;
            timeout.tv_usec = 0;
            setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
            setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

            memset(&addr, 0, sizeof(addr));
            host->port = 443;
            addr.sin_port = htons(host->port);
            addr.sin_family = hostname->h_addrtype;
            addr.sin_addr.s_addr = *(uint32_t *)hostname->h_addr;

            result = connect(sock, (struct sockaddr*)&addr, sizeof(addr));
            if (result) {
                host->port = 80;
                addr.sin_port = htons(host->port);
                result = connect(sock, (struct sockaddr*)&addr, sizeof(addr));
                host->reachable = (result == 0);
            } else {
                host->reachable = true;
            }
            if (engine->verbose > 1) {
                RKLog("%s %u) %s / %d.%d.%d.%d:%d   %s   %s\n", engine->name,
                     j,
                     host->name,
                     addr.sin_addr.s_addr & 0xFF,
                    (addr.sin_addr.s_addr >> 8) & 0xFF,
                    (addr.sin_addr.s_addr >> 16) & 0xFF,
                    (addr.sin_addr.s_addr >> 24) & 0xFF,
                    host->port,
                    RKVariableInString("known", &host->known, RKValueTypeBool),
                    RKVariableInString("reachable", &host->reachable, RKValueTypeBool));
            }
            close(sock);

            // Skip the rest if this host is known and reachable
            if (host->known && host->reachable) {
                break;
            }

            for (s = 0; s < 10 && engine->state & RKEngineStateWantActive; s++) {
                usleep(10000);
            }

            j = RKNextModuloS(j, engine->hostCount);
        }

        allKnown = true;
        allReachable = true;
        anyReachable = false;
        for (k = 0; k < engine->hostCount; k++) {
            RKHostTarget *host = &engine->hosts[k];
            anyReachable |= host->reachable;
            allReachable &= host->reachable;
            allKnown &= host->known;
        }
        engine->anyReachable = anyReachable;
        engine->allReachable = allReachable;
        engine->allKnown = allKnown;
        if (engine->verbose > 1) {
            RKLog("%s allKnown = %s   allReachable = %s   anyReachable = %s\n",
                  engine->name,
                  coloredBoolean(engine->allKnown),
                  coloredBoolean(engine->allReachable),
                  coloredBoolean(engine->anyReachable));
        }

        for (s = 0; s < 10 && engine->state & RKEngineStateWantActive; s++) {
            usleep(RKHostMonitorPingInterval * 100000);
        }
        engine->tic++;
    }

    engine->state ^= RKEngineStateActive;
    return NULL;
}

#pragma mark - Life Cycle

RKHostMonitor *RKHostMonitorInit(void) {
    RKHostMonitor *engine = (RKHostMonitor *)malloc(sizeof(RKHostMonitor));
    if (engine == NULL) {
        RKLog("Error. Unable to allocate a host monitor.\n");
        return NULL;
    }
    memset(engine, 0, sizeof(RKHostMonitor));
    sprintf(engine->name, "%s<InternetMonitor>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(RKEngineColorHostMonitor) : "",
            rkGlobalParameters.showColor ? RKNoColor : "");
    engine->state = RKEngineStateAllocated;
    RKHostMonitorAddHost(engine, "www.x.com");
    RKHostMonitorAddHost(engine, "www.ou.edu");
    RKHostMonitorAddHost(engine, "www.apple.com");
    RKHostMonitorAddHost(engine, "www.amazon.com");
    RKHostMonitorAddHost(engine, "www.nvidia.com");
    RKHostMonitorAddHost(engine, "www.google.com");
    RKHostMonitorAddHost(engine, "www.netflix.com");
    RKHostMonitorAddHost(engine, "www.facebook.com");
    RKHostMonitorAddHost(engine, "www.microsoft.com");
    RKHostMonitorAddHost(engine, "www.cloudflare.com");
    return engine;
}

void RKHostMonitorFree(RKHostMonitor *engine) {
    if (engine->state & RKEngineStateWantActive) {
        RKHostMonitorStop(engine);
    }
    free(engine);
}

#pragma mark - Properties

void RKHostMonitorSetVerbose(RKHostMonitor *engine, const int verbose) {
    engine->verbose = verbose;
}

void RKHostMonitorAddHost(RKHostMonitor *engine, const char *hostname) {
    if (engine->state & RKEngineStateWantActive) {
        RKLog("%s Cannot add host after the engine has started.\n", engine->name);
        return;
    }
    int k;
    for (k = 0; k < engine->hostCount; k++) {
        if (!strcmp(engine->hosts[k].name, hostname)) {
            RKLog("%s Host '%s' already exists.\n", engine->name, hostname);
            return;
        }
    }
    k = engine->hostCount++;
    engine->hosts = realloc(engine->hosts, engine->hostCount * sizeof(RKHostTarget));
    strncpy(engine->hosts[k].name, hostname, RKNameLength - 1);
    engine->hosts[k].name[RKNameLength - 1] = '\0';
    engine->hosts[k].known = true;
    engine->hosts[k].reachable = true;
    engine->memoryUsage = sizeof(RKHostMonitor) + engine->hostCount * sizeof(RKHostTarget);
    engine->state |= RKEngineStateProperlyWired;
}

#pragma mark - Interactions

int RKHostMonitorStart(RKHostMonitor *engine) {
    //RKLog("%s %d %s", engine->name, engine->workerCount, engine->hosts[0]);
    RKLog("%s Starting ...\n", engine->name);
    engine->tic = 0;
    engine->state |= RKEngineStateActivating;
    if (pthread_create(&engine->tidHostWatcher, NULL, hostWatcher, engine) != 0) {
        RKLog("Error. Failed to start a host watcher.\n");
        return RKResultFailedToStartHostWatcher;
    }
    while (engine->tic == 0) {
        usleep(10000);
    }
    return RKResultSuccess;
}

int RKHostMonitorStop(RKHostMonitor *engine) {
    if (engine->state & RKEngineStateDeactivating) {
        if (engine->verbose) {
            RKLog("%s Info. Engine is being or has been deactivated.\n", engine->name);
        }
        return RKResultEngineDeactivatedMultipleTimes;
    }
    if (!(engine->state & RKEngineStateWantActive)) {
        RKLog("%s Not active.\n", engine->name);
        return RKResultEngineDeactivatedMultipleTimes;
    }
    RKLog("%s Stopping ...\n", engine->name);
    engine->state |= RKEngineStateDeactivating;
    engine->state ^= RKEngineStateWantActive;
    pthread_join(engine->tidHostWatcher, NULL);
    engine->state ^= RKEngineStateDeactivating;
    RKLog("%s Stopped.\n", engine->name);
    if (engine->state != (RKEngineStateAllocated | RKEngineStateProperlyWired)) {
        RKLog("%s Inconsistent state 0x%04x\n", engine->name, engine->state);
    }
    return RKResultSuccess;
}
