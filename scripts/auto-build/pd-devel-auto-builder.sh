#!/bin/sh

HOSTNAME=`hostname | sed 's|\([a-zA-Z0-9-]*\)\..*|\1|'`
SYSTEM=`uname -s`
DATE=`date +%Y-%m-%d`
TIME=`date +%H.%M.%S`
SCRIPT=`echo $0| sed 's|.*/\(.*\)|\1|g'`

case $SYSTEM in 
	 Linux)
		  echo "Configuring for GNU/Linux"
		  ;;
	 Darwin)
		  echo "Configuring for Darwin/Mac OS X"
		  ;;
	 MINGW*)
		  echo "Configuring for MinGW/Windows"
		  ;;
	 CYGWIN*)
		  echo "Configuring for Cygwin/Windows"
		  ;;
	 *)
		  echo "ERROR: Platform $SYSTEM not supported!"
		  exit
		  ;;
esac



echo "This currently does nothing, but it could..."

# if the below word prints, the status report email is not sent
echo SUCCESS
