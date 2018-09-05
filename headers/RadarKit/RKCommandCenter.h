//
//  RKCommandCenter.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 1/5/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_CommandCenter__
#define __RadarKit_CommandCenter__

#include <RadarKit/RKRadar.h>

#define RKCommandCenterMaxConnections 32
#define RKCommandCenterMaxRadars       4

typedef struct rk_user {
    char                             login[64];
    RKStream                         access;                                                       // Authorized access priviledge
    RKStream                         streams;                                                      // Current streams
    RKStream                         streamsToRestore;                                             // Streams to restore after reset
    RKStream                         streamsInProgress;                                            // Streams in progress
    uint64_t                         tic;                                                          // Counter of socketStreamHandler() calls
    uint64_t                         ticForStatusStream;                                           // Counter of socketStreamHandler() for status streams
    double                           timeLastOut;                                                  // Time since the last basic stream was sent
    double                           timeLastHealthOut;                                            // Time since the last health stream was sent
    double                           timeLastDisplayIQOut;                                         // Time since the last display I/Q was sent
    double                           timeLastIn;                                                   // Time since the last command was received
    uint32_t                         statusIndex;                                                  // The index to radar status
    uint32_t                         healthIndex;                                                  // The index to health
    uint32_t                         rayStatusIndex;                                               // The index to RKMomentEngine->raySatusBuffer
    uint32_t                         pulseIndex;                                                   // The index to the latest pulse
    uint32_t                         rayIndex;                                                     // The index to the latest ray
    uint32_t                         pingCount;                                                    // Counter of ping
    uint32_t                         commandCount;                                                 // Counter of command
    uint32_t                         controlFirstUID;                                              // UUID of the first control
    uint32_t                         scratchSpaceIndex;                                            // The index to the scratch space to use
    RKTextPreferences                textPreferences;                                              // Text preference for terminal output
    uint16_t                         pulseDownSamplingRatio;                                       // Additional down-sampling ratio for pulse live stream
    uint16_t                         rayDownSamplingRatio;                                         // Additional down-sampling ratio for ray live stream
    uint16_t                         asciiArtStride;                                               // Gate stride for ASCII art
    uint16_t                         ascopeMode;                                                   // The ASCope mode: 1-4
    pthread_mutex_t                  mutex;                                                        //
    char                             string[RKMaximumPacketSize];                                  // A local storage to buffer a packet
    char                             scratch[RKMaximumPacketSize];                                 // A local storage as scratch space
    char                             commandResponse[RKMaximumPacketSize];                         // A local storage as feedback
    RKInt16C                         samples[2][RKMaximumGateCount];                               // A local storage for raw I/Q for AScope
    RKOperator                       *serverOperator;                                              // The reference to the socket server operator
    RKRadar                          *radar;                                                       // The reference to the radar object
    uint8_t                          productCount;                                                 // Product count from PyRadarKit
    RKProductId                      productIds[RKMaximumProductCount];                            // Product identifiers for active algorithms of PyRadarKit
    RKProductDesc                    productDescriptions[RKMaximumProductCount];                   // Product descriptions for active algorithms of PyRadarKit
} RKUser;

typedef struct rk_command_center {
    // User set variables
    RKName                           name;
    int                              verbose;
    RKRadar                          *radars[RKCommandCenterMaxRadars];
    
    // Program set variables
    bool                             relayMode;
    bool                             suspendHandler;
    RKServer                         *server;
    int                              radarCount;
    RKUser                           users[RKCommandCenterMaxConnections];
    pthread_mutex_t                  mutex;
    
    // Status / health
    size_t                           memoryUsage;
} RKCommandCenter;

RKCommandCenter *RKCommandCenterInit(void);
void RKCommandCenterFree(RKCommandCenter *);

void RKCommandCenterSetVerbose(RKCommandCenter *, const int);
void RKCommandCenterSetPort(RKCommandCenter *, const int);
void RKCommandCenterAddRadar(RKCommandCenter *, RKRadar *);
void RKCommandCenterRemoveRadar(RKCommandCenter *, RKRadar *);

void RKCommandCenterStart(RKCommandCenter *);
void RKCommandCenterStop(RKCommandCenter *);
void RKCommandCenterSkipToCurrent(RKCommandCenter *, RKRadar *);

#endif /* __RadarKit_RKCommandCenter__ */
