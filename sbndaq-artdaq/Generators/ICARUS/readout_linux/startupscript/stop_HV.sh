#!/bin/bash -l

export HOME=/local/home/dconline

#ROOT
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
export DCONLINE_PATH=${HOME}/myOnline
export PATH=${PATH}:${DCONLINE_PATH}/bin

tim=`date`

if [ "$1" = "" ]; then
    echo "Usage : ./stop_HV.sh <process name> [HVserver]/[LVmonitor]/[HVmonitor]"
else
prog=$1
    if [ "$1" = "HVserver" ]; then
	exec="DCHVControlServer"
    elif [ "$1" = "HVmonitor" ]; then
	exec="DCHVMonitorServer"
    fi

# try to kill the semaphore if defined only in the case of the HVServer
    if [ "$prog" = "HVserver" ] ; then
	semaphore=`ipcs sem | grep "dconline" | grep "666" | awk '{print $2}' `
	if [ "$semaphore" != "" ]; then
	    echo "Found previous semaphore $semaphore - killing ..."
	    echo "Killing process $1 at $tim" >> ${DCONLINE_PATH}/DCOV/log/"$prog".log
	    ipcrm sem $semaphore
	fi
    fi


# try to kill the process if still running before restarting
    pid=`ps aux | grep "$exec" | grep -v "start_HV.sh" | grep -v "emacs" | grep -v "grep" | awk '{print $2}' `

    if [ "$pid" != "" ]; then
	echo "Found previous process $pid - running $1, killing ..."
	echo "Killing process $1 at $tim" >> ${DCONLINE_PATH}/DCOV/log/"$prog".log
	kill -9 $pid
    fi

fi

exit 0
