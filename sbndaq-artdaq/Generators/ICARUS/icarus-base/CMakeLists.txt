link_directories($ENV{CAENCOMM_LIB})

cet_add_compiler_flags(-Wno-deprecated-declarations )

cet_make_library(
  LIBRARY_NAME
    icarus-base_A2795Board
  SOURCE
    Cudpux.cc A2795Board.cc veto.cc
  LIBRARIES
    artdaq_DAQdata
    CAENComm
)

cet_make_library(
  LIBRARY_NAME
    icarus-base_VetoUDP
  SOURCE
    VetoUDP.cc
  LIBRARIES
    artdaq_DAQdata
    icarus-base_A2795Board
)

cet_make_library(
  LIBRARY_NAME
    icarus-base_PhysCrate
  SOURCE
    PhysCrate.cc
  LIBRARIES
    artdaq_DAQdata
   icarus-base_A2795Board
)

cet_make_exec(VetoUDP
  SOURCE
    VetoUDP_main.cc
  LIBRARIES
    artdaq_DAQdata
    icarus-base_VetoUDP
)

install_source()
install_headers()
