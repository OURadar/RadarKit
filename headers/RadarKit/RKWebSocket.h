//
//  RKWebSocket.h
//  RadarKit
//
//  WebSocket backend for calling home server
//
//  Created by Boonleng Cheong on 2/9/2022.
//  Copyright (c) 2022 Boonleng Cheong. All rights reserved.
//

#ifndef __RadarKit_WebSocket__
#define __RadarKit_WebSocket__

#include <RadarKit/RKFoundation.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/crypto.h>

#include "_rfc_6455.h"

#define RKWebSocketFrameSize                     (1024 * 1024)
#define RKWebSocketPayloadDepth                  1000
#define RKWebSocketTimeoutDeltaMicroseconds      10000
#define RKWebSocketTimeoutThresholdSeconds       20.0

#ifndef htonll
#define htonll(x) (((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
#define ntohll(x) (((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))
#endif

//typedef uint8_t RKWebSocketSSLFlag;
//enum {
//    RKWebSocketFlagSSLAuto,
//    RKWebSocketFlagSSLOff,
//    RKWebSocketFlagSSLOn
//};

typedef struct rk_websocket_payload {
    void    *source;
    size_t  size;
} RKWebSocketPayload;

typedef struct rk_websocket RKWebSocket;

struct rk_websocket {
    RKName                   name;
    char                     host[80];
    char                     path[80];
    int                      port;
    bool                     useSSL;
    int                      verbose;
    
    void                     *parent;

    void                     (*onOpen)(RKWebSocket *);
    void                     (*onClose)(RKWebSocket *);
    void                     (*onError)(RKWebSocket *);
    void                     (*onMessage)(RKWebSocket *, void*, size_t);

    char                     ip[INET6_ADDRSTRLEN];                             // IP in string
    struct sockaddr_in       sa;                                               // Socket address
    int                      sd;                                               // Socket descriptor
    SSL_CTX                  *sslContext;                                      // SSL context
    SSL                      *ssl;                                             // SSL
    char                     secret[26];                                       //
    char                     digest[64];                                       // Handshake Sec-WebSocket-Accept
    char                     upgrade[64];                                      // Handshake Upgrade
    char                     connection[64];                                   // Handshake Connection
    bool                     wantActive;
    bool                     connected;

    pthread_t                threadId;                                         // Own thread ID
    pthread_attr_t           threadAttributes;                                 // Thread attributes
    pthread_mutex_t          lock;                                             // Thread safety mutex of the server

    RKWebSocketPayload       payloads[RKWebSocketPayloadDepth];                // References of payloads
    uint16_t                 payloadHead;                                      // The one ahead
    uint16_t                 payloadTail;                                      // The one following

    useconds_t               timeoutDeltaMicroseconds;                         // Timeout of select()
    uint32_t                 timeoutThreshold;                                 // Internal variable
    uint32_t                 timeoutCount;                                     // Internal variable

    uint8_t                  frame[RKWebSocketFrameSize];                      // A local buffer to store a frame
};

RKWebSocket *RKWebSocketInit(const char *, const char *);
void RKWebSocketFree(RKWebSocket *);

void RKWebSocketSetPath(RKWebSocket *, const char *);
void RKWebSocketSetParent(RKWebSocket *, const void *);
void RKWebSocketSetVerbose(RKWebSocket *, const int);
void RKWebSocketSetPingInterval(RKWebSocket *, const float);
void RKWebSocketSetOpenHandler(RKWebSocket *, void (*)(RKWebSocket *));
void RKWebSocketSetCloseHandler(RKWebSocket *, void (*)(RKWebSocket *));
void RKWebSocketSetMessageHandler(RKWebSocket *, void (*)(RKWebSocket *, void *, size_t));
void RKWebSocketSetErrorHandler(RKWebSocket *, void (*)(RKWebSocket *));

// This is technically RKWebSocketStartAsClient(). Sorry, but no plans to make RKWebSocketStartAsServer()
void RKWebSocketStart(RKWebSocket *);

// Stop the socket
void RKWebSocketStop(RKWebSocket *);

// Wait until all payloads are sent
// NOTE: Do no use this within handler functions (OpenHandler, MessageHandler, etc.)
void RKWebSocketWait(RKWebSocket *);

// Send a packet
int RKWebSocketSend(RKWebSocket *, void *, const size_t);

#endif
