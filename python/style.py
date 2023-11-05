from cycler import cycler

feature_scale = 1.0
context_properties = {
    'font.family': 'sans-serif',
    'font.sans-serif': ['Helvetica', 'Arial', 'Lucida Grande', 'DejaVu Sans'],
    'figure.facecolor': (0, 0, 0, 0),
    'axes.edgecolor': 'white',
    'axes.facecolor': 'black',
    'axes.labelcolor': 'white',
    'axes.linewidth': 1.0 * feature_scale,
    'axes.titlepad': 8.0 * feature_scale,
    'axes.prop_cycle': cycler('color', ['#17becf', '#bcbd22', '#e377c2', '#9467bd', '#ff7f0e']),
    'grid.color': (0.35, 0.35, 0.4, 1.0),
    'hatch.color': 'white',
    'text.color': 'white',
    'xtick.color': 'white',
    'xtick.direction': 'in',
    'xtick.major.pad': 9.0 * feature_scale,
    'xtick.major.size': 4.0 * feature_scale,
    'xtick.major.width': 1.0 * feature_scale,
    'ytick.color': 'white',
    'ytick.direction': 'in',
    'ytick.major.pad': 7.5 * feature_scale,
}
