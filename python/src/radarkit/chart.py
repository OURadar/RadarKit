import datetime
import numpy as np
import matplotlib.font_manager
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


class Chart:
    ax = [None] * 6
    cb = [None] * 6
    st = [None] * 6
    r = None
    e = None
    a = None
    labelfont = blib.getFontOfWeight(weight=500)
    titlefont = blib.getFontOfWeight(weight=700)

    def __init__(self, size=(1280, 720), dpi=72, s=1, titlecolor=None):
        self.size = size
        self.dpi = dpi
        self.s = s
        self.m = 5 * s
        self.p = 10 * s
        self.labelsize = 12 * s
        self.titlesize = 28 * s
        self.captionsize = 14 * s

        width, height = size
        self.figsize = (width / dpi, height / dpi)

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
            self.titlecolor = (0.8, 0.8, 0.8) if titlecolor is None else titlecolor
        else:
            self.gridcolor = (0, 0.4, 0.4)
            self.titlecolor = (0.2, 0.2, 0.2) if titlecolor is None else titlecolor
        self.path_effects = [
            matplotlib.patheffects.Stroke(linewidth=2 * self.s, foreground=matplotlib.rcParams["axes.facecolor"]),
            matplotlib.patheffects.Normal(),
        ]
        self.figprops = {
            "axes.linewidth": max(1, s // 1),
            "axes.axisbelow": False,
            "axes.facecolor": (1, 1, 1, 0.85) if hsv[2] < 0.5 else (0, 0, 0, 0.85),
            "axes.titlepad": 10 * s,
            "axes.labelpad": 6 * s,
            "axes.linewidth": max(1, s // 1),
            "xtick.direction": "in",
            "xtick.major.pad": 8 * s,
            "xtick.major.size": round(3 * s),
            "xtick.major.width": max(1, s // 1),
            "xtick.labelsize": self.labelsize,
            "ytick.direction": "in",
            "ytick.major.pad": 8 * s,
            "ytick.major.size": round(3 * s),
            "ytick.major.width": max(1, s // 1),
            "ytick.labelsize": self.labelsize,
        }

    def _get_pos(self, num):
        m = self.m
        # Available space for axes
        width, height = self.size
        ww = width
        hh = height - self.titlesize - self.m
        cols = num // 100
        rows = (num - cols * 100) // 10
        w = round((ww - (cols + 1) * m) / cols) / width
        h = round((hh - (rows + 1) * m) / rows) / height
        i = num % 10
        x = (i - 1) // rows
        y = rows - 1 - (i - 1) % rows
        # print(f'x = {x}  y = {y}  w = {w}  h = {h}')
        x = (x * w) + (x + 1) * m / width
        y = (y * h) + (y + 1) * m / height
        return [x, y, w, h]

    def _draw_box(self, q, c=matplotlib.rcParams["text.color"]):
        width, _ = self.size
        x0 = q[0] - 1 / width
        x1 = x0 + 10 / width
        x2 = q[0] + q[2]
        y0 = q[1]
        y1 = q[1] + q[3]
        x = [x1, x2, x2, x0, x0, x1]
        y = [y0, y0, y1, y1, y0, y0]
        line = matplotlib.lines.Line2D(x, y, color=c, linewidth=self.s, solid_joinstyle="miter")
        self.fig.add_artist(line)

    def _add_axes(self, num):
        width, height = self.size
        p = self.p
        q = self._get_pos(num)
        ax = self.fig.add_axes(q, frameon=True, snap=True)
        h = 10 * s
        q[0] += 2 * p / width
        q[1] += q[3] - (2 * p + self.captionsize + h) / height
        q[2] = round(0.4 * q[2] * width) / width
        q[3] = h / height
        cb = self.fig.add_axes(q, frameon=False, snap=True)
        self._draw_box(q)
        st = cb.set_title(f"{num}", fontproperties=self.labelfont)
        st.set(size=self.captionsize, path_effects=self.path_effects)
        return ax, cb, st


class ChartRHI(Chart):
    def __init__(self, size=(1280, 720), dpi=72, s=1, titlecolor=None):
        super().__init__(size, dpi, s, titlecolor)

        with plt.rc_context(self.figprops):
            self.fig = plt.figure(figsize=self.figsize, dpi=self.dpi, frameon=False)

            for i in range(6):
                self.ax[i], self.cb[i], self.st[i] = self._add_axes(231 + i)

            # Do a test run to get the proper position of the title, then use left alignment to avoid jitters in animations
            y = self.size[1] - self.titlesize - self.m / self.size[1]
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

    def set_data(self, r, e, prods, ymax=None):
        if self.r is r and self.e is e:
            self.mz.set_array(prods["Z"].ravel())
            self.mv.set_array(prods["V"].ravel())
            self.mw.set_array(prods["W"].ravel())
            self.md.set_array(prods["D"].ravel())
            self.mp.set_array(prods["P"].ravel())
            self.mr.set_array(rho2ind(prods["R"]).ravel())
            if ymax is not None:
                self.set_ylim(0, ymax)
        else:
            e_rad = np.radians(e)
            xx = np.outer(np.cos(e_rad), r)
            yy = np.outer(np.sin(e_rad), r)

            for ax in self.ax:
                ax.clear()

            self.mz = self.ax[0].pcolormesh(xx, yy, prods["Z"], shading="flat", cmap=zmap, vmin=-32, vmax=96)
            self.mv = self.ax[1].pcolormesh(xx, yy, prods["V"], shading="flat", cmap=vmap, vmin=-64, vmax=64)
            self.mw = self.ax[2].pcolormesh(xx, yy, prods["W"], shading="flat", cmap=wmap, vmin=0, vmax=12.8)
            self.md = self.ax[3].pcolormesh(xx, yy, prods["D"], shading="flat", cmap=dmap, vmin=-10, vmax=15.5)
            self.mp = self.ax[4].pcolormesh(xx, yy, prods["P"], shading="flat", cmap=pmap, vmin=-np.pi, vmax=np.pi)
            self.mr = self.ax[5].pcolormesh(xx, yy, rho2ind(prods["R"]), shading="flat", cmap=rmap, vmin=0, vmax=256)
            self.r = r
            self.e = e

            # Find out the meaningful range of the plot
            mask = np.logical_and(prods["Z"] > -20, prods["Z"] < 80)

            # Grid lines
            max_range = np.ceil(xx[0, -1])
            max_height = np.ceil(np.max(yy[1:, 1:][mask]) + 3) if ymax is None else ymax
            aa = np.radians(np.arange(0, 91, 2))
            cc = np.cos(np.radians(e[-1] + 5))
            ss = np.sin(np.radians(e[-1] + 5))
            ll = round(r[-1] * 0.1) * 10
            label_props = {
                "fontproperties": self.labelfont,
                "fontsize": self.labelsize,
                "color": self.gridcolor,
                "path_effects": self.path_effects,
                "horizontalalignment": "center",
                "verticalalignment": "center",
            }
            for ax in self.ax:
                # Range rings
                for r in [1, *range(10, 41, 10)]:
                    x = r * np.cos(aa)
                    y = r * np.sin(aa)
                    ax.plot(x, y, color=self.gridcolor, linewidth=0.5 * self.s)
                    x = r * cc
                    y = r * ss
                    if r < 2 or y > 0.9 * max_height:
                        continue
                    t = matplotlib.text.Text(x, y, f"{r} km", **label_props)
                    ax.add_artist(t)
                # Angular grid
                for r in np.radians(range(0, 86, 10)):
                    x = np.array([1, ll]) * np.cos(r)
                    y = np.array([1, ll]) * np.sin(r)
                    ax.plot(x, y, color=self.gridcolor, linewidth=0.5 * self.s)
                # Clear the axis splines
                ax.set_xlim(0, max_range)
                ax.set_ylim(0, max_height)
                ax.set_xticks([])
                ax.set_yticks([])

            # Colorbars
            plt.colorbar(self.mz, ax=self.ax[0], cax=self.cb[0], orientation="horizontal")
            plt.colorbar(self.mv, ax=self.ax[1], cax=self.cb[1], orientation="horizontal")
            plt.colorbar(self.mw, ax=self.ax[2], cax=self.cb[2], orientation="horizontal")
            plt.colorbar(self.md, ax=self.ax[3], cax=self.cb[3], orientation="horizontal")
            plt.colorbar(self.mp, ax=self.ax[4], cax=self.cb[4], orientation="horizontal")
            plt.colorbar(self.mr, ax=self.ax[5], cax=self.cb[5], orientation="horizontal")
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
            ticks = np.arange(-20, 61, 20)
            self.cb[0].set_xticks(ticks, labels=ticks, **tick_props)
            self.cb[0].set_xlim(-10, 75)
            ticks = np.arange(-20, 21, 10)
            self.cb[1].set_xticks(ticks, labels=ticks, **tick_props)
            self.cb[1].set_xlim(-30, 30)
            ticks = np.arange(2, 9, 2)
            self.cb[2].set_xticks(ticks, labels=ticks, **tick_props)
            self.cb[2].set_xlim(0, 10)
            ticks = np.arange(-4, 5, 2)
            self.cb[3].set_xticks(ticks, labels=ticks, **tick_props)
            self.cb[3].set_xlim(-6, 6)
            ticks = np.arange(-120, 121, 60)
            self.cb[4].set_xticks(np.radians(ticks), labels=ticks, **tick_props)
            self.cb[4].set_xlim(-np.pi, np.pi)
            ticks = np.array([0.73, 0.83, 0.93, 0.96, 0.99, 1.02])
            self.cb[5].set_xticks(rho2ind(ticks), labels=ticks, **tick_props)
            self.cb[5].set_xlim(0, 180)

        title = datetime.datetime.utcfromtimestamp(prods["time"]).strftime(r"%Y/%m/%d %H:%M:%S UTC")
        self.title.set_text(title)

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
