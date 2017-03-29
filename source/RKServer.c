//
//  RKServer.c
//  RadarKit
//
//  This collection is copied and modified from PortServer.
//
//  Created by Boon Leng Cheong on 9/16/15.
//
//

#include <RadarKit/RKServer.h>

// Internal function definitions

void *RKServerRoutine(void *);
void *RKOperatorRoutine(void *);
void *RKOperatorCommandRoutine(void *);
int RKOperatorCreate(RKServer *, int, const char *);
void RKOperatorFree(RKOperator *);
int RKDefaultWelcomeHandler(RKOperator *);
int RKDefaultTerminateHandler(RKOperator *);

// Implementation

#pragma mark -
#pragma mark Private functions


void *RKServerRoutine(void *in) {
    RKServer *M = (RKServer *)in;

    M->state = RKServerStateOpening;

    // Create the socket
    if ((M->sd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        RKLog("%s failed at socket().\n", M->name);
        M->state = RKServerStateNull;
        return NULL;
    }

    int ii;
    struct sockaddr_in sa;
    socklen_t sa_len = sizeof(struct sockaddr_in);

    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = INADDR_ANY;
    sa.sin_port = htons(M->port);

    // Avoid "Address already in use" error
    ii = 1;
    if (setsockopt(M->sd, SOL_SOCKET, SO_REUSEADDR, &ii, sizeof(ii)) == -1) {
        RKLog("%s Error. Failed at setsockopt().\n", M->name);
        M->state = RKServerStateNull;
        return NULL;
    }

    // Bind
    if (bind(M->sd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        RKLog("%s Error. Failed at bind().\n", M->name);
        M->state = RKServerStateNull;
        close(M->sd);
        return NULL;
    }

    // Listen
    if (listen(M->sd, M->maxClient + 1) < 0) {
        RKLog("%s Error. Gailed at listen().\n", M->name);
        M->state = RKServerStateNull;
        return NULL;
    }

    if (M->verbose) {
        RKLog("%s sd = %d   port = %d\n", M->name, M->sd, M->port);
    }

    M->state = RKServerStateActive;
    
    RKLog("%s listening on port %d\n", M->name, M->port);

    int             sid;
    fd_set          rfd;
    struct timeval  timeout;
    const char      busy_msg[] = "Server busy." RKEOL;

    // Accept connection requests and create and assign an operator for the client
    while (M->state == RKServerStateActive) {
        // use select to prevent accept() from blocking
        FD_ZERO(&rfd);
        FD_SET(M->sd, &rfd);
        timeout.tv_sec = 0; timeout.tv_usec = 100000;
        ii = select(M->sd + 1, &rfd, NULL, NULL, &timeout);
        if (ii > 0 && FD_ISSET(M->sd, &rfd)) {
            // accept a connection, this part shouldn't be blocked. Reuse sa since we no longer need it
            if ((sid = accept(M->sd, (struct sockaddr *)&sa, &sa_len)) == -1) {
                RKLog("%s Error. Failed at accept().\n", M->name);
                break;
            }
            if (M->verbose) {
                RKLog("%s answering %s:%d  (nclient = %d  sd = %d)\n",
                      M->name, inet_ntoa(sa.sin_addr), sa.sin_port, M->nclient, sid);
            }
            if (M->nclient >= M->maxClient) {
                RKLog("%s busy (nclient = #%d)\n", M->name, M->nclient);
                send(sid, busy_msg, strlen(busy_msg), 0);
                close(sid);
                continue;
            } else {
                RKOperatorCreate(M, sid, inet_ntoa(sa.sin_addr));
            }
        } else if (ii == 0) {
            // This isn't really an error, just mean no one is connecting & timeout...
            continue;
        } else if (ii == -1) {
            RKLog("%s Error. Failed at select().\n", M->name);
        } else {
            RKLog("%s Error. At an unknown state.\n", M->name);
        }
    }
    
    if (M->verbose > 1) {
        RKLog("%s returning ...\n", M->name);
    }
    
    M->state = RKServerStateFree;
    
    return NULL;
}


void *RKOperatorRoutine(void *in) {

    RKOperator      *O = (RKOperator *)in;
    RKServer        *M = O->M;

    int             r;

    FILE            *fp;
    fd_set          rfd;
    fd_set          wfd;
    fd_set          efd;

    //char            str[RKMaximumStringLength];
    char            *str;

    struct timeval  timeout;
    struct timeval  user_timeout = {M->timeoutSeconds, 0};
    struct timeval  latestReadTime;
    struct timeval  latestWriteTime;

    RKLog("%s %s started for the connection from %s (ireq = %d).\n", M->name, O->name, O->ip, M->ireq++);

    // Greet with welcome function
    if (M->w != NULL) {
        M->w(O);
    }

    // Initialize some time values
    gettimeofday(&latestReadTime, NULL);
    gettimeofday(&latestWriteTime, NULL);

    // Get a file descriptor for get line
    fp = fdopen(O->sid, "r");
    if (fp == NULL) {
        RKLog("%s unable to get a file descriptor fro socket descriptor.\n", M->name);
        return (void *)RKResultSDToFDError;
    }

    // Run loop for the read/write
    while (M->state == RKServerStateActive && O->state == RKOperatorStateActive) {
        //
        //  Stream worker
        //
        FD_ZERO(&wfd);
        FD_ZERO(&efd);
        FD_SET(O->sid, &wfd);
        FD_SET(O->sid, &efd);
        if (M->s != NULL) {
            timeout.tv_sec = 0;
            timeout.tv_usec = RKServerSelectTimeoutUs;
            r = select(O->sid + 1, NULL, &wfd, &efd, &timeout);
            if (r > 0) {
                if (FD_ISSET(O->sid, &efd)) {
                    // Exceptions
                    RKLog("%s encountered an exception error.\n", M->name);
                    break;
                } else if (FD_ISSET(O->sid, &wfd)) {
                    // Ready to write (stream)
                    gettimeofday(&latestWriteTime, NULL);
                    M->s(O);
                }
            } else if (r < 0) {
                // Errors
                RKLog("%s Error. Encountered something bad.\n", M->name);
                break;
            } else {
                // Timeout (r == 0)
                gettimeofday(&timeout, NULL);
                timersub(&timeout, &user_timeout, &timeout);
                if (timercmp(&timeout, &latestWriteTime, >=)) {
                    // Dismiss with a terminate function
                    RKLog("%s %s Encountered a timeout (%d seconds).\n", M->name, O->name, M->timeoutSeconds);
                    if (M->t != NULL) {
                        M->t(O);
                    }
                    break;
                }
            }
        }
        //
        //  Command queue
        //
        FD_ZERO(&rfd);
        FD_ZERO(&efd);
        FD_SET(O->sid, &rfd);
        FD_SET(O->sid, &efd);
        timeout.tv_sec = 0;
        timeout.tv_usec = RKServerSelectTimeoutUs;
        r = select(O->sid + 1, &rfd, NULL, &efd, &timeout);
        if (r > 0) {
            if (FD_ISSET(O->sid, &efd)) {
                // Exceptions
                RKLog("%s %s Encountered an exception error.\n", M->name, O->name);
                break;
            } else if (FD_ISSET(O->sid, &rfd)) {
                // Ready to read (command)
                gettimeofday(&latestReadTime, NULL);
                //if (fgets(str, RKMaximumStringLength, fp) == NULL) {
                str = O->commands[O->commandIndexWrite];
                if (fgets(str, RKMaximumStringLength - 1, fp) == NULL) {
                    // When the socket has been disconnected by the client
                    O->cmd = NULL;
                    RKLog("%s %s Disconnected.\n", M->name, O->name);
                    break;
                }
                stripTrailingUnwanted(str);
                O->commandIndexWrite = O->commandIndexWrite == RKServerBufferDepth - 1 ? 0 : O->commandIndexWrite + 1;
                memset(O->commands[O->commandIndexWrite], 0, RKMaximumStringLength);
            }
        } else if (r < 0) {
            // Errors
            RKLog("%s %s Error. Encountered an unexpected error.\n", M->name, O->name);
            break;
        } else if (M->options & RKServerOptionExpectBeacon) {
            // Timeout (r == 0)
            gettimeofday(&timeout, NULL);
            timersub(&timeout, &user_timeout, &timeout);
            if (timercmp(&timeout, &latestReadTime, >=)) {
                // Dismiss with a terminate function
                RKLog("%s %s Encountered a timeout (%d seconds).\n", M->name, O->name, M->timeoutSeconds);
                if (M->t != NULL) {
                    M->t(O);
                }
                break;
            }
        }
    } // while () ...

    if (M->verbose > 1) {
        RKLog(">%s %s operator routine returning ...\n", M->name, O->name);
    }
    M->ids[O->iid] = false;

    fclose(fp);

    return NULL;
}


void *RKOperatorCommandRoutine(void *in) {
    
    RKOperator      *O = (RKOperator *)in;
    RKServer        *M = O->M;
    
    while (true) {
        while (O->commandIndexRead == O->commandIndexWrite && M->state == RKServerStateActive && O->state == RKOperatorStateActive) {
            usleep(10000);
        }
        if (M->state != RKServerStateActive || O->state != RKOperatorStateActive) {
            break;
        }
        // The command to execute
        O->cmd = O->commands[O->commandIndexRead];
        if (M->c) {
            M->c(O);
        } else {
            RKLog("%s No command handler. cmd '%s' from Op-%03d (%s)\n", M->name, O->cmd, O->iid, O->ip);
        }
        O->commandIndexRead = O->commandIndexRead == RKServerBufferDepth - 1 ? 0 : O->commandIndexRead + 1;
    }
    
    if (M->verbose > 1) {
        RKLog(">%s %s command routine returning ...\n", M->name, O->name);
    }
    
    M->ids[O->iid] = false;
    return NULL;
}


int RKOperatorCreate(RKServer *M, int sid, const char *ip) {

    RKOperator *O = (RKOperator *)malloc(sizeof(RKOperator));
    if (O == NULL) {
        RKLog("%s failed to allocate an operator.\n", M->name);
        return 1;
    }
    memset(O, 0, sizeof(RKOperator));

    pthread_mutex_lock(&M->lock);

    int k = 0;
    while (M->ids[k]) {
        k++;
    }
    M->ids[k] = true;

    // Default operator parameters that should not be 0
    O->M = M;
    O->iid = k;
    O->sid = sid;
    O->state = RKOperatorStateActive;
    O->timeoutSeconds = 30;
    O->userResource = M->userResource;
    pthread_mutex_init(&O->lock, NULL);
    snprintf(O->ip, RKMaximumStringLength - 1, "%s", ip);
    snprintf(O->name, RKNameLength - 1, "%sO%d%s",
             rkGlobalParameters.showColor ? RKGetColorOfIndex(O->iid) : "",
             O->iid,
             rkGlobalParameters.showColor ? RKNoColor : "");
    O->delimString.type = RKNetworkPacketTypePlainText;
    O->delimString.size = 0;
    O->delimString.bytes[sizeof(RKNetDelimiter) - 6] = ' ';
    O->delimString.bytes[sizeof(RKNetDelimiter) - 5] = 8;
    O->delimString.bytes[sizeof(RKNetDelimiter) - 4] = ' ';         // Give a spac character to erase
    O->delimString.bytes[sizeof(RKNetDelimiter) - 3] = 8;           // Use a backspace to erase the previous character
    O->delimString.bytes[sizeof(RKNetDelimiter) - 2] = '\r';        // Return line
    O->delimString.bytes[sizeof(RKNetDelimiter) - 1] = '\0';        // End of string
    memcpy(&O->delimTx, &O->delimString, sizeof(RKNetDelimiter));
    O->delimTx.type = RKNetworkPacketTypeBytes;
    O->beacon.type = RKNetworkPacketTypeBeacon;

    if (pthread_create(&O->tidRead, NULL, RKOperatorRoutine, O)) {
        RKLog("%s Error. Failed to create RKOperatorRoutine().\n", M->name);
        pthread_mutex_unlock(&M->lock);
        return RKResultErrorCreatingOperatorRoutine;
    }
    if (pthread_create(&O->tidExecute, NULL, RKOperatorCommandRoutine, O)) {
        RKLog("%s Error. Failed to create RKOperatorCommandRoutine().\n", M->name);
        pthread_mutex_unlock(&M->lock);
        return RKResultErrorCreatingOperatorCommandRoutine;
    }
    
    M->nclient++;
    
    pthread_mutex_unlock(&M->lock);
    
    return 0;
}


void RKOperatorFree(RKOperator *O) {

    RKServer *M = O->M;
    
    O->state = RKOperatorStateClosing;
    pthread_join(O->tidRead, NULL);
    pthread_join(O->tidExecute, NULL);
    pthread_mutex_destroy(&O->lock);
    close(O->sid);
    
    free(O);
    
    pthread_mutex_lock(&M->lock);
    M->nclient--;
    pthread_mutex_unlock(&M->lock);
}


int RKDefaultWelcomeHandler(RKOperator *O) {

    char msg[6 * RKMaximumStringLength];

    char *c = O->M->name;
    
    int ii;
    // Count the characters between two escape characters
    if (*c == '\033') {
        while (*c != 'm') {
            c++;
        }
        c++;
        char *e = c;
        while (*e != '\033') {
            e++;
        }
        ii = (int)(e - c) + 1;
    } else {
        ii = RKMaximumStringLength - 1;
    }
    
    int nlines = 2;
    char line_array[nlines][RKMaximumStringLength];
    snprintf(line_array[0], ii, "%s", c);
    sprintf(line_array[1], RKVersionString);
    size_t max_len = 0, len;
    for (ii = 0; ii < nlines; ii++) {
        len = strlen(line_array[ii]);
        if (max_len < len) {
            max_len = len;
        }
    }
    char border[max_len + 1];
    char padded_line[max_len + 1];
    memset(border, '-', max_len); border[max_len] = 0;

    // Compose the banner
    sprintf(msg, "+---%s---+\n", border);
    for (ii=0; ii<nlines; ii++) {
        memset(padded_line, ' ', max_len);
        len = strlen(line_array[ii]);
        memcpy(padded_line + (max_len - len) / 2, line_array[ii], len);
        sprintf(msg + strlen(msg), "|   %s   |\n", padded_line);
    }
    sprintf(msg + strlen(msg),
            "+---%s---+\n"
            "Hello there, my name is %s.\n"
            "What can I do for you?" RKEOL,
            border,
            O->name);
    send(O->sid, msg, strlen(msg), 0);
    return 0;
}


int RKDefaultTerminateHandler(RKOperator *O) {
    char str[] = "You are boring. I'm disconnecting you...\nBye." RKEOL;
    send(O->sid, str, strlen(str), 0);
    return 0;
}

#pragma mark -
#pragma mark Life Cycle

RKServer *RKServerInit(void) {
    RKServer *M = (RKServer *)malloc(sizeof(RKServer));
    if (M == NULL) {
        RKLog("Error. Unable to allocate RKServer.\n");
        return NULL;
    }
    memset(M, 0, sizeof(RKServer));
    M->port = 10000;
    M->maxClient = RKServerMaximumOperators;
    M->timeoutSeconds = 5;
    M->w = &RKDefaultWelcomeHandler;
    M->t = &RKDefaultTerminateHandler;
    pthread_mutex_init(&M->lock, NULL);

    // Ignore broken pipe for clients that disconnect unexpectedly
    signal(SIGPIPE, SIG_IGN);
    
    return M;
}


void RKServerFree(RKServer *M) {
    while (M->state > RKServerStateFree) {
        usleep(100000);
    }
    free(M);
}

#pragma mark -
#pragma mark Delegate functions

void RKServerSetName(RKServer *M, const char *name) {
    strcpy(M->name, name);
}

void RKServerSetWelcomeHandler(RKServer *M, int (*function)(RKOperator *)) {
    M->w = function;
}


void RKServerSetCommandHandler(RKServer *M, int (*function)(RKOperator *)) {
    M->c = function;
}


void RKServerSetTerminateHandler(RKServer *M, int (*function)(RKOperator *)) {
    M->t = function;
}


void RKServerSetStreamHandler(RKServer *M, int (*function)(RKOperator *)) {
    M->s = function;
}


void RKServerSetWelcomeHandlerToDefault(RKServer *M) {
    M->w = &RKDefaultWelcomeHandler;
}


void RKServerSetTerminateHandlerToDefault(RKServer *M) {
    M->t = &RKDefaultTerminateHandler;
}

void RKServerSetSharedResource(RKServer *M, void *resource) {
    M->userResource = resource;
}

#pragma mark -
#pragma mark Server actions


void RKServerStart(RKServer *M) {
    M->state = RKServerStateOpening;
    if (pthread_create(&M->threadId, NULL, RKServerRoutine, M)) {
        RKLog("%s Error. Unable to launch main server.\n", M->name);
    }
    while (M->state > RKServerStateNull && M->state < RKServerStateActive) {
        usleep(25000);
    }
    if (M->verbose) {
        RKLog("%s Started.\n", M->name);
    }
}


void RKServerWait(RKServer *M) {
    while (M->state > RKServerStateFree) {
        usleep(100000);
    }
}


void RKServerStop(RKServer *M) {
    if (M->state == RKServerStateActive) {
        M->state = RKServerStateClosing;
    }
}

#pragma mark -
#pragma mark Miscellaneous functions


// In counts of 10ms, should be plenty, in-transit buffer should be able to hold it
#define RKSocketTimeCountOf10ms  10

// Use as:
// RKOperatorSendPackets(operator, payload, size, payload, size, ..., NULL);

ssize_t RKOperatorSendPackets(RKOperator *O, ...) {

    va_list   arg;

    void      *payload;
    ssize_t   payloadSize = 1;

    ssize_t   grandTotalSentSize = 0;
    ssize_t   totalSentSize = 0;
    ssize_t   sentSize = 0;

    int timeout_count = 0;

    pthread_mutex_lock(&O->lock);

    va_start(arg, O);

    // Parse the input arguments until payload = NULL (last input)
    payload = va_arg(arg, void *);
    while (payload != NULL) {
        payloadSize = va_arg(arg, ssize_t);
        //printf("size %d @ %p\n", (int)payloadSize, payload);
        sentSize = 0;
        totalSentSize = 0;
        while (totalSentSize < payloadSize && timeout_count++ < RKSocketTimeCountOf10ms) {
            if ((sentSize = send(O->sid, payload + sentSize, payloadSize - sentSize, 0)) > 0) {
                totalSentSize += sentSize;
            }
            if (totalSentSize < payloadSize) {
                usleep(1000);
            } else if (sentSize < 0) {
                pthread_mutex_unlock(&O->lock);
                return RKResultIncompleteSend;
            }
        }
        grandTotalSentSize += totalSentSize;
        payload = va_arg(arg, void *);
    }
    
    va_end(arg);
    
    pthread_mutex_unlock(&O->lock);
    
    if (timeout_count >= RKSocketTimeCountOf10ms) {
        return RKResultTimeout;
    }
    
    return grandTotalSentSize;
}

ssize_t RKOperatorSendString(RKOperator *O, const char *string) {
    return RKOperatorSendPackets(O, string, strlen(string), NULL);
}

ssize_t RKOperatorSendDelimitedString(RKOperator *O, const char *string) {
    O->delimString.type = RKNetworkPacketTypePlainText;
    O->delimString.size = (uint32_t)strlen(string);
    return RKOperatorSendPackets(O, &O->delimString, sizeof(RKNetDelimiter), string, O->delimString.size, NULL);
}

ssize_t RKOperatorSendCommandResponse(RKOperator *O, const char *string) {
    O->delimString.type = RKNetworkPacketTypeCommandResponse;
    O->delimString.size = (uint32_t)strlen(string);
    return RKOperatorSendPackets(O, &O->delimString, sizeof(RKNetDelimiter), string, O->delimString.size, NULL);
}

ssize_t RKOperatorSendBeacon(RKOperator *O) {
    ssize_t s = RKOperatorSendPackets(O, &O->beacon, sizeof(RKNetDelimiter), NULL);
    //O->beacon.userParameter1++;
    return s;
}

void RKOperatorHangUp(RKOperator *O) {
    return RKOperatorFree(O);
}
