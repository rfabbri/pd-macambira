#!/bin/sh

# this script is the first attempt to have an automated updater and builder

SYSTEM=`uname -s`
DATE=`date +%Y-%m-%d`
TIME=`date +%H.%M.%S`
SCRIPT=`echo $0| sed 's|.*/\(.*\)|\1|g'`
LOGFILE=/home/pd/logs/${DATE}_-_${TIME}_-_${SCRIPT}_-_${SYSTEM}.txt

function upload_build ()
{
	 platform_folder=$1
	 archive_format=$2

# upload files to webpage
test -e /home/pd/auto-build/packages/${platform_folder}/build/Pd*.${archive_format} && \
	 rsync -a /home/pd/auto-build/packages/${platform_folder}/build/Pd*.${archive_format} \
	 rsync://128.238.56.50/upload/${DATE}/`ls -1 /home/pd/auto-build/packages/*/build/Pd*.${archive_format} | sed "s|.*/\(.*\)${archive_format}|\1${HOSTNAME}.${archive_format}|"` 
}


# convert into absolute path
cd `echo $0 | sed 's|\(.*\)/.*$|\1|'`/../..
auto_build_root_dir=`pwd`

echo "root: $auto_build_root_dir" 

# let rsync handle the cleanup with --delete
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

cd "${auto_build_root_dir}/packages/$BUILD_DIR"
make -C "${auto_build_root_dir}/packages" patch_pd
make install && make package

make test_package
make test_locations

case $SYSTEM in 
	 Linux)
		  upload_build linux_make tar.bz2                         >> $LOGFILE 2>&1
		  ;;
	 Darwin)
		  upload_build darwin_app dmg                             >> $LOGFILE 2>&1
		  ;;
	 MINGW*)
		  upload_build win32_inno exe                             >> $LOGFILE 2>&1
		  ;;
	 *)
		  echo "ERROR: Platform $SYSTEM not supported!"           >> $LOGFILE 2>&1
		  exit
		  ;;
esac
