#!/bin/bash

python -m build

version=$(../rkutil --version | awk {'print $2'})
archive=$(ls -t dist/radarkit-${version}.tar.gz)

echo "version = ${version}   archive = ${archive}"

#python -m twine upload --verbose --repository radarkit ${archive}
