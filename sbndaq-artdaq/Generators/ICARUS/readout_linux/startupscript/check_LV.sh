#!/bin/bash -l

source /local/home/dconline/.bashrc

STARTUPPATH=/local/home/dconline/startupscript
if [ "$1" != "" ]; then
LVMON="$STARTUPPATH/start_LV.sh LVmonitor $1"

if [ "$(pidof ./DCLVMonitorServer)" ]
then
    echo 'Doing nothing LVMonitor Server is running.'
else
    echo 'Restart LMonitor'
    eval $LVMON
    echo 'Done -  LVMonitor'
    exit
fi
    
else
echo 'usage: ./check_LV.sh <LVmonitor> <DCONLINE_PATH>'
fi
