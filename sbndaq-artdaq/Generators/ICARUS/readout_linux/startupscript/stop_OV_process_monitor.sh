#!/bin/bash -l

export HOME=$1
#ROOT
source ${HOME}/myROOT/bin/thisroot.sh   
export ROOTSYS=${HOME}/myROOT
export LD_LIBRARY_PATH=$ROOTSYS/lib:$LD_LIBRARY_PATH
export PATH=$ROOTSYS/bin:$PATH
alias root='root -l'

#DOGS
export DOGS_PATH=$HOME/myDOGS
export DOGS_DATA=$DOGS_PATH/DCDB
export LD_LIBRARY_PATH=$DOGS_PATH/lib:$LD_LIBRARY_PATH
export PATH=$DOGS_PATH/bin:$PATH

#DC ONLINE
export DCONLINE_PATH=${HOME}
export DCONLINE_PATH_LOCAL=/local/home/dconline/myOnline
export PATH=${PATH}:${DCONLINE_PATH}/bin

tim=`date`
prog="OV_Process_Monitor"
thishost=${HOSTNAME}
pid=`ps aux | grep "$prog" | grep -v "start_OV_tools.sh" | grep -v "emacs" | grep -v "grep" | awk '{print $2}' `


if [ "$pid" != "" ]; then
echo "Found previous process $pid on ${thishost} - running $prog, killing ..."
echo "Killing process $prog at $tim" >> ${DCONLINE_PATH_LOCAL}/DCOV/log/"$prog".log
kill -9 $pid
fi

exit 0
