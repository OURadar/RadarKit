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
    int r;
    int k;
    int timeoutCount;
    struct timeval timeout;
    bool readOkay;

    FILE *fid = NULL;

    char *buf = (char *)malloc(RKMaxPacketSize);
    if (buf == NULL) {
        RKLog("Error. Unable to allocate space for a buffer.\n");
        return (void *)RKResultErrorCreatingOperatorRoutine;
    }

    RKNetDelimiter *header = (RKNetDelimiter *)buf;

    RKClient *C = (RKClient *)in;
    C->userPayload = buf;

    if (C->verbose > 1) {
        RKLog("%s is working hard ...\n", C->name);
    }

    // Here comes the infinite loop until being stopped
    while (C->state < RKClientStateReconnecting) {

        C->state = RKClientStateConnecting;

        //if (C->verbose > 1) {
            RKLog("%s opening a %s socket ...\n", C->name,
                  C->type == RKNetworkSocketTypeTCP ? "TCP" :
                  (C->type == RKNetworkSocketTypeUDP ? "UDP" : "(NULL)"));
        //}
        if (C->type == RKNetworkSocketTypeTCP) {
            if ((C->sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                RKLog("Error. Unable to create a TCP socket.\n");
                C->state = RKClientStateDisconnected;
                return NULL;
            }
        } else if (C->type == RKNetworkSocketTypeUDP) {
            if ((C->sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
                RKLog("Error. Unable to create a UDP socket.\n");
                C->state = RKClientStateDisconnected;
                return NULL;
            }
        } else {
            RKLog("Error. Unable to determine the socket type %d.\n", C->type);
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
            RKLog("Error. Unable to resolve %s\n", C->hostname);
            sleep(10);
            continue;
        }
        strcpy(C->hostIP, inet_ntoa(*((struct in_addr *)h->h_addr_list[0])));

        C->state = RKClientStateConfiguringSocket;

        // Configure the socket
        C->sa.sin_family = AF_INET;
        C->sa.sin_port = htons(C->port);
        C->sa.sin_addr.s_addr = inet_addr(C->hostIP);

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
                k = 3;
                do {
                    if (C->verbose > 1) {
                        RKLog("%s Connection failed (errno = %d). Retry in %d second%s ...\n", C->name, k, k > 1 ? "s" : "");
                    }
                    k--;
                    sleep(1);
                } while (k > 0);
                continue;
            }
        }

        // Server is ready to receive
        if (C->init) {
            fid = NULL;
            FD_ZERO(&C->rfd);
            FD_ZERO(&C->wfd);
            FD_ZERO(&C->efd);
            FD_SET(C->sd, &C->rfd);
            FD_SET(C->sd, &C->wfd);
            FD_SET(C->sd, &C->efd);
            timeout.tv_sec = C->timeoutSeconds;
            timeout.tv_usec = 0;
            r = select(C->sd + 1, NULL, &C->wfd, &C->efd, &timeout);
            if (r > 0) {
                if (FD_ISSET(C->sd, &C->wfd)) {
                    C->state = RKClientStateConnected;
                    if (C->verbose > 1) {
                        RKLog("Connected.\n");
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
        } else if (C->verbose) {
            if (C->type == RKNetworkSocketTypeTCP) {
                RKLog("%s connected.\n", C->name);
            } else {
                // UDP may not mean connected at this point
                RKLog("%s initialized.\n", C->name);
            }
        }

        // Actively receive
        while (C->state < RKClientStateReconnecting) {
            FD_ZERO(&C->rfd);
            FD_ZERO(&C->wfd);
            FD_ZERO(&C->efd);
            FD_SET(C->sd, &C->rfd);
            FD_SET(C->sd, &C->wfd);
            FD_SET(C->sd, &C->efd);
            timeout.tv_sec = C->timeoutSeconds;
            timeout.tv_usec = 0;
            readOkay = false;
            r = select(C->sd + 1, &C->rfd, NULL, &C->efd, &timeout);
            if (C->verbose > 3 || FD_ISSET(C->sd, &C->efd)) {
                RKLog("%s select() returned r = %d   FD_ISSET(rfd) = %d   FD_ISSET(efd) = %d   errno = %d.\n",
                        C->name, r, FD_ISSET(C->sd, &C->rfd), FD_ISSET(C->sd, &C->efd), errno);
            }
            if (r == 0) {
                // Socket established but nothing from the server.
                if (C->verbose > 1) {
                    RKLog("%s Timeout during select() for read.\n", C->name);
                }
                break;
            } else if (r > 0 && FD_ISSET(C->sd, &C->rfd)) {
                switch (C->format) {

                    case RKNetworkMessageFormatConstantSize:

                        k = 0;
                        timeoutCount = 0;
                        while (timeoutCount++ < C->timeoutSeconds * 1000) {
                            if ((r = (int)read(C->sd, buf + k, C->blockLength - k)) > 0) {
                                k += r;
                                if (k >= C->blockLength) {
                                    break;
                                } else {
                                    usleep(1000);
                                }
                            } else if (errno != EAGAIN) {
                                if (timeoutCount % 1000 == 0) {
                                    RKLog("%s Error. RKMessageFormatFixedBlock   r=%d  k=%d  errno=%d (%s)  %d\n",
                                          C->name, r, k, errno, RKErrnoString(errno), timeoutCount);
                                }
//                                if (r == 0) {
//                                    timeoutCount--;
//                                }
                                continue;
                            }
                            RKLog("... errno = %d ...\n", errno);
                        }
                        if (timeoutCount >= C->timeoutSeconds * 1000) {
                            RKLog("%s Not a proper frame.  timeoutCount = %d  errno = %d\n", C->name, timeoutCount, errno);
                            break;
                        }
                        readOkay = true;
                        break;

                    case RKNetworkMessageFormatHeaderDefinedSize:

                        k = 0;
                        timeoutCount = 0;
                        while (timeoutCount++ < C->timeoutSeconds * 100) {
                            if ((r = (int)read(C->sd, buf + k, sizeof(RKNetDelimiter) - k)) > 0) {
                                k += r;
                                if (k > sizeof(RKNetDelimiter)) {
                                    RKLog("%s Error. Should not read larger than sizeof(RKNetDelimiter) = %zu\n", C->name, sizeof(RKNetDelimiter));
                                    break;
                                } else if (k == sizeof(RKNetDelimiter)) {
                                    break;
                                } else {
                                    usleep(10000);
                                }
                            } else if (errno != EAGAIN) {
                                RKLog("%s Error. RKMessageFormatFixedHeaderVariableBlock:1  r=%d  k=%d  errno=%d (%s)\n",
                                        C->name, r, k, errno, RKErrnoString(errno));
                                break;
                            }
                        }
                        if (k != sizeof(RKNetDelimiter) || timeoutCount > C->timeoutSeconds * 100 || errno != ETIMEDOUT) {
                            break;
                        }
                        k = 0;
                        timeoutCount = 0;
                        while (k < header->size && timeoutCount++ < C->timeoutSeconds * 100) {
                            if ((r = (int)read(C->sd, buf + sizeof(RKNetDelimiter) + k, header->size - k)) > 0) {
                                k += r;
                                if (k >= header->size) {
                                    break;
                                } else {
                                    usleep(10000);
                                }
                            } else if (errno != EAGAIN) {
                                RKLog("%s Error. PCMessageFormatFixedHeaderVariableBlock:2  r=%d  k=%d  errno=%d (%s)\n",
                                        C->name, r, k, errno, RKErrnoString(errno));
                                break;
                            }
                        }
                        if (timeoutCount >= RKNetworkTimeoutSeconds * 100 || errno != EAGAIN) {
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
            } else if (r > 0 && FD_ISSET(C->sd, &C->efd)) {
                RKLog("%s Error occurred.  r=%d  errno=%d (%s)\n", C->name, r, errno, RKErrnoString(errno));
                break;
            } else {
                RKLog("%s Error. r=%d  errno=%d (%s)\n", C->name, r, errno, RKErrnoString(errno));
                //fclose(fid);
                break;
            }

            if (readOkay == false) {
                close(C->sd);
                break;
            }
            C->recv(C);
        }

        C->state = RKClientStateConnecting;

        // Wait a while before trying to reconnect
        sleep(3);

    }
    free(buf);

    C->userPayload = NULL;
    C->state = RKClientStateDisconnected;

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
    strncpy(desc.hostname, hostname, RKMaximumStringLength - 1);
    desc.port = port;
    desc.type = RKNetworkSocketTypeTCP;
    desc.blocking = true;
    desc.reconnect = true;
    desc.timeoutSeconds = RKNetworkTimeoutSeconds;
    return RKClientInitWithDesc(desc);
}

void RKClientFree(RKClient *C) {
    if (C->state > RKClientStateConnecting && C->state < RKClientStateDisconnecting) {
        C->state = RKClientStateDisconnecting;
    }
    pthread_join(C->threadId, NULL);
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

void RKClientStart(RKClient *C) {
    pthread_attr_init(&C->threadAttributes);
    if (pthread_create(&C->threadId, &C->threadAttributes, theClient, C)) {
        RKLog("Error. Unable to launch a socket client.\n");
        return;
    }
    C->state = RKClientStateCreating;
    while (C->state == RKClientStateCreating) {
        usleep(100000);
    }
    return;
}
void RKClientStop(RKClient *C) {
    if (C->state > RKClientStateConnecting && C->state < RKClientStateDisconnecting) {
        C->state = RKClientStateDisconnecting;
    } else {
        RKLog("Error. Client does not seem to be running.\n");
        return;
    }
}
