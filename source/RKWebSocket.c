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
    e = strstr(b, "\r\n");
    size_t l = (size_t)(e - b);
    memcpy(argument, b, l);
    argument[l] = '\0';
    return argument;
}

static int RKSocketRead(RKWebSocket *R, uint32_t origin, size_t size) {
    if (R->useSSL) {
        return SSL_read(R->ssl, R->frame + origin, (int)size);
    }
    return (int)read(R->sd,R->frame + origin, size);
}

static int RKSocketWrite(RKWebSocket *R, size_t size) {
    if (R->useSSL) {
        return SSL_write(R->ssl, R->frame, (int)size);
    }
    return (int)write(R->sd, R->frame, size);
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

static int RKWebSocketPingPong(RKWebSocket *R, const bool ping, const char *message, const int len) {
    size_t size = RKWebSocketFrameEncode(R->frame,
        ping ? RFC6455_OPCODE_PING : RFC6455_OPCODE_PONG,
        message, message == NULL ? 0 : (len == 0 ? strlen(message) : len));
    int r = RKSocketWrite(R, size);
    if (r < 0) {
        fprintf(stderr, "Error. Unable to write. r = %d\n", r);
    } else if (R->verbose > 2) {
        printf("Frame of size %zu / %d sent.\n", size, r);
    }
    return r;
}

static int RKWebSocketPing(RKWebSocket *R, const char *message, const int len) {
    return RKWebSocketPingPong(R, true, message, len);
}

static int RKWebSocketPong(RKWebSocket *R, const char *message, const int len) {
    return RKWebSocketPingPong(R, false, message, len);
}

static void RKShowWebsocketFrameHeader(RKWebSocket *R) {
    uint8_t *c = (uint8_t *)R->frame;
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

static int RKWebSocketConnect(RKWebSocket *R) {
    int r;
    char *c;
    struct hostent *entry = gethostbyname(R->host);
    c = inet_ntoa(*((struct in_addr *)entry->h_addr_list[0]));
    if (c) {
        strcpy(R->ip, c);
    } else {
        fprintf(stderr, "Error getting IP address.\n");
        return -1;
    }

    if ((R->sd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        fprintf(stderr, "Error opening socket\n");
        return -1;
    }

    if (R->verbose) {
        printf("\nConnecting %s:%d %s...\n", R->ip, R->port,
            R->useSSL ? "(\033[38;5;220mssl\033[m) " : "");
    }

    R->sa.sin_family = AF_INET;
    R->sa.sin_port = htons(R->port);
    if (inet_pton(AF_INET, R->ip, &R->sa.sin_addr) <= 0) {
        fprintf(stderr, "Invalid address / address not supported \n");
        return -1;
    }
    if ((r = connect(R->sd, (struct sockaddr *)&R->sa, sizeof(struct sockaddr_in))) < 0) {
        fprintf(stderr, "Connection failed.  r = %d\n", r);
        return -1;
    }
    if (R->useSSL) {
        SSL_set_fd(R->ssl, R->sd);
        SSL_connect(R->ssl);
    }

    char *buf = (char *)R->frame;
    strcpy(R->secret, "RadarHub123456789abcde");
    FILE *fid = fopen("radarkit-radarhub-secret", "r");
    if (fid) {
        r = fscanf(fid, "%s", buf);
        if (r == 1 && strlen(buf) == 22) {
            if (R->verbose > 1) {
                printf("secret = '%s' (%zu)\n", buf, strlen(buf));
            }
            strcpy(R->secret, buf);
        }
        fclose(fid);
    } else if (R->verbose) {
        printf("Using default secret %s ...\n", R->secret);
    }
    sprintf(buf,
        "GET %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Key: %s==\r\n"
        "Sec-WebSocket-Version: 13\r\n"
        "\r\n",
        R->path,
        R->host,
        R->secret);
    if (R->verbose > 1) {
        printf("%s", buf);
    }

    RKSocketWrite(R, strlen(buf));
    do {
        r = RKSocketRead(R, r, RKWebSocketFrameSize - r);
    } while (r == 0 || strstr((char *)buf, "\r\n\r\n") == NULL);
    if (r < 0) {
        fprintf(stderr, "Error during handshake.\n");
        fprintf(stderr, "%s", (char *)buf);
        return -1;
    }
    buf[r] = '\0';
    if (R->verbose > 1) {
        printf("%s", buf);
    }

    // Verify the return digest, websocket upgrade, connection upgrade, etc.
    strcpy(R->digest, RKGetHandshakeArgument(buf, "Sec-WebSocket-Accept"));
    strcpy(R->upgrade, RKGetHandshakeArgument(buf, "Upgrade"));
    strcpy(R->connection, RKGetHandshakeArgument(buf, "Connection"));

    // This block is now hardcoded for default secret key, should replace ...
    if (strcmp(R->secret, "RadarHub123456789abcde") == 0 &&
        strcmp(R->digest, "O9QKgAZPEwFaLSqyFPYMHcGBp5g=")) {
        fprintf(stderr, "Error. R->digest = %s\n", R->digest);
        fprintf(stderr, "Error. Unexpected digest.\n");
        return -1;
    }

    if (strcasecmp(R->upgrade, "WebSocket")) {
        fprintf(stderr, "Error. R->upgrade = %s\n", R->upgrade);
        fprintf(stderr, "Error. Connection is not websocket.\n");
        return -1;
    }

    if (strcasecmp(R->connection, "upgrade")) {
        fprintf(stderr, "Error. R->connection = %s\n", R->connection);
        fprintf(stderr, "Error. Connection did not get upgraded.\n");
        return -1;
    }

    // Discard all pending deliveries
    R->payloadTail = R->payloadHead;

    // Call onOpen here for client to handle additional tasks after the connection is established.
    if (R->onOpen) {
        R->onOpen(R);
    }

    R->connected = true;

    return 0;
}

#pragma mark - Internal run loops

void *transporter(void *in) {
    RKWebSocket *R = (RKWebSocket *)in;

    int i, r;
    void *anchor = NULL;
    size_t size, targetFrameSize = 0;
    ws_frame_header *h = (ws_frame_header *)R->frame;
    char words[][5] = {"love", "hope", "cool", "cute", "sexy", "nice", "calm", "wish"};
    char uword[5] = "xxxx";
    char message[1024];

    fd_set rfd;
    fd_set wfd;
    fd_set efd;
    struct timeval timeout;
    time_t s1, s0;

    uint32_t origin = 0;
    uint32_t total = 0;

    while (R->wantActive) {

        RKWebSocketConnect(R);

        // Run loop for read and write
        while (R->wantActive && R->connected) {
            //
            //  Write
            //
            //  should always just pass right through
            //
            FD_ZERO(&wfd);
            FD_ZERO(&efd);
            FD_SET(R->sd, &wfd);
            FD_SET(R->sd, &efd);
            timeout.tv_sec = 0;
            timeout.tv_usec = R->timeoutDeltaMicroseconds;
            r = select(R->sd + 1, NULL, &wfd, &efd, &timeout);
            if (r > 0) {
                if (FD_ISSET(R->sd, &wfd)) {
                    // Ready to write. Keep sending the payloads until the tail catches up
                    while (R->payloadTail != R->payloadHead) {
                        uint16_t tail = R->payloadTail == RKWebSocketPayloadDepth - 1 ? 0 : R->payloadTail + 1;
                        const RKWebSocketPayload *payload = &R->payloads[tail];
                        if (R->verbose > 2) {
                            if (payload->size < 64) {
                                RKBinaryString(message, payload->source, payload->size);
                            } else {
                                RKHeadTailBinaryString(message, payload->source, payload->size);
                            }
                            printf("WRITE \033[38;5;154m%s\033[m (%zu)\n", message, payload->size);
                        }
                        size = RKWebSocketFrameEncode(R->frame, RFC6455_OPCODE_BINARY, payload->source, payload->size);
                        r = RKSocketWrite(R, size);
                        if (r < 0) {
                            if (R->verbose) {
                                fprintf(stderr, "Error. RKSocketWrite() = %d\n", r);
                            }
                            R->connected = false;
                            break;
                        } else if (r == 0) {
                            R->connected = false;
                            break;
                        }
                        R->payloadTail = tail;
                    }
                } else if (FD_ISSET(R->sd, &efd)) {
                    // Exceptions
                    if (R->verbose) {
                        fprintf(stderr, "Error. Exceptions during write cycle.\n");
                    }
                    break;
                } else {
                    // This shall not reach
                    printf("... w\n");
                }
            } else if (r < 0) {
                // Errors
                if (R->verbose) {
                    fprintf(stderr, "Error. select() = %d during write cycle.\n", r);
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
            FD_SET(R->sd, &rfd);
            FD_SET(R->sd, &efd);
            timeout.tv_sec = 0;
            timeout.tv_usec = R->timeoutDeltaMicroseconds;
            r = select(R->sd + 1, &rfd, NULL, &efd, &timeout);
            if (r > 0) {
                if (FD_ISSET(R->sd, &rfd)) {
                    // There is something to read
                    r = RKSocketRead(R, origin, RKWebSocketFrameSize);
                    if (r <= 0) {
                        if (R->verbose) {
                            fprintf(stderr, "Error. RKSocketRead() = %d   origin = %u\n", r, origin);
                        }
                        R->connected = false;
                        break;
                    }
                    if (origin == 0) {
                        targetFrameSize = RKWebSocketFrameGetTargetSize(R->frame);
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
                    size = RKWebSocketFrameDecode((void **)&anchor, R->frame);
                    if (!h->fin) {
                        fprintf(stderr, "I need upgrade!\n"
                                        "I need upgrade!\n"
                                        "I need upgrade!\n"
                                        "I cannot handle frames with h->fin = 0.\n");
                    }
                    if (R->verbose > 1) {
                        if (R->verbose > 2) {
                            printf("%2u read  ", total); RKShowWebsocketFrameHeader(R);
                        }
                        printf("S-%s: \033[38;5;220m%s%s\033[m (%zu)\n",
                            OPCODE_STRING(h->opcode), (char *)anchor, size > 64 ? " ..." : "", size);
                    }
                    R->timeoutCount = 0;
                } else if (FD_ISSET(R->sd, &efd)) {
                    // Exceptions
                    if (R->verbose) {
                        fprintf(stderr, "Error. Exceptions during read cycle.\n");
                    }
                    R->connected = false;
                    break;
                } else {
                    // This shall not reach
                    printf("... r\n");
                }
            } else if (r < 0) {
                // Errors
                if (R->verbose) {
                    fprintf(stderr, "Error. select() = %d during read cycle.\n", r);
                }
                R->connected = false;
                break;
            } else {
                // Timeout
                if (R->timeoutCount++ >= R->timeoutThreshold) {
                    R->timeoutCount = 0;
                    char *word = words[rand() % 8];
                    r = RKWebSocketPing(R, word, (int)strlen(word));
                    if (R->verbose > 1) {
                        ws_mask_key key = {.u32 = *((uint32_t *)&R->frame[2])};
                        for (i = 0; i < 4; i++) {
                            uword[i] = R->frame[6 + i] ^ key.code[i % 4];
                        }
                        printf("C-PING: \033[38;5;82m%s\033[m\n", uword);
                        if (R->verbose > 2) {
                            printf("%2d sent  ", r); RKShowWebsocketFrameHeader(R);
                        }
                    }
                }
            }
            // Interpret the payload if something was read
            if (size > 0) {
                if (h->opcode == RFC6455_OPCODE_PING) {
                    // Make a copy since payload --> R->buf or the behavior is unpredictable
                    memcpy(message, anchor, h->len); message[h->len] = '\0';
                    RKWebSocketPong(R, message, h->len);
                    if (R->verbose > 1) {
                        ws_mask_key key = {.u32 = *((uint32_t *)&R->frame[2])};
                        for (i = 0; i < 4; i++) {
                            uword[i] = R->frame[6 + i] ^ key.code[i % 4];
                        }
                        printf("C-PONG: \033[38;5;82m%s\033[m\n", uword);
                        if (R->verbose > 2) {
                            printf("%2d sent  ", r); RKShowWebsocketFrameHeader(R);
                        }
                    }
                } else if (h->opcode == RFC6455_OPCODE_PONG) {
                    R->timeoutCount = 0;
                } else if (h->opcode == RFC6455_OPCODE_CLOSE) {
                    R->connected = false;
                    break;
                } else if (h->opcode == RFC6455_OPCODE_TEXT || h->opcode == RFC6455_OPCODE_BINARY) {
                    if (R->onMessage) {
                        R->onMessage(R, anchor, size);
                    }
                }
                R->timeoutCount = 0;
            }
        } // while (R->wantActive && R->connected) ...
        if (R->sd) {
            if (R->verbose) {
                printf("Closing socket sd = %d ...\n", R->sd);
            }
            if (R->onClose) {
                R->onClose(R);
            }
            close(R->sd);
            R->sd = 0;
        }
        s1 = time(NULL);
        i = 0;
        do {
            s0 = time(NULL);
            r = (int)difftime(s0, s1);
            if (i != r && R->verbose) {
                i = r;
                if (r > 2) {
                    printf("\rNo connection. Retry in %d second%s ... ",
                        10 - r, 10 - r > 1 ? "s" : "");
                } else {
                    printf("\rNo connection.");
                }
                fflush(stdout);
            }
            usleep(200000);
        } while (R->wantActive && r < 10);
        if (R->verbose) {
            printf("\033[1K\r");
        }
    }

    if (R->verbose) {
        printf("R->wantActive = %s\n", R->wantActive ? "true" : "false");
    }

    return NULL;
}

#pragma mark - Life Cycle

RKWebSocket *RKWebSocketInit(const char *host, const char *path, const RKWebSocketSSLFlag flag) {
    char *c;
    size_t len;

    // printf("RKWebSocketInit()\n");

    RKWebSocket *R = (RKWebSocket *)malloc(sizeof(RKWebSocket));
    memset(R, 0, sizeof(RKWebSocket));
    pthread_attr_init(&R->threadAttributes);
    pthread_mutex_init(&R->lock, NULL);

    c = strstr(host, ":");
    if (c == NULL) {
        R->port = 80;
        strcpy(R->host, host);
    } else {
        R->port = atoi(c + 1);
        len = (size_t)(c - host);
        strncpy(R->host, host, len);
        R->host[len] = '\0';
    }
    if (strlen(path) == 0) {
        sprintf(R->path, "/");
    } else {
        strcpy(R->path, path);
    }
    if (flag == RKWebSocketFlagSSLAuto) {
        R->useSSL = R->port == 443;
    } else {
        R->useSSL = flag == RKWebSocketFlagSSLOn;
    }
    if (R->useSSL) {
        #if OPENSSL_VERSION_NUMBER < 0x10100000L
        R->sslContext = SSL_CTX_new(SSLv23_client_method());
        #else
        R->sslContext = SSL_CTX_new(TLS_client_method());
        #endif
        R->ssl = SSL_new(R->sslContext);
    }
    R->timeoutDeltaMicroseconds = RKWebSocketTimeoutDeltaMicroseconds;
    RKWebSocketSetPingInterval(R, RKWebSocketTimeoutThresholdSeconds);

    return R;
}

void RKWebSocketFree(RKWebSocket *R) {
    pthread_attr_destroy(&R->threadAttributes);
    pthread_mutex_destroy(&R->lock);
    free(R);
}

#pragma mark - Properties

void RKWebSocketSetVerbose(RKWebSocket *R, const int verbose) {
    R->verbose = verbose;
}

void RKWebSocketSetPath(RKWebSocket *R, const char *path) {
    strcpy(R->path, path);
}

void RKWebSocketSetPingInterval(RKWebSocket *R, const float period) {
    R->timeoutThreshold = (useconds_t)(period * 1.0e6f) / R->timeoutDeltaMicroseconds;
}

void RKWebSocketSetOpenHandler(RKWebSocket *R, void (*routine)(RKWebSocket *)) {
    R->onOpen = routine;
}

void RKWebSocketSetCloseHandler(RKWebSocket *R, void (*routine)(RKWebSocket *)) {
    R->onClose = routine;
}

void RKWebSocketSetMessageHandler(RKWebSocket *R, void (*routine)(RKWebSocket *, void *, size_t)) {
    R->onMessage = routine;
}

void RKWebSocketSetErrorHandler(RKWebSocket *R, void (*routine)(RKWebSocket *)) {
    R->onError = routine;
}

#pragma mark - Methods

void RKWebSocketStart(RKWebSocket *R) {
    pthread_mutex_lock(&R->lock);
    R->wantActive = true;
    pthread_mutex_unlock(&R->lock);
    if (pthread_create(&R->threadId, &R->threadAttributes, transporter, R)) {
        fprintf(stderr, "Unable to launch a run loop\n");
    }
    return;
}

void RKWebSocketStop(RKWebSocket *R) {
    pthread_mutex_lock(&R->lock);
    R->wantActive = false;
    pthread_mutex_unlock(&R->lock);
    pthread_join(R->threadId, NULL);
}

void RKWebSocketWait(RKWebSocket *R) {
    int k = 0;
    while (R->payloadTail != R->payloadHead && k++ < 10) {
        usleep(50000);
    }
}

// RKWebSocketSend() does not make a copy of the source.
// The input source must be allocated using malloc() or similar variants.
int RKWebSocketSend(RKWebSocket *R, void *source, size_t size) {
    pthread_mutex_lock(&R->lock);
    uint16_t k = R->payloadHead == RKWebSocketPayloadDepth - 1 ? 0 : R->payloadHead + 1;
    R->payloads[k].source = source;
    R->payloads[k].size = size;
    R->payloadHead = k;
    pthread_mutex_unlock(&R->lock);
    return 0;
}
