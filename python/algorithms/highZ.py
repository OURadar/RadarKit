import logging
import signal
import socket
import sys
import time
import threading
import struct

import radarkit

if __name__ == "__main__":
    parser = ArgumentParser(prog="highZ")
    parser.add_argument("-H", "--host", default='localhost', help="hostname (default localhost)")
    parser.add_argument("-p", "--port", default=10000, type=int, help="port (default 10000)")
    args = parser.parse_args()

    radarkit.test()

    radar = radarkit.Radar(ipAddress=args.host)
    radar.start()
