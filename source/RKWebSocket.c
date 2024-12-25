//
//  RKWebSocket.c
//  RadarKit
//
//  Created by Boonleng Cheong on 8/3/2021.
//  Copyright (c) 2021 Boonleng Cheong. All rights reserved.
//

#include <RadarKit/RKWebSocket.h>

#pragma mark - Static Methods

static char *RKGetHandshakeArgument(const char *buf, const char *key) {
    static char argument[80] = {0};
    char *b, *e;
    b = strstr(buf, key);
    if (b == NULL) {
        argument[0] = '\0';
        return argument;
    }
    b += strlen(key);
    while (*b == ':' || *b == ' ') {
        b++;
    }
    e = strstr(b, RKEOL);
    size_t l = (size_t)(e - b);
    memcpy(argument, b, l);
    argument[l] = '\0';
    return argument;
}

static int RKSocketRead(RKWebSocket *W, uint32_t origin, size_t size) {
    W->tic++;
    if (W->useSSL) {
        return SSL_read(W->ssl, W->frame + origin, (int)size);
    }
    return (int)read(W->sd,W->frame + origin, size);
}

static int RKSocketWrite(RKWebSocket *W, size_t size) {
    W->tic++;
    if (W->useSSL) {
        return SSL_write(W->ssl, W->frame, (int)size);
    }
    return (int)write(W->sd, W->frame, size);
}

static size_t RKWebSocketFrameEncode(void *buf, RFC6455_OPCODE code, const void *src, size_t size) {
    size_t r;
    ws_frame_header *h = buf;
    memset(h, 0, sizeof(ws_frame_header));
    h->fin = 1;
    h->mask = true;
    h->opcode = code;
    char *payload = buf + sizeof(ws_frame_header);
    if (size == 0) {
        if (src == NULL) {
            r = 0;
        } else {
            r = strlen((char *)src);
        }
    } else {
        r = size;
    }
    if (r > 65535) {
        h->len = 127;
        // Frame header can be up to 10 bytes
        if (r > RKWebSocketFrameSize - 10) {
            r = RKWebSocketFrameSize - 10;
            fprintf(stderr, "I am limited to %d bytes\n", RKWebSocketFrameSize - 10);
        }
        *((uint64_t *)payload) = htonll((uint64_t)r);
        payload += 8;
    } else if (r > 125) {
        h->len = 126;
        *((uint16_t *)payload) = htons((uint16_t)r);
        payload += 2;
    } else {
        h->len = r;
    }
    if (src) {
        if (h->mask) {
            ws_mask_key key = {.u32 = rand()};
            *((uint32_t *)payload) = key.u32;
            payload += 4;
            memcpy(payload, src, r);
            for (int i = 0; i < r; i++) {
                payload[i] ^= key.code[i % 4];
            }
        } else {
            // Should not happen in this module
            memcpy(payload, src, r);
        }
        payload[r] = '\0';
    }
    r = (size_t)((void *)payload - buf) + r;
    return r;
}

static size_t RKWebSocketFrameDecode(void **dst, void *buf) {
    size_t r;
    ws_frame_header *h = (ws_frame_header *)buf;
    char *payload = buf + sizeof(ws_frame_header);
    if (h->len == 127) {
        r = ntohll(*(uint64_t *)payload);
        payload += 8;
    } else if (h->len == 126) {
        r = ntohs(*(uint16_t *)payload);
        payload += 2;
    } else {
        r = h->len;
    }
    payload[r + 4 * h->mask] = '\0';
    if (h->mask) {
        ws_mask_key key = {.u32 = *(uint32_t *)payload};
        for (int i = 0; i < h->len; i++) {
            payload[i] ^= key.code[i % 4];
        }
    }
    *dst = payload;
    return r;
}

static size_t RKWebSocketFrameGetTargetSize(void *buf) {
    size_t r = sizeof(ws_frame_header);
    void *xlen = buf + sizeof(ws_frame_header);
    ws_frame_header *h = (ws_frame_header *)buf;
    if (h->len == 127) {
        r += 8 + ntohll(*(uint64_t *)xlen);
    } else if (h->len == 126) {
        r += 2 + ntohs(*(uint16_t *)xlen);
    } else {
        r += h->len;
    }
    return r;
}

static int RKWebSocketPingPong(RKWebSocket *W, const bool ping, const char *message, const int len) {
    size_t size = RKWebSocketFrameEncode(W->frame,
        ping ? RFC6455_OPCODE_PING : RFC6455_OPCODE_PONG,
        message, message == NULL ? 0 : (len == 0 ? strlen(message) : len));
    int r = RKSocketWrite(W, size);
    if (r < 0) {
        fprintf(stderr, "Error. Unable to write. r = %d\n", r);
    } else if (W->verbose > 2) {
        printf("Frame of size %zu / %d sent.\n", size, r);
    }
    return r;
}

static int RKWebSocketPing(RKWebSocket *W, const char *message, const int len) {
    return RKWebSocketPingPong(W, true, message, len);
}

static int RKWebSocketPong(RKWebSocket *W, const char *message, const int len) {
    return RKWebSocketPingPong(W, false, message, len);
}

static void RKShowWebsocketFrameHeader(RKWebSocket *W) {
    uint8_t *c = (uint8_t *)W->frame;
    ws_frame_header *h = (ws_frame_header *)c;
    if (h->mask) {
        printf("%02x %02x   %02x %02x %02x %02x    %02x %02x %02x %02x %02x %02x %02x %02x ..."
            "    fin=%d  opcode=%x  mask=%d  len=%d\n",
            c[0], c[1],
            c[2], c[3], c[4], c[5],
            c[6], c[7], c[8], c[9], c[10], c[11], c[12], c[13],
            h->fin, h->opcode, h->mask, h->len);
    } else {
        printf("%02x %02x                  %02x %02x %02x %02x %02x %02x %02x %02x ..."
            "    fin=%d  opcode=%x  mask=%d  len=%d\n",
            c[0], c[1],
            c[2], c[3], c[4], c[5], c[6], c[7], c[8], c[9],
            h->fin, h->opcode, h->mask, h->len);
    }
}

static int RKWebSocketConnect(RKWebSocket *W) {
    int k, r;
    char *c;
    struct hostent *entry = gethostbyname(W->host);
    if (entry == NULL) {
        RKLog("%s Error. Failed to resolve '%s'\n", W->name, W->host);
        return -1;
    }
    c = inet_ntoa(*((struct in_addr *)entry->h_addr_list[0]));
    if (c) {
        strcpy(W->ip, c);
    } else {
        RKLog("%s Error. Failed getting IP address.\n", W->name);
        return -1;
    }

    if ((W->sd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        RKLog("%s Error. Failed opening socket\n", W->name);
        return -1;
    }

    if (W->verbose) {
        RKLog("%s Connecting %s:%d %s...\n", W->name,
              W->ip, W->port,
              W->useSSL
               ? (rkGlobalParameters.showColor ? "(\033[38;5;220mssl\033[m) " : "(ssl) ")
               : "");
    }

    W->sa.sin_family = AF_INET;
    W->sa.sin_port = htons(W->port);
    if (inet_pton(AF_INET, W->ip, &W->sa.sin_addr) <= 0) {
        RKLog("%s Error. Invalid address / address not supported\n", W->name);
        return -1;
    }
    if ((r = connect(W->sd, (struct sockaddr *)&W->sa, sizeof(struct sockaddr_in))) < 0) {
        RKLog("%s Error. Connection failed.  r = %d\n", W->name, r);
        return -1;
    }
    if (W->useSSL) {
        SSL_set_fd(W->ssl, W->sd);
        SSL_connect(W->ssl);
    }

    char *buf = (char *)W->frame;
    strcpy(W->secret, "RadarHub123456789abcde");
    FILE *fid = fopen("radarkit-radarhub-secret", "r");
    if (fid) {
        r = fscanf(fid, "%s", buf);
        if (r == 1 && strlen(buf) == 22) {
            if (W->verbose > 1) {
                RKLog("%s Using secret %s (%zu) ...\n", W->name, buf, strlen(buf));
            }
            strcpy(W->secret, buf);
        }
        fclose(fid);
    } else if (W->verbose) {
        RKLog("%s Using default secret %s ...\n", W->name, W->secret);
    }
    sprintf(buf,
        "GET %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Key: %s==\r\n"
        "Sec-WebSocket-Version: 13\r\n"
        "\r\n",
        W->path,
        W->host,
        W->secret);
    if (W->verbose > 2) {
        printf("%s", buf);
    }
    r = RKSocketWrite(W, strlen(buf));
    if (r <= 0) {
        fprintf(stderr, "Error during handshake  (r = %d).\n", r);
        return -1;
    }
    buf[0] = '\0';
    k = 0;
    r = 0;
    do {
        r += RKSocketRead(W, r, RKWebSocketFrameSize - r);
        usleep(10000);
    } while (k++ < 300 && W->wantActive && (r == 0 || strstr((char *)buf, "\r\n\r\n") == NULL));
    if (r <= 0) {
        if (W->verbose) {
            RKLog("Info. Timed out during handshake  (r = %d).\n", r);
            if (r > 0) {
                RKLog("Info. r =\n%s", (char *)buf);
            }
        }
        return -1;
    }
    buf[r] = '\0';
    if (W->verbose > 1) {
        printf("%s", buf);
    }

    // Verify the return digest, websocket upgrade, connection upgrade, etc.
    strcpy(W->digest, RKGetHandshakeArgument(buf, "Sec-WebSocket-Accept"));
    strcpy(W->upgrade, RKGetHandshakeArgument(buf, "Upgrade"));
    strcpy(W->connection, RKGetHandshakeArgument(buf, "Connection"));

    // This block is now hardcoded for default secret key, should replace ...
    if (strcmp(W->secret, "RadarHub123456789abcde") == 0 &&
        strcmp(W->digest, "O9QKgAZPEwFaLSqyFPYMHcGBp5g=")) {
        fprintf(stderr, "Error. W->digest = %s\n", W->digest);
        fprintf(stderr, "Error. Unexpected digest.\n");
        return -1;
    }

    if (strcasecmp(W->upgrade, "WebSocket")) {
        fprintf(stderr, "Error. W->upgrade = %s\n", W->upgrade);
        fprintf(stderr, "Error. Connection is not websocket.\n");
        return -1;
    }

    if (strcasecmp(W->connection, "upgrade")) {
        fprintf(stderr, "Error. W->connection = %s\n", W->connection);
        fprintf(stderr, "Error. Connection did not get upgraded.\n");
        return -1;
    }

    // Discard all pending deliveries
    W->payloadTail = W->payloadHead;

    // Call onOpen here for client to handle additional tasks after the connection is established.
    if (W->onOpen) {
        W->onOpen(W);
    }

    W->connected = true;

    return 0;
}

#pragma mark - Internal run loops

void *transporter(void *in) {
    RKWebSocket *W = (RKWebSocket *)in;

    int i, r;
    void *anchor = NULL;
    size_t size, targetFrameSize = 0;
    ws_frame_header *h = (ws_frame_header *)W->frame;
    char words[][5] = {"love", "hope", "cool", "cute", "sexy", "nice", "calm", "wish"};
    char uword[5] = "xxxx";
    char message[256];
    char show[256];

    fd_set rfd;
    fd_set wfd;
    fd_set efd;
    struct timeval timeout;
    time_t s1, s0;

    uint32_t origin = 0;
    uint32_t total = 0;
    uint32_t *p32;

    W->tic = 1;

    while (W->wantActive) {

        RKWebSocketConnect(W);

        // Run loop for read and write
        while (W->wantActive && W->connected) {
            //
            //  Write
            //
            //  should always just pass right through
            //
            FD_ZERO(&wfd);
            FD_ZERO(&efd);
            FD_SET(W->sd, &wfd);
            FD_SET(W->sd, &efd);
            timeout.tv_sec = 0;
            timeout.tv_usec = W->timeoutDeltaMicroseconds;
            r = select(W->sd + 1, NULL, &wfd, &efd, &timeout);
            if (r > 0) {
                if (FD_ISSET(W->sd, &efd)) {
                    // Exceptions
                    if (W->verbose) {
                        fprintf(stderr, "Error. Exceptions during write cycle.\n");
                    }
                    break;
                } else if (FD_ISSET(W->sd, &wfd)) {
                    // Ready to write. Keep sending the payloads until the tail catches up
                    while (W->payloadTail != W->payloadHead) {
                        uint16_t tail = W->payloadTail == RKWebSocketPayloadDepth - 1 ? 0 : W->payloadTail + 1;
                        const RKWebSocketPayload *payload = &W->payloads[tail];
                        if (W->verbose > 2) {
                            if (payload->size < 64) {
                                RKBinaryString(message, payload->source, payload->size);
                            } else {
                                RKRadarHubPayloadString(message, payload->source, payload->size);
                            }
                            printf("RKWebSocket.transporter: WRITE \033[38;5;154m%s\033[m (%zu)\n",
                                   message, payload->size);
                        }
                        size = RKWebSocketFrameEncode(W->frame, RFC6455_OPCODE_BINARY, payload->source, payload->size);
                        r = RKSocketWrite(W, size);
                        if (r < 0) {
                            if (W->verbose) {
                                RKLog("%s Error. RKSocketWrite() = %d\n", W->name, r);
                            }
                            W->connected = false;
                            break;
                        } else if (r == 0) {
                            W->connected = false;
                            break;
                        }
                        W->payloadTail = tail;
                    }
                } else {
                    // This shall not reach
                    printf("... w\n");
                }
            } else if (r < 0) {
                // Errors
                if (W->verbose) {
                    RKLog("%s Error. select() = %d during write cycle\n", W->name, r);
                }
                break;
            }
            //
            //  Read
            //
            //  wait up to tv_usec if there is nothing left to send
            //
            size = 0;
            FD_ZERO(&rfd);
            FD_ZERO(&efd);
            FD_SET(W->sd, &rfd);
            FD_SET(W->sd, &efd);
            timeout.tv_sec = 0;
            timeout.tv_usec = W->timeoutDeltaMicroseconds;
            r = select(W->sd + 1, &rfd, NULL, &efd, &timeout);
            if (r > 0) {
                if (FD_ISSET(W->sd, &efd)) {
                    // Exceptions
                    if (W->verbose) {
                        fprintf(stderr, "Error. Exceptions during read cycle.\n");
                    }
                    W->connected = false;
                    break;
                } else if (FD_ISSET(W->sd, &rfd)) {
                    // There is something to read
                    r = RKSocketRead(W, origin, RKWebSocketFrameSize);
                    if (r <= 0) {
                        if (W->verbose) {
                            fprintf(stderr, "Error. RKSocketRead() = %d   origin = %u\n", r, origin);
                        }
                        W->connected = false;
                        break;
                    }
                    if (origin == 0) {
                        targetFrameSize = RKWebSocketFrameGetTargetSize(W->frame);
                        total = r;
                    } else {
                        total += r;
                    }
                    if (total < targetFrameSize) {
                        origin += r;
                        continue;
                    } else {
                        origin = 0;
                    }
                    size = RKWebSocketFrameDecode((void **)&anchor, W->frame);
                    if (!h->fin) {
                        fprintf(stderr, "I need upgrade!\n"
                                        "I need upgrade!\n"
                                        "I need upgrade!\n"
                                        "I cannot handle frames with h->fin = 0.\n");
                    }
                    if (W->verbose > 1) {
                        if (W->verbose > 2) {
                            printf("%2u read  ", total); RKShowWebsocketFrameHeader(W);
                        }
                        if (size != 4 && h->opcode == RFC6455_OPCODE_PING) {
                            RKBytesInHex(show, anchor, size);
                        } else if (size > 32) {
                            RKHeadTailBytesInHex(show, anchor, size);
                        } else {
                            memcpy(show, anchor, size);
                            show[size] = '\0';
                        }
                        RKLog("%s S-%s: %s%s%s (%zu)\n", W->name,
                              OPCODE_STRING(h->opcode),
                              rkGlobalParameters.showColor ? RKOrangeColor : "",
                              show,
                              rkGlobalParameters.showColor ? RKNoColor : "",
                              size);
                    }
                    W->timeoutCount = 0;
                } else {
                    // This shall not reach
                    printf("... r\n");
                }
            } else if (r < 0) {
                // Errors
                if (W->verbose) {
                    RKLog("%s Error. select() = %d during read cycle.\n", W->name, r);
                }
                W->connected = false;
                break;
            } else {
                // Timeout
                if (W->timeoutCount++ >= W->timeoutThreshold) {
                    W->timeoutCount = 0;
                    char *word = words[rand() % 8];
                    r = RKWebSocketPing(W, word, (int)strlen(word));
                    if (W->verbose > 1) {
                        p32 = (uint32_t *)&W->frame[2];
                        ws_mask_key key = {.u32 = *p32};
                        for (i = 0; i < 4; i++) {
                            uword[i] = W->frame[6 + i] ^ key.code[i % 4];
                        }
                        RKLog("%s C-PING: %s%s%s (%zu)\n", W->name,
                              rkGlobalParameters.showColor ? RKLimeColor : "",
                              uword,
                              rkGlobalParameters.showColor ? RKNoColor : "",
                              strlen(word));
                        if (W->verbose > 2) {
                            printf("%s %2d sent  ", W->name, r); RKShowWebsocketFrameHeader(W);
                        }
                    }
                }
            }
            // Interpret the payload if something was read
            if (size > 0) {
                if (h->opcode == RFC6455_OPCODE_PING) {
                    // Make a copy since payload --> W->buf or the behavior is unpredictable
                    memcpy(message, anchor, h->len); message[h->len] = '\0';
                    RKWebSocketPong(W, message, h->len);
                    if (W->verbose > 1) {
                        p32 = (uint32_t *)&W->frame[2];
                        ws_mask_key key = {.u32 = *p32};
                        for (i = 0; i < 4; i++) {
                            uword[i] = W->frame[6 + i] ^ key.code[i % 4];
                        }
                        if (h->len != 4) {
                            RKBytesInHex(show, message, h->len);
                        } else {
                            memcpy(show, message, h->len + 1);
                        }
                        RKLog("%s C-PONG: %s%s%s (%zu)\n", W->name,
                              rkGlobalParameters.showColor ? RKLimeColor : "",
                              show,
                              rkGlobalParameters.showColor ? RKNoColor : "",
                              h->len);
                        if (W->verbose > 2) {
                            printf("%2d sent  ", r); RKShowWebsocketFrameHeader(W);
                        }
                    }
                } else if (h->opcode == RFC6455_OPCODE_PONG) {
                    W->timeoutCount = 0;
                } else if (h->opcode == RFC6455_OPCODE_CLOSE) {
                    W->connected = false;
                    break;
                } else if (h->opcode == RFC6455_OPCODE_TEXT || h->opcode == RFC6455_OPCODE_BINARY) {
                    if (W->onMessage) {
                        W->onMessage(W, anchor, size);
                    }
                }
                W->timeoutCount = 0;
            }
        } // while (W->wantActive && W->connected) ...
        if (W->sd) {
            if (W->verbose > 1) {
                RKLog("%s Closing socket sd = %d ...\n", W->name, W->sd);
            }
            if (W->onClose) {
                W->onClose(W);
            }
            close(W->sd);
            W->sd = 0;
        }
        s1 = time(NULL);
        i = 0;
        do {
            s0 = time(NULL);
            r = (int)difftime(s0, s1);
            if (i != r && W->verbose > 1) {
                i = r;
                if (r > 2) {
                    printf("\r%s No connection. Retry in %d second%s ... ", W->name,
                        10 - r, 10 - r > 1 ? "s" : "");
                } else {
                    RKLog("%s No connection.\n", W->name);
                }
                fflush(stdout);
            }
            usleep(200000);
        } while (W->wantActive && r < 10);
        if (W->verbose > 1) {
            printf("\033[1K\r");
            fflush(stdout);
        }
    }

    if (W->verbose > 1) {
        RKLog("%s W->wantActive = %s\n", W->name, W->wantActive ? "true" : "false");
    }

    return NULL;
}

#pragma mark - Life Cycle

RKWebSocket *RKWebSocketInit(const char *host, const char *path) {
    char *c, *n;
    size_t len;

    RKWebSocket *W = (RKWebSocket *)malloc(sizeof(RKWebSocket));
    memset(W, 0, sizeof(RKWebSocket));
    sprintf(W->name, "%s<  RKWebSocket  >%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(RKEngineColorWebSocket) : "",
            rkGlobalParameters.showColor ? RKNoColor : "");
    pthread_attr_init(&W->threadAttributes);
    pthread_mutex_init(&W->lock, NULL);

    // Default if host == NULL
    if (host == NULL || strlen(host) == 0) {
        RKLog("%s Error. Input host cannot be %s\n", W->name, host);
        return NULL;
    }
    // Look for protocol https / http
    if (strstr(host, "https:") != NULL) {
        W->useSSL = true;
    } else {
        W->useSSL = false;
    }
    // Look for port number at the end
    if ((c = strstr(host, "://")) != NULL) {
        c += 3;
    } else {
        c = (char *)host;
    }
    n = strstr(c, ":");
    if (n == NULL) {
        W->port = 80;
        strcpy(W->host, host);
    } else {
        W->port = atoi(n + 1);
        len = (size_t)(n - c);
        if (len) {
            strncpy(W->host, c, len);
            W->host[len] = '\0';
        } else {
            sprintf(W->host, "localhost");
        }
        if (W->useSSL == false && W->port == 443) {
            W->useSSL = true;
        }
    }
    // Look for pathway
    if (path == NULL || strlen(path) == 0) {
        sprintf(W->path, "/");
    } else {
        strcpy(W->path, path);
    }
    // Establish the SSL context if necessary
    if (W->useSSL) {
        #if OPENSSL_VERSION_NUMBER < 0x10100000L
        W->sslContext = SSL_CTX_new(SSLv23_client_method());
        #else
        W->sslContext = SSL_CTX_new(TLS_client_method());
        #endif
        W->ssl = SSL_new(W->sslContext);
    }
    W->timeoutDeltaMicroseconds = RKWebSocketTimeoutDeltaMicroseconds;
    RKWebSocketSetPingInterval(W, RKWebSocketTimeoutThresholdSeconds);

    return W;
}

void RKWebSocketFree(RKWebSocket *W) {
    pthread_attr_destroy(&W->threadAttributes);
    pthread_mutex_destroy(&W->lock);
    free(W);
}

#pragma mark - Properties

void RKWebSocketSetPath(RKWebSocket *W, const char *path) {
    strcpy(W->path, path);
}

void RKWebSocketSetParent(RKWebSocket *W, const void *parent) {
    W->parent = (void *)parent;
}

void RKWebSocketSetVerbose(RKWebSocket *W, const int verbose) {
    W->verbose = verbose;
}

void RKWebSocketSetPingInterval(RKWebSocket *W, const float period) {
    W->timeoutThreshold = (useconds_t)(period * 1.0e6f) / W->timeoutDeltaMicroseconds;
}

void RKWebSocketSetOpenHandler(RKWebSocket *W, void (*routine)(RKWebSocket *)) {
    W->onOpen = routine;
}

void RKWebSocketSetCloseHandler(RKWebSocket *W, void (*routine)(RKWebSocket *)) {
    W->onClose = routine;
}

void RKWebSocketSetMessageHandler(RKWebSocket *W, void (*routine)(RKWebSocket *, void *, size_t)) {
    W->onMessage = routine;
}

void RKWebSocketSetErrorHandler(RKWebSocket *W, void (*routine)(RKWebSocket *)) {
    W->onError = routine;
}

#pragma mark - Methods

void RKWebSocketStart(RKWebSocket *W) {
    if (W->verbose) {
        RKLog("%s RKWebSocketStart() %s%s%s:%d%s%s\n", W->name,
              rkGlobalParameters.showColor ? (W->useSSL ? RKMonokaiGreen : RKMonokaiYellow) : "",
              W->useSSL ? "https://" : "http://",
              W->host, W->port, W->path,
              rkGlobalParameters.showColor ? RKNoColor : "");
    }
    pthread_mutex_lock(&W->lock);
    W->wantActive = true;
    pthread_mutex_unlock(&W->lock);
    if (pthread_create(&W->threadId, &W->threadAttributes, transporter, W)) {
        fprintf(stderr, "Unable to launch a run loop\n");
    }
    return;
}

void RKWebSocketStop(RKWebSocket *W) {
    pthread_mutex_lock(&W->lock);
    W->wantActive = false;
    pthread_mutex_unlock(&W->lock);
    pthread_join(W->threadId, NULL);
}

void RKWebSocketWait(RKWebSocket *W) {
    int k = 0;
    while (W->payloadTail != W->payloadHead && k++ < 10) {
        usleep(10000);
    }
}

// RKWebSocketSend() does not make a copy of the source.
// The input source must be allocated using malloc() or similar variants.
int RKWebSocketSend(RKWebSocket *W, void *source, const size_t size) {
    pthread_mutex_lock(&W->lock);
    uint16_t k = W->payloadHead == RKWebSocketPayloadDepth - 1 ? 0 : W->payloadHead + 1;
    W->payloads[k].source = source;
    W->payloads[k].size = size;
    W->payloadHead = k;
    pthread_mutex_unlock(&W->lock);
    return 0;
}
