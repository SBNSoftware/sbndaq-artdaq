#!/bin/bash -l


if [ "$1" != "" ]; then
export HOME=$1
else
echo "HOME dir need to be specified, assign default"
export HOME="/local/home/dconline"
fi

runnumber=$2

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
export DCONLINE_PATH=${HOME}/myOnline
export PATH=${PATH}:${DCONLINE_PATH}/bin

tim=`date`
prog="dc_ov_calib"
thishost=${HOSTNAME}

pid=`ps aux | grep "$prog" | grep -v "start_OV_tools.sh" | grep -v "emacs" | grep -v "grep" | awk '{print $2}' `


if [ "$pid" != "" ]; then
echo "Found previous process $pid - running $prog on ${thishost}, killing ..."
echo "Killing process $prog at $tim" >> ${DCONLINE_PATH}/DCOV/log/"$prog".log
kill -9 $pid
fi

echo "Starting process $prog on ${thishost}"
echo "Starting process $prog at $tim" >>${DCONLINE_PATH}/DCOV/log/"$prog".log

# starting the Process Monitor
cd ${DOGS_PATH}/DCCalibAna/DCOVCalib
# pass just the folder where the data will be
`nohup ./dc_ov_calib -t fardet -f $DOGS_PATH/DATA/*${runnumber}* -p $DOGS_PATH/DCCalibAna/DCOVCalib/output/Run_${runnumber} -w >> ${DOGS_PATH}/DCCalibAna/DCOVCalib/log/Run_${runnumber}.log 2>>${DOGS_PATH}/DCCalibAna/DCOVCalib/log/Run_${runnumber}.log < /dev/null &`

exit 0
