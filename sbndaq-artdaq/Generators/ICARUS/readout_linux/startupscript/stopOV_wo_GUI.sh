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
sleep 2
# DCMonitorSkeleton
./stop_launch.sh ${DCONLINE_PATH}/DCMonitor/bin/DCMonitorSkeleton
# DCMonitorServer
./stop_launch.sh $DCONLINE_PATH/DCMonitor/bin/DCMonitorServer
# DCGaibuServer
./stop_launch.sh $DCONLINE_PATH/DCGaibuServer/bin/DCGaibuServer
# DCLogMessageServer
./stop_launch.sh $DCONLINE_PATH/DCLogMessageServer/bin/DCLogMessageServer
# DCDummyDAQ
#./stop_launch.sh $DCONLINE_PATH/DCRunControl/bin/DCRCDummyDAQ nu
#./stop_launch.sh $DCONLINE_PATH/DCRunControl/bin/DCRCDummyDAQ mu
# DCRunControlServer
./stop_launch.sh $DCONLINE_PATH/DCRunControl/bin/DCRunControlServer
#stop the DCHVControlServer and DCHVMonitorServer on pop
$DCONLINE_PATH/DCOV/readout/startupscript/stop_HV.sh HVserver
$DCONLINE_PATH/DCOV/readout/startupscript/stop_HV.sh HVmonitor
$DCONLINE_PATH/DCOV/readout/startupscript/stop_HV.sh LVmonitor
#stop the OV EBuilder and OMonitor on pop
$DCONLINE_PATH/DCOV/readout/startupscript/stop_OV_tools.sh EventBuilder
$DCONLINE_PATH/DCOV/readout/startupscript/stop_OV_tools.sh DOGSifierOV
$DCONLINE_PATH/DCOV/readout/startupscript/stop_OV_tools.sh DCOVMonitor
#stop the OV DAQ on snap using ssh
#$DCONLINE_PATH/DCOV/readout/script/stop_readout.sh readout
#ssh root@$DAQ_MACHINE "~dconline/dchooz/dconline/trunk/DCOV/readout/script/stop_readout.sh readout"
sleep 2
