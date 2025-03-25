source /grid/fermiapp/products/artdaq/setup

# keep alias set by daq interface setup
shopt -s expand_aliases

# These 2 are need to be set if daqinterface is not setup as a ups product
# daqinterface uses ARTDAQ_DAQINTERFACE_DIR and also wants to print its
# own version number (and saves it in the metadata)
export ARTDAQ_DAQINTERFACE_DIR=$HOME/SBND_DAQ3/artdaq-utilities-daqinterface
export ARTDAQ_DAQINTERFACE_VERSION=v3_03_00x

echo SBNDDAQ_READOUT_DIR=$SBNDDAQ_READOUT_DIR
export DAQINTERFACE_USER_SOURCEFILE=$HOME/SBND_DAQ3/dev/srcs/sbnddaq_readout/DAQInterfaceSetup/user_sourcefile
source $ARTDAQ_DAQINTERFACE_DIR/source_me
