//
//  RKHostMonitor.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 2/24/18.
//  Copyright © Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKHostMonitor.h>

#define RKHostMonitorPacketSize 32

enum {
    RKICMPv4EchoRequest = 8,
    RKICMPv4EchoReply = 0
};

typedef struct rk_icmp_header {
    uint8_t     type;                // 8 for IPV4, 128 for IPV6
    uint8_t     code;                // Always 0
    uint16_t    checksum;
    uint16_t    identifier;
    uint16_t    sequenceNumber;
    // data...
} RKICMPHeader;

typedef struct rk_ipv4_header {
    uint8_t     versionAndHeaderLength;
    uint8_t     differentiatedServices;
    uint16_t    totalLength;
    uint16_t    identification;
    uint16_t    flagsAndFragmentOffset;
    uint8_t     timeToLive;
    uint8_t     protocol;
    uint16_t    headerChecksum;
    uint8_t     sourceAddress[4];
    uint8_t     destinationAddress[4];
    // options...
    // data...
} RKIPV4Header;

#pragma mark - Helper Functions

static uint16_t rk_host_monitor_checksum (void *in, size_t len) {
    uint16_t *buf = (uint16_t *)in;
    uint16_t sum = 0;
    for (sum = 0; len > 1; len -= 2) {
        sum += *buf++;
    }
    if (len == 1) {
        sum += *(uint8_t *)buf;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return ~sum;
}

#pragma mark - Delegate Workers

static void *hostPinger(void *in) {
    RKUnitMonitor *me = (RKUnitMonitor *)in;
    RKHostMonitor *engine = me->parent;

    int k;
    int sd;
    struct sockaddr_in targetAddress;
    struct sockaddr_in returnAddress;

    const int c = me->id;
    const int value = 50;
    struct protoent *protocol = getprotobyname("ICMP");
    struct hostent *hostname = gethostbyname(engine->hosts[c]);

    char name[RKNameLength];
    
    char buf[RKHostMonitorPacketSize];
    RKICMPHeader *icmpHeader;
    RKIPV4Header *ipv4Header = (RKIPV4Header *)buf;
    socklen_t returnLength;
    uint16_t receivedChecksum;
    uint16_t calculatedChecksum;
    
    ssize_t r = 0;
    size_t txSize = RKHostMonitorPacketSize - sizeof(RKIPV4Header);
    size_t ipHeaderLength, offset;
    
    if (rkGlobalParameters.showColor) {
        pthread_mutex_lock(&engine->mutex);
        k = snprintf(name, RKNameLength - 1, "%s", rkGlobalParameters.showColor ? RKGetColor() : "");
        pthread_mutex_unlock(&engine->mutex);
    } else {
        k = 0;
    }
    if (engine->workerCount > 9) {
        k += sprintf(name + k, "H%02d", c);
    } else {
        k += sprintf(name + k, "H%d", c);
    }
    if (rkGlobalParameters.showColor) {
        sprintf(name + k, RKNoColor);
    }

    if (engine->verbose) {
        RKLog(">%s %s Started.   host = %s\n",
              engine->name, name, engine->hosts[c]);
    }
    
    me->tic++;
    me->sequenceNumber = 0;
    me->identifier = rand() & 0xffff;
    
    // Resolve my host
    memset(&targetAddress, 0, sizeof(struct sockaddr));
    targetAddress.sin_family = hostname->h_addrtype;
    targetAddress.sin_port = htons(80);
    targetAddress.sin_addr.s_addr = *(unsigned int *)hostname->h_addr;
    if (engine->verbose) {
        RKLog(">%s %s %s -> 0x%08x -> %d.%d.%d.%d (%d)\n",
              engine->name, name, engine->hosts[c], targetAddress.sin_addr.s_addr,
              (targetAddress.sin_addr.s_addr & 0xff),
              (targetAddress.sin_addr.s_addr & 0x0000ff00) >> 8,
              (targetAddress.sin_addr.s_addr & 0x00ff0000) >> 16,
              (targetAddress.sin_addr.s_addr & 0xff000000) >> 24, hostname->h_length);
    }

    // Open a socket, set some properties
    if ((sd = socket(AF_INET, SOCK_DGRAM, protocol->p_proto)) < 0) {
        RKLog("%s %s Error. Unable to open a socket.  sd = %d\n", engine->name, name, sd);
        return NULL;
    }
    if (setsockopt(sd, IPPROTO_IP, IP_TTL, &value, sizeof(value))) {
        RKLog("%s %s Error. Failed in setsockopt().\n", engine->name, name);
        return NULL;
    }
    if (fcntl(sd, F_SETFL, O_NONBLOCK)) {
        RKLog("%s %s Error. Failed in fcntl().\n", engine->name, name);
        return NULL;
    }
    
    while (engine->state & RKEngineStateActive) {
        // Pong
        if ((r = recvfrom(sd, &buf, RKNameLength, 0, (struct sockaddr *)&returnAddress, &returnLength)) > 0) {
            offset = (size_t)-1;
            if (r > sizeof(RKIPV4Header) + sizeof(RKICMPHeader) && ipv4Header->protocol == IPPROTO_ICMP) {
                ipHeaderLength = (ipv4Header->versionAndHeaderLength & 0x0f) * sizeof(uint32_t);
                if (r >= ipHeaderLength + sizeof(RKICMPHeader)) {
                    offset = ipHeaderLength;
                }
            }
            if (offset != (size_t)-1) {
                icmpHeader = (RKICMPHeader *)(buf + offset);
                receivedChecksum = icmpHeader->checksum;
                icmpHeader->checksum = 0;
                calculatedChecksum = rk_host_monitor_checksum(buf + offset, r - offset);
                if (receivedChecksum == calculatedChecksum &&
                    icmpHeader->type == RKICMPv4EchoReply &&
                    icmpHeader->code == 0 &&
                    icmpHeader->identifier == me->identifier &&
                    icmpHeader->sequenceNumber == me->sequenceNumber) {
                    me->state = RKHostStateReachable;
                    if (engine->verbose > 1) {
                        RKLog(">%s %s Got sequenceNumber %d (receiveAddress length %d)\n",
                              engine->name, name, icmpHeader->sequenceNumber, returnLength);
                    }
                }
            }
        } else {
            me->state = RKHostStateUnreachable;
        }

        // Reset buffer
        memset(buf, 0, sizeof(buf));

        // Prepare echo request packet
        icmpHeader = (RKICMPHeader *)buf;
        icmpHeader->type = RKICMPv4EchoRequest;
        icmpHeader->code = 0;
        icmpHeader->checksum = 0;
        icmpHeader->identifier = me->identifier;
        icmpHeader->sequenceNumber = ++me->sequenceNumber;
        icmpHeader->checksum = rk_host_monitor_checksum(buf, txSize);

        // Ping
        if ((r = sendto(sd, buf, txSize, 0, (struct sockaddr *)&targetAddress, sizeof(struct sockaddr))) == -1) {
            RKLog(">%s %s Error in sendto() -> %d  %d.", engine->name, name, r, errno);
        } else if (engine->verbose > 1) {
            RKLog(">%s %s Ping %s with %d bytes\n", engine->name, name, engine->hosts[c], txSize);
        }

        // Now we wait
        k = 0;
        while (k++ < 20 && engine->state & RKEngineStateActive) {
            usleep(100000);
        }
    }

    if (engine->verbose) {
        RKLog(">%s %s Stopped.\n", engine->name, name);
    }
    
    return NULL;
}

static void *hostWatcher(void *in) {
    RKHostMonitor *engine = (RKHostMonitor *)in;
    
    int k;
    
    engine->state |= RKEngineStateActive;
    engine->state ^= RKEngineStateActivating;
    
    engine->workers = (RKUnitMonitor *)malloc(engine->workerCount * sizeof(RKUnitMonitor));
    if (engine->workers == NULL) {
        RKLog(">%s Error. Unable to allocate an RKUnitMonitor.\n", engine->name);
        return (void *)RKResultFailedToCreateUnitWorker;
    }
    memset(engine->workers, 0, engine->workerCount * sizeof(RKUnitMonitor));
    engine->memoryUsage += engine->workerCount * sizeof(RKUnitMonitor);
    
    for (k = 0; k < engine->workerCount; k++) {
        RKUnitMonitor *worker = &engine->workers[k];
        
        worker->id = k;
        worker->parent = engine;
        
        // Workers that actually ping the hosts
        if (pthread_create(&worker->tid, NULL, hostPinger, worker) != 0) {
            RKLog(">%s Error. Failed to start a host pinger", engine->name);
            return (void *)RKResultFailedToStartHostPinger;
        }
        
        while (worker->tic == 0 && engine->state & RKEngineStateActive) {
            usleep(10000);
        }
    }
    
    // Increase the tic once to indicate the engine is ready
    engine->tic++;
    
    if (engine->verbose) {
        RKLog("%s Started.   mem = %s B  state = %x\n", engine->name, RKIntegerToCommaStyleString(engine->memoryUsage), engine->state);
    }

    // Wait here while the engine should stay active
    while (engine->state & RKEngineStateActive) {
        // Consolidate all the state from all unit watcher
        for (k = 0; k < engine->workerCount; k++) {
            RKUnitMonitor *worker = &engine->workers[k];
            if (worker->sequenceNumber > 1) {
                RKLog(">%s %s (%d) %s\n", engine->name,
                      engine->hosts[k],
                      worker->sequenceNumber,
                      worker->state == RKHostStateReachable ? "ok" : "not reachable");
            }
        }
        // Wait one minute, do it with multiples of 0.1s for a responsive exit
        k = 0;
        while (k++ < 20 && engine->state & RKEngineStateActive) {
            usleep(100000);
        }
    }

    for (k = 0; k < engine->workerCount; k++) {
        pthread_join(engine->workers[k].tid, NULL);
    }
    free(engine->workers);

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
    sprintf(engine->name, "%s<NetworInspector>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(RKEngineColorHostMonitor) : "",
            rkGlobalParameters.showColor ? RKNoColor : "");
    engine->state = RKEngineStateAllocated;
    engine->hosts = (RKHostAddress *)malloc(sizeof(RKHostAddress));
    memset(engine->hosts, 0, sizeof(RKHostAddress));
    pthread_mutex_init(&engine->mutex, NULL);
    RKHostMonitorAddHost(engine, "bumblebee.arrc.ou.edu");
    return engine;
}

void RKHostMonitorFree(RKHostMonitor *engine) {
    if (engine->state & RKEngineStateActive) {
        RKHostMonitorStop(engine);
    }
    pthread_mutex_destroy(&engine->mutex);
    free(engine);
}

#pragma mark - Properties

void RKHostMonitorSetVerbose(RKHostMonitor *engine, const int verbose) {
    engine->verbose = verbose;
}

void RKHostMonitorAddHost(RKHostMonitor *engine, const char *address) {
    int k = engine->workerCount++;
    engine->hosts = realloc(engine->hosts, engine->workerCount * sizeof(RKHostAddress));
    strncpy(engine->hosts[k], address, RKNameLength - 1);
    engine->memoryUsage = sizeof(RKHostMonitor) + engine->workerCount * sizeof(RKHostAddress);
}

#pragma mark - Interactions

int RKHostMonitorStart(RKHostMonitor *engine) {
    //RKLog("%s %d %s", engine->name, engine->workerCount, engine->hosts[0]);
    if (engine->verbose) {
        RKLog("%s Starting ...\n", engine->name);
    }
    engine->state |= RKEngineStateActivating;
    if (pthread_create(&engine->tidHostWatcher, NULL, hostWatcher, engine) != 0) {
        RKLog("Error. Failed to start a host watcher.\n");
        return RKResultFailedToStartHostWatcher;
    }
    while (engine->tic == 0) {
        usleep(1000);
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
    if (engine->verbose) {
        RKLog("%s Stopping ...\n", engine->name);
    }
    engine->state |= RKEngineStateDeactivating;
    engine->state ^= RKEngineStateActive;
    // pthread_join ...
    engine->state ^= RKEngineStateDeactivating;
    if (engine->verbose) {
        RKLog("%s Stopped.\n", engine->name);
    }
    if (engine->state != (RKEngineStateAllocated | RKEngineStateProperlyWired)) {
        RKLog("%s Inconsistent state 0x%04x\n", engine->name, engine->state);
    }
    return RKResultSuccess;
}
