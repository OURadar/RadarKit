from . import sweep
from . import overlay

import numpy as np
import matplotlib.patheffects
import matplotlib.pyplot as plt

import blib

zmap = blib.matplotlibColormap("rsz")
vmap = blib.matplotlibColormap("v")
wmap = blib.matplotlibColormap("w")
dmap = blib.matplotlibColormap("rsd")
pmap = blib.matplotlibColormap("p")
rmap = blib.matplotlibColormap("rsr")


def rho2ind(values):
    m3 = values > 0.93
    m2 = np.logical_and(values > 0.7, ~m3)
    index = values * 52.8751
    index[m2] = values[m2] * 300.0 - 173.0
    index[m3] = values[m3] * 1000.0 - 824.0
    return index


def shade(shape, xy=(0.5, 0.5), rgba=[0, 0, 0, 0.5], direction="southeast"):
    if direction == "n" or direction == "north":
        x = np.ones((shape[1],))
        y = blib.colormap.fleximap(shape[0], [0, xy[1], 1], [[1], [1], [0]])
        a = np.outer(y, x)
    elif direction == "ne" or direction == "northeast":
        x = blib.colormap.fleximap(shape[1], [0, xy[0], 1], [[0], [1], [1]])
        y = blib.colormap.fleximap(shape[0], [0, xy[1], 1], [[1], [1], [0]])
        a = np.outer(y, x)
    elif direction == "e" or direction == "east":
        x = blib.colormap.fleximap(shape[1], [0, xy[0], 1], [[0], [1], [1]])
        y = np.ones((shape[0],))
        a = np.outer(y, x)
    elif direction == "se" or direction == "southeast":
        x = blib.colormap.fleximap(shape[1], [0, xy[0], 1], [[0], [1], [1]])
        y = blib.colormap.fleximap(shape[0], [0, xy[1], 1], [[0], [1], [1]])
        a = np.outer(y, x)
    elif direction == "s" or direction == "south":
        x = np.ones((shape[1],))
        y = blib.colormap.fleximap(shape[0], [0, xy[1], 1], [[0], [1], [1]])
        a = np.outer(y, x)
    elif direction == "sw" or direction == "southwest":
        x = blib.colormap.fleximap(shape[1], [0, xy[0], 1], [[1], [1], [0]])
        y = blib.colormap.fleximap(shape[0], [0, xy[1], 1], [[0], [1], [1]])
        a = np.outer(y, x)
    elif direction == "w" or direction == "west":
        x = blib.colormap.fleximap(shape[1], [0, xy[0], 1], [[1], [1], [0]])
        y = np.ones((shape[0],))
        a = np.outer(y, x)
    elif direction == "nw" or direction == "northwest":
        x = blib.colormap.fleximap(shape[1], [0, xy[0], 1], [[1], [1], [0]])
        y = blib.colormap.fleximap(shape[0], [0, xy[1], 1], [[1], [1], [0]])
        a = np.outer(y, x)
    else:
        x = blib.colormap.fleximap(shape[1], [0, xy[0], 1], [[0], [1], [0]])
        y = blib.colormap.fleximap(shape[0], [0, xy[1], 1], [[0], [1], [0]])
        a = np.outer(y, x)
    m = np.empty((*shape, 4))
    m[:, :, :] = rgba
    m[:, :, 3] *= a
    return m


class Chart:
    fig = None
    dpi = 72
    s = 1.0
    titlecolor = None
    orientation = "horizontal"
    ax = [None] * 6
    cb = [None] * 6
    st = [None] * 6
    r = None
    e = None
    a = None
    overlay = None
    labelfont = blib.getFontOfWeight(weight=500)
    titlefont = blib.getFontOfWeight(weight=700)

    def __init__(self, size=(1280, 720), **kwargs):
        self.size = size
        if "dpi" in kwargs:
            self.dpi = kwargs["dpi"]
        if "s" in kwargs:
            self.s = kwargs["s"]
        if "titlecolor" in kwargs:
            self.titlecolor = kwargs["titlecolor"]
        if "orientation" in kwargs:
            self.orientation = kwargs["orientation"]
        self.m = 5 * self.s
        self.p = 10 * self.s
        self.labelsize = 12 * self.s
        self.titlesize = 28 * self.s
        self.captionsize = 14 * self.s

        width, height = self.size
        self.figsize = (width / self.dpi, height / self.dpi)

        color = matplotlib.rcParams["text.color"]
        if color in matplotlib.colors.CSS4_COLORS:
            rgba = matplotlib.colors.to_rgba(matplotlib.colors.CSS4_COLORS[color])
            hsv = matplotlib.colors.rgb_to_hsv(rgba[:3])
        elif color[0] == "#":
            rgba = matplotlib.colors.to_rgba(color)
            hsv = matplotlib.colors.rgb_to_hsv(rgba[:3])
        else:
            hsv = (0, 0, 1)
        if hsv[2] > 0.5:
            self.gridcolor = (0.2, 1.0, 1.0)
            self.titlecolor = (0.8, 0.8, 0.8) if self.titlecolor is None else self.titlecolor
        else:
            self.gridcolor = (0, 0.4, 0.4)
            self.titlecolor = (0.2, 0.2, 0.2) if self.titlecolor is None else self.titlecolor
        self.path_effects = [
            matplotlib.patheffects.Stroke(linewidth=2 * self.s, foreground=matplotlib.rcParams["axes.facecolor"]),
            matplotlib.patheffects.Normal(),
        ]
        self.figprops = {
            "axes.linewidth": max(1, self.s // 1),
            "axes.axisbelow": False,
            "axes.facecolor": (1, 1, 1, 0.85) if hsv[2] < 0.5 else (0, 0, 0, 0.85),
            "axes.titlepad": 10 * self.s,
            "axes.labelpad": 6 * self.s,
            "axes.linewidth": max(1, self.s // 1),
            "xtick.direction": "in",
            "xtick.major.pad": 8 * self.s,
            "xtick.major.size": round(3 * self.s),
            "xtick.major.width": max(1, self.s // 1),
            "xtick.labelsize": self.labelsize,
            "ytick.direction": "in",
            "ytick.major.pad": 8 * self.s,
            "ytick.major.size": round(3 * self.s),
            "ytick.major.width": max(1, self.s // 1),
            "ytick.labelsize": self.labelsize,
        }

    def _get_pos(self, num):
        # Available space for axes
        width, height = self.size
        ww = width
        hh = height - self.titlesize - self.m
        cols = num // 100
        rows = (num - cols * 100) // 10
        w = round((ww - (cols + 1) * self.m) / cols) / width
        h = round((hh - (rows + 1) * self.m) / rows) / height
        i = num % 10
        if cols < rows:
            x = (i - 1) // rows
            y = rows - 1 - (i - 1) % rows
        else:
            x = (i - 1) % cols
            y = rows - 1 - (i - 1) // cols
        # print(f'x = {x}  y = {y}  w = {w}  h = {h}')
        x = (x * w) + (x + 1) * self.m / width
        y = (y * h) + (y + 1) * self.m / height
        return [x, y, w, h]

    def _draw_box(self, q, xoff=0, yoff=0, c=matplotlib.rcParams["text.color"], b=matplotlib.rcParams["axes.facecolor"]):
        width, height = self.size
        x0 = q[0]
        x1 = x0 + 10 / width
        x2 = q[0] + q[2]
        y0 = q[1]
        y1 = q[1] + q[3]
        x0 += xoff / width
        y0 += yoff / height
        x = [x1, x2, x2, x0, x0, x1]
        y = [y0, y0, y1, y1, y0, y0]
        line = matplotlib.lines.Line2D(x, y, color=c, linewidth=self.s, solid_joinstyle="miter")
        rect = matplotlib.patches.Rectangle((q[0], q[1]), q[2], q[3], color=b, zorder=-1)
        self.fig.add_artist(line)
        self.fig.add_artist(rect)

    def _add_axes(self, num):
        width, height = self.size
        q = self._get_pos(num)
        ax = self.fig.add_axes(q, frameon=False, snap=True)
        ax.set(xticks=[], yticks=[])
        self._draw_box(q)
        t = 10 * self.s
        p = self.p
        f = [0, 0, 0, 0.0]
        g = [0, 0, 0, 0.5]
        n = 50
        if self.orientation[:4] == "vert":
            # Background shade, from right: p, 3 * labelsize, p, t, p, captionsize, 2p
            w = 5 * p + 3 * self.labelsize + t + self.captionsize
            bq = [
                q[0] + q[2] - w / width,
                q[1],
                w / width,
                q[3],
            ]
            z = shade((80, 50), (0.4, 0.5), [0, 0, 0, 0.5], direction="se")
            # Colorbar
            c = min(height - 4 * p, 512 * self.s) / height
            w = 3 * self.captionsize + t
            cq = [
                q[0] + q[2] - w / width,
                q[1] + 2 * p / height,
                t / width,
                c * q[3],
            ]
        else:
            c = min(width - 4 * p, 512 * self.s) / width
            # Background shade, from top: p, captionsize, p, t, p, labelsize, 2p
            h = 5 * p + self.captionsize + t + self.labelsize
            bq = [
                q[0],
                q[1] + q[3] - h / height,
                q[2],
                h / height,
            ]
            z = shade((50, 80), (0.4, 0.5), [0, 0, 0, 0.5], direction="nw")
            # Colorbar
            h = 2 * p + self.captionsize + t
            cq = [
                q[0] + 2 * p / width,
                q[1] + q[3] - h / height,
                c * q[2],
                t / height,
            ]
        # Axis for background shade
        bx = self.fig.add_axes(bq, frameon=False, snap=True)
        bx.imshow(z, aspect="auto")
        bx.set(xticks=[], yticks=[])
        # Axis for colorbar
        cb = self.fig.add_axes(cq, frameon=False, snap=True)
        cb.set(xticks=[], yticks=[])
        self._draw_box(cq, xoff=-1)
        if self.orientation[:4] == "vert":
            st = matplotlib.text.Text(cq[0] - (self.captionsize + p) / width, cq[1] + 0.5 * cq[3], f"{num}")
            st.set(
                fontproperties=self.labelfont,
                horizontalalignment="center",
                path_effects=self.path_effects,
                rotation=self.orientation,
                size=self.captionsize,
                verticalalignment="center",
            )
            self.fig.add_artist(st)
        else:
            st = cb.set_title(f"{num}", fontproperties=self.labelfont)
            st.set(size=self.captionsize, path_effects=self.path_effects)
        return ax, cb, st

    def _update_data_only(self, sweep: sweep.Sweep):
        self.mz.set_array(sweep.products["Z"].ravel())
        self.mv.set_array(sweep.products["V"].ravel())
        self.mw.set_array(sweep.products["W"].ravel())
        self.md.set_array(sweep.products["D"].ravel())
        self.mp.set_array(sweep.products["P"].ravel())
        self.mr.set_array(rho2ind(sweep.products["R"]).ravel())

    def _update_coordinate_data(self, xx, yy, sweep: sweep.Sweep):
        if self.overlay:
            self.mz.remove()
            self.mv.remove()
            self.mw.remove()
            self.md.remove()
            self.mp.remove()
            self.mr.remove()

        self.mz = self.ax[0].pcolormesh(xx, yy, sweep.products["Z"], shading="flat", cmap=zmap, vmin=-32, vmax=96)
        self.mv = self.ax[1].pcolormesh(xx, yy, sweep.products["V"], shading="flat", cmap=vmap, vmin=-64, vmax=64)
        self.mw = self.ax[2].pcolormesh(xx, yy, sweep.products["W"], shading="flat", cmap=wmap, vmin=0, vmax=12.8)
        self.md = self.ax[3].pcolormesh(xx, yy, sweep.products["D"], shading="flat", cmap=dmap, vmin=-10, vmax=15.5)
        self.mp = self.ax[4].pcolormesh(xx, yy, sweep.products["P"], shading="flat", cmap=pmap, vmin=-np.pi, vmax=np.pi)
        self.mr = self.ax[5].pcolormesh(xx, yy, rho2ind(sweep.products["R"]), shading="flat", cmap=rmap, vmin=0, vmax=256)
        self.r = sweep.meshCoordinate.r
        self.a = sweep.meshCoordinate.a
        self.e = sweep.meshCoordinate.e

    def _setup_colorbars(self):
        # Colorbars
        plt.colorbar(self.mz, ax=self.ax[0], cax=self.cb[0], orientation=self.orientation)
        plt.colorbar(self.mv, ax=self.ax[1], cax=self.cb[1], orientation=self.orientation)
        plt.colorbar(self.mw, ax=self.ax[2], cax=self.cb[2], orientation=self.orientation)
        plt.colorbar(self.md, ax=self.ax[3], cax=self.cb[3], orientation=self.orientation)
        plt.colorbar(self.mp, ax=self.ax[4], cax=self.cb[4], orientation=self.orientation)
        plt.colorbar(self.mr, ax=self.ax[5], cax=self.cb[5], orientation=self.orientation)
        # Colorbar labels
        self.st[0].set_text("Z - Reflectivity (dBZ)")
        self.st[1].set_text("V - Velocity (m/s)")
        self.st[2].set_text("W - Spectrum Width (m/s)")
        self.st[3].set_text("D - Differential Reflectivity (dB)")
        self.st[4].set_text("P - Differential Phase (°)")
        self.st[5].set_text("R - Correlation Coefficient")

        # Colorbar ticks
        tick_props = {
            "fontproperties": self.labelfont,
            "fontsize": self.labelsize,
            "path_effects": self.path_effects,
        }

        def setup_ticks(k, ticks, ticklabels, lo, hi):
            if self.orientation == "vertical":
                self.cb[k].set_yticks(ticks, labels=ticklabels, **tick_props)
                self.cb[k].set_ylim(lo, hi)
            else:
                self.cb[k].set_xticks(ticks, labels=ticklabels, **tick_props)
                self.cb[k].set_xlim(lo, hi)

        ticks = np.arange(-20, 61, 20)
        setup_ticks(0, ticks, ticks, -10, 75)
        ticks = np.arange(-20, 21, 10)
        setup_ticks(1, ticks, ticks, -30, 30)
        ticks = np.arange(2, 9, 2)
        setup_ticks(2, ticks, ticks, 0, 10)
        ticks = np.arange(-4, 5, 2)
        setup_ticks(3, ticks, ticks, -6, 6)
        ticks = np.arange(-120, 121, 60)
        setup_ticks(4, np.radians(ticks), ticks, -np.pi, np.pi)
        ticks = np.array([0.73, 0.83, 0.93, 0.96, 0.99, 1.02])
        setup_ticks(5, rho2ind(ticks), ticks, 0, 180)

    def set(self, **kwargs):
        if "title" in kwargs:
            self.set_title(kwargs["title"])
        if "xlim" in kwargs:
            self.set_xlim(kwargs["xlim"][0], kwargs["xlim"][1])
        if "ylim" in kwargs:
            self.set_ylim(kwargs["ylim"][0], kwargs["ylim"][1])

    def set_title(self, title):
        self.title.set_text(title)

    def set_xlim(self, lo, hi):
        for ax in self.ax:
            ax.set_xlim(lo, hi)

    def set_ylim(self, lo, hi):
        for ax in self.ax:
            ax.set_ylim(lo, hi)

    def close(self):
        plt.close(self.fig)


class ChartRHI(Chart):
    def __init__(self, size=(1280, 720), **kwargs):
        super().__init__(size, **kwargs)

        with plt.rc_context(self.figprops):
            self.fig = plt.figure(figsize=self.figsize, dpi=self.dpi, frameon=False)

            for i in range(6):
                self.ax[i], self.cb[i], self.st[i] = self._add_axes(231 + i)

            # Do a test run to get the proper position of the title, then use left alignment to avoid jitters in animations
            y = (self.size[1] - self.titlesize - self.m) / self.size[1]
            title_props = {
                "fontproperties": self.titlefont,
                "fontsize": self.titlesize,
                "color": self.titlecolor,
                "horizontalalignment": "center",
                "verticalalignment": "bottom",
            }
            title = self.fig.text(0.5, y, "8888/88/88 88:88:88 UTC", **title_props)
            extent = title.get_window_extent()
            title.remove()
            title_props["horizontalalignment"] = "left"
            x = extent.x0 / self.size[0] * self.dpi / self.fig.dpi
            self.title = self.fig.text(x, y, "", **title_props)

    def _update_data_only(self, sweep: sweep.Sweep, ymax=None):
        super()._update_data_only(sweep)
        if ymax is not None:
            self.set_ylim(0, ymax)

    def set_data(self, sweep: sweep.Sweep, ymax=None):
        if self.r is sweep.meshCoordinate.r and self.e is sweep.meshCoordinate.e:
            return self._update_data_only(sweep, ymax)

        e_rad = np.radians(sweep.meshCoordinate.e)
        xx = np.outer(np.cos(e_rad), sweep.meshCoordinate.r)
        yy = np.outer(np.sin(e_rad), sweep.meshCoordinate.r)

        self._update_coordinate_data(xx, yy, sweep)

        if self.overlay is None:
            # Find out the meaningful range of the plot
            mask = np.logical_and(sweep.products["Z"] > -20, sweep.products["Z"] < 80)
            h_max = np.ceil(np.max(yy[1:, 1:][mask]) + 3) if ymax is None else ymax
            extent = (0, 0, sweep.meshCoordinate.r[-1], max(sweep.meshCoordinate.e))

            self.overlay = overlay.PolarGrid(extent=extent, ymax=h_max, s=self.s)
            self.overlay.load()

            for ax in self.ax:
                self.overlay.draw(ax)

            super()._setup_colorbars()

        title = sweep.time.strftime(r"%Y/%m/%d %H:%M:%S UTC")
        self.title.set_text(title)


class ChartPPI(Chart):
    def __init__(self, size=(1280, 720), **kwargs):
        if "orientation" not in kwargs:
            kwargs["orientation"] = "vertical"
        super().__init__(size, **kwargs)

        with plt.rc_context(self.figprops):
            self.fig = plt.figure(figsize=self.figsize, dpi=self.dpi, frameon=False)

            for i in range(6):
                self.ax[i], self.cb[i], self.st[i] = self._add_axes(321 + i)

            # Do a test run to get the proper position of the title, then use left alignment to avoid jitters in animations
            y = (self.size[1] - self.titlesize - self.m) / self.size[1]
            title_props = {
                "fontproperties": self.titlefont,
                "fontsize": self.titlesize,
                "color": self.titlecolor,
                "horizontalalignment": "center",
                "verticalalignment": "bottom",
            }
            title = self.fig.text(0.5, y, "8888/88/88 88:88:88 UTC", **title_props)
            extent = title.get_window_extent()
            title.remove()
            title_props["horizontalalignment"] = "left"
            x = extent.x0 / self.size[0] * self.dpi / self.fig.dpi
            self.title = self.fig.text(x, y, "", **title_props)

    def __repr__(self):
        return self.fig.__repr__()

    def set_data(self, sweep: sweep.Sweep, rmax=None):
        if self.r is sweep.meshCoordinate.r and self.a is sweep.meshCoordinate.a:
            return self._update_data_only(sweep)

        e_rad = np.radians(sweep.scanElevation)
        a_rad = np.radians(sweep.meshCoordinate.a)
        xx = np.outer(np.cos(e_rad) * np.cos(a_rad), sweep.meshCoordinate.r)
        yy = np.outer(np.cos(e_rad) * np.sin(a_rad), sweep.meshCoordinate.r)

        self._update_coordinate_data(xx, yy, sweep)

        if self.overlay is None:
            origin = (sweep.longitude, sweep.latitude)
            aspect = self.ax[0].bbox.height / self.ax[0].bbox.width
            max_range = sweep.meshCoordinate.r[-1] if rmax is None else rmax
            extent = (-max_range, -max_range * aspect, max_range, max_range * aspect)
            density = self.ax[0].bbox.width / max_range / 2

            # Get the map overlay
            self.overlay = overlay.Overlay(origin=origin, extent=extent, density=density, rmax=1.2 * max_range, s=self.s)
            self.overlay.load()
            if self.orientation == "vertical":
                exclude = (0.5 * max_range, -max_range * aspect, 1.1 * max_range, 0)
                self.overlay.exclude(exclude)
            for ax in self.ax:
                self.overlay.draw(ax)
            super()._setup_colorbars()

        title = sweep.time.strftime(r"%Y/%m/%d %H:%M:%S UTC")
        self.title.set_text(title)

    def set(self, **kwargs):
        if "rmax" in kwargs:
            aspect = self.ax[0].bbox.height / self.ax[0].bbox.width
            max_range = kwargs["rmax"]
            for ax in self.ax:
                ax.set(xlim=(-max_range, max_range), ylim=(-max_range * aspect, max_range * aspect))

    def set_rmax(self, rmax):
        self.set(rmax=rmax)

    # TODO: Add methods to set rmax, x-offset, y-offset, etc.
