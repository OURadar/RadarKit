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

//#ifdef __cplusplus
//extern "C" {
//#endif

typedef int RKServerState;

enum RKServerState {
    RKServerStateNull,
    RKServerStateFree,
    RKServerStateClosing,
    RKServerStateActive
};

typedef int RKOperatorOption;

enum RKOperatorOption {
    RKOperatorOptionNone         = 0,
    RKOperatorOptionKeepAlive    = 1
};


typedef struct rk_server    RKServer;
typedef struct rk_operator  RKOperator;

struct rk_server {
    int              sd;                             //!< Socket descriptor
    int              port;                           //!< Port number of the server
    int              maxClient;                      //!< Maximum number of client connections
    int              timeoutInSec;                   //!< Timeout in seconds

    int              nclient;                        //!< Number of clients
    int              ireq;                           //!< A global instance request
    int              state;                          //!< A global flag for infinite loop
    pthread_t        tid;                            //!< Own thread ID
    pthread_mutex_t  lock;                           //!< Thread safety mutex of the server

    char             name[RKMaximumStringLength];    //!< A program name

    int              (*w)(RKOperator *);             //!< Function that sends welcome message
    int              (*c)(RKOperator *);             //!< Function that answers command
    int              (*s)(RKOperator *);             //!< Function that keeps streaming content
    int              (*t)(RKOperator *);             //!< Function that terminates connection
    
    void             *usr;                           //!< User pointer
};

struct rk_operator  {
    RKServer         *M;                              //!< Pointer to main server for common resources
    RKOperatorOption option;                         //!< Keep alive flag

    int              iid;                            //!< Instant identifier
    int              timeoutInSec;                   //!< Timeout in seconds
    int              sid;                            //!< Socket identifier of the client
    RKServerState    state;                          //!< Connection state
    pthread_t        tid;                            //!< Thread ID
    pthread_mutex_t  lock;                           //!< Thread safety mutex of the attendant

    char             name[RKMaximumStringLength];    //!< Attendant's name

    char             *cmd;                           //!< Latest command

    void             *usr;                           //!< User pointer
};


ssize_t RKOperatorSendPackets(RKOperator *O, ...);

RKServer *RKServerInit(void);
void RKServerFree(RKServer *M);

void RKServerSetWelcomeHandler(RKServer *M, int (*function)(RKOperator *));
void RKServerSetCommandHandler(RKServer *M, int (*function)(RKOperator *));
void RKServerSetTerminateHandler(RKServer *M, int (*function)(RKOperator *));
void RKServerSetStreamHandler(RKServer *M, int (*function)(RKOperator *));
void RKServerSetWelcomeHandlerToDefault(RKServer *M);
void PSServerSetTerminateHandlerToDefault(RKServer *M);

void RKServerActivate(RKServer *M);
void RKServerWait(RKServer *M);
void RKServerStop(RKServer *M);

//#ifdef __cplusplus
//}
//#endif

#endif /* defined(___RadarKit_RKServer__) */
