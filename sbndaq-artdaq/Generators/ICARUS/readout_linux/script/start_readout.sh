#!/bin/bash -l

export HOME="/home/nfs/icarus/crt/bottom/readout_linux"

#DC ONLINE
export DCONLINE_PATH_LOCAL="/home/nfs/icarus/crt/bottom/readout_linux/crt"
export DCONLINE_PATH=${HOME}
export PATH=${PATH}:${DCONLINE_PATH}/bin


tim=`date`

if [ "$1" = "" ]; then
echo "Usage : ./start_readout.sh <process name> <argument>"
else

if [ "$2" = "" ]; then
prog=$1
arg=""
pid=`ps aux | grep "$1" | grep -v "start_readout.sh" | grep -v "vim" | grep -v "grep" | awk '{print $2}' `
else
prog=$1
arg=$2
arg2=$3
pid=`ps aux | grep "$1" | grep -v "start_readout.sh" | grep -v "vim" | grep -v "grep" | awk '{print $2}' `
fi

#echo "pid="$pid

if [ "$pid" != "" ]; then
echo "Found previous instances of readout. Killing..."
cd $DCONLINE_PATH/script
#echo "Stopping readout at $tim" >> $DCONLINE_PATH_LOCAL/DCOV/log/stop_readout.log
perl -S /home/nfs/icarus/crt/bottom/readout_linux/script/stop.pl 
#>> $DCONLINE_PATH/DCOV/log/stop_readout.log`
sleep 2
fi
echo "Starting"
cd $DCONLINE_PATH/bin
if [ "$4" = "" ]; then
`./$prog -d $arg`
sleep 3
else
`./$prog -d $arg -g $arg2 &`

sleep 3
fi

fi



exit 0
