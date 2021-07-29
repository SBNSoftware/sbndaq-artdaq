#!/bin/zsh
source ~/.profile

cd $DCONLINE_PATH/DCOV/readout/startupscript
# ./stopOV_GUI.sh
#
# DCRunControlGUI
./start_launch.sh $DCONLINE_PATH/bin startRunControlGUI
#
# DCGaibuGUI
./start_launch.sh $DCONLINE_PATH/bin startGaibuGUI
#
# DCMonitorViewerGUI
./start_launch.sh $DCONLINE_PATH/bin startMonitorGUI
#
# DCLogMessageServerGUI
./start_launch.sh $DCONLINE_PATH/bin startLogMessageGUI
