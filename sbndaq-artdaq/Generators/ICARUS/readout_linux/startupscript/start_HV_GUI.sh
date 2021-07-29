#!/bin/bash


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

#Java
unset CLASSPATH


cd $DCONLINE_PATH/DCOV/readout/startupscript

tim=`date`
echo "starting ......"

#export DISPLAY=:0

prog=DCOVHVGUI

pid=`ps aux | grep "java controlGUI.DCHVMain OV" | grep -v "start_HV_GUI.sh" | grep -v "emacs" | grep -v "grep" | awk '{print $2}' `

if [ "$pid" != "" ]; then
echo "Found previous process $pid - running $1, killing ..."

echo "Killing process $1 at $tim" >> $DCONLINE_PATH/DCOV/log/DCOV_HV_GUI.log&
kill -9 $pid
fi

echo '<Starting DCHVcontrolGUI for OV...>'

classPath=$DCONLINE_PATH/DCSlowControl_OVHV/DCHVControl:$DCONLINE_PATH/DCTGUJava/classes:$DOGS_PATH/DCOnlineDB/interfaces/java/lib/dc_online_db.jar:/usr/share/java/mysql-connector-java.jar

object=controlGUI.DCHVMain
log=$DCONLINE_PATH/DCOV/log/DCHVControlGUI.log 


echo "Logs found in: $log"
echo "Using class path: $classPath"
java -classpath $classPath $object OV 1 >> $log &
#java -classpath $classPath $object OV 1

echo '<done>'
#EOF
