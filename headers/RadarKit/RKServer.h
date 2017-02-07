//
//  RKServer.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/17/15.
//
//

#ifndef __RadarKit_RKServer__
#define __RadarKit_RKServer__

#include <RadarKit/RKNetwork.h>

#define RKServerMaximumOperators    16
#define RKServerSelectTimeoutUs     200;

//#ifdef __cplusplus
//extern "C" {
//#endif

typedef int RKServerState;

enum RKServerState {
    RKServerStateNull,
    RKServerStateFree,
    RKServerStateOpening,
    RKServerStateClosing,
    RKServerStateActive
};

typedef int RKOperatorState;
enum RKOperatorState {
    RKOperatorStateNull,
    RKOperatorStateFree,
    RKOperatorStateClosing,
    RKOperatorStateActive
};

typedef int RKServerOption;
enum RKServerOption {
    RKServerOptionNone           = 0,
    RKServerOptionExpectBeacon   = 1
};


typedef struct rk_server    RKServer;
typedef struct rk_operator  RKOperator;

struct rk_server {
    char             name[RKNameLength];             // A program name
    int              verbose;
    
    int              sd;                             // Socket descriptor
    int              port;                           // Port number of the server
    int              maxClient;                      // Maximum number of client connections
    int              timeoutSeconds;                 // Timeout in seconds
    RKServerOption   options;                        // Server options

    bool             ids[RKServerMaximumOperators];  // Operator IDs
    int              nclient;                        // Number of clients
    int              ireq;                           // A global instance request
    int              state;                          // A global flag for infinite loop
    pthread_t        threadId;                       // Own thread ID
    pthread_mutex_t  lock;                           // Thread safety mutex of the server

    int              (*w)(RKOperator *);             // Function that sends initial welcome message
    int              (*c)(RKOperator *);             // Function that answers command
    int              (*s)(RKOperator *);             // Function that keeps streaming content
    int              (*t)(RKOperator *);             // Function that terminates connection
    
    void             *userResource;                  // User pointer
};

struct rk_operator  {
    RKServer         *M;                             // Pointer to main server for common resources

    int              iid;                            // Instant identifier
    int              timeoutSeconds;                 // Timeout in seconds
    int              sid;                            // Socket identifier of the client
    RKOperatorState  state;                          // Connection state
    pthread_t        threadId;                       // Thread ID
    pthread_mutex_t  lock;                           // Thread safety mutex of the attendant

    char             name[RKNameLength];             // Operator name
    char             ip[RKMaximumStringLength];      // Client's IP address

    RKNetDelimiter   delimString;                    // Convenient delimiter for text response of commands
    RKNetDelimiter   delimTx;                        // Convenient delimiter for streaming
    RKNetDelimiter   beacon;                         // Beacon

    char             *cmd;                           // Latest command

    void             *userResource;                  // User pointer
};


ssize_t RKOperatorSendPackets(RKOperator *, ...);
ssize_t RKOperatorSendString(RKOperator *, const char *);
ssize_t RKOperatorSendDelimitedString(RKOperator *, const char *);
ssize_t RKOperatorSendBeacon(RKOperator *);
void RKOperatorHangUp(RKOperator *);

RKServer *RKServerInit(void);
void RKServerFree(RKServer *);

void RKServerSetName(RKServer *, const char *);
void RKServerSetWelcomeHandler(RKServer *, int (*)(RKOperator *));
void RKServerSetCommandHandler(RKServer *, int (*)(RKOperator *));
void RKServerSetTerminateHandler(RKServer *, int (*)(RKOperator *));
void RKServerSetStreamHandler(RKServer *, int (*)(RKOperator *));
void RKServerSetWelcomeHandlerToDefault(RKServer *);
void RKServerSetTerminateHandlerToDefault(RKServer *);
void RKServerSetSharedResource(RKServer *, void *);

void RKServerStart(RKServer *);
void RKServerWait(RKServer *);
void RKServerStop(RKServer *);

//#ifdef __cplusplus
//}
//#endif

#endif /* defined(___RadarKit_RKServer__) */
