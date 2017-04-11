//
//  RKRelay.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 4/11/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_Relay__
#define __RadarKit_Relay__

#include <RadarKit/RKRadar.h>

typedef struct rk_relay {
    // User defined variables
    RKClient               *client;
    uint32_t               responseIndex;
    char                   responses[RKHealthRelayTweetaFeedbackDepth][RKMaximumStringLength];
    char                   latestCommand[RKMaximumStringLength];
    pthread_t              tidBackground;
    RKStream               streams;
//    RKUserFlag   streamsLevel2;
} RKRelay;

#endif
