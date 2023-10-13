#!/bin/bash

trap 'echo -e "\nshow.sh: Be patient"' INT

for file in $@; do
    echo -e "\033[38;5;172m${file}\033[m"
done
