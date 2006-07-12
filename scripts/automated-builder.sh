#!/bin/sh

# this script is the first attempt to have an automated updater and builder

cvs_root_dir=`echo $0 | sed 's|\(.*\)/.*$|\1|'`/..
SYSTEM=`uname -s`

cd "${cvs_root_dir}/packages"
make unpatch_pd
../scripts/update-developer-layout.sh
make patch_pd

BUILD_DIR=.
if [ "$SYSTEM" == "Linux" ]; then
	 BUILD_DIR=linux_make
fi

if [ "$SYSTEM" == "Darwin" ]; then
	 BUILD_DIR=darwin_app
fi

cd "$BUILD_DIR"
make distclean
make package_clean
rm -rf build
make install && make package

#make test_locations
#make test_package
