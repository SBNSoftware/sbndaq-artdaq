
if(DEFINED ENV{CAENCOMM_DIR})
    link_directories($ENV{CAENCOMM_LIB})
    include_directories($ENV{CAENCOMM_INC})
endif()

cet_make_library(
  LIBRARY_NAME
    icarus-base_A2795Board
  SOURCE
    Cudpux.cc A2795Board.cc veto.cc
  LIBRARIES
    artdaq::DAQdata
    CAENComm
)
set_source_files_properties(A2795Board.cc PROPERTIES COMPILE_FLAGS "-Wno-deprecated-declarations")

cet_make_library(
  LIBRARY_NAME
    icarus-base_VetoUDP
  SOURCE
    VetoUDP.cc
  LIBRARIES
    artdaq::DAQdata
    icarus-base_A2795Board
)

cet_make_library(
  LIBRARY_NAME
    icarus-base_PhysCrate
  SOURCE
    PhysCrate.cc
  LIBRARIES
    artdaq::DAQdata
   icarus-base_A2795Board
)

cet_make_exec(VetoUDP
  SOURCE
    VetoUDP_main.cc
  LIBRARIES
    artdaq::DAQdata
    icarus-base_VetoUDP
)

install_source()
install_headers()
