#!/bin/bash
# build the 3 msd objects using flext and its configuration

cd ../msd
../../../grill/flext/build.sh pd gcc
cd ../msd2D
../../../grill/flext/build.sh pd gcc	
cd ../msd3D
../../../grill/flext/build.sh pd gcc
