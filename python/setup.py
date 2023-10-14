from setuptools import setup, find_packages

from radarkit import __version__

setup(
    name='radarkit',
    version=__version__,
    install_requires=[
        'wheel',
        'tqdm',
        'numpy',
    ],
    packages=find_packages(),
    include_package_data=True,
    platforms=['darwin', 'linux'],
)
