//
//  RKNetwork.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/17/15.
//
//

#ifndef __RadarKit_RKNetwork__
#define __RadarKit_RKNetwork__

#include <RadarKit/RKFoundation.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>

//#ifdef __cplusplus
//extern "C" {
//#endif

typedef int RKSocketType;
typedef int RKMessageFormat;

enum RKSocketType {
    RKSocketTypeTCP,
    RKSocketTypeUDP
};

enum RKMessageFormat {
    RKMessageFormatNewLine,
    RKMessageFormatFixedBlock,
    RKMessageFormatFixedHeaderVariableBlock
};

typedef struct {
    uint32_t type;
    uint32_t rawSize;
    uint32_t userParameter1;
    uint32_t userParameter2;
} RKNetDelimiterPacket;



//#ifdef __cplusplus
//}
//#endif

#endif /* defined(___RadarKit_RKNetwork__) */
