#!/bin/zsh
source ~/.profile


REMOTE_HOST="crackle.nevis.columbia.edu"
REMOTE_DAQ="snap.nevis.columbia.edu"

USER="dcdaq"

REMOTE_PATH="~/dchooz/dconline/trunk/DCOV/readout/startupscript"

cd ${DCONLINE_PATH}/DCOV/readout/startupscript
./stopOV_GUI.sh
sleep 10

echo "Starting all the server processes on $HOST with username $USER"

ssh $REMOTE_HOST -l $USER "nohup ${REMOTE_PATH}/stopOV_wo_GUI.sh $REMOTE_DAQ >>${REMOTE_PATH}/log/stopOV_wo_GUI.log 2>>${REMOTE_PATH}/log/stopOV_wo_GUI.log < /dev/null &"
sleep 15

ssh $REMOTE_HOST -l $USER "nohup ${REMOTE_PATH}/startOV_wo_GUI.sh $REMOTE_DAQ >>${REMOTE_PATH}/log/startOV_wo_GUI.log 2>>${REMOTE_PATH}/log/startOV_wo_GUI.log < /dev/null &"
sleep 30




