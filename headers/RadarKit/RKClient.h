//
//  RKClient.h
//  RadarKit
//
//  This collection is ported and slightly modified from PortClient.
//  Most changes are to accomodate the design philosophy of RadarKit.
//  RKClient and PortClient are not interchangeable!
//
//  Created by Boon Leng Cheong on 1/4/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_Client__
#define __RadarKit_Client__

#include <RadarKit/RKNetwork.h>

//#ifdef __cplusplus
//extern "C" {
//#endif

typedef int RKClientState;

enum RKClientState {
    RKClientStateNull,
    RKClientStateCreating,
    RKClientStateResolvingIP,
    RKClientStateConfiguringSocket,
    RKClientStateConnecting,
    RKClientStateConnected,
    RKClientStateReconnecting,
    RKClientStateDisconnecting,
    RKClientStateDisconnected
};

typedef struct rk_client_desc {
    RKName                   name;                               // A program name
    RKName                   hostname;                           // Hostname
    int                      port;                               // Port number of the server
    int                      timeoutSeconds;                     // Timeout in seconds
    RKNetworkSocketType      type;                               // Socket type
    RKNetworkMessageFormat   format;                             // Payload format
    int                      blockLength;                        // Payload length
    bool                     blocking;                           // Blocking read
    bool                     reconnect;                          // Reconnect if connection fails
    int                      verbose;                            // Verbosity level
    void                     *userResource;                      // A pointer to user resource
} RKClientDesc;

typedef struct rk_client RKClient;

struct rk_client {
    // User set parameters
    //    struct rk_client_desc;                                 // Need -fms-extension to compile this properly
    RKName                   name;                               // A program name
    RKName                   hostname;                           // Hostname
    int                      port;                               // Port number of the server
    int                      timeoutSeconds;                     // Timeout in seconds
    RKNetworkSocketType      type;                               // Socket type
    RKNetworkMessageFormat   format;                             // Payload format
    int                      blockLength;                        // Payload length
    bool                     blocking;                           // Blocking read
    bool                     reconnect;                          // Reconnect if connection fails
    int                      verbose;                            // Verbosity level
    void                     *userResource;                      // A pointer to user resource
    void                     *userPayload;                       // A pointer to user payload

    int                      (*init)(RKClient *);                // Connection initialization handler
    int                      (*recv)(RKClient *);                // Receive handler
    int                      (*exit)(RKClient *);                // Connection exit handler

    // Program set parameters
    char                     hostIP[32];                         // Host IP in numbers
    struct sockaddr_in       sa;                                 // Socket address
    int                      sd;                                 // Socket descriptor

    int                      ireq;                               // A global instance request
    RKClientState            state;                              // A global flag for infinite loop
    bool                     safeToClose;                        // A flag indicating safe to close

    pthread_t                threadId;                           // Own thread ID
    pthread_attr_t           threadAttributes;                   // Thread attributes
    pthread_mutex_t          lock;                               // Thread safety mutex of the server

    fd_set                   rfd;                                // Read ready
    fd_set                   wfd;                                // Write ready
    fd_set                   efd;                                // Error occurred

    RKNetDelimiter           netDelimiter;                       // A storage for latest delimiter
};

RKClient *RKClientInitWithDesc(RKClientDesc);
RKClient *RKClientInitWithHostnamePort(const char *, const int);
RKClient *RKClientInit(void);
void RKClientFree(RKClient *);

void RKClientSetUserResource(RKClient *, void *);
void RKClientSetGreetHandler(RKClient *, int (*)(RKClient *));
void RKClientSetReceiveHandler(RKClient *, int (*)(RKClient *));
void RKClientSetCloseHandler(RKClient *, int (*)(RKClient *));

void RKClientStart(RKClient *, const bool waitForConnection);
void RKClientStop(RKClient *);

//#ifdef __cplusplus
//}
//#endif

#endif /* defined(___RadarKit_RKClient__) */
