import logging

from .main import *

from . import prod

__version__ = str(RKVersionString())

logger = logging.getLogger("RadarKit")
