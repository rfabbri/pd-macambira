#!/bin/sh

cvs_root_dir=`echo $0 | sed 's|\(.*\)/.*$|\1|'`/..
UNAME=`uname -s`
RECIPIENTS="hans@eds.org"
DATE=`date +%Y-%m-%d_%H.%M.%S`
LOGFILE="/tmp/pd-autobuild-${DATE}.txt"

cd $cvs_root_dir
scripts/update-developer-layout.sh

# Apple Mac OS X 
if [ "${UNAME}" == "Darwin" ]; then
cd packages/darwin_app
fi

# GNU/Linux
if [ "${UNAME}" == "Darwin" ]; then
cd packages/linux_make
fi

# MinGW for MS Windows
if [ "${UNAME}" == "MINGW32_NT-5.1" ]; then
cd packages/win32_inno
fi

make install && make package > $LOGFILE 2>&1

if [ "${UNAME}" == "MINGW32_NT-5.1" ]; then
	 echo "No mailer for windows yet"
else
	 cat $LOGFILE | mail -s "Pd Autobuild Log on $UNAME - $DATE" $RECIPIENTS
fi
