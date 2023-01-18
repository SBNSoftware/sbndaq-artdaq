#!/bin/bash -l

export HOME=/local/home/dconline
#ROOT
export MANPATH=:
source ${HOME}/myROOT/bin/thisroot.sh   
export ROOTSYS=${HOME}/myROOT
export LD_LIBRARY_PATH=$ROOTSYS/lib:$LD_LIBRARY_PATH
export PATH=$ROOTSYS/bin:$PATH
alias root='root -l'

#DOGS
export DOGS_PATH=${HOME}/myDOGS
export DOGS_DATA=$DOGS_PATH/DCDB
export LD_LIBRARY_PATH=$DOGS_PATH/lib:$LD_LIBRARY_PATH
export PATH=$DOGS_PATH/bin:$PATH

#DC ONLINE
export DCONLINE_PATH=${HOME}/myOnline
export PATH=${PATH}:${DCONLINE_PATH}/bin

cd $DCONLINE_PATH/DCOV/readout/script

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
echo "Stopping readout at $tim" >> $DCONLINE_PATH/DCOV/log/stop.log
`./stop >> $DCONLINE_PATH/DCOV/log/stop_readout.log`

sleep 1
fi

fi


exit 0
