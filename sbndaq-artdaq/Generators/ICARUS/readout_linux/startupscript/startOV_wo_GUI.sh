#!/bin/bash -l

DAQ_MACHINE=$1
# only run in local mode
export HOME=/local/home/dconline
#ROOT
source ${HOME}/myROOT/bin/thisroot.sh   
export ROOTSYS=${HOME}/myROOT
export LD_LIBRARY_PATH=$ROOTSYS/lib:$LD_LIBRARY_PATH
export PATH=$ROOTSYS/bin:$PATH
alias root='root -l'

#DOGS
export DOGS_PATH=${HOME}/myDOGS
export DOGS_DATA=$DOGS_PATH/DCDB
export LD_LIBRARY_PATH=$DOGS_PATH/lib:$LD_LIBRARY_PATH
export PATH=$DOGS_PATH/bin:$PATH

#DC ONLINE
export DCONLINE_PATH=${HOME}/myOnline
export PATH=${PATH}:${DCONLINE_PATH}/bin

cd $DCONLINE_PATH/DCOV/readout/startupscript
./stopOV_wo_GUI.sh $DAQ_MACHINE
sleep 10
#
# DCMonitorSkeleton
./start_launch.sh ${DCONLINE_PATH}/DCMonitor/bin DCMonitorSkeleton
#
# DCMonitorServer
./start_launch.sh $DCONLINE_PATH/DCMonitor/bin DCMonitorServer
#
# DCGaibuServer
./start_launch.sh $DCONLINE_PATH/DCGaibuServer/bin DCGaibuServer
#
# DCLogMessageServer
./start_launch.sh $DCONLINE_PATH/DCLogMessageServer/bin DCLogMessageServer
#
sleep 5
# DAQ on snap
#ssh root@$DAQ_MACHINE "/local/home/dconline/dchooz/dconline/trunk/DCOV/readout/script/start_OV_RC.sh startOVDAQ.pl"
ssh root@$DAQ_MACHINE "/etc/init.d/OVreadout start"
#
# DCDummyDAQ
#./start_launch.sh $DCONLINE_PATH/DCRunControl/bin DCRCDummyDAQ nu
#./start_launch.sh $DCONLINE_PATH/DCRunControl/bin DCRCDummyDAQ mu
# sleep 2
#
# DCRunControlServer
./start_launch.sh $DCONLINE_PATH/DCRunControl/bin DCRunControlServer
# sleep 2
#
#start the DCHVControlServer and DCHVMonitorServer on pop
#$DCONLINE_PATH/DCOV/readout/startupscript/start_HV.sh HVserver
#sleep 1
#$DCONLINE_PATH/DCOV/readout/startupscript/start_HV.sh HVmonitor
sleep 1
# $DCONLINE_PATH/DCOV/readout/startupscript/start_HV.sh LVmonitor
# sleep 1

