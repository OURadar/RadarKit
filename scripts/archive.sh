#!/bin/bash

# This script depends on blib-sh (https://git.arrc.ou.edu/cheo4524/blib-sh)

SEARCH_LOC=(${HOME}/Developer/blib-sh ${HOME}/blib-sh)

trap 'echo -e "\narchive.sh: Be patient"' INT

HAS_BLIB=0
for folder in ${SEARCH_LOC[*]}; do
	if [[ -f ${folder}/blib.sh ]]; then
		HAS_BLIB=1
		. ${folder}/blib.sh
	fi
done

if [ ${HAS_BLIB} -eq 1 ]; then
	if [ -d "/data/log" ]; then
		LOGFILE="/data/log/archive-$(date +%Y%m%d).log"
	elif [ -d "data/log" ]; then
		LOGFILE="data/log/archive-$(date +%Y%m%d).log"
	else
		LOGFILE="${HOME}/archive.log"
	fi
fi

# Use the first file as template for the archive filename
if [[ "${1}" == *-Z.nc ]]; then
    afile="${1%%-Z.nc}.txz"
elif [[ "${1}" == *.nc ]]; then
    afile="${1%%.nc}.txz"
else
    afile="${1}.txz"
fi
folder=${1%/*}
cd $folder

# Archive filename without the path
afile=${afile##*/}
files=""
for file in $@; do
	files="$files ${file##*/}"
done

# The command to compress the files into a tgz archive
cmd="tar -cJf ${afile} ${files}"
eval $cmd
# Remove the original .nc files
#rm -f $files

# Go back to the previous folder
cd - > /dev/null

if [ ${HAS_BLIB} -eq 1 ]; then
	log ${afile}
fi
