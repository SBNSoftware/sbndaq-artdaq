source /daq/software/products/setup
source /daq/software/products_dev/setup
setup mrb
source /home/nfs/$USER/sbndaq/localProducts_sbndaq_v*/setup
mrbslp

echo -e "\n\nkilling all running artdaq processes before starting"
killall -9 artdaqDriver
killall -9 art

#Create run directory
timestamp=`date +%Y%m%d%H%M%S`
runname="/home/nfs/$USER/runs/Run"$timestamp # NFS disk
mkdir -p $runname
cd $runname
logname="Run"$timestamp".log"
echo -e "\n\nData will be saved at $runname \n\n"

export TRACE_MSGMAX=0
export TRACE_FILE=/run/user/${UID}/tbuffer

treset # reset trace buffer
toffSg 0-63 # disable all trace names from stdout
toffMg 0-63 # disable all trace names from memory
tonM 0-63 -n Crate # enable Crate
tonM 0-2 -n ControllerModule # enable ControllerModule (up to info level)
tonM 0-63 -n TPCFEM # enable TPCFEM
tonM 0-63 -n TriggerModule # enable TriggerModule
tonM 0-63 -n XMITModule # enable XMITModule
tonM 0-63 -n XMITReader # enable XMITReader
tonM 0-1 -n XMITReader # enable XMITReader on stdout
tonM 0-19 -n NevisControllerPCIeCard # enable NevisControllerPCIeCard (less verbose)
#tonM 0-63 -n NevisControllerPCIeCard # enable NevisControllerPCIeCard
tonM 0-63 -n NevisReadoutPCIeCard # enable NevisReadoutPCIeCard
tonM 0-3 -n NevisPCIeCard # enable NevisPCIeCard (up to debug level)
#tonM 0-63 -n NevisPCIeCard # enable NevisPCIeCard
#tonM 0-17 -n NevisTPCGenerator # enable generator (up to fillfrag level)
tmodeS 1
tmodeM 1

echo -e "\n\nType 1 - 6 to choose the fcl driver to run:"
echo -e "\t(1) WIB data: 2-stream with External triggers"
echo -e "\t(2) WIB data: 2-stream with Internal triggers (1 Hz)"
echo -e "\t(3) WIB data: 2-stream with Controller triggers (1 Hz)"
echo -e "\t(4) Nevis fake data: 2-stream with external triggers"
echo -e "\t(5) Nevis fake data: 2-stream with internal triggers (1 Hz)"
echo -e "\t(6) Nevis fake data: 2-stream with controller triggers (1 Hz)"
echo -e "\n--> Press Ctrl + C anytime to stop the DAQ <--\n"
read DRVR
if [ "$DRVR" = "1" ]; then
    thefcl=/home/nfs/$USER/sbndaq/srcs/sbndaq/dab/artdaqDriver/NevisTPC2StreamEXT_driver.fcl
elif [ "$DRVR" = "2" ]; then
    thefcl=/home/nfs/$USER/sbndaq/srcs/sbndaq/dab/artdaqDriver/NevisTPC2StreamCALIB_driver.fcl
elif [ "$DRVR" = "3" ]; then
    thefcl=/home/nfs/$USER/sbndaq/srcs/sbndaq/dab/artdaqDriver/NevisTPC2StreamCTRL_driver.fcl
elif [ "$DRVR" = "4" ]; then
    thefcl=/home/nfs/$USER/sbndaq/srcs/sbndaq/dab/artdaqDriver/fake_NevisTPC2StreamEXT_driver.fcl
elif [ "$DRVR" = "5" ]; then
    thefcl=/home/nfs/$USER/sbndaq/srcs/sbndaq/dab/artdaqDriver/fake_NevisTPC2StreamCALIB_driver.fcl
elif [ "$DRVR" = "6" ]; then
    thefcl=/home/nfs/$USER/sbndaq/srcs/sbndaq/dab/artdaqDriver/fake_NevisTPC2StreamCTRL_driver.fcl
else
    echo -e "\n\nOption not recognized. Run manually the fcl driver file\n\n"
    exit 1
fi

echo -e "\n\n Running $thefcl \n\n"
tonM 0-3 -n NevisTPC2StreamGeneratorNUandSNXMIT # enable generator (up to debug level)
tonS 0-3 -n NevisTPC2StreamGeneratorNUandSNXMIT
artdaqDriver -c $thefcl | tee $logname
#artdaqDriver -c $thefcl | tee $logname &
#sleep 120m # let it run for 2h

echo "killing artdaq processes"
killall -9 artdaqDriver
killall -9 art

# Currently requires to Ctr+C to kill it
# Read the trace buffer
echo -e "\n\n To read the TRACE buffer type\n tshow -F |tdelta -d 1 -ct 1 | less"

#cd $HOME # Do not return to $HOME to be able to append the decode/plot script after this
