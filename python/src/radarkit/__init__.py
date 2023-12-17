import logging

from .main import *

from . import sweep

__version__ = str(RKVersionString())

logger = logging.getLogger("RadarKit")
