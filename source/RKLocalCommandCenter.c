//
//  RKLocalCommandCenter.c
//  _RadarKit
//
//  Created by Boon Leng Cheong on 9/16/16.
//  Copyright © 2016 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKLocalCommandCenter.h>

int socketCommandHandler(RKOperator *O) {
    switch (O->cmd[0]) {
        case 'a':
            RKOperatorSendString(O, "Hello" RKEOL);
            break;
        default:
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
