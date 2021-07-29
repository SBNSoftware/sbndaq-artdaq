#!/bin/bash -l

cd /home/dconline/bin

#ROOT
export ROOTSYS=/home/dconline/root
export LD_LIBRARY_PATH=$ROOTSYS/lib:$LD_LIBRARY_PATH
export PATH=$ROOTSYS/bin:$PATH
alias root='root -l'

#DOGS
export DOGS_PATH=~/dchooz/DOGS
export DOGS_DATA=$DOGS_PATH/DCDB
export LD_LIBRARY_PATH=$DOGS_PATH/lib:$LD_LIBRARY_PATH
export PATH=$DOGS_PATH/bin:$PATH

#DC ONLINE
export DCONLINE_PATH=${HOME}/dchooz/dconline/trunk
export PATH=${PATH}:${DCONLINE_PATH}/bin


if [ "$1" = "" ]; then
echo "Usage : ./launch.sh <process name> <argument>"
else

if [ "$2" = "" ]; then
prog=$1
arg=""
pid=`ps aux | grep "$1" | grep -v "launch.sh" | grep -v "emacs" | grep -v "grep" | awk '{print $2}' `
else
prog=$1
arg=$2
pid=`ps aux | grep "$1 $2" | grep -v "launch.sh" | grep -v "emacs" | grep -v "grep" | awk '{print $2}' `
fi

#pid=`ps aux | grep "$1" | grep -v "launch.sh" | grep -v "emacs" | grep -v "grep" | awk '{print $2}' `

if [ "$pid" != "" ]; then
echo "Found previous process $pid - running $1, killing ..."

kill -9 $pid
fi

echo "Starting process $1 with nohup"

if [ "$arg" = "" ]; then

`nohup ./$1 > /home/dconline/log/"$1".log 2>/home/dconline/log/"$1".log < /dev/null &`

else

`nohup ./$1 $arg > /home/dconline/log/"$1".log 2>/home/dconline/log/"$1".log < /dev/null &`


fi

fi


exit 0
