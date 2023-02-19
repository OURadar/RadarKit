//
//  RadarHubTypes.h
//  RadarKit
//
//  Created by Boonleng Cheong on 2/2/23.
//  Copyright © 2023 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarHub_Types__
#define __RadarHub_Types__

enum RadarHubType {
    RadarHubTypeHandshake       = 1,             // JSON message {"command":"radarConnect", "pathway":"px1000", "name":"PX-1000"}
    RadarHubTypeControl         = 2,             // JSON control {"Go":{...}, "Stop":{...},...}
    RadarHubTypeHealth          = 3,             // JSON health {"Transceiver":{...}, "Pedestal":{...},...}
    RadarHubTypeReserve4        = 4,             //
    RadarHubTypeScope           = 5,             // Scope data in binary
    RadarHubTypeResponse        = 6,             // Plain text response
    RadarHubTypeReserved7       = 7,             //
    RadarHubTypeReserved8       = 8,             //
    RadarHubTypeReserved9       = 9,             //
    RadarHubTypeReserved10      = 10,            //
    RadarHubTypeReserved11      = 11,            //
    RadarHubTypeReserved12      = 12,            //
    RadarHubTypeReserved13      = 13,            //
    RadarHubTypeReserved14      = 14,            //
    RadarHubTypeReserved15      = 15,            //
    RadarHubTypeRadialZ         = 16,            //
    RadarHubTypeRadialV         = 17,            //
    RadarHubTypeRadialW         = 18,            //
    RadarHubTypeRadialD         = 19,            //
    RadarHubTypeRadialP         = 20,            //
    RadarHubTypeRadialR         = 21             //
};

#endif
