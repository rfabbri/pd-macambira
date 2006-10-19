#!/bin/sh

# the source dir where this script is
SCRIPT_DIR=`echo $0 | sed 's|\(.*\)/.*$|\1|'`
. $SCRIPT_DIR/auto-build-common

# the name of this script
SCRIPT=`echo $0| sed 's|.*/\(.*\)|\1|g'`

case $SYSTEM in 
	 linux)
		  echo "Configuring for GNU/Linux"
		  ;;
	 darwin)
		  echo "Configuring for Darwin/Mac OS X"
		  ;;
	 mingw*)
		  echo "Configuring for MinGW/Windows"
		  ;;
	 cygwin*)
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
