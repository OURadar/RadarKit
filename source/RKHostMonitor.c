//
//  RKHostMonitor.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 2/24/18.
//  Copyright © Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKHostMonitor.h>

#define RKHostMonitorPacketSize 32

//
// The following structures are obtained from an example from Apple:
// https://developer.apple.com/library/content/samplecode/SimplePing/
//
// They also work on CentOS 7.3.
// NOTE: To allow root to use icmp sockets, run:
//
//     sysctl -w net.ipv4.ping_group_range="0 0"
//

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

    char buff[RKHostMonitorPacketSize];
    RKICMPHeader *icmpHeader;
    RKIPV4Header *ipv4Header = (RKIPV4Header *)buff;
    socklen_t returnLength = sizeof(struct sockaddr);
    uint16_t receivedChecksum;
    uint16_t calculatedChecksum;
    
    ssize_t r = 0;
    size_t txSize = RKHostMonitorPacketSize - sizeof(RKIPV4Header);
    size_t ipHeaderLength, offset;

    double period;
    struct timeval time;
    fd_set rfd, efd;

	// Initiate a variable to store my name
	RKName name;
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

    // Resolve my host
    struct hostent *hostname = gethostbyname(engine->hosts[c]);
    if (hostname == NULL) {
        RKLog("%s %s Unable to resolve %s.", engine->name, name, engine->hosts[c]);
        me->state = RKHostStateUnknown;
        me->tic = 2;
        return NULL;
    }
    memset(&targetAddress, 0, sizeof(struct sockaddr_in));
    targetAddress.sin_family = hostname->h_addrtype;
    targetAddress.sin_addr.s_addr = *(unsigned int *)hostname->h_addr;

    // Open a socket, set some properties for ICMP
    if ((sd = socket(AF_INET, SOCK_DGRAM, protocol->p_proto)) < 0) {
        RKLog("%s %s Error. Unable to open a socket.  sd = %d   errno = %d\n", engine->name, name, sd, errno);
        if (errno == EACCES) {
            RKLog("%s %s Info. Run 'sysctl -w net.ipv4.ping_group_range=\"0 0\"' ...\n", engine->name, name);
            RKLog("%s %s Info. to allow root to use icmp sockets\n", engine->name, name);
        }
        me->state = RKHostStateUnknown;
        me->tic = 2;
        return NULL;
    }
    if (setsockopt(sd, IPPROTO_IP, IP_TTL, &value, sizeof(value))) {
        RKLog("%s %s Error. Failed in setsockopt().\n", engine->name, name);
        me->state = RKHostStateUnknown;
        me->tic = 2;
        return NULL;
    }
    if (fcntl(sd, F_SETFL, O_NONBLOCK)) {
        RKLog("%s %s Error. Failed in fcntl().\n", engine->name, name);
        me->state = RKHostStateUnknown;
        me->tic = 2;
        return NULL;
    }

    if (engine->verbose) {
        RKLog(">%s %s Started.   host = %s (%d.%d.%d.%d)   sd = %d\n",
              engine->name,
              name,
              engine->hosts[c],
              (targetAddress.sin_addr.s_addr & 0xff),
              (targetAddress.sin_addr.s_addr & 0x0000ff00) >> 8,
              (targetAddress.sin_addr.s_addr & 0x00ff0000) >> 16,
              (targetAddress.sin_addr.s_addr & 0xff000000) >> 24,
              sd);
    }
    
    me->tic = 1;
    me->sequenceNumber = 1;
    me->identifier = rand() & 0xffff;
    gettimeofday(&me->latestTime, NULL);

    while (engine->state & RKEngineStateActive) {

        // Reset buffer
        memset(buff, 0, RKHostMonitorPacketSize);

        // Prepare echo request packet
        icmpHeader = (RKICMPHeader *)buff;
        icmpHeader->type = RKICMPv4EchoRequest;
        icmpHeader->code = 0;
        icmpHeader->checksum = 0;
        icmpHeader->identifier = me->identifier;
        icmpHeader->sequenceNumber = me->sequenceNumber;
        icmpHeader->checksum = rk_host_monitor_checksum(buff, txSize);

        pthread_mutex_lock(&engine->mutex);

        // Ping
        if ((r = sendto(sd, buff, txSize, 0, (struct sockaddr *)&targetAddress, sizeof(struct sockaddr))) == -1) {
            if (engine->verbose > 1) {
                RKLog(">%s %s Error. Command sendto() -> %d  %d  %s.", engine->name, name, r, errno, RKErrnoString(errno));
            }
            // Now we wait a little
            pthread_mutex_unlock(&engine->mutex);
            k = 0;
            while (k++ < 10 && engine->state & RKEngineStateActive) {
                usleep(100000);
            }
            // Delay reporting since other threads may still be waiting for recvfrom()
            me->state = RKHostStateUnreachable;
            me->tic++;
            continue;
        } else if (engine->verbose > 1) {
            RKLog(">%s %s Ping %s (%d.%d.%d.%d)   seq = %d   size = %d bytes\n",
                  engine->name, name,
                  engine->hosts[c],
                  (targetAddress.sin_addr.s_addr & 0xff),
                  (targetAddress.sin_addr.s_addr & 0x0000ff00) >> 8,
                  (targetAddress.sin_addr.s_addr & 0x00ff0000) >> 16,
                  (targetAddress.sin_addr.s_addr & 0xff000000) >> 24,
                  me->sequenceNumber,
                  txSize);
        }

        // This part does not work as expected.
        if (engine->verbose > 1) {
            RKLog("%s %s select()\n", engine->name, name);
        }
        FD_ZERO(&rfd);
        FD_ZERO(&efd);
        time.tv_sec = 0;
        time.tv_usec = 200000;
        k = select(sd + 1, &rfd, NULL, &efd, &time);
        if (engine->verbose > 1) {
            RKLog("%s %s select() -> %d\n", engine->name, name, k);
        }

        // Reset buffer
        memset(buff, 0, RKHostMonitorPacketSize);

        // Pong
        k = 0;
        offset = (size_t)-1;
        returnAddress.sin_addr.s_addr = 0;
        while (returnAddress.sin_addr.s_addr != targetAddress.sin_addr.s_addr && k++ < engine->workerCount && engine->state & RKEngineStateActive) {
            if ((r = recvfrom(sd, &buff, RKNameLength, 0, (struct sockaddr *)&returnAddress, &returnLength)) > 0) {
                if (engine->verbose > 2) {
                    RKLog("%s %s recvfrom()   sd = %d   %d.%d.%d.%d\n",
                          engine->name, name, sd,
                          (returnAddress.sin_addr.s_addr & 0xff),
                          (returnAddress.sin_addr.s_addr & 0x0000ff00) >> 8,
                          (returnAddress.sin_addr.s_addr & 0x00ff0000) >> 16,
                          (returnAddress.sin_addr.s_addr & 0xff000000) >> 24);
                }
                if (r == txSize) {
                    offset = 0;
                } else if (r > sizeof(RKIPV4Header) + sizeof(RKICMPHeader) && ipv4Header->protocol == protocol->p_proto) {
                    ipHeaderLength = (ipv4Header->versionAndHeaderLength & 0x0f) * sizeof(uint32_t);
                    if (r >= ipHeaderLength + sizeof(RKICMPHeader)) {
                        offset = ipHeaderLength;
                    } else {
                        RKLog("%s %s Error. Unexpected packet size.\n", engine->name, name);
                        break;
                    }
                }
            }
        }
        if (offset != (size_t)-1 && returnAddress.sin_addr.s_addr == targetAddress.sin_addr.s_addr) {
            icmpHeader = (RKICMPHeader *)(buff + offset);
            receivedChecksum = icmpHeader->checksum;
            icmpHeader->checksum = 0;
            calculatedChecksum = rk_host_monitor_checksum(buff + offset, r - offset);
            if (engine->verbose > 1) {
                RKLog("%s %s r = %u   %d.%d.%d.%d   sd = %d\n", engine->name, name, r,
                      (returnAddress.sin_addr.s_addr & 0xff),
                      (returnAddress.sin_addr.s_addr & 0x0000ff00) >> 8,
                      (returnAddress.sin_addr.s_addr & 0x00ff0000) >> 16,
                      (returnAddress.sin_addr.s_addr & 0xff000000) >> 24,
                      sd);
                RKLog(">%s %s checksum       = %   6d   %   6d\n", engine->name, name, receivedChecksum, calculatedChecksum);
                RKLog(">%s %s identifier     = 0x%04x   0x%04x\n", engine->name, name, icmpHeader->identifier, me->identifier);
                RKLog(">%s %s sequenceNumber = %   6d   %   6d\n", engine->name, name, icmpHeader->sequenceNumber, me->sequenceNumber);
            }
            // Ignore identifier for this since it can be different for UDP implementations
            if (receivedChecksum == calculatedChecksum &&
                icmpHeader->type == RKICMPv4EchoReply &&
                icmpHeader->code == 0 &&
                icmpHeader->sequenceNumber == me->sequenceNumber) {
                me->state = RKHostStateReachable;
                me->tic++;
                me->sequenceNumber++;
                gettimeofday(&me->latestTime, NULL);
            }
        } else {
            // Timed out
            gettimeofday(&time, NULL);
            period = RKTimevalDiff(time, me->latestTime);
            if (engine->verbose > 1) {
                RKLog(">%s %s r = %d   delta: %.3e   %d.%d.%d.%d  %d %d", engine->name, name, r, RKTimevalDiff(time, me->latestTime),
                      (targetAddress.sin_addr.s_addr & 0xff),
                      (targetAddress.sin_addr.s_addr & 0x0000ff00) >> 8,
                      (targetAddress.sin_addr.s_addr & 0x00ff0000) >> 16,
                      (targetAddress.sin_addr.s_addr & 0xff000000) >> 24, k, offset);
            }
            if (period > (double)me->pingIntervalInSeconds * 3.0) {
                me->state = RKHostStateUnreachable;
                me->tic++;
            } else if (me->sequenceNumber > 1 && period > (double)me->pingIntervalInSeconds * 1.5) {
                me->state = RKHostStatePartiallyReachable;
                me->tic++;
            }
            pthread_mutex_unlock(&engine->mutex);
            // Wait less if this round failed.
            k = 0;
            while (k++ < 10 && engine->state & RKEngineStateActive) {
                usleep(100000);
            }
            continue;
        }

        pthread_mutex_unlock(&engine->mutex);

        // Now we wait
        k = 0;
        while (k++ < me->pingIntervalInSeconds * 10 && engine->state & RKEngineStateActive) {
            usleep(100000);
        }
    }
    
    if (sd) {
        close(sd);
    }

    if (engine->verbose) {
        RKLog(">%s %s Stopped.\n", engine->name, name);
    }
    
    return NULL;
}

static void *hostWatcher(void *in) {
    RKHostMonitor *engine = (RKHostMonitor *)in;
    
    int k;
    bool allTrue, allKnown, allReachable, anyReachable;
    
    engine->state |= RKEngineStateActive;
    engine->state ^= RKEngineStateActivating;

	if (engine->workers != NULL) {
		RKLog("%s Workers already allocated.\n", engine->name);
		return NULL;
	}
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
        worker->pingIntervalInSeconds = RKHostMonitorPingInterval;
        
        // Workers that actually ping the hosts
        if (pthread_create(&worker->tid, NULL, hostPinger, worker) != 0) {
            RKLog(">%s Error. Failed to start a host pinger", engine->name);
            return (void *)RKResultFailedToStartHostPinger;
        }
        
        while (worker->tic == 0 && engine->state & RKEngineStateActive) {
            usleep(10000);
        }
    }
    
    if (engine->verbose) {
        RKLog("%s Started.   mem = %s B   state = %x\n", engine->name, RKIntegerToCommaStyleString(engine->memoryUsage), engine->state);
    }

	// Increase the tic once to indicate the engine is ready
	engine->tic++;

    // Wait another tic for the first ping to respond
    do {
        allTrue = true;
        for (k = 0; k < engine->workerCount; k++) {
            RKUnitMonitor *worker = &engine->workers[k];
            allTrue &= worker->tic == 1;
        }
        usleep(10000);
    } while (allTrue && engine->state & RKEngineStateActive);

    if (engine->verbose > 2) {
        for (k = 0; k < engine->workerCount; k++) {
            RKUnitMonitor *worker = &engine->workers[k];
            RKLog(">%s tic[%d] = %d\n", engine->name, k, worker->tic);
        }
    }

    // Wait here while the engine should stay active
    while (engine->state & RKEngineStateActive) {
        // Consolidate all the state from all unit watcher
        allKnown = true;
        allReachable = true;
        anyReachable = false;
        for (k = 0; k < engine->workerCount; k++) {
            RKUnitMonitor *worker = &engine->workers[k];
            allKnown &= worker->state != RKHostStateUnknown;
            allReachable &= worker->state == RKHostStateReachable;
            anyReachable |= worker->state == RKHostStateReachable;
            if (engine->verbose > 1) {
                RKLog("%s %s %s%s%s\n", engine->name,
                      engine->hosts[k],
                      rkGlobalParameters.showColor ? (worker->state == RKHostStateReachable ? RKGreenColor : (worker->state == RKHostStatePartiallyReachable ? RKOrangeColor : RKRedColor)) : "",
                      worker->state == RKHostStateReachable ? "responded" : (worker->state == RKHostStatePartiallyReachable ? "delayed" : "unreachable"),
                      rkGlobalParameters.showColor ? RKNoColor : "");
            }
        }
        engine->allKnown = allKnown;
        engine->allReachable = allReachable;
        engine->anyReachable = anyReachable;
        // Wait one minute, do it with multiples of 0.1s for a responsive exit
        k = 0;
        while (k++ < RKHostMonitorPingInterval * 10 && engine->state & RKEngineStateActive) {
            usleep(100000);
        }
    }

	RKLog("%s workerCount = %d\n", engine->name, engine->workerCount);

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
    sprintf(engine->name, "%s<NetworkAssessor>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(RKEngineColorHostMonitor) : "",
            rkGlobalParameters.showColor ? RKNoColor : "");
    engine->state = RKEngineStateAllocated;
    pthread_mutex_init(&engine->mutex, NULL);
    RKHostMonitorAddHost(engine, "8.8.8.8");
    RKHostMonitorAddHost(engine, "arrc.ou.edu");
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
	if (engine->state & RKEngineStateActive) {
		RKLog("%s Cannot add host after the engine has started.\n", engine->name);
		return;
	}
    int k = engine->workerCount++;
    engine->hosts = realloc(engine->hosts, engine->workerCount * sizeof(RKName));
    strncpy(engine->hosts[k], address, RKNameLength - 1);
    engine->memoryUsage = sizeof(RKHostMonitor) + engine->workerCount * sizeof(RKName);
    engine->state |= RKEngineStateProperlyWired;
}

#pragma mark - Interactions

int RKHostMonitorStart(RKHostMonitor *engine) {
    //RKLog("%s %d %s", engine->name, engine->workerCount, engine->hosts[0]);
    if (engine->verbose) {
        RKLog("%s Starting ...\n", engine->name);
    }
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
	if (!(engine->state & RKEngineStateActive)) {
		RKLog("%s Not active.\n", engine->name);
		return RKResultEngineDeactivatedMultipleTimes;
	}
    if (engine->verbose) {
        RKLog("%s Stopping ...\n", engine->name);
    }
    engine->state |= RKEngineStateDeactivating;
    engine->state ^= RKEngineStateActive;
    if (engine->tidHostWatcher) {
        pthread_join(engine->tidHostWatcher, NULL);
		engine->tidHostWatcher = (pthread_t)0;
	} else {
		RKLog("%s Invalid thread ID.\n", engine->name);
	}
    engine->state ^= RKEngineStateDeactivating;
    if (engine->verbose) {
        RKLog("%s Stopped.\n", engine->name);
    }
    if (engine->state != (RKEngineStateAllocated | RKEngineStateProperlyWired)) {
        RKLog("%s Inconsistent state 0x%04x\n", engine->name, engine->state);
    }
    return RKResultSuccess;
}
