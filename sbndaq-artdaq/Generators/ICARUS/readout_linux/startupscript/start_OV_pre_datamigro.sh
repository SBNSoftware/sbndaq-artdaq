#!/bin/bash -l

runnumber=$1

if [ "$2" != "" ]; then
export HOME=$2
else
echo "HOME dir need to be specified, assign default"
export HOME="/local/home/dconline/myOnline"
fi

prog='pre-datamigro'

#DC ONLINE
export DCONLINE_PATH=${HOME}
export DCONLINE_PATH_LOCAL="/local/home/dconline/myOnline"
export PATH=${PATH}:${DCONLINE_PATH}/bin

tim=`date`
thishost=${HOSTNAME}

# starting the Process Monitor
cd ${DCONLINE_PATH_LOCAL}/DCOV/readout/macro
# pass just the folder where the data will be
`nohup perl $prog.pl $runnumber $HOME >> ${DCONLINE_PATH_LOCAL}/DCOV/log/"$prog".log 2>>${DCONLINE_PATH_LOCAL}/DCOV/log/"$prog".log < /dev/null &`

exit 0
