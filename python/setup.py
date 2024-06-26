from setuptools import setup, find_packages

from src.radarkit import __version__

setup(
    name='radarkit',
    version=__version__,
    install_requires=[
        'wheel',
        'numpy',
        'tqdm',
    ],
    include_package_data=True,
    packages=find_packages(where="src"),
    package_dir={"": "src"},
)
