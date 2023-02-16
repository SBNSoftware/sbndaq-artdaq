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
export DOGS_PATH=${HOME}/dchooz/DOGS
export DOGS_DATA=$DOGS_PATH/DCDB
export LD_LIBRARY_PATH=$DOGS_PATH/lib:$LD_LIBRARY_PATH
export PATH=$DOGS_PATH/bin:$PATH

#DC ONLINE
export DCONLINE_PATH=${HOME}/dchooz/dconline/trunk
export DCONLINE_MOUNT=/home/dconline/dchooz/dconline
export PATH=${PATH}:${DCONLINE_PATH}/bin

cd $DCONLINE_PATH/DCOV/readout/macro

tim=`date`

if [ "$1" = "" ]; then
echo "Usage : ./stop_OV_RC.sh <process name> <argument>"
else

if [ "$2" = "" ]; then
prog=$1
arg=""
pid=`ps aux | grep "$1" | grep -v "stop_OV_RC.sh" | grep -v "emacs" | grep -v "grep" | grep -v "tail -f" | awk '{print $2}' `
else
prog=$1
arg=$2
pid=`ps aux | grep "$1 $2" | grep -v "stop_OV_RC.sh" | grep -v "emacs" | grep -v "grep" | grep -v "tail -f" | awk '{print $2}' `
fi

#pid=`ps aux | grep "$1" | grep -v "launch.sh" | grep -v "emacs" | grep -v "grep" | awk '{print $2}' `

if [ "$pid" != "" ]; then
echo "Found previous process $pid - running $1, killing ..."
echo "Killing process $1 at $tim" >> $DCONLINE_PATH/DCOV/log/"$1".log
kill -9 $pid
fi

fi
exit 0
