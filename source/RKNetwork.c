//
//  RKNetwork.c
//  RadarKit
//
//  Created by Boonleng Cheong on 3/17/15.
//
//

#include <RadarKit/RKNetwork.h>

// Use as:
// RKNetworkSendPackets(operator, payload, size, payload, size, ..., NULL);
//
// Note:
// This function is not mutex proctected.
ssize_t RKNetworkSendPackets(int sd, ...) {
    va_list   arg;

    void      *payload;
    ssize_t   payloadSize = 1;

    ssize_t   grandTotalSentSize = 0;
    ssize_t   totalSentSize = 0;
    ssize_t   sentSize = 0;

    int       timeout_count = 0;

    va_start(arg, sd);

    // Parse the input arguments until payload = NULL (last input)
    payload = va_arg(arg, void *);
    while (payload != NULL) {
        payloadSize = va_arg(arg, ssize_t);
        // RKLog("RKNetworkSendPackets : payloadSize = %d @ %p\n", (int)payloadSize, payload);
        sentSize = 0;
        totalSentSize = 0;
        while (totalSentSize < payloadSize && timeout_count++ < RKNetworkTimeoutSeconds / 10) {
            if ((sentSize = send(sd, payload + sentSize, payloadSize - sentSize, 0)) > 0) {
                totalSentSize += sentSize;
            }
            if (totalSentSize < payloadSize) {
                usleep(10000);
            } else if (sentSize < 0) {
                RKLog("RKNetworkSendPackets Unable to send()\n");
                return RKResultIncompleteSend;
            }
        }
        grandTotalSentSize += totalSentSize;
        //RKLog("RKNetworkSendPackets : grandTotalSentSize = %d\n", (int)grandTotalSentSize);
        payload = va_arg(arg, void *);
    }

    va_end(arg);

    if (timeout_count >= RKNetworkTimeoutSeconds / 10) {
        return RKResultTimeout;
    }
    return grandTotalSentSize;
}

#define SHOW_PACKET_NUMBER(x) \
printf(RKSkyBlueColor #x RKNoColor " = " RKLimeColor "%s" RKNoColor "\n", \
RKIntegerToCommaStyleString(x));

void RKNetworkShowPacketTypeNumbers(void) {
    SHOW_FUNCTION_NAME
    SHOW_PACKET_NUMBER(RKNetworkPacketTypeProcessorStatus);
    SHOW_PACKET_NUMBER(RKNetworkPacketTypeRayDisplay);
    SHOW_PACKET_NUMBER(RKNetworkPacketTypeAlertMessage);
    SHOW_PACKET_NUMBER(RKNetworkPacketTypeConfig);
    SHOW_PACKET_NUMBER(RKNetworkPacketTypeSweep);
    SHOW_PACKET_NUMBER(RKNetworkPacketTypeSweepHeader);
    SHOW_PACKET_NUMBER(RKNetworkPacketTypeSweepRay);
}
