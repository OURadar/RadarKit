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

typedef struct rk_server    RKNetworkServer;
typedef struct rk_operator  RKOperator;

struct RKNetworkServer {
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
    RKNetworkServer *M;                              //!< Pointer to main server for common resources

    int              i;                              //!< Instant number
    int              option;                         //!< Keep alive flag
    int              timeoutInSec;                   //!< Timeout in seconds

    int              sid;                            //!< Socket identifier of the client
    int              state;                          //!< Connection state
    pthread_t        tid;                            //!< Thread ID
    pthread_mutex_t  lock;                           //!< Thread safety mutex of the attendant

    char             name[RKMaximumStringLength];    //!< Attendant's name

    char             *cmd;                           //!< Latest command

    void             *usr;                           //!< User pointer
};


ssize_t RKSendPackets(RKOperator *O, ...);

//#ifdef __cplusplus
//}
//#endif

#endif /* defined(___RadarKit_RKServer__) */
