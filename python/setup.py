from distutils.core import setup, Extension

# Define the extension module
rkstruct = Extension('rkstruct', sources=['rkstruct.c'])

# Run the setup
setup(ext_modules=[rkstruct])
