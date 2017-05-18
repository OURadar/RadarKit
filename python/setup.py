from distutils.core import setup, Extension

# Define the extension module
radarkit = Extension('radarkit', sources=['radarkitmodule.c'])

# Run the setup
setup(ext_modules=[radarkit])
