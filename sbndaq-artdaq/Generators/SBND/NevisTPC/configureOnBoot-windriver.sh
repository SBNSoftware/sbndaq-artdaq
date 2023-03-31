#!/bin/bash
# set up drivers for sbnd
#GAL: instructions https://cdcvs.fnal.gov/redmine/projects/sbndaq/wiki/Rebuild_the_windriver

install_windriver1260="\
sbn-daq01,\
sbnd-tpc01,\
sbnd-tpc02,\
sbnd-tpc03,\
sbnd-tpc04,\
sbnd-tpc05,\
sbnd-tpc06,\
sbnd-tpc07,\
sbnd-tpc08,\
sbnd-tpc09,\
sbnd-tpc10,\
sbnd-tpc11"


if [[ $install_windriver1260 =~ (^|,)$(hostname -s)($|,) ]]; then
 echo "Info: Configuring Windriver 12.06."
 . /daq/software/products/setup
 setup windriver v12_06_00 -q prof
 if [ $? -ne 0 ]; then 
  echo "Error: Windriver 12.06 product setup failed."
  exit 1
 fi  

 if [ ! -f  /lib/modules/$(uname -r)/kernel/drivers/misc/windrvr1260.ko ]; then
    if [ ! -d "$WINDRIVER_DIR/source/WinDriver/redist/" ]; then
       echo "Error: Windriver source was not found."
       exit 1
    fi
    echo "Info: Installing Windriver 12.06."
    cd $WINDRIVER_DIR/source/WinDriver/redist/
     ./configure
     make install
     wdreg windrvr1260 auto
     chown -R artdaq $WINDRIVER_DIR/source
     if [ ! -f  /lib/modules/$(uname -r)/kernel/drivers/misc/windrvr1260.ko ]; then
       echo "Error: Windriver installation failed."
       exit 1
     fi
 fi

 wdreg windrvr1260 auto

 chmod a+rw /dev/windrvr1260

 if [ $(lsmod |grep windrvr1260|wc -l) >0 ];then
   echo "Info: Windriver 12.06 is running."
 else
   echo "Error: Windriver 12.06 is not running."
 fi
fi

