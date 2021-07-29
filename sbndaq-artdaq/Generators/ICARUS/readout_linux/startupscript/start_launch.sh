#!/bin/bash

# only run in local mode
export HOME=/local/home/dconline

#ROOT
source ${HOME}/myROOT/bin/thisroot.sh
export ROOTSYS=${HOME}/myROOT
export LD_LIBRARY_PATH=$ROOTSYS/lib:$LD_LIBRARY_PATH
export PATH=$ROOTSYS/bin:$PATH

#
export DOGS_PATH=${HOME}/myDOGS
export DOGS_DATA=$DOGS_PATH/DCDB/data/
export PATH=$DOGS_PATH/bin:$PATH
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$DOGS_PATH/lib

export DCONLINE_PATH=${HOME}/myOnline
export PATH=${PATH}:${DCONLINE_PATH}/bin

export DCGENSPEC=${DOGS_PATH}/DCGLG4sim/generators/DCGenSpec
export DCGENSPEC_DATA=$DCGENSPEC/dat
export LD_LIBRARY_PATH=$DCGENSPEC/lib:$LD_LIBRARY_PATH

mypath=$1
prog=$2
arg=$3

if [ "$1" = "" ] || [ "$2" = "" ]; then
echo "Usage : ./start_launch.sh <process path> <process name> <argument>"
else

if [ "$arg" = "" ]; then
#prog=$1
#arg=""
pid=`ps aux | grep "$prog" | grep -v "launch.sh" | grep -v "emacs" | grep -v "grep" | awk '{print $2}' `
else
#prog=$1
#arg=$2
pid=`ps aux | grep "$prog $arg" | grep -v "launch.sh" | grep -v "emacs" | grep -v "grep" | awk '{print $2}' `
fi

#pid=`ps aux | grep "$1" | grep -v "launch.sh" | grep -v "emacs" | grep -v "grep" | awk '{print $2}' `

tim=`date`
#echo $tim

if [ "$pid" != "" ]; then
echo "Found previous process $pid - running $prog..." #, killing ..."
else
#kill -9 $pid
#fi

echo "Starting process $prog with nohup"

if [ "$arg" = "" ]; then


echo "\nProcess $prog started at $tim\n" >> log/"$prog".log
`nohup $mypath/$prog >> log/"$prog".log &`


else

echo "\nProcess $prog started at $tim\n" >> log/"$prog"_"$arg".log
`nohup $mypath/$prog $arg >> log/"$prog"_"$arg".log &`

fi

fi

fi

exit 0
