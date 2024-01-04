import os
import blib
import json
import importlib
import matplotlib
import matplotlib.patheffects
import numpy as np

# prefix = os.path.join(os.path.dirname(importlib.util.find_spec("radarkit").origin), "maps")
prefix = os.path.join(os.path.split(__file__)[0], "maps")

r_earth = 6378.0
deg2rad = np.pi / 180.0
rad2deg = 1.0 / deg2rad
pop_big = 50000
pop_med = 95000  # Norman is 95694


class MapColor:
    def __init__(self, theme="light"):
        if theme == "light":
            self.ring = "#00bbff"
            self.state = "#40bf91"
            self.county = "#40bf91"
            self.highway = "#e6b955"
            self.text = "#000000"
        else:
            self.ring = "#78dcff"
            self.state = "#96e6c8"
            self.county = "#83e2bf"
            self.highway = "#e6b955"
            self.text = "#ffffff"


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


def get(poly, origin=(-97.46381, 35.23682), extent=(-160, -90, 160, 90), density=6.0):
    if isinstance(poly, str):
        if poly[:5] == "@ring":
            r = np.array([float(s) for s in poly.split("/")[1:]])
            a = np.arange(0, 2.01 * np.pi, np.pi / 180)
            x, y = np.cos(a), np.sin(a)
            rings = []
            for k in r:
                rings.append(k * np.array([x, y]).transpose())
            return rings
        poly = _read(name=poly)
        return get(poly, origin=origin, extent=extent, density=density)
    if isinstance(poly, dict) and "transform" in poly:
        coords = coordsFromPoly(poly)
        x_min, y_min, x_max, y_max = extent
        rotation = makeRotationForCoord(*origin)
        subset = []
        for c in coords:
            pos = project(np.radians(c), rotation)
            # outside = np.logical_or(np.logical_or(p[:, 0] < x_min, p[:, 0] > x_max), np.logical_or(p[:, 1] < y_min, p[:, 1] > y_max))
            # if np.all(outside):
            #     continue
            # subset.append(p)
            inside = np.logical_and(
                np.logical_and(pos[:, 0] > x_min, pos[:, 0] < x_max), np.logical_and(pos[:, 1] > y_min, pos[:, 1] < y_max)
            )
            if np.any(inside):
                subset.append(pos[:, :2])
        return subset
    elif isinstance(poly, list) and isinstance(poly[0], dict) and "G" in poly[0]:
        coords = np.array([t["G"]["C"] for t in poly])
        labels = [t["P"]["N"] for t in poly]
        pops = [t["P"]["P"] for t in poly]
        x_min, y_min, x_max, y_max = extent
        rotation = makeRotationForCoord(*origin)
        pos = project(np.radians(coords), rotation)
        rr = np.hypot(pos[:, 0], pos[:, 1])
        inside = np.logical_and(
            np.logical_and(pos[:, 0] > x_min, pos[:, 0] < x_max), np.logical_and(pos[:, 1] > y_min, pos[:, 1] < y_max)
        )
        popped = [
            2500 * np.log(p) / p < density if p > 0 else r < 100 if density > 4 else r < 10 or density > 99
            for p, r in zip(pops, rr)
        ]
        show = np.logical_and(inside, popped)
        text = [(*pos[:2], colors.text, label, pop) for pos, label, pop, s in zip(pos, labels, pops, show) if s]
        return text
    else:
        raise ValueError(f"Unknown polygon format: {type(poly)}")


def radii(max_range=70.0, about=7):
    deltas = [1, 2, 5, 10, 20, 25, 50, 100, 200, 250, 500, 1000]
    k = np.argmin(max_range / np.array(deltas) >= about)
    delta = deltas[k]
    return np.array([1.0, *np.arange(delta, max_range, delta)])


class Grid:
    path_effects = [matplotlib.patheffects.Stroke(linewidth=2.5, foreground=(0.0, 0.0, 0.0, 0.6)), matplotlib.patheffects.Normal()]
    label_props = {
        "fontproperties": blib.getFontOfWeight(weight=600),
        "horizontalalignment": "center",
        "verticalalignment": "center",
        "path_effects": path_effects,
    }
    s = 1
    size1 = 12
    size2 = 14
    size3 = 16


"""
    extent in (xmin, ymin, xmax, ymax)
"""


class Overlay(Grid):
    density = 4.0
    rmax = 70.0
    labels = None
    county = None
    highway = None
    rings = None

    def __init__(self, origin=(-97.46381, 35.23682), extent=(-160, -90, 160, 90), **kwargs):
        self.origin = origin
        self.extent = extent
        if "density" in kwargs:
            self.density = kwargs["density"]
        if "rmax" in kwargs:
            self.rmax = kwargs["rmax"]
        if "s" in kwargs:
            self.s = kwargs["s"]
            self.size1 = 12 * self.s
            self.size2 = 14 * self.s
            self.size3 = 16 * self.s
        # TODO: Calculate "about" from extent and density
        self.radii = radii(max_range=self.rmax, about=7)

    def _pop2size(self, pop):
        return self.size3 if pop > pop_big else self.size2 if pop > pop_med else self.size1

    def load(self):
        self.labels = get("city", self.origin, self.extent, self.density)
        self.county = get("county", self.origin, self.extent)
        self.highway = get("highway", self.origin, self.extent)
        name = "@ring/" + "/".join([f"{r:.0f}" for r in self.radii])
        self.rings = get(name, self.origin, self.extent)
        # Append ring labels to self.labels
        c, s = np.cos(np.pi / 4), np.sin(np.pi / 4)
        for r in self.radii[1:]:
            label = f"{r:.0f} km"
            self.labels.append((r * c, r * s, colors.ring, label, 1000))
            self.labels.append((-r * c, -r * s, colors.ring, label, 1000))
        self.labels.sort(key=lambda x: x[-1], reverse=True)

    def exclude(self, extent):
        removals = []
        for label in self.labels:
            x, y, _, _, _ = label
            if x < extent[0] or y < extent[1] or x > extent[2] or y > extent[3]:
                continue
            removals.append(label)
        for label in removals:
            self.labels.remove(label)

    def draw(self, ax):
        ax.set(xlim=self.extent[::2], ylim=self.extent[1::2], xticks=[], yticks=[])
        linewidth = 1.5 * self.s
        for p in self.county:
            ax.add_line(matplotlib.lines.Line2D(p[:, 0], p[:, 1], color=colors.county, linewidth=linewidth))
        for p in self.highway:
            ax.add_line(matplotlib.lines.Line2D(p[:, 0], p[:, 1], color=colors.highway, linewidth=linewidth))
        for p in self.rings:
            ax.add_line(matplotlib.lines.Line2D(p[:, 0], p[:, 1], color=colors.ring, linewidth=self.s))
        occupied = []
        for x, y, c, label, pop in self.labels:
            size = self._pop2size(pop)
            text = matplotlib.text.Text(x, y, label, color=c, fontsize=size, **self.label_props)
            ax.add_artist(text)
            rect = text.get_window_extent().padded(10)
            overlap = False
            for o in occupied:
                if o.overlaps(rect):
                    overlap = True
                    break
            if overlap:
                text.remove()
            else:
                occupied.append(rect)


"""
    extent in (rmin, emin, rmax, emax)
"""


class PolarGrid(Grid):
    extent = (0, 0, 60, 40)
    ymax = 20
    grids = []
    labels = []

    def __init__(self, **kwargs):
        if "extent" in kwargs:
            self.extent = kwargs["extent"]
        if "s" in kwargs:
            self.s = kwargs["s"]
            self.size1 = 12 * self.s
            self.size2 = 14 * self.s
            self.size3 = 16 * self.s
        if "ymax" in kwargs:
            self.ymax = kwargs["ymax"]
        self.radii = radii(max_range=self.extent[2], about=5)

    def load(self):
        e_max = self.extent[3]
        self.grids = []
        self.ee = np.radians(np.arange(0, e_max, 2))
        xx = np.outer(np.cos(self.ee), self.radii)
        yy = np.outer(np.sin(self.ee), self.radii)

        # Range rings
        for k in range(xx.shape[1]):
            x = xx[:, k]
            y = yy[:, k]
            self.grids.append((x, y))
        # Elevation lines
        for e in np.arange(10, e_max, 10):
            e_rad = np.radians(e)
            cc = np.cos(e_rad)
            ss = np.sin(e_rad)
            x = [cc, cc * self.radii[-1]]
            y = [ss, ss * self.radii[-1]]
            self.grids.append((x, y))
        # Labels
        self.labels = []
        cc = np.cos(np.pi / 6)
        ss = np.sin(np.pi / 6)
        for r in self.radii[1:]:
            x = r * cc
            y = r * ss
            if y > 0.85 * self.ymax:
                break
            self.labels.append((x, y, f"{r:.0f} km"))

    def draw(self, ax):
        ax.set(xlim=[0, self.extent[2]], ylim=[0, self.ymax], xticks=[], yticks=[])
        for x, y in self.grids:
            line = matplotlib.lines.Line2D(x, y, color=colors.ring, linewidth=self.s)
            ax.add_artist(line)
        for x, y, label in self.labels:
            text = matplotlib.text.Text(x, y, label, color=colors.ring, fontsize=self.size1, **self.label_props)
            ax.add_artist(text)
