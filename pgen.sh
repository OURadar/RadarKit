#!/bin/bash

RUN=1
VERBOSE=0

# Show help text
function showHelp() {
    echo "${0#*/} [-n] [-v] FILENAME"
    echo ""
    echo "    -n   no execution, just show commands"
    echo "    -v   increases verbosity"
    echo ""
    echo "Boonleng Cheong"
    echo "Advanced Radar Research Center"
    echo "The University of Oklahoma"
}

##############
#
#   M A I N
#
##############


while getopts 'hnv' OPTION; do
    case "${OPTION}" in
        c)
            COUNT=${OPTARG}
            ;;
        h)
            showHelp
            exit 0
            ;;
        n)
            RUN=0
            ;;
        v)
            VERBOSE=$((VERBOSE+1))
            ;;
        ?)
            echo "Uknown"
            exit 1
            ;;
    esac
done
shift "$((OPTIND - 1))"

dst="${1%/*}"
echo $"dst = ${dst}"
cmd="pgen ${1}"
if [ ${VERBOSE} -gt 0 ]; then
	echo "${cmd}"
fi
if [ ${RUN} -eq 1 ]; then
	eval "${cmd}"
fi

if [ -f ~/Developer/iradar/python/imgen.py ]; then
	nc="${1%.rkc}*Z.nc"
	cmd="~/Developer/iradar/python/imgen.py -v -d ${dst} ${nc}"
	if [ ${VERBOSE} -gt 0 ]; then
		echo "${cmd}"
	fi
	if [ ${RUN} -eq 1 ]; then
		eval "${cmd}"
	fi

	if [ -f ~/Developer/ldm-radar-data/stitchFigure.sh ]; then
		png="${nc%.nc}.png"
		cmd="~/Developer/ldm-radar-data/stitchFigure.sh ${png}"
		if [ ${VERBOSE} -gt 0 ]; then
			echo "${cmd}"
		fi
		if [ ${RUN} -eq 1 ]; then
			eval "${cmd}"
		fi
	fi
else
	echo "No imgen.py found"
fi
