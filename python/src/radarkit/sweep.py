import os
import re
import radar
import tarfile
import netCDF4
import datetime
import numpy as np

re_3parts = re.compile(
    r"(?P<name>.+)-"
    + r"(?P<time>20[0-9][0-9](0[0-9]|1[012])([0-2][0-9]|3[01])-([01][0-9]|2[0-3])[0-5][0-9][0-5][0-9])-"
    + r"(?P<scan>[EAN][0-9]+\.[0-9]+)"
)


class MeshCoordinate:
    e = [0]
    a = [0]
    r = [0]

    def __repr__(self):
        return f"MeshCoordinate: {len(self.e)} x {len(self.a)} x {len(self.r)}"


class Sweep:
    def __init__(self, input=None, verbose=0, **kwargs):
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
        self.time = None
        self.longitude = -97.0
        self.latitude = 32.0
        self.scanType = "UNK"
        self.sweepElevation = 0.0
        self.sweepAzimuth = 0.0
        self.prf = 1
        self.waveform = "u"
        self.gatewidth = 1.0
        self.elevations = 0.0
        self.azimuths = 0.0
        self.products = {}
        self.archive = None
        self.meshCoordinate = MeshCoordinate()
        if input is None:
            return
        if isinstance(input, str) and os.path.exists(input):
            self.read(input, verbose=verbose)
        elif isinstance(input, np.ndarray) and len(input.shape) == 3 and input.shape[0] == 6:
            self.plain(input, **kwargs)

    def __repr__(self):
        a = (
            self.sweepElevation
            if self.scanType.lower() == "ppi"
            else self.sweepAzimuth if self.scanType.lower() == "rhi" else -999.0
        )
        return f"Sweep: {self.scanType} {a:.1f} - {list(self.products.keys())}"

    def __str__(self):
        return f"Sweep: {self.scanType} of {self.archive}"

    def read(self, input, verbose=0):
        data = radar.read(input, verbose=verbose)
        if data is None:
            return None
        self.archive = input
        self.time = datetime.datetime.fromtimestamp(data["time"], tz=datetime.timezone.utc)
        self.longitude = data["longitude"]
        self.latitude = data["latitude"]
        # Scan type from filename
        parts = radar.re_3parts.match(os.path.basename(input))
        if parts:
            parts = parts.groupdict()
            if parts["scan"][0] == "E":
                self.scanType = "PPI"
            elif parts["scan"][0] == "A":
                self.scanType = "RHI"
        self.sweepElevation = data["sweepElevation"]
        self.sweepAzimuth = data["sweepAzimuth"]
        self.prf = data["prf"]
        self.waveform = data["waveform"]
        self.gatewidth = data["gatewidth"]
        self.elevations = data["elevations"]
        self.azimuths = data["azimuths"]
        self.products = data["products"]
        mask = self.products["Z"] < -999.0
        for key in self.products.keys():
            self.products[key][mask] = np.nan
        # Generate coordinate arrays that are 1 element extra than each dimension
        if self.scanType.lower() == "rhi":
            de = self.elevations[-1] - self.elevations[-2]
            self.meshCoordinate.e = [*self.elevations, self.elevations[-1] + de]
        elif self.scanType.lower() == "ppi":
            da = self.azimuths[-1] - self.azimuths[-2]
            self.meshCoordinate.a = [*self.azimuths, self.azimuths[-1] + da]
        self.meshCoordinate.r = np.arange(self.products["Z"].shape[1] + 1) * 1.0e-3 * self.gatewidth

    def plain(self, array, **kwargs):
        symbols = kwargs["symbols"] if "symbols" in kwargs else ["Z", "V", "W", "D", "P", "R"]
        for i, symbol in enumerate(symbols):
            self.products.update({symbol: array[i]})
        shape = self.products["Z"].shape
        if "scanType" in kwargs or "type" in kwargs:
            self.scanType = kwargs["scanType"] if "scanType" in kwargs else kwargs["type"]
        else:
            if self.products["Z"].shape[0] < 90:
                self.scanType = "RHI"
            else:
                self.scanType = "PPI"
        if "gatewidth" in kwargs:
            self.gatewidth = kwargs["gatewidth"]
        if "e" in kwargs:
            self.meshCoordinate.e = kwargs["e"]
        if "a" in kwargs:
            self.meshCoordinate.a = kwargs["a"]
        if "r" in kwargs:
            self.meshCoordinate.r = kwargs["r"]
        if self.scanType.lower() == "rhi":
            if len(self.meshCoordinate.e) != shape[0] + 1:
                self.meshCoordinate.e = np.arange(shape[0] + 1, dtype=float)
                if "de" in kwargs:
                    self.meshCoordinate.e *= kwargs["de"]
            if len(self.meshCoordinate.r) != shape[1] + 1:
                self.meshCoordinate.r = np.arange(shape[1] + 1, dtype=float) * 1.0e-3 * self.gatewidth
                if "dr" in kwargs:
                    self.meshCoordinate.r *= kwargs["dr"]
        elif self.scanType.lower() == "ppi":
            if len(self.meshCoordinate.a) != shape[0] + 1:
                self.meshCoordinate.a = np.arange(shape[0] + 1)
                if "da" in kwargs:
                    self.meshCoordinate.a *= kwargs["da"]
            if len(self.meshCoordinate.r) != shape[1] + 1:
                self.meshCoordinate.r = np.arange(shape[1] + 1) * 1.0e-3 * self.gatewidth
                if "dr" in kwargs:
                    self.meshCoordinate.r *= kwargs["dr"]
