#!/bin/sh

# the source dir where this script is
SCRIPT_DIR=`echo $0 | sed 's|\(.*\)/.*$|\1|'`
. $SCRIPT_DIR/auto-build-common

# the name of this script
SCRIPT=`echo $0| sed 's|.*/\(.*\)|\1|g'`

case $SYSTEM in 
	 linux)
		  configure_options="--enable-alsa --enable-jack"
		  platform_name=`uname -m`
		  ;;
	 darwin)
		  configure_options=""
		  platform_name=`uname -p`
		  ;;
	 mingw*)
		  configure_options=""
		  platform_name=i386
		  ;;
	 cygwin*)
		  configure_options=""
		  platform_name=i386
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

package_name="/tmp/pd-${DATE}-${SYSTEM}-${HOSTNAME}-${platform_name}.tar.bz2"

# let rsync handle the cleanup with --delete
rsync -av --delete rsync://128.238.56.50/distros/pd-main/ \
	 ${auto_build_root_dir}/


cd ${auto_build_root_dir}/pd/src && \
	 autoconf && \
	 ./configure $configure_options && \
	 make  && \
	 cd ../../ && \
	 tar cjf $package_name pd && \
	 rsync -a ${package_name} rsync://128.238.56.50/upload/${DATE}/ && \
	 echo SUCCESS
rm -f -- $package_name

