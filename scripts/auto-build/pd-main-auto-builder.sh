#!/bin/sh

HOSTNAME=`hostname`
SYSTEM=`uname -s`
DATE=`date +%Y-%m-%d`
TIME=`date +%H.%M.%S`
SCRIPT=`echo $0| sed 's|.*/\(.*\)|\1|g'`

ROOT_DIR=/home/pd/auto-build/pd-main/pure-data

case $SYSTEM in 
	 Linux)
		  configure_options="--enable-alsa --enable-jack"
		  platform_name=`uname -m`
		  ;;
	 Darwin)
		  configure_options="--enable-jack"
		  platform_name=`uname -p`
		  ;;
	 MINGW*)
		  configure_options=""
		  platform_name=i386
		  ;;
	 *)
		  echo "ERROR: Platform $SYSTEM not supported!"
		  exit
		  ;;
esac

package_name="${ROOT_DIR{}/pd-${DATE}-${SYSTEM}-${HOSTNAME}-${platform_name}.tar.bz2"

cd ${ROOT_DIR}/pure-data/pd/src && \
	 autoconf && \
	 ./configure $configure_options && \
	 make  && \
	 cd ../../ && \
	 tar cjf $package_name pd && \
	 rsync -a ${package_name} rsync://128.238.56.50/upload/${DATE}/
	 

