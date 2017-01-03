//
//  RKLocalCommandCenter.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 9/16/16.
//  Copyright (c) 2016-2017 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKLocalCommandCenter.h>

int socketCommandHandler(RKOperator *O) {
    char string[RKMaximumStringLength];
    switch (O->cmd[0]) {
        case 'a':
            RKOperatorSendString(O, "Hello" RKEOL);
            break;
        default:
            snprintf(string, RKMaximumStringLength, "Unknown command '%s'." RKEOL, O->cmd);
            RKOperatorSendString(O, string);
            break;
    }
    return 0;
}

int socketStreamHandler(RKOperator *O) {
    static struct timeval t0, t1;

    ssize_t r;

    gettimeofday(&t0, NULL);

    // Check IQ
    // Check MM
    // Check system health

    // Heart beat
    double td = RKTimevalDiff(t0, t1);
    if (td >= 1.0f) {
        t1 = t0;
        r = RKOperatorSendBeacon(O);
        if (r < 0) {
            RKLog("Beacon failed (r = %d).\n", r);
            RKOperatorHangUp(O);
        }
    }

    return 0;
}
