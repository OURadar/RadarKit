//
//  RKClient.c
//  RadarKit
//
//  This collection is copied and modified from PortClient.
//
//  Created by Boonleng Cheong on 1/3/17.
//  Copyright © 2017-2021 Boonleng Cheong. All rights reserved.
//

#include <RadarKit/RKClient.h>

// Internal functions

void *theClient(void *in);

// Implementations

#pragma mark - Helper Functions

void *theClient(void *in) {
    RKClient *C = (RKClient *)in;

    int r;
    int k;
    int flags;
    int readCount, timeoutCount;
    struct timeval timeout;
    bool readOkay;

    C->state = RKClientStateCreating;

    FILE *fid = NULL;

    void *buf = (void *)malloc(RKMaximumPacketSize);
    if (buf == NULL) {
        RKLog("Error. Unable to allocate space for a buffer.\n");
        return (void *)RKResultErrorCreatingOperatorRoutine;
    }
    void *delimiter = &C->netDelimiter;
    char *cbuf = (char *)buf;

    C->userPayload = buf;

    if (C->verbose > 1) {
        RKLog("%s is working hard ...\n", C->name);
    }

    char ping[] = "ping" RKEOL;

    // Here comes the infinite loop until being stopped
    while (C->state < RKClientStateDisconnecting) {

        C->state = RKClientStateResolvingIP;

        // Resolve hostname to IP address
        if (C->verbose > 1) {
            RKLog("%s Resolving IP address ...\n", C->name);
        }
        struct hostent *h = gethostbyname2(C->hostname, AF_INET);
        if (h == NULL) {
            RKLog("%s Error. Unable to resolve '%s'\n", C->name, C->hostname);
            k = RKNetworkReconnectSeconds * 10;
            do {
                usleep(100000);
            } while (k-- > 0 && C->state < RKClientStateReconnecting);
            continue;
        }
        strcpy(C->hostIP, inet_ntoa(*((struct in_addr *)h->h_addr_list[0])));

        if (C->verbose > 1) {
            RKLog("%s Opening a %s socket ...\n", C->name,
                  C->type == RKNetworkSocketTypeTCP ? "TCP" :
                  (C->type == RKNetworkSocketTypeUDP ? "UDP" : "(NULL)"));
        }
        if (C->type == RKNetworkSocketTypeTCP) {
            if ((C->sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                RKLog("%s Error. Unable to create a TCP socket.\n", C->name);
                C->state = RKClientStateDisconnected;
                return NULL;
            }
        } else if (C->type == RKNetworkSocketTypeUDP) {
            if ((C->sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
                RKLog("%s Error. Unable to create a UDP socket.\n", C->name);
                C->state = RKClientStateDisconnected;
                return NULL;
            }
        } else {
            RKLog("%s Error. Unable to determine the socket type %d.\n", C->name, C->type);
            C->state = RKClientStateDisconnected;
            return NULL;
        }
        if (C->blocking == false) {
            flags = fcntl(C->sd, F_GETFL);
            if (fcntl(C->sd, F_SETFL, flags | O_NONBLOCK) == -1) {
                RKLog("%s Error. Unable to use non-blocking option.\n", C->name);
            }
        }
        r = 1;
        #if defined(__APPLE__)
        setsockopt(C->sd, SOL_SOCKET, SO_NOSIGPIPE, &r, sizeof(r));
        #endif
        if (C->type == RKNetworkSocketTypeUDP) {
            setsockopt(C->sd, SOL_SOCKET, SO_BROADCAST, &r, sizeof(r));
        }

        pthread_mutex_lock(&C->lock);
        if (C->state == RKClientStateResolvingIP) {
            C->state = RKClientStateConfiguringSocket;
        }
        pthread_mutex_unlock(&C->lock);

        // Configure the socket
        C->sa.sin_family = AF_INET;
        C->sa.sin_port = htons(C->port);
        C->sa.sin_addr.s_addr = inet_addr(C->hostIP);
        //memset(C->sa.sin_zero, 0, 8);
        //RKLog("%s <RKClient> %d.%d.%d.%d\n", C->name, (C->sa.sin_addr.s_addr >> 24) & 0xFF, (C->sa.sin_addr.s_addr >> 16) & 0xFF, (C->sa.sin_addr.s_addr >> 8) & 0xFF, C->sa.sin_addr.s_addr & 0xFF);

        if (C->verbose > 1) {
            RKLog("%s Configuring socket ...\n", C->name);
        }

        pthread_mutex_lock(&C->lock);
        if (C->state == RKClientStateConfiguringSocket) {
            C->state = RKClientStateConnecting;
        }
        pthread_mutex_unlock(&C->lock);

        // Connect through the IP address and port number
        if (C->verbose) {
            RKLog("%s Connecting %s:%d ...\n", C->name, C->hostIP, C->port);
        }
        if ((r = connect(C->sd, (struct sockaddr *)&C->sa, sizeof(struct sockaddr))) < 0) {
            // In progress is not a true failure
            if (errno != EINPROGRESS) {
                close(C->sd);
                k = RKNetworkReconnectSeconds * 10;
                do {
                    if (C->verbose > 1 && k % 10 == 0) {
                        RKLog("%s Connection failed (errno = %d). Retry in %d second%s ...\n", C->name, errno, k, k > 1 ? "s" : "");
                    }
                    usleep(100000);
                } while (k-- > 0 && C->state < RKClientStateDisconnecting);
                continue;
            }
        }

        // Server is almost ready to receive (could still be in EINPROGRESS, then select() -> 0)
        if (C->init) {
            fid = NULL;
            FD_ZERO(&C->wfd);
            FD_ZERO(&C->efd);
            FD_SET(C->sd, &C->wfd);
            FD_SET(C->sd, &C->efd);
            k = 0;
            do {
                timeout.tv_sec = 0;
                timeout.tv_usec = 100000;
                r = select(C->sd + 1, NULL, &C->wfd, &C->efd, &timeout);
            } while (r == 0 && k++ < 10 * C->timeoutSeconds && C->state < RKClientStateDisconnecting);
            if (r > 0) {
                if (FD_ISSET(C->sd, &C->efd)) {
                    if (errno != EINPROGRESS) {
                        RKLog("%s Error. r = %d   errno = %d (%s)\n", C->name, r, errno, RKErrnoString(errno));
                    }
                    close(C->sd);
                    continue;
                } else if (FD_ISSET(C->sd, &C->wfd)) {
                    // Ready to write to socket
                    if (C->init(C)) {
                        if (C->verbose) {
                            RKLog("%s Unable to connect %s:%d.\n", C->name, C->hostIP, C->port);
                        }
                        close(C->sd);
                        // Wait a while before trying to reconnect
                        k = 52;
                        do {
                            if (k % 10 == 0 && C->verbose > 1) {
                                RKLog("%s Reconnect in %d sec%s ...\n", C->name, k / 10, k / 10 > 1 ? "s" : "");
                            }
                            usleep(100000);
                        } while (k-- > 3 && C->state < RKClientStateDisconnecting);
                        continue;
                    }
                }
            } else {
                if (C->verbose > 1) {
                    RKLog("Timeout during initialization.\n");
                }
                close(C->sd);
                continue;
            }
        }

        // Actively receive
        timeoutCount = 0;
        while (C->state < RKClientStateReconnecting) {
            FD_ZERO(&C->rfd);
            FD_ZERO(&C->efd);
            FD_SET(C->sd, &C->rfd);
            FD_SET(C->sd, &C->efd);
            timeout.tv_sec = 0;
            timeout.tv_usec = 100000;
            readOkay = false;
            r = select(C->sd + 1, &C->rfd, NULL, &C->efd, &timeout);
            if (C->verbose > 3 || FD_ISSET(C->sd, &C->efd)) {
                RKLog("%s select() returned r = %d   FD_ISSET(rfd) = %d   FD_ISSET(efd) = %d   errno = %d.\n",
                      C->name, r, FD_ISSET(C->sd, &C->rfd), FD_ISSET(C->sd, &C->efd), errno);
            }
            if (r == 0) {
                // No errors just yet. The server could be sending out payload slowly
                if (timeoutCount++ / 10 > C->timeoutSeconds) {
                    // Send in a beacon signal to see if there's a response
                    if (C->ping) {
                        FD_ZERO(&C->wfd);
                        FD_ZERO(&C->efd);
                        FD_SET(C->sd, &C->wfd);
                        FD_SET(C->sd, &C->efd);
                        timeout.tv_sec = 0;
                        timeout.tv_usec = 1000;
                        r = select(C->sd + 1, NULL, &C->wfd, &C->efd, &timeout);
                        if (r > 0) {
                            if (FD_ISSET(C->sd, &C->efd)) {
                                // Exceptions
                                RKLog("%s encountered an exception error.\n", C->name);
                                break;
                            } else if (FD_ISSET(C->sd, &C->wfd)) {
                                if (C->verbose > 1) {
                                    RKLog("%s Sending ping\n", C->name);
                                }
                                pthread_mutex_lock(&C->lock);
                                RKNetworkSendPackets(C->sd, ping, strlen(ping), NULL);
                                pthread_mutex_unlock(&C->lock);
                            }
                        } else {
                            RKLog("%s Error. r=%d  errno=%d (%s)\n", C->name, r, errno, RKErrnoString(errno));
                            break;
                        }
                    } else {
                        // Socket established but nothing from the server.
                        RKLog("%s Timed out\n", C->name);
                    } // if (C->ping) ...
                    break;
                }
            } else if (r > 0 && FD_ISSET(C->sd, &C->rfd)) {
                //RKLog("%s Warning C->state = %d   format = %d\n", C->name, C->state, C->format);
                switch (C->format) {
                    case RKNetworkMessageFormatConstantSize:
                        k = 0;
                        readCount = 0;
                        while (readCount++ < C->timeoutSeconds * 100) {
                            if ((r = (int)read(C->sd, buf + k, C->blockLength - k)) > 0) {
                                k += r;
                                if (k >= C->blockLength) {
                                    break;
                                } else {
                                    usleep(10000);
                                }
                            } else if (errno != EAGAIN) {
                                if (C->verbose > 1) {
                                    RKLog("%s Error. RKMessageFormatFixedBlock   r=%d  k=%d  errno=%d (%s)  %d\n",
                                          C->name, r, k, errno, RKErrnoString(errno), readCount);
                                }
                                readCount = C->timeoutSeconds * 100;
                                pthread_mutex_lock(&C->lock);
                                if (C->state < RKClientStateDisconnecting) {
                                    C->state = RKClientStateReconnecting;
                                }
                                pthread_mutex_unlock(&C->lock);
                                break;
                            } else {
                                usleep(10000);
                            }
                            if (C->verbose > 1) {
                                RKLog("... errno = %d ...\n", errno);
                            }
                        }
                        if (readCount >= C->timeoutSeconds * 100) {
                            if (C->verbose > 1) {
                                RKLog("%s Not a proper frame.  timeoutCount = %d  errno = %d\n", C->name, readCount, errno);
                            }
                            break;
                        }
                        readOkay = true;
                        break;
                    case RKNetworkMessageFormatHeaderDefinedSize:
                        // The delimiter first
                        k = 0;
                        readCount = 0;
                        while (readCount++ < C->timeoutSeconds * 100) {
                            if ((r = (int)read(C->sd, delimiter + k, sizeof(RKNetDelimiter) - k)) > 0) {
                                if (r > 0) {
                                    k += r;
                                    if (k == sizeof(RKNetDelimiter)) {
                                        break;
                                    } else if (k > sizeof(RKNetDelimiter)) {
                                        RKLog("%s Error. Should not read larger than sizeof(RKNetDelimiter) = %zu\n", C->name, sizeof(RKNetDelimiter));
                                        break;
                                    }
                                } else {
                                    usleep(10000);
                                }
                            } else if (errno != EAGAIN) {
                                if (C->verbose > 1) {
                                    RKLog("%s Error. RKMessageFormatFixedHeaderVariableBlock:1  r = %d  k = %d  errno = %d (%s)\n",
                                          C->name, r, k, errno, RKErrnoString(errno));
                                }
                                readCount = C->timeoutSeconds * 10000;
                                pthread_mutex_lock(&C->lock);
                                if (C->state < RKClientStateDisconnecting) {
                                    C->state = RKClientStateReconnecting;
                                } else {
                                    pthread_mutex_unlock(&C->lock);
                                    break;
                                }
                                pthread_mutex_unlock(&C->lock);
                                break;
                            } else {
                                usleep(10000);
                            }
                        }
                        if (k != sizeof(RKNetDelimiter) || readCount > C->timeoutSeconds * 100) {
                            if (errno != ECONNREFUSED) {
                                RKLog("%s Error. Incomplete read().   errno = %d (%s)\n", C->name, errno, RKErrnoString(errno));
                            }
                            break;
                        }
                        // If the delimiter specifies 0 payload, it could just be a beacon
                        if (C->netDelimiter.size == 0) {
                            if (C->verbose > 1) {
                                RKLog("%s netDelimiter.size = 0\n", C->name);
                            }
                            readOkay = true;
                            break;
                        } else if (C->netDelimiter.size > RKMaximumPacketSize) {
                            RKLog("%s Error. Payload size = %s (type %d) is more than what I can handle.\n",
                                  C->name, RKIntegerToCommaStyleString(C->netDelimiter.size), C->netDelimiter.type);
                            readOkay = false;
                            break;
                        }
                        // Now the actual payload
                        k = 0;
                        readCount = 0;
                        while (readCount++ < C->timeoutSeconds * 100) {
                            if ((r = (int)read(C->sd, buf + k, C->netDelimiter.size - k)) > 0) {
                                #ifdef DEBUG_RKCLIENT
                                RKLog("%s read() -> %d / %d / %d    readCount = %d / %d\n",
                                      C->name, r, C->netDelimiter.size - k,  C->netDelimiter.size, readCount, C->timeoutSeconds * 100);
                                #endif
                                if (r > 0) {
                                    k += r;
                                    if (k == C->netDelimiter.size) {
                                        break;
                                    } else if (k > C->netDelimiter.size) {
                                        RKLog("%s Error. Should not read larger than specified size = %zu\n", C->name, C->netDelimiter.size);
                                        break;
                                    }
                                } else {
                                    usleep(10000);
                                }
                            } else if (errno != EAGAIN) {
                                RKLog("%s Error. PCMessageFormatFixedHeaderVariableBlock:2  size=%d   r=%d  k=%d  errno=%d (%s)\n",
                                      C->name, C->netDelimiter.size, r, k, errno, RKErrnoString(errno));
                                readCount = C->timeoutSeconds * 1000;
                                pthread_mutex_lock(&C->lock);
                                if (C->state < RKClientStateDisconnecting) {
                                    C->state = RKClientStateReconnecting;
                                }
                                pthread_mutex_unlock(&C->lock);
                                break;
                            } else {
                                usleep(10000);
                            }
                        }
                        #ifdef DEBUG_RKCLIENT
                        RKLog("%s k = %d   r = %d   readCount = %d\n", C->name, k, r, readCount);
                        for (int j = 0; j < 20; j++) {
                            printf(" %02x/%c", (int)*(cbuf + j), *(cbuf + j));
                        }
                        printf("\n");
                        #endif
                        // Add a NULL character to the end of payload
                        cbuf[k] = '\0';
                        if (readCount >= C->timeoutSeconds * 1000) {
                            break;
                        }
                        readOkay = true;
                        break;
                    case RKNetworkMessageFormatNewLine:
                    default:
                        if (fid == NULL) {
                            fid = fdopen(C->sd, "r");
                            if (fid < 0) {
                                RKLog("%s Error. Unable to open a file descriptor for socket.\n", C->name);
                                return (void *)-1;
                            }
                        }
                        if (fgets(buf, RKMaximumPacketSize, fid) != NULL) {
                            readOkay = true;
                        }
                        break;
                } // switch (C->format) ...
                if (readOkay == false) {
                    if (C->verbose > 1) {
                        RKLog("%s Server not connected.\n", C->name);
                    }
                    pthread_mutex_lock(&C->lock);
                    if (C->state < RKClientStateDisconnecting) {
                        C->state = RKClientStateReconnecting;
                    }
                    pthread_mutex_unlock(&C->lock);
                    close(C->sd);
                    fid = NULL;
                    continue;
                } else {
                    pthread_mutex_lock(&C->lock);
                    if (C->state < RKClientStateDisconnecting && C->state != RKClientStateConnected) {
                        if (C->verbose) {
                            RKLog("%s Connected.\n", C->name);
                        }
                        C->state = RKClientStateConnected;
                    }
                    pthread_mutex_unlock(&C->lock);
                }
                timeoutCount = 0;
                C->recv(C);
                cbuf[0] = '\0';
            } else if (r > 0 && FD_ISSET(C->sd, &C->efd)) {
                RKLog("%s Error occurred.  r=%d  errno=%d (%s)\n", C->name, r, errno, RKErrnoString(errno));
                break;
            } else {
                RKLog("%s Error. r=%d  errno=%d (%s)\n", C->name, r, errno, RKErrnoString(errno));
                break;
            } // if (r == 0) ...
        }

        // Wait a while before trying to reconnect
        k = 31;
        while (k-- > 1 && C->state < RKClientStateDisconnecting) {
            if (k % 10 == 0 && C->verbose > 1) {
                RKLog("Reconnect in %d secs ...\n", k / 10);
            }
            usleep(100000);
        };
    } // Here comes ... will jump out if C->state >= RKClientStateDisconnecting
    free(buf);

    C->userPayload = NULL;
    C->state = RKClientStateDisconnected;

    if (C->verbose) {
        RKLog("%s Disconnected.\n", C->name);
    }
    if (C->exit) {
        C->exit(C);
    }
    return NULL;
}

#pragma mark - Threads

#pragma mark - Life Cycle

RKClient *RKClientInitWithDesc(RKClientDesc desc) {
    if (desc.format == RKNetworkMessageFormatConstantSize && desc.blockLength == 0) {
        RKLog("Block length may not be 0 for constant packet size.\n");
        return NULL;
    } else if (desc.format != RKNetworkMessageFormatConstantSize && desc.blockLength > 0) {
        RKLog("Block length is ignored for variable packet size.\n");
        return NULL;
    }
    RKClient *C = (RKClient *)malloc(sizeof(RKClient));
    if (C == NULL) {
        RKLog("Error. Unable to allocate RKClient.\n");
        return NULL;
    }
    memset(C, 0, sizeof(RKClient));
    // Copy the first part in which the first part must be identical
    if (desc.timeoutSeconds == 0) {
        desc.timeoutSeconds = RKNetworkTimeoutSeconds;
    }
    memcpy(C, &desc, sizeof(RKClientDesc));

    // Ignore broken pipe for bad connections
    signal(SIGPIPE, SIG_IGN);

    return C;
}

RKClient *RKClientInitWithHostnamePort(const char *hostname, const int port) {
    RKClientDesc desc;
    memset(&desc, 0, sizeof(RKClientDesc));
    strncpy(desc.hostname, hostname, RKNameLength - 1);
    desc.port = port;
    desc.type = RKNetworkSocketTypeTCP;
    desc.blocking = true;
    desc.reconnect = true;
    desc.timeoutSeconds = RKNetworkTimeoutSeconds;
    return RKClientInitWithDesc(desc);
}

RKClient *RKClientInit(void) {
    RKClientDesc desc;
    memset(&desc, 0, sizeof(RKClientDesc));
    sprintf(desc.hostname, "localhost");
    desc.port = 9000;
    desc.type = RKNetworkSocketTypeTCP;
    desc.blocking = true;
    desc.reconnect = true;
    desc.timeoutSeconds = RKNetworkTimeoutSeconds;
    return RKClientInitWithDesc(desc);
}

void RKClientFree(RKClient *C) {
    RKClientStop(C);
    free(C);
    return;
}

#pragma mark -
#pragma mark Properties


void RKClientSetUserResource(RKClient *C, void *resource) {
    C->userResource = resource;
}

void RKClientSetGreetHandler(RKClient *C, int (*routine)(RKClient *)) {
    C->init = routine;
}

void RKClientSetReceiveHandler(RKClient *C, int (*routine)(RKClient *)) {
    C->recv = routine;
}

void RKClientSetCloseHandler(RKClient *C, int (*routine)(RKClient *)) {
    C->exit = routine;
}

#pragma mark - Interactions

void RKClientStart(RKClient *C, const bool waitForConnection) {
    pthread_mutex_init(&C->lock, NULL);
    pthread_attr_init(&C->threadAttributes);
    if (pthread_create(&C->threadId, &C->threadAttributes, theClient, C)) {
        RKLog("Error. Unable to launch a socket client.\n");
        return;
    }
    if (!waitForConnection) {
        if (C->verbose > 1) {
            RKLog("%s Immediate return.\n", C->name);
        }
        return;
    }
    while (C->state < RKClientStateConnecting) {
        usleep(100000);
    }
    return;
}

void RKClientStop(RKClient *C) {
    if (C->state > RKClientStateCreating && C->state < RKClientStateDisconnecting) {
        RKLog("%s Disconnecting ...\n", C->name);
        pthread_mutex_lock(&C->lock);
        C->state = RKClientStateDisconnecting;
        pthread_mutex_unlock(&C->lock);
        pthread_join(C->threadId, NULL);
        pthread_attr_destroy(&C->threadAttributes);
        pthread_mutex_destroy(&C->lock);
        C->state = RKClientStateNull;
    } else if (C->verbose > 1) {
        RKLog("%s Info. Client does not seem to be running.\n", C->name);
        return;
    }
}
