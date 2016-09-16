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
int RKOperatorCreate(RKServer *, int);
int RKDefaultWelcomeHandler(RKOperator *);
int RKDefaultTerminateHandler(RKOperator *);

// Implementation

#pragma mark -
#pragma mark Private functions


void *RKServerRoutine(void *in) {
    RKServer *M = (RKServer *)in;

    M->state = RKServerStateActive;

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
    RKLog("RKServerRoutine() has SD = %d\n", M->sd);

    // Bind
    if (bind(M->sd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        RKLog("Error. RKServerRoutine() failed at bind().\n");
        close(M->sd);
        M->state = RKServerStateNull;
        return NULL;
    }

    // Listen
    if (listen(M->sd, M->maxClient + 1) < 0) {
        RKLog("Error. RKServerRoutine() failed at listen().\n");
        M->state = RKServerStateNull;
        return NULL;
    }

    int             sid;
    fd_set          rfd;
    struct timeval  timeout;
    const char      busy_msg[] = "Server busy." RKEOL;

    // Accept connection requests and create and assign an operator for the client
    while (M->state == RKServerStateActive) {
        // use select to prevent accept() from blocking
        FD_ZERO(&rfd);
        FD_SET(M->sd, &rfd);
        timeout.tv_sec = 0; timeout.tv_usec = 250000;
        ii = select(M->sd + 1, &rfd, NULL, NULL, &timeout);
        if (ii > 0 && FD_ISSET(M->sd, &rfd)) {
            // accept a connection, this part shouldn't be blocked. Reuse sa since we no longer need it
            if ((sid = accept(M->sd, (struct sockaddr *)&sa, &sa_len)) == -1) {
                RKLog("Error. RKServerRoutine() failed at accept().\n");
                break;
            }
            RKLog("RKServerRoutine() answering %s:%d (%d)\n", inet_ntoa(sa.sin_addr), sa.sin_port, M->nclient);
            if (M->nclient >= M->maxClient) {
                RKLog("RKServerRoutine() busy (nclient = #%d)\n", M->nclient);
                send(sid, busy_msg, strlen(busy_msg), 0);
                close(sid);
                continue;
            } else {
                RKOperatorCreate(M, sid);
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

    RKOperator      *A = (RKOperator *)in;
    RKServer        *M = A->M;

    int             r;

    FILE            *fp;
    fd_set          rfd;
    fd_set          wfd;
    fd_set          efd;

    struct timeval  timeout;

    RKLog("Op-%03d started for customer %d.\n", A->iid, M->ireq);

    // Greet with welcome function
    if (M->w != NULL) {
        M->w(A);
    }

    int		        mwait = 0;
    int             rwait = 0;
    int             wwait = 0;
    char            str[RKMaximumStringLength];
    char            *c;

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
    fp = fdopen(A->sid, "r");

    // Run loop for the read/write
    while (M->state == RKServerStateActive && A->state == RKServerStateActive) {
        FD_ZERO(&rfd);
        FD_ZERO(&wfd);
        FD_ZERO(&efd);
        FD_SET(A->sid, &rfd);
        FD_SET(A->sid, &wfd);
        FD_SET(A->sid, &efd);
        //
        //  Stream worker
        //
        if (M->s != NULL) {
            timeout.tv_sec = 0;
            timeout.tv_usec = 1000;
            r = select(A->sid + 1, NULL, &wfd, &efd, &timeout);
            if (r > 0) {
                if (FD_ISSET(A->sid, &efd)) {
                    // Exceptions
                    fprintf(stderr, "Exception(s) occurred.\n");
                    break;
                } else if (FD_ISSET(A->sid, &wfd)) {
                    // Ready to write (stream)
                    wwait = 0;
                    M->s(A);
                }
            } else if (r < 0) {
                // Errors
                fprintf(stderr, "Error(s) occurred.\n");
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
        timeout.tv_usec = 100;
        r = select(A->sid + 1, &rfd, NULL, &efd, &timeout);
        if (r > 0) {
            if (FD_ISSET(A->sid, &efd)) {
                // Exceptions
                RKLog("Error. Exception(s) occurred.\n");
                break;
            } else if (FD_ISSET(A->sid, &rfd)) {
                // Ready to read (command)
                rwait = 0;
                if (fgets(str, RKMaximumStringLength, fp) == NULL) {
                    // When the socket has been disconnected by the client
                    //fprintf(stderr, "selected() indicated ready for read but nothing.\n");
                    A->cmd = NULL;
                    break;
                }
                if (M->c != NULL) {
                    // Strip out \r, \n, white space, \10 (BS), etc.
                    c = str + strlen(str) - 1;
                    while (c >= str && (*c == '\r' || *c == '\n' || *c == ' ' || *c == 10)) {
                        *c-- = '\0';
                    }
                    // Set the command so that handle_command function can retrieve this
                    A->cmd = str;
                    if (M->c) {
                        M->c(A);
                    } else {
                        RKLog("cmd '%s' has no processor.\n", A->cmd);
                    }
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
        if (mwait > M->timeoutInSec * 10 && !(A->option & RKOperatorOptionKeepAlive)) {
            RKLog("Op-%03d encountered a timeout.\n", A->iid);
            // Dismiss with a terminate function
            if (M->t != NULL) {
                M->t(A);
            }
            break;
        }
    } // while () ...

    RKLog("Op-%03d returning ...\n", A->iid);

    fclose(fp);
    close(A->sid);
    pthread_mutex_destroy(&A->lock);

    free(A);

    pthread_mutex_lock(&M->lock);
    M->nclient--;
    pthread_mutex_unlock(&M->lock);

    return NULL;
}


int RKOperatorCreate(RKServer *M, int sid) {

    RKOperator *A = (RKOperator *)malloc(sizeof(RKOperator));

    if (A == NULL) {
        RKLog("Error Failed to allocate RKOperator.\n");
        return 1;
    }

    pthread_mutex_lock(&M->lock);

    // Default operator parameters
    A->M = M;
    A->sid = sid;
    A->iid = sid - M->sd;
    A->state = RKServerStateActive;
    A->option = RKOperatorOptionNone;
    A->timeoutInSec = 10;
    A->usr = M->usr;
    RKServerSetWelcomeHandlerToDefault(M);
    PSServerSetTerminateHandlerToDefault(M);
    pthread_mutex_init(&A->lock, NULL);
    snprintf(A->name, RKMaximumStringLength - 1, "Op-%03d", A->iid);
    
    if (pthread_create(&A->tid, NULL, RKOperatorRoutine, A)) {
        RKLog("Error. Failed to create RKOperatorRoutine().\n");
        pthread_mutex_unlock(&M->lock);
        return 2;
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


int RKDefaultTerminateHandler(RKOperator *A) {
    char str[] = "You are boring. I'm disconnecting you...\nBye." RKEOL;
    send(A->sid, str, strlen(str), 0);
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
    M->maxClient = 2;
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
    if (pthread_create(&M->tid, NULL, RKServerRoutine, M)) {
        RKLog("Error. Unable to launch main server.\n");
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
#define PS_MAX_TIMEOUT_COUNT  10

ssize_t RKOperatorSendPackets(RKOperator *A, ...) {

    va_list   arg;

    void      *payload;
    ssize_t   payload_size = 1;

    ssize_t   grant_total_sent_size = 0;
    ssize_t   total_sent_size = 0;
    ssize_t   sent_size = 0;

    int timeout_count = 0;

    pthread_mutex_lock(&A->lock);

    va_start(arg, A);

    // Parse the input arguments until payload = NULL (last input)
    payload = va_arg(arg, void *);
    while (payload != NULL) {
        payload_size = va_arg(arg, ssize_t);
        //printf("%d @ %p\n", (int)len, payload);
        sent_size = 0;
        total_sent_size = 0;
        while (total_sent_size < payload_size && timeout_count++ < PS_MAX_TIMEOUT_COUNT) {
            if ((sent_size = send(A->sid, payload+sent_size, payload_size-sent_size, 0)) > 0) {
                total_sent_size += sent_size;
            }
            if (total_sent_size < payload_size) {
                usleep(10000);
            } else if (sent_size == -1) {
                pthread_mutex_unlock(&A->lock);
                return RKResultIncompleteSend;
            }
        }
        grant_total_sent_size += total_sent_size;
        payload = va_arg(arg, void *);
    }
    
    va_end(arg);
    
    pthread_mutex_unlock(&A->lock);
    
    if (timeout_count >= PS_MAX_TIMEOUT_COUNT) {
        return RKResultTimeout;
    }
    
    return payload_size;
}
