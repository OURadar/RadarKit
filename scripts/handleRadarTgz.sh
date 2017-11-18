#!/bin/bash
#
# Remember to create a symbolic link under ${LDMHOME}/bin to this script
#

RANGE=58
BOONLIB=${HOME}/boonlib-sh
APP=/Users/boonleng/Applications/genimg.app/Contents/MacOS/genimg

. ${BOONLIB}/boonlib.sh

LOGFILE="${HOME}/logs/handleRadarTgz-$(date +'%Y%m%d').log"

filename=${1}

if [[ -z ${filename} ]]; then
    slog "No input file."
    exit 1
fi

slog "Processing ${filename} ..."

if [[ ! -d /Volumes/RAMDisk ]]; then
    ${BOONLIB}/makeramdisk.sh
    mkdir -p /Volumes/RAMDisk/figs
fi

# The the nc files within the archive
ncFiles=$(tar -tf ${filename})
tar -xf ${filename} -C /Volumes/RAMDisk

# Change working directory to the RAM drive
cd /Volumes/RAMDisk/

# Generate image using the APP
cmd="${APP} -r ${RANGE} -y -8 ${ncFiles}"
# slog ${cmd}
outfiles=$(${cmd})
# echo ${outfiles}

# Derive png file names from the nc files
pngFiles=""
for file in ${ncFiles}; do pngFiles="${pngFiles} figs/${file/.nc/.png}"; done
pngFiles="${pngFiles:1}"
zFile="${pngFiles%%\ *}"
if [[ ! -f ${zFile} ]]; then slog "Unable to create figures. Persmission denied?"; fi

# Move figures to ..../PX-1000/2017MMDD/figs/
dest="${filename%_original*}figs/"
if [[ ! -d ${dest} ]]; then
    mkdir -p ${dest}
fi
slog "Moving figures to ${dest} ..."
mv ${pngFiles} ${dest}

dest="${filename%\/_original*}"
fullpathPngFiles=""
for file in ${pngFiles}; do fullpathPngFiles="${fullpathPngFiles} ${dest}/${file}"; done
fullpathPngFiles="${fullpathPngFiles:1}"

# slog "${fullpathPngFiles}"

# Insert the figures into the LDM queue
pqinsert ${fullpathPngFiles}

rm -f ${ncFiles}
cd -
