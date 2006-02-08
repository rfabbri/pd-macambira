#!/bin/sh


PD_ROOT=$1

NETRECEIVE_PATCH=/tmp/.____pd_netreceive____.pd
PORT_NUMBER=55556
LOG_FILE=/tmp/load_every_object-log-`date +20%y-%m-%d_%H.%M.%S`.txt

TEST_DIR=/tmp
TEST_PATCH=.____test_patch.pd

helpdir=${PD_ROOT}/doc
objectsdir=${PD_ROOT}/extra
bindir=${PD_ROOT}/bin


function make_netreceive_patch () 
{
	 rm $1
	 touch $1
	 echo '#N canvas 222 130 454 304 10;' >> $1
	 echo "#X obj 111 83 netreceive $PORT_NUMBER 0 old;" >> $1
}

function make_patch ()
{
	 rm $2
	 touch $2
	 object=`echo $1|sed 's|^\(.*\)\.[adilnpruwx_]*$|\1|'`
	 echo '#N canvas 222 130 454 304 10;' >> $2
	 echo "#X obj 111 83 $object;" >> $2
}

function open_patch ()
{
	 echo "OPENING: $1 $2" >> $LOG_FILE
	 echo "; pd open $1 $2;" | ${bindir}/pdsend $PORT_NUMBER localhost tcp
}

function close_patch ()
{
	 echo "CLOSING: $1" >> $LOG_FILE
	 echo "; pd-$1 menuclose;" | ${bindir}/pdsend $PORT_NUMBER localhost tcp
}

UNAME=`uname -s`
if [ $UNAME == "Darwin" ]; then
	 EXTENSION=pd_darwin
elif [ $UNAME == "Linux" ]; then
	 EXTENSION=pd_linux
else
	 EXTENSION=dll
fi

echo "Searching for $EXTENSION"

make_netreceive_patch $NETRECEIVE_PATCH

touch $LOG_FILE
${bindir}/pd -nogui -stderr -open $NETRECEIVE_PATCH >> $LOG_FILE 2>&1 &

#wait for pd to start
sleep 30

for file in `find $objectsdir -name "*.${EXTENSION}"`; do
	 echo $file
	 filename=`echo $file|sed 's|.*/\(.*\.[adilnpruwx_]*\)$|\1|'`
	 dir=`echo $file|sed 's|\(.*\)/.*\.[adilnpruwx_]*$|\1|'`
	 make_patch $filename ${TEST_DIR}/${TEST_PATCH}
	 open_patch ${TEST_PATCH} ${TEST_DIR}
	 sleep 1
	 close_patch ${TEST_PATCH}
done


echo "COMPLETED!" >> $LOG_FILE
echo "; pd quit;" | ${bindir}/pdsend $PORT_NUMBER localhost tcp
