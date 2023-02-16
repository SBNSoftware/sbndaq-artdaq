#!/bin/bash -l

export HOME="/scratch_local/crt_tests/backend_data"

#DC ONLINE
export DCONLINE_PATH_LOCAL="/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout"
export DCONLINE_PATH=${HOME}
export PATH=${PATH}:${DCONLINE_PATH_LOCAL}/bin

cd $DCONLINE_PATH_LOCAL/script

tim=`date`

if [ "$1" = "" ]; then
echo "Usage : ./stop_readout.sh <process name> <argument>"
else

if [ "$2" = "" ]; then
prog=$1
arg=""
pid=`ps aux | grep "$1" | grep -v "stop_readout.sh" | grep -v "emacs" | grep -v "grep" | awk '{print $2}' `
else
prog=$1
arg=$2
pid=`ps aux | grep "$1 $2" | grep -v "stop_readout.sh" | grep -v "emacs" | grep -v "grep" | awk '{print $2}' `
fi

if [ "$pid" != "" ]; then
echo "Killing.."
echo "Stopping readout at $tim" >> $DCONLINE_PATH/logs/stop.log
`./stop >> $DCONLINE_PATH/logs/stop_readout.log`

sleep 1
fi

fi


exit 0
