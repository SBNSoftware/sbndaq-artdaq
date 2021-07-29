#!/bin/bash -l

Run_number=$1

export HOME=/local/home/dconline
export MANPATH=/usr/share/man # must be before thisroot.sh                                                                                                     
source ${HOME}/myROOT/bin/thisroot.sh
export ROOTSYS=${HOME}/myROOT

export DCONLINE_PATH="/nfs/dconline/myOnline"
export DCONLINE_PATH_LOCAL=${HOME}
export PATH=${PATH}:${DCONLINE_PATH}/bin
export LD_LIBRARY_PATH=$DCONLINE_PATH/lib:$LD_LIBRARY_PATH

export DOGS_PATH=${HOME}/myDOGS
export PATH=$DOGS_PATH/bin:$PATH
export LD_LIBRARY_PATH=$DOGS_PATH/lib:$LD_LIBRARY_PATH
export DOGS_DATA=$DOGS_PATH/DCDB/data/

export EDITOR=emacs

prog='OV_DataMigro_reprocess'

tim=`date`
thishost=${HOSTNAME}

cd ${DCONLINE_PATH}/DCDataMigro/scripts/
source configDCDataMigro@OV.sh

`nohup ./runTransferOV -f ${Run_number} >> ${DCONLINE_PATH_LOCAL}/DCOV/log/"$prog".log 2>>${DCONLINE_PATH_LOCAL}/DCOV/log/"$prog".log < /dev/null &`

exit 0
