import numpy
import scipy
import radarkit

class main(object):
    def __init__(self):
        self.threshold = 1.0
        radarkit.init()

    # Every algorithm should have this function defined
    def process(self,sweep):
        print('lowZ algorithm\n')
        radarkit.showColors()

    # Every algorithm should have this function defined
    def name(self):
        string = 'High Reflectivity'
        return string
