# NOTE: THIS FILE SHOULD ONLY BE SOURCED

# always start by renewing kticket
kticket

# get artdaq
source /grid/fermiapp/products/artdaq/setup

# keep alias set by daq interface setup
shopt -s expand_aliases

# get path to this script for location stuff
THISDIR=`dirname ${BASH_SOURCE[0]}`

# use THISDIR to get path to DAQInterface stuff
DAQINTERFACE_CONFIG_DIR=$THISDIR/../DAQInterfaceSetup

# setup artdaq_daqinterface v3_12_06
if [[ -z $DAQINTERFACE_STANDARD_SOURCEFILE_SOURCED ]]; then
   source $DAQINTERFACE_CONFIG_DIR/setup_daq_interface_basic.sh
fi

# startup daq interface processes
DAQInterface &

# give some time for DAQInterface to start
sleep 5

# Config for DAQInterface
CONFIG=ConfigA # Basleine config for new DAQ testing
BOOT=$DAQINTERFACE_CONFIG_DIR/boot.txt
COMPS="tpc01"

echo "RAN WITH CONFIG: $CONFIG"

# do it
if [[ -z $1 ]]
then
  just_do_it.sh $BOOT 0 --config $CONFIG --comps $COMPS
else
  just_do_it.sh $BOOT 0 --config $CONFIG --comps $COMPS --run_number $1
fi

# kill on cleanup
echo "FINISHING CLEANUP"
kill_daqinterface_on_partition.sh $DAQINTERFACE_PARTITION_NUMBER
# copy over trace file

# save the TRACE buffer from the run
# get the last run number from the number of files in /daqdata/logs/daq_interface_run_records
# a bit hacky, but should work for now
last_run_no=$(ls /daqdata/log/daq_interface_run_records/ | wc -l)

# save the last trace buffer to /daqdata/log
# echo "SAVING TRACE FILE"
# cp /tmp/trace_debug_sbnd_daq /daqdata/log/daq_trace_files/sbnd_r${last_run_no}_trace_log


