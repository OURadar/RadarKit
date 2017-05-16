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

class Radar(object):
    """Handles the connection to the radar (created by RadarKit)

    This class allows to retrieval of base data from the radar

    """
    def __init__(self, ipAddress=IP_ADDRESS, port=RADADRKIT_PORT, timeout=0.1):
        self.ipAddress = ipAddress
        self.port = port
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.settimeout(timeout)

        self.netDelimiter = bytearray(16)
        self.payload = bytearray(BUFFER_SIZE)

    def _next_packet(self):
    	try:
    		self.socket.recv_into(self.netDelimiter, 16)


    	except (socket.timeout, ValueError) as e:
    		logger.exception(e)
    		raise OSError('Couldn\'t retrieve socket data')

    def close(self):
    	self.socket.close()

    def __del__(self):
    	self.close()
