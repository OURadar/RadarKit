import logging
import signal
import socket
import sys
import time
import threading
import struct

from argparse import ArgumentParser

import radarkit

if __name__ == "__main__":
    parser = ArgumentParser(prog="highZ")
    parser.add_argument("-H", "--host", default='localhost', help="hostname (default localhost)")
    parser.add_argument("-p", "--port", default=10000, type=int, help="port (default 10000)")
    args = parser.parse_args()

    print("Connecting to %s:%d ..." % (args.host, args.port))

    radar = radarkit.Radar(ipAddress=args.host)
    