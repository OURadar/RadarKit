//
//  RKClient.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/17/15.
//
//

#ifndef __RadarKit_RKClient__
#define __RadarKit_RKClient__

#include <RadarKit/RKNetwork.h>

//#ifdef __cplusplus
//extern "C" {
//#endif

enum RKNetworkClientState {
    RKNetworkClientStateNull,
    RKNetworkClientStateResolvingIP,
    RKNetworkClientStateConfiguringSocket,
    RKNetworkClientStateConnecting,
    RKNetworkClientStateConnected,
    RKNetworkClientStateDisconnecting,
    RKNetworkClientStateDisconnected
};

//#ifdef __cplusplus
//}
//#endif

#endif /* defined(___RadarKit_RKClient__) */
