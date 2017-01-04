//
//  RKNetwork.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/17/15.
//
//

#include <RadarKit/RKNetwork.h>

// Use as:
// RKNetworkSendPackets(operator, payload, size, payload, size, ..., NULL);

ssize_t RKNetworkSendPackets(int sd, ...) {
    va_list   arg;

    void      *payload;
    ssize_t   payloadSize = 1;

    ssize_t   grandTotalSentSize = 0;
    ssize_t   totalSentSize = 0;
    ssize_t   sentSize = 0;

    int timeout_count = 0;

    va_start(arg, sd);

    // Parse the input arguments until payload = NULL (last input)
    payload = va_arg(arg, void *);
    while (payload != NULL) {
        payloadSize = va_arg(arg, ssize_t);
        RKLog("RKNetworkSendPackets : payloadSize = %d @ %p\n", (int)payloadSize, payload);
        sentSize = 0;
        totalSentSize = 0;
        while (totalSentSize < payloadSize && timeout_count++ < RKNetworkTimeoutSeconds / 10) {
            if ((sentSize = send(sd, payload + sentSize, payloadSize - sentSize, 0)) > 0) {
                totalSentSize += sentSize;
            }
            if (totalSentSize < payloadSize) {
                usleep(10000);
            } else if (sentSize < 0) {
                return RKResultIncompleteSend;
            }
        }
        grandTotalSentSize += totalSentSize;
        payload = va_arg(arg, void *);
    }

    va_end(arg);

    if (timeout_count >= RKNetworkTimeoutSeconds / 10) {
        return RKResultTimeout;
    }

    return grandTotalSentSize;
}
