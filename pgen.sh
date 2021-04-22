#!/bin/bash

cmd="pgen ${1}"
echo ${cmd}
eval ${cmd}

nc="${1%.rkc}*Z.nc"
cmd="~/Developer/iradar/python/imgen.py -v -d ~/Downloads/raxpol ${nc}"
echo ${cmd}
eval ${cmd}

png="${nc%.nc}.png"
cmd="~/Developer/ldm-radar-data/stitchFigure.sh ${png}"
echo ${cmd}
eval ${cmd}
