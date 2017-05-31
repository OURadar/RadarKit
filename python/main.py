import sys
import logging
import signal
import socket
import time
import threading
import struct

from argparse import ArgumentParser

import radarkit

sys.path.insert(0, 'algorithms')

import highZ

if __name__ == "__main__":
    parser = ArgumentParser(prog="main")
    parser.add_argument("-H", "--host", default='localhost', help="hostname (default localhost)")
    parser.add_argument("-p", "--port", default=10000, type=int, help="port (default 10000)")
    args = parser.parse_args()

    print('Version {}'.format(sys.version_info))

    radarkit.test()

    radar = radarkit.Radar(ipAddress=args.host)
    radar.addAlgorithm('highZ')
    radar.addAlgorithm('lowZ')
    radar.start()
