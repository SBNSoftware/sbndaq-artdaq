#!/bin/bash -l

export HOME="/local/home/dconline/myOnline"

#DC ONLINE
export DCONLINE_PATH=${HOME}
export DCONLINE_PATH_LOCAL=/local/home/dconline/myOnline
export PATH=${PATH}:${DCONLINE_PATH}/bin

tim=`date`
prog="erase_data"
prog1="erase_timing_data"

thishost=${HOSTNAME}

pid=`ps aux | grep "$prog" | grep -v "start_OV_cleanup.sh" | grep -v "emacs" | grep -v "grep" | awk '{print $2}' `
pid1=`ps aux | grep "$prog1" | grep -v "start_OV_cleanup.sh" | grep -v "emacs" | grep -v "grep" | awk '{print $2}' `


if [ "$pid" != "" ]; then
echo "Found previous process $pid on ${thishost} - running $prog, killing ..."
echo "Killing process $prog at $tim" >> ${DCONLINE_PATH_LOCAL}/DCOV/log/"$prog".log
kill -9 $pid
fi

if [ "$pid1" != "" ]; then
echo "Found previous process $pid1 on ${thishost} - running $prog1, killing ..."
echo "Killing process $prog1 at $tim" >> ${DCONLINE_PATH_LOCAL}/DCOV/log/"$prog1".log
kill -9 $pid1
fi

echo "Starting process $prog and later $prog1 on ${thishost}"
echo "Starting process $prog at $tim" >>${DCONLINE_PATH_LOCAL}/DCOV/log/"$prog".log

# starting the Cleanup process
cd ${DCONLINE_PATH}/DCOV/readout/macro

# pass just the folder where the data will be
`nohup perl $prog.pl >> ${DCONLINE_PATH_LOCAL}/DCOV/log/"$prog".log 2>>${DCONLINE_PATH_LOCAL}/DCOV/log/"$prog".log < /dev/null &`
`nohup perl $prog1.pl >> ${DCONLINE_PATH_LOCAL}/DCOV/log/"$prog1".log 2>>${DCONLINE_PATH_LOCAL}/DCOV/log/"$prog1".log < /dev/null &`

exit 0
