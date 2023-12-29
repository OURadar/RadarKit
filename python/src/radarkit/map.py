import os
import json
import importlib
import numpy as np

# prefix = os.path.join(os.path.dirname(importlib.util.find_spec("radarkit").origin), "maps")
prefix = os.path.join(os.path.split(__file__)[0], "maps")

r_earth = 6378.0
deg2rad = np.pi / 180.0
rad2deg = 1.0 / deg2rad
weight_big = 50000
weight_med = 95000  # Norman is 95694


class MapColor:
    def __init__(self, theme="light"):
        if theme == "light":
            self.ring = "#00bbff"
            self.state = "#40bf91"
            self.county = "#40bf91"
            self.highway = "#e6b955"
        else:
            self.ring = "#78dcff"
            self.state = "#96e6c8"
            self.county = "#83e2bf"
            self.highway = "#e6b955"


colors = MapColor("dark")

if not os.path.exists(prefix):
    prefix, _ = os.path.split(__file__)
    prefix = os.path.join(prefix, "maps")
    if not os.path.exists(prefix):
        raise FileNotFoundError(f"Could not find maps directory: {prefix}")


def _read(name="county"):
    if name == "state":
        filename = "states-10m.stq.json"
    elif name == "county":
        filename = "gz_2010_us_050_00_500k.stq.json"
    elif name == "interstate" or name == "highway":
        filename = "intrstat.stq.json"
    elif name == "city":
        filename = "citiesx020.shp.json"
    else:
        filename = name
    path = os.path.join(prefix, filename)
    if not os.path.exists(path):
        print(f"Could not find map: {path}")
        return None
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


def coordsFromPolyNaive(poly):
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


def coordsFromPoly(poly):
    coords = []
    w = poly["transform"]["scale"]
    b = poly["transform"]["translate"]
    for arc in poly["arcs"]:
        lat = w[1] * arc[0][1] + b[1]
        if lat < -89:
            continue
        line = np.cumsum(np.array(arc), axis=0)
        coords.append(line * w + b)
    return coords


def get(poly, origin=(-97.46381, 35.23682), extent=(-160, 160, -90, 90)):
    if isinstance(poly, str):
        poly = _read(name=poly)
        return get(poly, origin=origin, extent=extent)
    if isinstance(poly, dict) and "transform" in poly:
        coords = coordsFromPoly(poly)
        x_min, x_max, y_min, y_max = extent
        rotation = makeRotationForCoord(*origin)
        subset = []
        for c in coords:
            p = project(np.radians(c), rotation)
            # outside = np.logical_or(np.logical_or(p[:, 0] < x_min, p[:, 0] > x_max), np.logical_or(p[:, 1] < y_min, p[:, 1] > y_max))
            # if np.all(outside):
            #     continue
            # subset.append(p)
            inside = np.logical_and(
                np.logical_and(p[:, 0] > x_min, p[:, 0] < x_max), np.logical_and(p[:, 1] > y_min, p[:, 1] < y_max)
            )
            if np.any(inside):
                subset.append(p[:, :2])
        return subset
    elif isinstance(poly, list) and isinstance(poly[0], dict) and "G" in poly[0]:
        coords = np.array([t["G"]["C"] for t in poly])
        labels = [t["P"]["N"] for t in poly]
        weights = [t["P"]["P"] for t in poly]
        x_min, x_max, y_min, y_max = extent
        rotation = makeRotationForCoord(*origin)
        p = project(np.radians(coords), rotation)
        inside = np.logical_and(np.logical_and(p[:, 0] > x_min, p[:, 0] < x_max), np.logical_and(p[:, 1] > y_min, p[:, 1] < y_max))
        text = [(*pos[:2], label, weight) for pos, label, weight, s in zip(p, labels, weights, inside) if s]
        text.sort(key=lambda x: x[3], reverse=True)
        return text
    else:
        raise ValueError(f"Unknown polygon format: {type(poly)}")
