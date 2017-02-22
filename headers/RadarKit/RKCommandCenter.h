//
//  RKCommandCenter.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 1/5/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_RKCommandCenter__
#define __RadarKit_RKCommandCenter__

#include <RadarKit/RKRadar.h>

#define RKCommandCenterMaxConnections 32

typedef uint64_t RKUserFlag;

enum RKUserFlag {
    RKUserFlagNull                 = 0,                      //
    RKUserFlagControl              = 1,                      // Controls
    RKUserFlagStatusHealth         = (1 << 1),               //
    RKUserFlagStatusPulses         = (1 << 2),               //
    RKUserFlagStatusRays           = (1 << 3),               //
    RKUserFlagStatusPositions      = (1 << 4),               //
    RKUserFlagStatusHealthOld      = (1 << 5),               //
    RKUserFlagStatusEngines        = (1 << 6),               //
    RKUserFlagStatusAll            = 0xFE,                   //
    RKUserFlagDisplayIQ            = (1 << 8),               // Low rate IQ
    RKUserFlagDisplayIQFiltered    = (1 << 9),               // filtered IQ (usually matched filter)
    RKUserFlagProductIQ            = (1 << 10),              // Full rate IQ
    RKUserFlagProductIQFiltered    = (1 << 11),              // Full rate filtered IQ
    RKUserFlagDisplayZ             = (1 << 16),              // Display
    RKUserFlagDisplayV             = (1 << 17),              //
    RKUserFlagDisplayW             = (1 << 18),              //
    RKUserFlagDisplayD             = (1 << 19),              //
    RKUserFlagDisplayP             = (1 << 20),              //
    RKUserFlagDisplayR             = (1 << 21),              //
    RKUserFlagDisplayK             = (1 << 22),              //
    RKUserFlagDisplayS             = (1 << 23),              //
    RKUserFlagDisplayZVWDPRKS      = 0x0000FF0000,           //
    RKUserFlagProductZ             = (1ULL << 32),           // Products
    RKUserFlagProductV             = (1ULL << 33),           //
    RKUserFlagProductW             = (1ULL << 34),           //
    RKUserFlagProductD             = (1ULL << 35),           //
    RKUserFlagProductP             = (1ULL << 36),           //
    RKUserFlagProductR             = (1ULL << 37),           //
    RKUserFlagProductK             = (1ULL << 38),           //
    RKUserFlagProductS             = (1ULL << 39),           //
    RKUserFlagProductZVWDPRKS      = 0xFF00000000ULL,        //
    RKUserFlagEverything           = 0xFFFFFFFFFFULL
};

typedef struct  rk_user {
    char         login[64];
    RKUserFlag   access;             // Authorized access priviledge
    RKUserFlag   accessLevel2;
    RKUserFlag   streams;
    RKUserFlag   streamsLevel2;
    double       timeLastOut;
    double       timeLastHealthOut;
    double       timeLastDisplayIQOut;
    double       timeLastIn;
    uint32_t     healthIndex;
    uint32_t     rayStatusIndex;
    uint32_t     pulseIndex;
    uint16_t     pulseDownSamplingRatio;
    uint32_t     rayIndex;
    uint16_t     rayDownSamplingRatio;
    char         string[RKMaximumStringLength];
    RKInt16C     samples[2][RKGateCount];
    RKOperator   *serverOperator;
    RKRadar      *radar;
} RKUser;

typedef struct rk_command_center {
    // User set variables
    char         name[RKNameLength];
    int          verbose;
    RKRadar      *radars[4];
    
    // Program set variables
    bool         suspendHandler;
    RKServer     *server;
    int          radarCount;
    int          developerInspect;
    RKUser       users[RKCommandCenterMaxConnections];
} RKCommandCenter;

RKCommandCenter *RKCommandCenterInit(void);
void RKCommandCenterFree(RKCommandCenter *);

void RKCommandCenterSetVerbose(RKCommandCenter *, const int);
void RKCommandCenterAddRadar(RKCommandCenter *, RKRadar *);
void RKCommandCenterRemoveRadar(RKCommandCenter *, RKRadar *);

void RKCommandCenterStart(RKCommandCenter *);
void RKCommandCenterStop(RKCommandCenter *);

#endif /* __RadarKit_RKCommandCenter__ */
