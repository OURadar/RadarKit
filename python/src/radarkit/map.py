import os
import json
import importlib
import numpy as np

prefix = os.path.join(os.path.dirname(importlib.util.find_spec("radarkit").origin), "maps")
r_earth = 6378.0
deg2rad = np.pi / 180.0
rad2deg = 1.0 / deg2rad
ring_color = '#78dcff'
weight_big = 50000
weight_med = 95000      # Norman is 95694


def get(name='county'):
    if name == 'county':
        path = os.path.join(prefix, 'counties-10m.stq.json')
    else:
        path = os.path.join(prefix, 'counties-10m.stq.json')
    print(path)
    with open(path, 'r') as fid:
        data = json.load(fid)
    return data

def makeRotationX(phi):
    return np.array([[1.0, 0.0, 0.0], [0.0, np.cos(phi), np.sin(phi)], [0.0, -np.sin(phi), np.cos(phi)]])


def makeRotationY(phi):
    return np.array([[np.cos(phi), 0.0, -np.sin(phi)], [0.0, 1.0, 0.0], [np.sin(phi), 0.0, np.cos(phi)]])


def makeRotationZ(phi):
    return np.array([[np.cos(phi), np.sin(phi), 0.0], [-np.sin(phi), np.cos(phi), 0.0], [0.0, 0.0, 1.0]])


def makeRotationForCoord(lon=-97.46381, lat=35.23682):
    return np.matmul(makeRotationY(-lon * deg2rad), makeRotationX(lat * deg2rad))
