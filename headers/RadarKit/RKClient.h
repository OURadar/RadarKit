//
//  RKClient.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/17/15.
//
//

#ifndef __RadarKit_RKClient__
#define __RadarKit_RKClient__

#include <RadarKit/RKNetwork.h>

//#ifdef __cplusplus
//extern "C" {
//#endif

typedef int RKClientState;

enum RKClientState {
    RKClientStateNull,
    RKClientStateResolvingIP,
    RKClientStateConfiguringSocket,
    RKClientStateConnecting,
    RKClientStateConnected,
    RKClientStateDisconnecting,
    RKClientStateDisconnected
};

typedef struct RKClientDesc {
    char                    hostname[RKMaximumStringLength];  // Hostname
    int                     port;                             // Port number of the server
    int                     timeoutInSeconds;                 // Timeout in seconds
    RKSocketType            type;                             // Socket type
    RKMessageFormat         format;                           // Payload format
    int                     blockLength;                      // Payload length
    bool                    blocking;                         // Blocking read
    bool                    reconnect;
    int                     verb;
} RKClientDesc;

typedef struct rk_client RKClient;

struct rk_client {
    char                  hostname[RKMaximumStringLength];  // Hostname
    int                   port;                             // Port number of the server
    int                   timeoutInSeconds;                 // Timeout in seconds
    RKSocketType          type;                             // Socket type
    RKMessageFormat       format;                           // Payload format
    int                   blockLength;                      // Payload length
    bool                  blocking;                         // Blocking read
    bool                  reconnect;
    int                   verb;

    int                   (*i)(RKClient *);                 // Initialization handler
    int                   (*r)(RKClient *, const char *);   // Receive handler

    // Everything past here should be internal to the framework
  
    char                  host_IP[32];                      // Host IP in numbers
    socklen_t             sa_len;                           // Address length (IPv4 / IPv6)
    struct sockaddr_in    sa;                               // Socket address
    int                   sd;                               // Socket descriptor
  
    bool                  run;                              // A flag for infinite run
    bool                  safe_to_close;                    // A flag indicating safe to close
    int                   ireq;                             // A global instance request
    RKClientState         state;                            // A global flag for infinite loop

    pthread_t             tid;                              // Own thread ID
    pthread_attr_t        tattr;                            // Thread attributes
    pthread_mutex_t       tlock;                            // Thread safety mutex of the server

    fd_set                rfd;                              // Read ready
    fd_set                wfd;                              // Write ready
    fd_set                efd;                              // Error occurred

    char                  name[RKMaximumStringLength];      // A program name

    void                  *usr;                             // User pointer
};


//#ifdef __cplusplus
//}
//#endif

#endif /* defined(___RadarKit_RKClient__) */
