#!/bin/sh

# this script is the first attempt to have an automated updater and builder

SYSTEM=`uname -s`
DATE=`date +%Y-%m-%d`
TIME=`date +%H.%M.%S`
SCRIPT=`echo $0| sed 's|.*/\(.*\)|\1|g'`


# convert into absolute path
cd `echo $0 | sed 's|\(.*\)/.*$|\1|'`/../..
auto_build_root_dir=`pwd`
echo "root: $auto_build_root_dir" 

# let rsync handle the cleanup with --delete
rsync -av --delete rsync://128.238.56.50/distros/pd-extended/ \
	 ${auto_build_root_dir}/

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
make install && make package && make test_package
make test_locations


upload_build ()
{
    platform_folder=$1
    build_folder=$2
    archive_format=$3
    
    echo "Uploading $1 $2 $3"
# upload files to webpage
    echo ${auto_build_root_dir}/packages/${platform_folder}/${build_folder}/Pd*.${archive_format} 
    test -e ${auto_build_root_dir}/packages/${platform_folder}/${build_folder}/Pd*.${archive_format} && \
	rsync -a ${auto_build_root_dir}/packages/${platform_folder}/${build_folder}/Pd*.${archive_format} \
	rsync://128.238.56.50/upload/${DATE}/`ls -1 ${auto_build_root_dir}/packages/${platform_folder}/${build_folder}/Pd*.${archive_format} | sed "s|.*/\(.*\)\.${archive_format}|\1-${HOSTNAME}.${archive_format}|"` 
}

if [ "$SYSTEM" == "Linux" ]; then
    upload_build linux_make build tar.bz2
fi
if [ "$SYSTEM" == "Darwin" ]; then
    upload_build darwin_app . dmg
fi
if [ "`echo $SYSTEM | sed -n 's|\(MINGW\)|\1|p'`" == "MINGW" ]; then
    upload_build win32_inno Output exe
fi

