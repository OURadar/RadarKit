//
//  RKClient.c
//  RadarKit
//
//  This collection is copied and modified from PortClient.
//
//  Created by Boon Leng Cheong on 1/3/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKClient.h>

// Internal functions

void *theClient(void *in);

// Implementations

#pragma mark -
#pragma mark Helper Functions

void *theClient(void *in) {
    RKClient *C = (RKClient *)in;

    int r;
    int k;
    int readCount, timeoutCount;
    struct timeval timeout;
    struct timeval previousBeaconTime;
    bool readOkay;

    FILE *fid = NULL;

    void *buf = (void *)malloc(RKMaxPacketSize);
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

        if (C->verbose > 1) {
            RKLog("%s opening a %s socket ...\n", C->name,
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
            fcntl(C->sd, F_SETFL, O_NONBLOCK);
        }
        if (C->type == RKNetworkSocketTypeUDP) {
            r = 1;
            setsockopt(C->sd, SOL_SOCKET, SO_BROADCAST, &r, sizeof(r));
        }

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

        C->state = RKClientStateConfiguringSocket;

        //RKLog("%s <RKClient> --> %d", C->name, sizeof(C->sa.sin_addr.s_addr));

        // Bind to a specific interface
//        struct sockaddr_in local_addr;
//        local_addr.sin_family = AF_INET;
//        local_addr.sin_port = htons(0);
//        local_addr.sin_addr.s_addr = inet_addr("192.168.2.2");
//        if (bind(C->sd, (struct sockaddr *)&local_addr, sizeof(struct sockaddr_in))) {
//            RKLog("%s Error. Unable to fix an interface (errno = %d)\n", C->name, errno);
//        }

        // Configure the socket
        C->sa.sin_family = AF_INET;
        C->sa.sin_port = htons(C->port);
        C->sa.sin_addr.s_addr = inet_addr(C->hostIP);
        //memset(C->sa.sin_zero, 0, 8);
        //RKLog("%s <RKClient> %d.%d.%d.%d\n", C->name, (C->sa.sin_addr.s_addr >> 24) & 0xFF, (C->sa.sin_addr.s_addr >> 16) & 0xFF, (C->sa.sin_addr.s_addr >> 8) & 0xFF, C->sa.sin_addr.s_addr & 0xFF);

        if (C->verbose > 1) {
            RKLog("%s Configuring socket ...\n", C->name);
        }

        C->state = RKClientStateConnecting;

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

        // Server is ready to receive
        if (C->init) {
            fid = NULL;
            FD_ZERO(&C->wfd);
            FD_ZERO(&C->efd);
            FD_SET(C->sd, &C->wfd);
            FD_SET(C->sd, &C->efd);
            timeout.tv_sec = C->timeoutSeconds;
            timeout.tv_usec = 0;
            r = select(C->sd + 1, NULL, &C->wfd, &C->efd, &timeout);
            if (r > 0) {
                if (FD_ISSET(C->sd, &C->wfd)) {
                    C->state = RKClientStateConnected;
                    if (C->verbose) {
                        RKLog("%s Connected.\n", C->name);
                    }
                } else if (FD_ISSET(C->sd, &C->efd)) {
                    RKLog("Error. r = %d   errno = %d (%s)\n", r, errno, RKErrnoString(errno));
                }
                C->init(C);
            } else {
                if (C->verbose > 1) {
                    RKLog("Timeout during initialization.\n");
                }
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
                if (timeoutCount++ / 10 > C->timeoutSeconds) {
                    // Socket established but nothing from the server.
                    if (C->verbose > 1) {
                        RKLog("%s Timeout during select() for read.\n", C->name);
                    }
                    break;
                }
                //continue;
            } else if (r > 0 && FD_ISSET(C->sd, &C->rfd)) {
                switch (C->format) {

                    case RKNetworkMessageFormatConstantSize:

                        k = 0;
                        readCount = 0;
                        while (readCount++ < C->timeoutSeconds * 1000) {
                            if ((r = (int)read(C->sd, buf + k, C->blockLength - k)) > 0) {
                                k += r;
                                if (k >= C->blockLength) {
                                    break;
                                } else {
                                    usleep(1000);
                                }
                            } else if (errno != EAGAIN) {
                                if (C->verbose > 1) {
                                    RKLog("%s Error. RKMessageFormatFixedBlock   r=%d  k=%d  errno=%d (%s)  %d\n",
                                          C->name, r, k, errno, RKErrnoString(errno), readCount);
                                }
                                readCount = C->timeoutSeconds * 1000;
                                if (C->state < RKClientStateDisconnecting) {
                                    C->state = RKClientStateReconnecting;
                                }
                            }
                            if (C->verbose > 1) {
                                RKLog("... errno = %d ...\n", errno);
                            }
                        }
                        if (readCount >= C->timeoutSeconds * 1000) {
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
                                k += r;
                                if (k == sizeof(RKNetDelimiter)) {
                                    break;
                                } else if (k > sizeof(RKNetDelimiter)) {
                                    RKLog("%s Error. Should not read larger than sizeof(RKNetDelimiter) = %zu\n", C->name, sizeof(RKNetDelimiter));
                                    break;
                                } else {
                                    usleep(10000);
                                }
                            } else if (errno != EAGAIN) {
                                if (C->verbose > 1) {
                                    RKLog("%s Error. RKMessageFormatFixedHeaderVariableBlock:1  r=%d  k=%d  errno=%d (%s)\n",
                                          C->name, r, k, errno, RKErrnoString(errno));
                                }
                                readCount = C->timeoutSeconds * 1000;
                                if (C->state < RKClientStateDisconnecting) {
                                    C->state = RKClientStateReconnecting;
                                }
                                break;
                            }
                        }
                        if (k != sizeof(RKNetDelimiter) || readCount > C->timeoutSeconds * 100) {
                            break;
                        }
                        // If the delimiter specifies 0 payload, it could just be a beacon
                        if (C->netDelimiter.size == 0) {
                            RKLog("%s netDelimiter.size = 0\n", C->name);
                            readOkay = true;
                            break;
                        }
                        // Now the actual payload
                        k = 0;
                        readCount = 0;
                        while (readCount++ < C->timeoutSeconds * 100) {
                            if ((r = (int)read(C->sd, buf + k, C->netDelimiter.size - k)) > 0) {
                                k += r;
                                if (k >= C->netDelimiter.size) {
                                    break;
                                } else {
                                    usleep(10000);
                                }
                            } else if (errno != EAGAIN) {
                                RKLog("%s Error. PCMessageFormatFixedHeaderVariableBlock:2  r=%d  k=%d  errno=%d (%s)\n",
                                        C->name, r, k, errno, RKErrnoString(errno));
                                readCount = C->timeoutSeconds * 1000;
                                if (C->state < RKClientStateDisconnecting) {
                                    C->state = RKClientStateReconnecting;
                                }
                                break;
                            }
                        }
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
                        if (fgets(buf, RKMaxPacketSize, fid) != NULL) {
                            readOkay = true;
                        }
                        break;
                }

                if (readOkay == false) {
                    RKLog("%s Server disconnected.\n", C->name);
                    if (C->state < RKClientStateDisconnecting) {
                        C->state = RKClientStateReconnecting;
                    }
                    close(C->sd);
                    fid = NULL;
                    continue;
                } else if (C->state < RKClientStateDisconnecting && C->state != RKClientStateConnected) {
                    if (C->verbose) {
                        RKLog("%s Connected.\n", C->name);
                    }
                    C->state = RKClientStateConnected;
                }
                timeoutCount = 0;
                C->recv(C);
            } else if (r > 0 && FD_ISSET(C->sd, &C->efd)) {
                RKLog("%s Error occurred.  r=%d  errno=%d (%s)\n", C->name, r, errno, RKErrnoString(errno));
                break;
            } else {
                RKLog("%s Error. r=%d  errno=%d (%s)\n", C->name, r, errno, RKErrnoString(errno));
                break;
            }

            // Send in a beacon signal
            gettimeofday(&timeout, NULL);
            timeout.tv_sec -= 1;
            if (timercmp(&timeout, &previousBeaconTime, >=)) {
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
                        gettimeofday(&previousBeaconTime, NULL);
                        //RKLog("%s beacon\n", C->name);
                        RKNetworkSendPackets(C->sd, ping, strlen(ping), NULL);
                    }
                } else {
                    RKLog("%s Error. r=%d  errno=%d (%s)\n", C->name, r, errno, RKErrnoString(errno));
                    break;
                }
            }
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
    return NULL;
}

#pragma mark -
#pragma mark Threads

#pragma mark -
#pragma mark Life Cycle

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

#pragma mark -
#pragma mark Interactions

void RKClientStart(RKClient *C, const bool waitForConnection) {
    pthread_attr_init(&C->threadAttributes);
    if (pthread_create(&C->threadId, &C->threadAttributes, theClient, C)) {
        RKLog("Error. Unable to launch a socket client.\n");
        return;
    }
    C->state = RKClientStateCreating;
    if (!waitForConnection) {
        return;
    }
    while (C->state == RKClientStateCreating) {
        usleep(100000);
    }
    return;
}

void RKClientStop(RKClient *C) {
    if (C->state > RKClientStateCreating && C->state < RKClientStateDisconnecting) {
        RKLog("%s Disconnecting ...\n", C->name);
        C->state = RKClientStateDisconnecting;
        pthread_join(C->threadId, NULL);
    } else if (C->verbose > 1) {
        RKLog("%s Info. Client does not seem to be running.\n", C->name);
        return;
    }
}
