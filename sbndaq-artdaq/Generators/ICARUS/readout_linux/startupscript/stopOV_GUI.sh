#!/bin/zsh
source ~/.profile

cd $DCONLINE_PATH/DCOV/readout/startupscript
# DCRunControlGUI
./stop_launch.sh RunControlGUI
# DCGaibuGUI
./stop_launch.sh GaibuViewer
# DCMonitorGUI
./stop_launch.sh MonitorViewer
# DCLogMessageServer
./stop_launch.sh DCLogMessageViewerJavaGUI
sleep 2
