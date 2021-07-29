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
prog="DCProcessControlBranch"
thishost=${HOSTNAME}

pid=`ps aux | grep "$prog" | grep -v "start_process_control_OV.sh" | grep -v "emacs" | grep -v "grep" | awk '{print $2}' `

if [ "$pid" != "" ]; then
echo "Found previous process $pid on ${thishost} - running $prog, killing ..."
kill -9 $pid
fi

cd ${DCONLINE_PATH}/bin        
export DCONLINE_PATH=$myonlinepath
`./startProcessControlServerBranchOV >> ${HOME}/myOnline/DCOV/log/ProcessControl.log 2>>${HOME}/myOnline/DCOV/log/ProcessControl.log < /dev/null `

exit 0