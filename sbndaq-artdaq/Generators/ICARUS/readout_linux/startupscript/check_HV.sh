#!/bin/bash -l

source /local/home/dconline/.bashrc

STARTUPPATH=/local/home/dconline/startupscript
HVCON="$STARTUPPATH/start_HV.sh HVserver"
HVMON="$STARTUPPATH/start_HV.sh HVmonitor"

if [ "$(pidof ./DCHVControlServer)" ]
then
    echo 'Doing Nothing Control Server is running.'
else
    echo 'Restarted Control and Monitor due to Control'
    eval $HVCON
    eval $HVMON
    echo 'Done - Control and Monitor due to Control'
    exit
fi

if [ "$(pidof ./DCHVMonitorServer)" ]
then
    echo 'Doing nothing Monitor Server is running.'
else
    echo 'Restart Control and Monitor due to Monitor' 
    eval $HVCON
    eval $HVMON
    echo 'Done -  Control and Monitor due to Monitor'
    exit
fi
    