#!/bin/sh

PD_ROOT=$1
NETRECEIVE_PATCH=/tmp/.____pd_netreceive____.pd
PORT_NUMBER=55555
LOG_FILE=/tmp/load_every_help-log-`date +20%y-%m-%d_%H.%M.%S`.txt

helpdir=${PD_ROOT}/doc
bindir=${PD_ROOT}/bin

function make_netreceive_patch () 
{
	 rm $1
	 touch $1
	 echo '#N canvas 222 130 454 304 10;' >> $1
	 echo "#X obj 111 83 netreceive $PORT_NUMBER 0 old;" >> $1
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



make_netreceive_patch $NETRECEIVE_PATCH

touch $LOG_FILE
${bindir}/pd -nogui -stderr -open $NETRECEIVE_PATCH >> $LOG_FILE 2>&1 &

#wait for pd to start
sleep 30

for file in `find $helpdir -name '*.pd'`; do
	 filename=`echo $file|sed 's|.*/\(.*\.pd\)$|\1|'`
	 dir=`echo $file|sed 's|\(.*\)/.*\.pd$|\1|'`
	 open_patch $filename $dir
	 sleep 1
	 close_patch $filename
done

echo "COMPLETED!" >> $LOG_FILE
echo "; pd quit;" | ${bindir}/pdsend $PORT_NUMBER localhost tcp
