#!/bin/bash -l

export HOME=/home/dconline

#ROOT
export ROOTSYS=/home/dconline/root
export LD_LIBRARY_PATH=$ROOTSYS/lib:$LD_LIBRARY_PATH
export PATH=$ROOTSYS/bin:$PATH
alias root='root -l'

#DOGS
export DOGS_PATH=$HOME/dchooz/DOGS
export DOGS_DATA=$DOGS_PATH/DCDB
export LD_LIBRARY_PATH=$DOGS_PATH/lib:$LD_LIBRARY_PATH
export PATH=$DOGS_PATH/bin:$PATH

#DC ONLINE
export DCONLINE_PATH=${HOME}/dchooz/dconline/trunk
export PATH=${PATH}:${DCONLINE_PATH}/bin

tim=`date`
prog=DCOVHVGUI

pid=`ps aux | grep "controlGUI.DCHVMain OV 1" | grep -v "start_HV_GUI.sh" | grep -v "emacs" | grep -v "grep" | awk '{print $2}' `

if [ "$pid" != "" ]; then
echo "Found previous process $pid - running $1, killing ..."

echo "Killing process $1 at $tim" >> $DCONLINE_PATH/DCOV/log/DCOV_HV_GUI.log&
kill -9 $pid
fi

