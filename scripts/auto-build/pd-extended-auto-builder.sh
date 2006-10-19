#!/bin/sh
# this script is the first attempt to have an automated updater and builder

# the source dir where this script is
SCRIPT_DIR=`echo $0 | sed 's|\(.*\)/.*$|\1|'`
. $SCRIPT_DIR/auto-build-common

# the name of this script
SCRIPT=`echo $0| sed 's|.*/\(.*\)|\1|g'`

BUILD_DIR=.
case $SYSTEM in 
	 linux)
		  BUILD_DIR=linux_make
		  echo "Configuring to use $BUILD_DIR on GNU/Linux"
		  ;;
	 darwin)
		  BUILD_DIR=darwin_app
		  echo "Configuring to use $BUILD_DIR on Darwin/Mac OS X"
		  ;;
	 mingw*)
		  BUILD_DIR=win32_inno
		  echo "Configuring to use $BUILD_DIR on MinGW/Windows"
		  ;;
	 cygwin*)
		  BUILD_DIR=win32_inno
		  echo "Configuring to use $BUILD_DIR on Cygwin/Windows"
		  ;;
	 *)
		  echo "ERROR: Platform $SYSTEM not supported!"
		  exit
		  ;;
esac


# convert into absolute path
cd `echo $0 | sed 's|\(.*\)/.*$|\1|'`/../..
auto_build_root_dir=`pwd`
echo "root: $auto_build_root_dir" 

# let rsync handle the cleanup with --delete
rsync -av --delete rsync://128.238.56.50/distros/pd-extended/ \
	 ${auto_build_root_dir}/

cd "${auto_build_root_dir}/packages/$BUILD_DIR"
make -C "${auto_build_root_dir}/packages" patch
make install && make package && make test_package
make test_locations


upload_build ()
{
    platform_folder=$1
    build_folder=$2
    archive_format=$3

	 archive="${auto_build_root_dir}/packages/${platform_folder}/${build_folder}/Pd*.${archive_format}"
    
    echo "upload specs $1 $2 $3"
    echo "Uploading $archive"
    test -e ${archive} && rsync -a ${archive} \
		  rsync://128.238.56.50/upload/${DATE}/`ls -1 ${archive} | sed "s|.*/\(.*\)\.${archive_format}|\1-${HOSTNAME}.${archive_format}|"`  &&  echo SUCCESS
}


case $SYSTEM in 
	 linux)
		  upload_build linux_make build tar.bz2
		  ;;
	 darwin)
		  upload_build darwin_app . dmg
		  ;;
	 mingw*)
		  upload_build win32_inno Output exe
		  ;;
	 cygwin*)
		  upload_build win32_inno Output exe
		  ;;
esac

