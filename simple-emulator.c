//
//  main.c
//  SimpleEmulator
//
//  Created by Boon Leng Cheong on 5/13/18.
//  Copyright © 2018 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit.h>
#include <getopt.h>

RKRadar *myRadar = NULL;

static void handleSignals(int signal) {
    if (myRadar == NULL) {
        return;
    }
    fprintf(stderr, "\n");
    RKLog("Caught a %s (%d)  radar->state = 0x%x\n", RKSignalString(signal), signal, myRadar->state);
    RKStop(myRadar);
}

int main(int argc, const char * argv[]) {

    RKSetWantScreenOutput(true);

    char *term = getenv("TERM");
    if (term == NULL || (strcasestr(term, "color") == NULL && strcasestr(term, "ansi") == NULL)) {
        RKSetWantColor(false);
    }

    myRadar = RKInit();
    
    if (myRadar == NULL) {
        RKLog("Error. Could not allocate a radar.\n");
        exit(EXIT_FAILURE);
    }

    // Catch Ctrl-C and exit gracefully
    signal(SIGINT, handleSignals);
    signal(SIGQUIT, handleSignals);
    signal(SIGKILL, handleSignals);
    
    // Make a command center and add the radar to it
    RKCommandCenter *center = RKCommandCenterInit();
    RKCommandCenterSetVerbose(center, 1);
    RKCommandCenterStart(center);
    RKCommandCenterAddRadar(center, myRadar);
    
    // Set some properties of the radar
    RKSetVerbosity(myRadar, 1);
    RKSetRecordingLevel(myRadar, 0);
    RKSetProcessingCoreCounts(myRadar, 2, 2, 2);
    
    // Add some control buttons
    RKAddControlAsLabelAndCommand(myRadar, "10us pulse", "t w s10");
    RKAddControlAsLabelAndCommand(myRadar, "20us pulse", "t w s20");
    RKAddControlAsLabelAndCommand(myRadar, "PPI EL 3 deg @ 45 dps", "p ppi 3 45");
    RKAddControlAsLabelAndCommand(myRadar, "PPI EL 5 deg @ 25 dps", "p ppi 5 12");

    RKSetTransceiver(myRadar,
                     NULL,
                     RKTestTransceiverInit,
                     RKTestTransceiverExec,
                     RKTestTransceiverFree);

    RKSetPedestal(myRadar,
                  NULL,
                  RKTestPedestalInit,
                  RKTestPedestalExec,
                  RKTestPedestalFree);

    RKSetHealthRelay(myRadar,
                     NULL,
                     RKTestHealthRelayInit,
                     RKTestHealthRelayExec,
                     RKTestHealthRelayFree);

    RKGoLive(myRadar);

    usleep(1000000);
    RKLog("Starting a new PPI ...\n");
    RKExecuteCommand(myRadar, "p ppi 4 45", NULL);

    RKWaitWhileActive(myRadar);

    RKCommandCenterRemoveRadar(center, myRadar);
    RKCommandCenterStop(center);
    RKCommandCenterFree(center);

    RKFree(myRadar);

    return EXIT_SUCCESS;
}
