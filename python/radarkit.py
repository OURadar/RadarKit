"""
Python wrapper for C functions to interact with RadarKit
"""

import logging
import math
import socket
import struct
import sys

__all__ = ['RadarKit', 'RADADRKIT_PORT', 'IP_ADDRESS']

logger = logging.getLogger(__name__)

IP_ADDRESS = '127.0.0.1'
RADADRKIT_PORT = 10000
BUFFER_SIZE = 10240
PACKET_DELIM_SIZE = 16

netType = b'H'
subType = b'H'
packedSize = b'I'
decodedSize = b'I'
delimiterPad = b'HH'
RKNetDelimiter = netType + subType + packedSize + decodedSize + delimiterPad
del netType, subType, packedSize, decodedSize, delimiterPad

class Radar(object):
    """Handles the connection to the radar (created by RadarKit)

    This class allows to retrieval of base data from the radar

    """
    def __init__(self, ipAddress=IP_ADDRESS, port=RADADRKIT_PORT, timeout=2):
        self.ipAddress = ipAddress
        self.port = port
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.settimeout(timeout)

        self.verbose = 1
        self.netDelimiter = bytearray(16)
        self.payload = bytearray(BUFFER_SIZE)

    def _recv(self):
        try:
            length = self.socket.recv_into(self.netDelimiter, PACKET_DELIM_SIZE)
            if length != PACKET_DELIM_SIZE:
                raise ValueError('Length should be {}, not {}'.format(PACKET_DELIM_SIZE, length))
            delimiter = struct.unpack(RKNetDelimiter, self.netDelimiter)

            payloadType = delimiter[0]
            payloadSize = delimiter[2]
            print('Delimiter type {} of size {} {}'.format(payloadType, payloadSize, delimiter[3]))

            if payloadSize > 0:
                length = self.socket.recv_into(self.payload, payloadSize)
                if length != payloadSize:
                    raise ValueError('Length should be {}, not {}'.format(payloadSize, length))
                if self.verbose > 1:
                    print(self.payload.decode('utf-8'))

        except (socket.timeout, ValueError) as e:
            logger.exception(e)
            raise OSError('Couldn\'t retrieve socket data')

    def start(self):
        self.active = True

        print('Connecting ...')
        self.socket.connect((self.ipAddress, self.port))
        self.socket.send(b'sh\r\n')

        while self.active:
            self._recv()

    def close(self):
        self.socket.close()

    def __del__(self):
        self.close()
