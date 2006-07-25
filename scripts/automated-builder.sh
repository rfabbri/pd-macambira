#!/bin/sh

# this script is the first attempt to have an automated updater and builder

SYSTEM=`uname -s`
DATE=`date +%Y-%m-%d`
TIME=`date +%H.%M.%S`
SCRIPT=`echo $0| sed 's|.*/\(.*\)|\1|g'`
LOGFILE=/home/pd/logs/${DATE}_-_${TIME}_-_${SCRIPT}_-_${SYSTEM}.txt

auto_build_root_dir=`echo $0 | sed 's|\(.*\)/.*$|\1|'`/..
# convert into absolute path
cd ${auto_build_root_dir}
auto_build_root_dir=`pwd`

echo "root: $auto_build_root_dir" 

cd ${auto_build_root_dir}/packages
# let rsync handle the cleanup
#make unpatch_pd

rsync -av --delete rsync://128.238.56.50/pure-data/ ${auto_build_root_dir}/

BUILD_DIR=.
if [ "$SYSTEM" == "Linux" ]; then
	 BUILD_DIR=linux_make
fi
if [ "$SYSTEM" == "Darwin" ]; then
	 BUILD_DIR=darwin_app
fi
if [ "`echo $SYSTEM | sed -n 's|\(MINGW\)|\1|p'`" == "MINGW" ]; then
	 BUILD_DIR=win32_inno
fi

cd "$BUILD_DIR"
pwd
make distclean
make package_clean
rm -rf build
make -C "${auto_build_root_dir}/packages" patch_pd
make install && make package

make test_package
make test_locations
