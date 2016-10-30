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
int RKOperatorCreate(RKServer *, int, const char *);
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
        RKLog("RKServerRoutine() failed at socket().\n");
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
        RKLog("Error. RKServerRoutine() failed at setsockopt().\n");
        M->state = RKServerStateNull;
        return NULL;
    }

    // Bind
    if (bind(M->sd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        RKLog("Error. RKServerRoutine() failed at bind().\n");
        M->state = RKServerStateNull;
        close(M->sd);
        return NULL;
    }

    // Listen
    if (listen(M->sd, M->maxClient + 1) < 0) {
        RKLog("Error. RKServerRoutine() failed at listen().\n");
        M->state = RKServerStateNull;
        return NULL;
    }

    RKLog("RKServerRoutine()  sd = %d  port = %d\n", M->sd, M->port);

    M->state = RKServerStateActive;

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
                RKLog("Error. RKServerRoutine() failed at accept().\n");
                break;
            }
            RKLog("RKServerRoutine() answering %s:%d  (nclient = %d  sd = %d)\n", inet_ntoa(sa.sin_addr), sa.sin_port, M->nclient, sid);
            if (M->nclient >= M->maxClient) {
                RKLog("RKServerRoutine() busy (nclient = #%d)\n", M->nclient);
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
            RKLog("Error. RKServerRoutine() failed at select().\n");
        } else {
            RKLog("Error. RKServerRoutine at an unknown state.\n");
        }
    }
    
    RKLog("RKServerRoutine() retiring ...\n");
    
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

    struct timeval  timeout;

    RKLog("Op-%03d started for customer %d.\n", O->iid, M->ireq);

    // Greet with welcome function
    if (M->w != NULL) {
        M->w(O);
    }

    int		        mwait = 0;
    int             rwait = 0;
    int             wwait = 0;
    char            str[RKMaximumStringLength];

    // Get whatever that is in-transit, should look at server options
    /*
     FD_ZERO(&rfd);
     FD_SET(A->sid, &rfd);
     timeout.tv_sec = 0;   timeout.tv_usec = 100000;
     r = select(A->sid + 1, &rfd, NULL, &efd, &timeout);
     if (r > 0 && FD_ISSET(A->sid, &rfd)) {
     read(A->sid, str, PS_MAX_STR);
     //printf("Flushed : %s\n", str);
     }
     */

    // Get a file descriptor for get line
    fp = fdopen(O->sid, "r");
    if (fp == NULL) {
        RKLog("Unable to get a file descriptor fro socket descriptor.\n");
        return (void *)RKResultSDToFDError;
    }

    // Run loop for the read/write
    while (M->state == RKServerStateActive && O->state == RKOperatorStateActive) {
        FD_ZERO(&rfd);
        FD_ZERO(&wfd);
        FD_ZERO(&efd);
        FD_SET(O->sid, &rfd);
        FD_SET(O->sid, &wfd);
        FD_SET(O->sid, &efd);
        //
        //  Stream worker
        //
        if (M->s != NULL) {
            timeout.tv_sec = 0;
            timeout.tv_usec = 1000;
            r = select(O->sid + 1, NULL, &wfd, &efd, &timeout);
            if (r > 0) {
                if (FD_ISSET(O->sid, &efd)) {
                    // Exceptions
                    fprintf(stderr, "Exception(s) occurred.\n");
                    break;
                } else if (FD_ISSET(O->sid, &wfd)) {
                    // Ready to write (stream)
                    wwait = 0;
                    M->s(O);
                }
            } else if (r < 0) {
                // Errors
                RKLog("Error. Something bad occurred.\n");
                break;
            } else {
                // Timeout (r == 0)
                wwait++;
            }
        }
        //
        //  Command worker
        //
        timeout.tv_sec = 0;
        timeout.tv_usec = 1000;
        r = select(O->sid + 1, &rfd, NULL, &efd, &timeout);
        if (r > 0) {
            if (FD_ISSET(O->sid, &efd)) {
                // Exceptions
                RKLog("Error. Exception(s) occurred.\n");
                break;
            } else if (FD_ISSET(O->sid, &rfd)) {
                // Ready to read (command)
                rwait = 0;
                if (fgets(str, RKMaximumStringLength, fp) == NULL) {
                    // When the socket has been disconnected by the client
                    O->cmd = NULL;
                    RKLog("Error. Disconnected by client.\n");
                    break;
                }
                stripTrailingUnwanted(str);
                // Set the command so that a command handler can retrieve this
                O->cmd = str;
                if (M->c) {
                    M->c(O);
                } else {
                    RKLog("No command handler. cmd '%s' from Op-%03d (%s)\n", O->cmd, O->iid, O->ip);
                }
            }
        } else if (r < 0) {
            // Errors
            RKLog("Error. Something bad occurred in RKOperatorRoutine().\n");
            break;
        } else {
            // Timeout (r == 0)
            rwait++;
        }
        // Process the timeout
        mwait = wwait < rwait ? wwait : rwait;
        if (mwait > M->timeoutInSec * 10 && !(O->option & RKOperatorOptionKeepAlive)) {
            RKLog("Op-%03d encountered a timeout.\n", O->iid);
            // Dismiss with a terminate function
            if (M->t != NULL) {
                M->t(O);
            }
            break;
        }
    } // while () ...

    RKLog("Op-%03d returning ...\n", O->iid);

    fclose(fp);
    close(O->sid);
    pthread_mutex_destroy(&O->lock);

    free(O);

    pthread_mutex_lock(&M->lock);
    M->nclient--;
    pthread_mutex_unlock(&M->lock);

    return NULL;
}


int RKOperatorCreate(RKServer *M, int sid, const char *ip) {

    RKOperator *O = (RKOperator *)malloc(sizeof(RKOperator));
    if (O == NULL) {
        RKLog("Error Failed to allocate RKOperator.\n");
        return 1;
    }
    memset(O, 0, sizeof(RKOperator));

    pthread_mutex_lock(&M->lock);

    // Default operator parameters that should not be 0
    O->M = M;
    O->sid = sid;
    O->iid = sid - M->sd;
    O->state = RKOperatorStateActive;
    O->option = RKOperatorOptionNone;
    O->timeoutInSec = 10;
    O->usr = M->usr;
    RKServerSetWelcomeHandlerToDefault(M);
    PSServerSetTerminateHandlerToDefault(M);
    pthread_mutex_init(&O->lock, NULL);
    snprintf(O->ip, RKMaximumStringLength - 1, "%s", ip);
    snprintf(O->name, RKMaximumStringLength - 1, "Op-%03d", O->iid);
    O->delim.type = RKPacketTypePlainText;
    O->delim.rawSize = 0;
    O->delim.bytes[sizeof(RKNetDelimiter) - 2] = '\r';
    O->delim.bytes[sizeof(RKNetDelimiter) - 1] = '\0';
    O->beacon.type = RKPacketTypeBeacon;
    O->beacon.rawSize = 0;
    O->beacon.bytes[sizeof(RKNetDelimiter) - 2] = '\0';
    O->beacon.bytes[sizeof(RKNetDelimiter) - 1] = '\0';

    if (pthread_create(&O->tid, NULL, RKOperatorRoutine, O)) {
        RKLog("Error. Failed to create RKOperatorRoutine().\n");
        pthread_mutex_unlock(&M->lock);
        return RKResultErrorCreatingOperatorRoutine;
    }
    
    M->nclient++;
    
    pthread_mutex_unlock(&M->lock);
    
    return 0;
}


int RKDefaultWelcomeHandler(RKOperator *A) {

    char msg[6 * RKMaximumStringLength];

    char *line_array[] = {A->M->name, "v1.0"};
    size_t max_len = 0, len;
    int ii;
    int nlines = sizeof(line_array)/sizeof(void *);
    for (ii=0; ii<nlines; ii++) {
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
            A->name);
    send(A->sid, msg, strlen(msg), 0);
    return 0;
}


int RKDefaultTerminateHandler(RKOperator *O) {
    char str[] = "You are boring. I'm disconnecting you...\nBye." RKEOL;
    send(O->sid, str, strlen(str), 0);
    return 0;
}

#pragma mark -
#pragma mark Initialization and deallocation

RKServer *RKServerInit(void) {
    RKServer *M = (RKServer *)malloc(sizeof(RKServer));
    if (M == NULL) {
        RKLog("Error. Unable to allocate RKServer.\n");
        return NULL;
    }
    memset(M, 0, sizeof(RKServer));
    M->port = 10000;
    M->maxClient = 8;
    M->timeoutInSec = 5;
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


void PSServerSetTerminateHandlerToDefault(RKServer *M) {
    M->t = &RKDefaultTerminateHandler;
}


#pragma mark -
#pragma mark Server actions


void RKServerActivate(RKServer *M) {
    M->state = RKServerStateOpening;
    if (pthread_create(&M->tid, NULL, RKServerRoutine, M)) {
        RKLog("Error. Unable to launch main server.\n");
    }
    while (M->state > RKServerStateNull && M->state < RKServerStateActive) {
        usleep(25000);
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
        //printf("%d @ %p\n", (int)payloadSize, payload);
        sentSize = 0;
        totalSentSize = 0;
        while (totalSentSize < payloadSize && timeout_count++ < RKSocketTimeCountOf10ms) {
            if ((sentSize = send(O->sid, payload + sentSize, payloadSize - sentSize, 0)) > 0) {
                totalSentSize += sentSize;
            }
            if (totalSentSize < payloadSize) {
                usleep(10000);
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
    O->delim.rawSize = (uint32_t)strlen(string) + 1;
    return RKOperatorSendPackets(O, &O->delim, sizeof(RKNetDelimiter), string, O->delim.rawSize, NULL);
}

ssize_t RKOperatorSendBeacon(RKOperator *O) {
    ssize_t s = RKOperatorSendPackets(O, &O->beacon, sizeof(RKNetDelimiter), NULL);
    //O->beacon.userParameter1++;
    return s;
}

void RKOperatorHangUp(RKOperator *O) {
    O->state = RKOperatorStateClosing;
}
