#!/bin/bash -l

export DCONLINE_PATH=$1

#HOME=${DCONLINE_PATH%/*}
HOME=/local/home/dconline

#ROOT
source ${HOME}/myROOT/bin/thisroot.sh   
export ROOTSYS=${HOME}/myROOT
export LD_LIBRARY_PATH=$ROOTSYS/lib:$LD_LIBRARY_PATH
export PATH=$ROOTSYS/bin:$PATH
alias root='root -l'

#DOGS
export DOGS_PATH=$HOME/myDOGS
export DOGS_DATA=$DOGS_PATH/DCDB
export LD_LIBRARY_PATH=$DOGS_PATH/lib:$LD_LIBRARY_PATH
export PATH=$DOGS_PATH/bin:$PATH

#DC ONLINE
export DCONLINE_PATH_LOCAL=/local/home/dconline/myOnline
export PATH=${PATH}:${DCONLINE_PATH}/bin

tim=`date`

if [ "$2" = "" ]; then
echo "Usage : ./start_OV_tools.sh <process name> <argument> <folder> <runnumber> <options>"

else

if [ "$2" = "EventBuilder" ]; then
prog=$2

if [ "$3" = "" ]; then
echo "Usage : ./start_OV_tools.sh EventBuilder <argument> [<DataFolder>]"
else
prog=$2
arg=$3
disk=$4
runnumber=$5
threshold=$6
trigger=$7
runtype=$8
daqhost=$9
ebdisk=${10}
fi

else
prog=$2
folder=$3

fi

if [ "$prog" = "EventBuilder" ]; then
pid=`ps aux | grep "$2 -r $runnumber" | grep -v "start_OV_tools.sh" | grep -v "emacs" | grep -v "grep" | awk '{print $2}' `
else
pid=`ps aux | grep "$2" | grep -v "start_OV_tools.sh" | grep -v "emacs" | grep -v "grep" | awk '{print $2}' `
fi


if [ "$pid" != "" ]; then
echo "Found previous process $pid - running $2, killing ..."

if [ "$prog" = "EventBuilder" ]; then
echo "Found previous instance of EventBuilder processing RunNumber $runnumber"
cd ${DCONLINE_PATH}/DCOV/tools
`perl notify_gaibu.pl "Found previous instance of EventBuilder processing RunNumber $runnumber"`
fi

echo "Killing process $2 at $tim" >> ${DCONLINE_PATH_LOCAL}/DCOV/log/"$prog".log
kill -9 $pid
fi

echo "Starting process $2 with nohup"
echo "Starting process $2 at $tim" >>${DCONLINE_PATH_LOCAL}/DCOV/log/"$prog".log

if [ "$prog" = "EventBuilder" ]; then
#only if we run event builder with dogsfier
	if [ "$arg" = 1 ]; then
	        # echo "removing ${DOGS_PATH}/DATA/*.root" 
		# rm ${DOGS_PATH}/DATA/*.root
		pid=`ps aux | grep "DOGSifierOV $runnumber" | grep -v "emacs" | grep -v "grep" | grep -v "tail" | grep -v "watch" | awk '{print $2}' `

		# MT: DOGSifier should only be started if it doesn't already exist
		if [ "$pid" = "" ]; then
		    echo "Starting DOGSifier ...at path ${DCONLINE_PATH_LOCAL}"
		    eb_output_path=/data${ebdisk}/OVDAQ/DATA/
		    cd ${DCONLINE_PATH_LOCAL}/DCDOGSifier/bin         #this will run on the local copy
                    `nohup ./DOGSifierOV $runnumber $eb_output_path >> ${DCONLINE_PATH_LOCAL}/DCOV/log/DOGSifier/DOGSifierOV_$runnumber.log 2>>${DCONLINE_PATH_LOCAL}/DCOV/log/DOGSifier/DOGSifierOV_$runnumber.log < /dev/null &`
		    cd ${DCONLINE_PATH_LOCAL}/DCOV/readout/macro
		    `nohup perl checkDF.pl $runnumber >> ${DCONLINE_PATH_LOCAL}/DCOV/log/DCDFMonitor.log 2>>${DCONLINE_PATH_LOCAL}/DCOV/log/DCDFMonitor.log < /dev/null &`
		fi

		#if [ "$pid" != "" ]; then
		#	echo "Found previous process $pid - running DOGSfier, killing ..."
		#	kill -9 $pid
		#fi		

		#echo "Starting DOGSifier ... ${DCONLINE_PATH}"
		#cd ${DCONLINE_PATH}/DCDOGSifier/bin
                #`nohup ./DOGSifierOV $runnumber >> ${DCONLINE_PATH_LOCAL}/DCOV/log/DOGSifierOV.log 2>>${DCONLINE_PATH_LOCAL}/DCOV/log/DOGSifierOV.log < /dev/null &`
	fi 

cd ${DCONLINE_PATH_LOCAL}/DCOV/EBuilder/bin
# 
# pass the data folder as -d where the run number -r will appear and then runnumber
# -D tell the EventBuilder to use the dogsfier
`nohup ./$prog -r $runnumber -D $arg -d $disk -T $trigger -t $threshold -R $runtype -H $daqhost -e $ebdisk >> ${DCONLINE_PATH_LOCAL}/DCOV/log/"$prog".log 2>>${DCONLINE_PATH_LOCAL}/DCOV/log/"$prog".log < /dev/null &`
# 
else
# 
# starting the Online Monitor
cd ${DCONLINE_PATH_LOCAL}/DCOV/OVMonitor/bin
# pass just the folder where the data will be
`nohup ./$prog $folder >> ${DCONLINE_PATH_LOCAL}/DCOV/log/"$prog".log 2>>${DCONLINE_PATH_LOCAL}/DCOV/log/"$prog".log < /dev/null &`


fi

fi

exit 0
