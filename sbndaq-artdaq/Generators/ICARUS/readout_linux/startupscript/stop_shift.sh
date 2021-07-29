#!/bin/zsh
source ~/.profile

REMOTEHOST="crackle.nevis.columbia.edu"
DAQHOST="snap.nevis.columbia.edu"

USER="dconline"

REMOTE_PATH="/local/home/dconline/dchooz/dconline/trunk/DCOV/readout/startupscript"

echo "Stopping GUI interface on localhost"
cd ${DCONLINE_PATH}/DCOV/readout/startupscript

./stopOV_GUI.sh
ssh -Y -l $USER $REMOTEHOST "ssh -Y -l $USER crackle.nevis.columbia.edu "${REMOTE_PATH}/stop_HV_GUI.sh""

sleep 10

echo "Stopping all the server processes on $HOST with username $USER"

ssh -l $USER $REMOTEHOST "nohup ${REMOTE_PATH}/stopOV_wo_GUI.sh $DAQHOST >>${REMOTE_PATH}/log/stopOV_wo_GUI.log 2>>${REMOTE_PATH}/log/stopOV_wo_GUI.log < /dev/null &"
sleep 10



