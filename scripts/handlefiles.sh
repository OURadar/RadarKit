#!/bin/bash

. ${HOME}/boonlib-sh/boonlib.sh

LOGFILE=${HOME}/handlefiles.log

# slog $@

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
slog $afile

