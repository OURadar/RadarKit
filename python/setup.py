from distutils.core import setup, Extension

# Define the extension module
radarkit = Extension('radarkit', sources=['radarkit.c'])

# Run the setup
setup(ext_modules=[radarkit])
