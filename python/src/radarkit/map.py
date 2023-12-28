import os
import json
import importlib
import numpy as np

prefix = os.path.join(os.path.dirname(importlib.util.find_spec("radarkit").origin), "maps")
r_earth = 6378.0
deg2rad = np.pi / 180.0
rad2deg = 1.0 / deg2rad
ring_color = "#78dcff"
weight_big = 50000
weight_med = 95000  # Norman is 95694


def get(name="county"):
    if name == "county":
        path = os.path.join(prefix, "counties-10m.stq.json")
    else:
        path = os.path.join(prefix, "counties-10m.stq.json")
    print(path)
    with open(path, "r") as fid:
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


def project(coords, rotation=makeRotationForCoord()):
    lon, lat = coords[:, 1], coords[:, 0]
    m = r_earth * np.cos(lon)
    y = r_earth * np.sin(lon)
    z = m * np.cos(lat)
    x = m * np.sin(lat)
    p = np.array((x, y, z)).transpose()
    return np.matmul(p, rotation)


def coordsFromPoly(poly):
    coords = []
    w = poly["transform"]["scale"]
    b = poly["transform"]["translate"]
    for arc in poly["arcs"]:
        lat = w[1] * arc[0][1] + b[1]
        if lat < -89:
            continue
        line = []
        point = [0, 0]
        for p in arc:
            point[0] += p[0]
            point[1] += p[1]
            lon = w[0] * point[0] + b[0]
            lat = w[1] * point[1] + b[1]
            line.append([lon, lat])
        coords.append(np.array(line))
    return coords


def subsetPaths(poly, origin=(-97.46381, 35.23682), extent=(-160, 160, -90, 90)):
    x_min, x_max, y_min, y_max = extent
    rotation = makeRotationForCoord(*origin)
    coords = coordsFromPoly(poly)
    subset = []
    for c in coords:
        p = project(np.radians(c), rotation)
        outside = np.logical_or(np.logical_or(p[:, 0] < x_min, p[:, 0] > x_max), np.logical_or(p[:, 1] < y_min, p[:, 1] > y_max))
        if np.all(outside):
            continue
        subset.append(p)
    return subset
