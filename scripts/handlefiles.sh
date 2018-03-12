#!/bin/bash

HAS_BLIB=0
if [ -f ${HOME}/blib-sh/blib.sh ]; then
    HAS_BLIB=1
    . ${HOME}/blib-sh/blib.sh
    if [ -d "/data/log" ]; then
        LOGFILE="/data/log/handlefiles-$(date +%Y%m%d).log"
    elif [ -d "data/log" ]; then
        LOGFILE="data/log/handlefiles-$(date +%Y%m%d).log"
    else
        LOGFILE="${HOME}/handlefiles.log"
    fi
fi

# Use the first file as template for the archive filename
afile="${1%%-[ZVWDPR].nc}.tgz"
folder=${1%/*}
cd $folder

# Archive filename without the path
afile=${afile##*/}
files=""
for file in $@; do
	files="$files ${file##*/}"
done

# The command to compress the files into a tgz archive
cmd="tar -czf $afile $files"
#slog $cmd
eval $cmd
# Remove the original .nc files
#rm -f $files

# Go back to the previous folder
cd - > /dev/null

if [ ${HAS_BLIB} ]; then
    log $afile
fi
