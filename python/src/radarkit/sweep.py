import os
import re
import radar
import datetime
import numpy as np

from ._ctypes_ import RKProductCollectionInitWithFilename

re_3parts = re.compile(
    r"(?P<name>.+)-"
    + r"(?P<time>20[0-9][0-9](0[0-9]|1[012])([0-2][0-9]|3[01])-([01][0-9]|2[0-3])[0-5][0-9][0-5][0-9])-"
    + r"(?P<scan>[EAN][0-9]+\.[0-9])"
)


def wrapDegrees(angle):
    return (angle + 180.0) % 360.0 - 180.0


class MeshCoordinate:
    e = [0]
    a = [0]
    r = [0]
    x = [0]
    y = [0]
    z = [0]
    kind = "polar"

    def __repr__(self):
        if self.kind == "cartesian":
            return f"MeshCoordinate({len(self.z)} x {len(self.y)} x {len(self.x)})"
        elif self.kind == "polar":
            return f"MeshCoordinate({len(self.e)} x {len(self.a)} x {len(self.r)})"


symbolMapping = {"DBZ": "Z", "VEL": "V", "WIDTH": "W", "ZDR": "D", "PHIDP": "P", "RHOHV": "R"}


class Sweep:
    archive = None
    prefix = None
    time = None
    longitude = -97.0
    latitude = 32.0
    scanType = "unk"
    sweepElevation = 0.0
    sweepAzimuth = 0.0
    prf = 1
    waveform = "u"
    gatewidth = 1.0
    elevations = 0.0
    azimuths = 0.0
    products = {}
    meshCoordinate = MeshCoordinate()

    def __init__(self, source=None, **kwargs):
        """
        Sweep class to handle radar data

        Parameters
        ----------
        input : str or np.ndarray
            If str, it is the path to a file or a tar archive containing radar data.
            Currently support RadarKit format only.

            If np.ndarray, it is a 3D array of radar data with shape (6, n, m) where
            n is the number of elevation or azimuth angles and m is the number of range gates.

        Returns
        -------
        A Sweep object
        """
        if source is None:
            return
        if isinstance(source, np.ndarray) and source.ndim == 3 and (source.shape[0] == 6 or "symbols" in kwargs):
            self.plain(source, **kwargs)
        elif isinstance(source, str) and os.path.exists(source):
            self.file(source)

    def __repr__(self):
        a = (
            self.sweepElevation
            if self.scanType.lower() == "ppi"
            else self.sweepAzimuth if self.scanType.lower() == "rhi" else -999.0
        )
        return f"Sweep: {self.scanType} {a:.1f} - {list(self.products.keys())}"

    def _get_prefix(self):
        if self.archive is None:
            return
        basename = os.path.basename(self.archive)
        if "_V06" in basename:
            self.prefix = basename[:4]
        else:
            self.prefix = basename.split("-")[0]

    def file(self, filename):
        collection = RKProductCollectionInitWithFilename(filename).contents
        product = collection.products[0]
        self.archive = filename
        self._get_prefix()
        self.time = datetime.datetime.fromtimestamp(product.header.startTime, tz=datetime.timezone.utc)
        self.longitude = product.header.longitude
        self.latitude = product.header.latitude
        self.scanType = "ppi" if product.header.isPPI else "rhi" if product.header.isRHI else "unk"
        self.sweepElevation = product.header.sweepElevation
        self.sweepAzimuth = product.header.sweepAzimuth
        self.prf = 1.0 / product.header.prt[0]
        self.waveform = str(product.header.waveformName)
        self.gatewidth = product.header.gateSizeMeters
        self.elevations = np.ctypeslib.as_array(product.startElevation, (product.header.rayCount,))
        self.azimuths = np.ctypeslib.as_array(product.startAzimuth, (product.header.rayCount,))
        self.products = {}
        for k in range(collection.count):
            product = collection.products[k]
            symbol = str(product.desc.symbol.decode("utf-8"))
            symbol = symbolMapping.get(symbol, symbol)
            values = np.ctypeslib.as_array(product.data, (product.header.rayCount, product.header.gateCount))
            self.products.update({symbol: values})
        # Generate coordinate arrays that are 1 element extra than each dimension
        if self.scanType == "rhi":
            de = self.elevations[-1] - self.elevations[-2]
            self.meshCoordinate.e = [*self.elevations, self.elevations[-1] + de]
        elif self.scanType == "ppi":
            da = self.azimuths[-1] - self.azimuths[-2]
            self.meshCoordinate.a = [*self.azimuths, self.azimuths[-1] + da]
        self.meshCoordinate.r = np.arange(self.products["Z"].shape[1] + 1) * self.gatewidth

    def plain(self, array, **kwargs):
        symbols = kwargs.get("symbols", ["Z", "V", "W", "D", "P", "R"])
        self.products = {}
        for i, symbol in enumerate(symbols):
            self.products.update({symbol: array[i, :, :]})
        self.scanType = kwargs["scanType"] if "scanType" in kwargs else kwargs.get("type", "unk")
        self.archive = kwargs.get("archive", None)
        self._get_prefix()
        if "time" in kwargs:
            self.time = kwargs["time"]
            if isinstance(self.time, float) or isinstance(self.time, int):
                self.time = datetime.datetime.fromtimestamp(self.time, tz=datetime.timezone.utc)
        if "latitude" in kwargs:
            self.latitude = kwargs["latitude"]
        if "longitude" in kwargs:
            self.longitude = kwargs["longitude"]
        if "gatewidth" in kwargs:
            self.gatewidth = kwargs["gatewidth"]
        elif "dr" in kwargs:
            self.gatewidth = kwargs["dr"]
        if "e" in kwargs:
            self.meshCoordinate.e = kwargs["e"]
            if len(self.meshCoordinate.e) == array.shape[1]:
                de = self.meshCoordinate.e[-1] - self.meshCoordinate.e[-2]
                self.meshCoordinate.e = np.append(self.meshCoordinate.e, self.meshCoordinate.e[-1] + de)
        if "a" in kwargs:
            self.meshCoordinate.a = kwargs["a"]
            if len(self.meshCoordinate.a) == array.shape[1]:
                da = self.meshCoordinate.a[-1] - self.meshCoordinate.a[-2]
                self.meshCoordinate.a = np.append(self.meshCoordinate.a, wrapDegrees(self.meshCoordinate.a[-1] + da))
        if "r" in kwargs:
            self.meshCoordinate.r = kwargs["r"]
            if len(self.meshCoordinate.r) == array.shape[2]:
                dr = self.meshCoordinate.r[-1] - self.meshCoordinate.r[-2]
                self.meshCoordinate.r = np.append(self.meshCoordinate.r, self.meshCoordinate.r[-1] + dr)
        if "x" in kwargs:
            self.meshCoordinate.kind = "cartesian"
            self.meshCoordinate.x = kwargs["x"]
            if len(self.meshCoordinate.x) == array.shape[2]:
                dx = self.meshCoordinate.x[-1] - self.meshCoordinate.x[-2]
                self.meshCoordinate.x = np.append(self.meshCoordinate.x, self.meshCoordinate.x[-1] + dx)
        if "y" in kwargs:
            self.meshCoordinate.kind = "cartesian"
            self.meshCoordinate.y = kwargs["y"]
            if len(self.meshCoordinate.y) == array.shape[1]:
                dy = self.meshCoordinate.y[-1] - self.meshCoordinate.y[-2]
                self.meshCoordinate.y = np.append(self.meshCoordinate.y, self.meshCoordinate.y[-1] + dy)
        # Fix the meshCoordinate arrays to have one extra element in each dimension
        if self.meshCoordinate.kind == "polar":
            if len(self.meshCoordinate.e) != array.shape[1] + 1:
                self.meshCoordinate.e = np.arange(array.shape[1] + 1, dtype=float) * kwargs.get("de", 1.0)
            if len(self.meshCoordinate.a) != array.shape[1] + 1:
                self.meshCoordinate.a = wrapDegrees(np.arange(array.shape[1] + 1, dtype=float) * kwargs.get("da", 1.0))
            if len(self.meshCoordinate.r) != array.shape[2] + 1:
                self.meshCoordinate.r = np.arange(array.shape[2] + 1, dtype=float) * self.gatewidth
        elif self.meshCoordinate.kind == "cartesian":
            if len(self.meshCoordinate.x) != array.shape[2] + 1:
                self.meshCoordinate.x = np.arange(array.shape[2] + 1, dtype=float) * kwargs.get("dx", 1.0)
            if len(self.meshCoordinate.y) != array.shape[1] + 1:
                self.meshCoordinate.y = np.arange(array.shape[1] + 1, dtype=float) * kwargs.get("dy", 1.0)
        # TODO: Think about z
        self.elevations = self.meshCoordinate.e[:-1]
        self.azimuths = self.meshCoordinate.a[:-1]
        self.range = self.meshCoordinate.r[:-1]
        self.sweepElevation = kwargs.get("sweepElevation", np.mean(self.elevations))
        self.sweepAzimuth = kwargs.get("sweepAzimuth", np.mean(self.azimuths))
        if self.scanType == "unk":
            ib = int(0.2 * len(self.azimuths))
            ie = int(0.8 * len(self.azimuths))
            if ie - ib > 10:
                if self.elevations[ib:ie].std() < 1.0:
                    self.scanType = "ppi"
                elif self.azimuths[ib:ie].std() < 1.0:
                    self.scanType = "rhi"
