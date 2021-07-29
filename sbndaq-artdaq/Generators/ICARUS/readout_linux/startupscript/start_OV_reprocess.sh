#!/bin/bash -l

export HOME="/local/home/dconline"

prog='Offline_reprocess_N'

#DC ONLINE
export DCONLINE_PATH=${HOME}/myOnline
export DCONLINE_PATH_LOCAL="/local/home/dconline/myOnline"
export PATH=${PATH}:${DCONLINE_PATH}/bin

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

tim=`date`
thishost=${HOSTNAME}

# starting the Process Monitor
cd ${DCONLINE_PATH_LOCAL}/DCOV/readout/macro
# pass just the folder where the data will be

`nohup perl $prog.pl >> ${DCONLINE_PATH_LOCAL}/DCOV/log/"$prog".log 2>>${DCONLINE_PATH_LOCAL}/DCOV/log/"$prog".log < /dev/null &`

exit 0
