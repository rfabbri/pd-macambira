#!/bin/sh
# this script is the first attempt to have an automated updater and builder

# the source dir where this script is
## this could be done more easily with ${0%/*}
SCRIPT_DIR=$(echo $0 | sed 's|\(.*\)/.*$|\1|')
. $SCRIPT_DIR/auto-build-common

# the name of this script
## this could be done more easily with ${0##*/}
SCRIPT=$(echo $0| sed 's|.*/\(.*\)|\1|g')

# convert into absolute path
cd "${SCRIPT_DIR}/../.."
auto_build_root_dir=`pwd`
echo "build root: $auto_build_root_dir" 
rsync_distro "$auto_build_root_dir"

cd "${auto_build_root_dir}/desiredata/src"
./configure \
	&& make \
	&& echo SUCCESS
