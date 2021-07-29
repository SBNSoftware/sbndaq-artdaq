#source ~/.profile

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

fi

exit 0
