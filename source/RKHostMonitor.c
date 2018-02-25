//
//  RKHostMonitor.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 2/24/18.
//  Copyright © Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKHostMonitor.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <netdb.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

#define PACKETSIZE    64

typedef union rk_host_monitor_packet {
    struct {
        //struct icmphdr header;
        char message[0];
    };
    char bytes[PACKETSIZE];
} RKHostMonitorPacket;

static uint16_t rk_host_monitor_checksum (void *in, int len) {
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

#pragma mark - Helper Functions

void ping(struct sockaddr_in address) {
}

#pragma mark - Delegate Workers

static void *hostPinger(void *in) {
    RKUnitMonitor *me = (RKUnitMonitor *)in;
    RKHostMonitor *engine = me->parent;

    int k;
    //struct packet packet;
    struct sockaddr_in address;

    const int c = me->id;
    const int value = 255;
    struct protoent *protocol = getprotobyname("ICMP");
    struct hostent *hostname = gethostbyname(engine->hosts[c]);

    char name[RKNameLength];
    
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
    
    // Resolve my host
    
    memset(&address, 0, sizeof(struct sockaddr));
    address.sin_family = hostname->h_addrtype;
    address.sin_addr.s_addr = *(unsigned int *)hostname->h_addr;
    RKLog(">%s %s %s -> 0x%08x -> %d.%d.%d.%d (%d)\n",
          engine->name, name, engine->hosts[c], address.sin_addr.s_addr,
          (address.sin_addr.s_addr & 0xff),
          (address.sin_addr.s_addr & 0x0000ff00) >> 8,
          (address.sin_addr.s_addr & 0x00ff0000) >> 16,
          (address.sin_addr.s_addr & 0xff000000) >> 24, hostname->h_length);

    RKLog("%s %s proto = %d / %d .\n", engine->name, name, protocol->p_proto, IPPROTO_ICMP);
    int sd = socket(AF_INET, SOCK_DGRAM, protocol->p_proto);
    if (sd < 0) {
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
    
#define RKHostMonitorPacketSize 64
    
    char buf[RKHostMonitorPacketSize];
    struct sockaddr_in receiveAddress;
    socklen_t receiveLength;
    struct ip *ipHeader = (struct ip *)buf;
//    struct tcphdr *tcpHeader = (struct tcphdr *)&ipHeader[1];
    
    ssize_t r = 0;
    
    while (engine->state & RKEngineStateActive) {
        // Ping
        RKLog(">%s %s ping %s\n", engine->name, name, engine->hosts[c]);
        
        if (recvfrom(sd, &buf, RKNameLength, 0, (struct sockaddr *)&receiveAddress, &receiveLength)) {
            RKLog(">%s %s got message %d: %02x\n", engine->name, name, receiveLength, buf[0]);
        }
        
        memset(buf, 0, sizeof(buf));

        ipHeader->ip_hl = 5;
        ipHeader->ip_v = 4;
        ipHeader->ip_tos = 0;
        ipHeader->ip_len = htons(RKHostMonitorPacketSize);
        ipHeader->ip_id = 0;
        ipHeader->ip_ttl = 1;
        ipHeader->ip_sum = rk_host_monitor_checksum(buf, sizeof(struct ip));
        
        if ((r = sendto(sd, buf, sizeof(buf), 0, (struct sockaddr *)&address, sizeof(struct sockaddr_in))) != 0) {
            RKLog(">%s %s Error in sendto() -> %d  %d.", engine->name, name, r, errno);
        }

        // Now we wait
        k = 0;
        while (k++ < 10 && engine->state & RKEngineStateActive) {
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
        // Wait one minute, do it with multiples of 0.1s for a responsive exit
        k = 0;
        while (k++ < 600 && engine->state & RKEngineStateActive) {
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
