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

export DCSLOWCONTROLDIR=$DCONLINE_PATH/DCSlowControl

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

#MORE MAY COME SOON.
if [ "$1" = "" ]; then
    echo "Usage : ./start_LV.sh <process name> [LVmonitor] <$DCONLINE_PATH>"
else
    prog=$1
    if [ "$2" = "" ]; then
#    echo "Usage : ./start_LV.sh <process name> [LVmonitor] <$DCONLINE_PATH>"
	echo "Not path specific assuming running on nfs" 
	myonlinepath="/nfs/dconline/myOnline"
    else
	myonlinepath=$2
    fi
    
    if [ "$1" = "LVmonitor" ]; then
	exec="DCLVMonitorServer"
    fi


# try to kill the process if still running before restarting
    pid=`ps aux | grep "$exec" | grep -v "start_LV.sh" | grep -v "emacs" | grep -v "grep" | awk '{print $2}' `

    if [ "$pid" != "" ]; then
	echo "Found previous process $pid - running $1, killing ..."
	echo "Killing process $1 at $tim" >> ${DCONLINE_PATH}/DCOV/log/"$prog".log
	kill -9 $pid
    fi


    echo "Starting process $1 with nohup"
    echo "Starting process $1 at $tim" >>${DCONLINE_PATH}/DCOV/log/"$prog".log

    if [ "$prog" = "LVmonitor" ]; then
	cd ${DCONLINE_PATH}/DCSlowControl/DCLowVoltServer/bin        
	export DCONLINE_PATH=$myonlinepath
	`nohup ./$exec >> ${HOME}/myOnline/DCOV/log/"$prog".log 2>>${HOME}/myOnline/DCOV/log/"$prog".log < /dev/null &`
    fi
    

fi
exit 0
