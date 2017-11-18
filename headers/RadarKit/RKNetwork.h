//
//  RKNetwork.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/17/15.
//
//

#ifndef __RadarKit_Network__
#define __RadarKit_Network__

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

typedef int RKNetworkSocketType;
typedef int RKNetworkMessageFormat;
typedef uint32_t RKNetworkPacketType;

enum RKNetworkSocketType {
    RKNetworkSocketTypeTCP = 1,
    RKNetworkSocketTypeUDP
};

enum RKNetworkMessageFormat {
    RKNetworkMessageFormatNewLine,                         // Line-by-line reading
    RKNetworkMessageFormatConstantSize,                    // Fixed length packets
    RKNetworkMessageFormatHeaderDefinedSize                // The header is of type RKNetDelimiter
};

enum RKNetworkPacketType {
    RKNetworkPacketTypeBytes,
    RKNetworkPacketTypeBeacon,
    RKNetworkPacketTypePlainText,
    RKNetworkPacketTypePulseData,
    RKNetworkPacketTypeRayData,
    RKNetworkPacketTypeHealth,
    RKNetworkPacketTypeControls,
    RKNetworkPacketTypeCommandResponse,
    RKNetworkPacketTypeRadarDescription,
    RKNetworkPacketTypeProcessorStatus,
    RKNetworkPacketTypeRayDisplay         = 'm',
    RKNetworkPacketTypeAlertMessage,
    RKNetworkPacketTypeConfig
};

#pragma pack(push, 1)

typedef union rk_net_delimiter {
    struct {
        uint16_t     type;                                 // Type
        uint16_t     subtype;                              // Sub-type
        uint32_t     size;                                 // Raw size in bytes to read / skip ahead
        uint32_t     decodedSize;                          // Decided size if this is a compressed block
    };
    RKByte bytes[16];                                      // Make this struct always fixed bytes
} RKNetDelimiter;

#pragma pack(pop)

ssize_t RKNetworkSendPackets(int, ...);

//#ifdef __cplusplus
//}
//#endif

#endif /* defined(___RadarKit_RKNetwork__) */
