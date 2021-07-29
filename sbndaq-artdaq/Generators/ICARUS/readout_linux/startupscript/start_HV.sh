#!/bin/bash -l

export HOME=/local/home/dconline

#ROOT
source ${HOME}/myROOT/bin/thisroot.sh  

#DOGS
export DOGS_PATH=$HOME/myDOGS
export DOGS_DATA=$DOGS_PATH/DCDB
export LD_LIBRARY_PATH=$DOGS_PATH/lib:$LD_LIBRARY_PATH
export PATH=$DOGS_PATH/bin:$PATH

#DC ONLINE
export DCONLINE_PATH=${HOME}/myOnline
export PATH=${PATH}:${DCONLINE_PATH}/bin

export DCSLOWCONTROLDIR=$DCONLINE_PATH/DCSlowControl_OVHV

# For CAEN library
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$DCONLINE_PATH/lib

#Get the needed Java libraries.
#
export CLASSPATH=/usr/share/java/mysql.jar:$CLASSPATH
export CLASSPATH=/usr/share/java/mysql-connector-java.jar:$CLASSPATH
export CLASSPATH=$DOGS_PATH/DCOnlineDB/interfaces/java/lib/dc_online_db.jar:$CLASSPATH
export CLASSPATH=$DCONLINE_PATH/DCTGUJava/classes:$CLASSPATH
export CLASSPATH=$DCSLOWCONTROLDIR/DCHVControl:$CLASSPATH

tim=`date`

if [ "$1" = "" ]; then
    echo "Usage : ./start_HV.sh <process name> [HVserver]/[HVmonitor]"
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


    echo "Starting process $1 with nohup"
    echo "Starting process $1 at $tim" >>${DCONLINE_PATH}/DCOV/log/"$prog".log

    if [ "$prog" = "HVserver" ]; then
	cd ${DCONLINE_PATH}/DCSlowControl_OVHV/DCHVControl/controlServer
	`nohup ./$exec --enableOV >> ${DCONLINE_PATH}/DCOV/log/"$prog".log 2>>${DCONLINE_PATH}/DCOV/log/"$prog".log < /dev/null &`
    elif [ "$prog" = "HVmonitor" ]; then
	cd ${DCONLINE_PATH}/DCSlowControl_OVHV/DCSlowMonitorServer/bin
	`nohup ./$exec --enableOV >> ${DCONLINE_PATH}/DCOV/log/"$prog".log 2>>${DCONLINE_PATH}/DCOV/log/"$prog".log < /dev/null &`
    fi


fi

exit 0
