import os
import tarfile
import netCDF4
import datetime
import numpy as np


class MeshCoordinate:
    e = [0]
    a = [0]
    r = [0]

    def __repr__(self):
        return f"MeshCoordinate: {len(self.e)} x {len(self.a)} x {len(self.r)}"


class Sweep:
    def __init__(self, input=None, verbose=0):
        self.time = None
        self.longitude = -97.0
        self.latitude = 32.0
        self.scanType = "UNK"
        self.scanElevation = None
        self.scanAzimuth = None
        self.prf = 1
        self.waveform = "u"
        self.gatewidth = 1.0
        self.elevation = 0.0
        self.azimuths = 0.0
        self.products = {}
        self.archive = None
        self.meshCoordinate = MeshCoordinate()
        if input:
            self.read(input, verbose=verbose)

    def __repr__(self):
        scanAngle = (
            self.scanElevation if self.scanType.lower() == "ppi" else self.scanAzimuth if self.scanType.lower() == "rhi" else -999.0
        )
        return f"Sweep: {self.scanType} {scanAngle:.1f}° - {list(self.products.keys())}"

    def __str__(self):
        return f"Sweep: {self.sweep} of {self.archive}"

    def read(self, input, verbose=0):
        path, basename = os.path.split(input)
        _, ext = os.path.splitext(basename)

        def _handle_fid(fid, symbol="Z"):
            if symbol == "Z":
                with netCDF4.Dataset("memory", mode="r", memory=fid.read()) as nc:
                    atts = nc.ncattrs()
                    name = nc.getncattr("TypeName")
                    self.time = datetime.datetime.utcfromtimestamp(nc.getncattr("Time"))
                    self.longitude = nc.getncattr("Longitude")
                    self.latitude = nc.getncattr("Latitude")
                    self.scanElevation = nc.getncattr("Elevation")
                    self.scanAzimuth = nc.getncattr("Azimuth")
                    self.prf = float(round(nc.getncattr("PRF-value") * 0.1) * 10.0)
                    self.waveform = nc.getncattr("Waveform") if "Waveform" in atts else ""
                    self.gatewidth = float(nc.variables["GateWidth"][:][0])
                    self.elevations = np.array(nc.variables["Elevation"][:], dtype=np.float32)
                    self.azimuths = np.array(nc.variables["Azimuth"][:], dtype=np.float32)
                    self.products.update({symbol: np.array(nc.variables[name][:], dtype=np.float32)})
            else:
                with netCDF4.Dataset("memory", mode="r", memory=fid.read()) as nc:
                    name = nc.getncattr("TypeName")
                    self.products.update({symbol: np.array(nc.variables[name][:], dtype=np.float32)})

        parts = basename.split("-")
        if parts[3].startswith("A"):
            self.scanType = "RHI"
        elif parts[3].startswith("E"):
            self.scanType = "PPI"
        else:
            self.scanType = "UNK"

        # Read from a tar archive
        if ext in [".tar", ".tgz", ".xz"]:
            self.archive = input
            with tarfile.open(input) as tar:
                for member in tar.getmembers():
                    if not member.isfile():
                        continue
                    file, _ = os.path.splitext(os.path.basename(member.name))
                    parts = file.split("-")
                    symbol = parts[4]
                    with tar.extractfile(member) as fid:
                        _handle_fid(fid, symbol)
        # Read from netCDF file
        elif ext == ".nc":
            symbols = ["Z", "V", "W", "D", "P", "R"]
            for symbol in symbols:
                parts[-1] = f"{symbol}.nc"
                basename = "-".join(parts)
                filename = os.path.join(path, basename)
                if verbose:
                    print(f"filename: {filename}")
                with open(filename, mode="rb") as fid:
                    _handle_fid(fid, symbol)
        else:
            print(f"Unknown file extension: {ext}")
            return None

        # Generate coordinate arrays that are 1 element extra than each dimension
        if self.scanType.lower() == "rhi":
            de = self.elevations[-1] - self.elevations[-2]
            self.meshCoordinate.e = [*self.elevations, self.elevations[-1] + de]
        elif self.scanType.lower() == "ppi":
            da = self.azimuths[-1] - self.azimuths[-2]
            self.meshCoordinate.a = [*self.azimuths, self.azimuths[-1] + da]
        self.meshCoordinate.r = np.arange(self.products["Z"].shape[1] + 1) * 1.0e-3 * self.gatewidth
        symbols = ["Z", "V", "W", "D", "P", "R"]
        mask = self.products["Z"] < -999.0
        for key in symbols:
            self.products[key][mask] = np.nan
