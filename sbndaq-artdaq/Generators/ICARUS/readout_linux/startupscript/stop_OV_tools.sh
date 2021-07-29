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
export DCONLINE_PATH_LOCAL="/local/home/dconline/myOnline"
export PATH=${PATH}:${DCONLINE_PATH}/bin

tim=`date`

if [ "$2" = "" ]; then
echo "Usage : ./stop_OV_tools.sh <process name> <argument>"

else

if [ "$2" = "DOGSifierOV" ]; then

pid=`ps aux | grep "$2" | grep -v "stop_OV_tools.sh" | grep -v "tail" | grep -v "emacs" | grep -v "grep" | awk '{print $2}' `

if [ "$pid" != "" ]; then

echo "Found previous process $pid - running $2, ending ..."
echo "Sending SIGTERM to process $2 at $tim" >> ${DCONLINE_PATH_LOCAL}/DCOV/log/"$prog".log
kill -15 $pid
sleep 5

fi

fi

pid=`ps aux | grep "$2" | grep -v "stop_OV_tools.sh" | grep -v "tail" | grep -v "emacs" | grep -v "grep" | awk '{print $2}' `

if [ "$pid" != "" ]; then

echo "Found previous process $pid - running $2, killing ..."

echo "Killing process $2 at $tim" >> ${DCONLINE_PATH_LOCAL}/DCOV/log/"$prog".log
kill -9 $pid
fi


fi

exit 0
