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

# Use the first file (usually the XXXXXX-Z.nc) as template for the archive filename
afile="${1%%-[ZVWDPR].nc}.txz"
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
