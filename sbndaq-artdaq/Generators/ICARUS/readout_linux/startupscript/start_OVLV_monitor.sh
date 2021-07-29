#!/bin/bash -l

if [ "$1" != "" ]; then
export HOME=$1
else
echo "HOME dir need to be specified, assign default"
export HOME="/local/home/dconline/myOnline"
fi

#DC ONLINE
export DCONLINE_PATH=${HOME}
export DCONLINE_PATH_LOCAL=/local/home/dconline/myOnline
export PATH=${PATH}:${DCONLINE_PATH}/bin

tim=`date`
prog="OV_LV_Monitor"
thishost=${HOSTNAME}

pid=`ps aux | grep "$prog" | grep -v "start_OV_tools.sh" | grep -v "emacs" | grep -v "grep" | awk '{print $2}' `


if [ "$pid" != "" ]; then
echo "Found previous process $pid on ${thishost} - running $prog, killing ..."
echo "Killing process $prog at $tim" >> ${DCONLINE_PATH_LOCAL}/DCOV/log/"$prog".log
kill -9 $pid
fi

echo "Starting process $prog on ${thishost}"
echo "Starting process $prog at $tim" >>${DCONLINE_PATH_LOCAL}/DCOV/log/"$prog".log

# starting the Process Monitor
cd ${DCONLINE_PATH_LOCAL}/DCOV/tools
# pass just the folder where the data will be
`nohup perl $prog.pl >> ${DCONLINE_PATH_LOCAL}/DCOV/log/"$prog".log 2>>${DCONLINE_PATH_LOCAL}/DCOV/log/"$prog".log < /dev/null &`


exit 0
