add_subdirectory(icarus-base)

include_directories( ${CAENCOMM_INC} )
#include_directories( ${CAENVME_INC} )
#include_directories( ${CAENDIGITIZER_INC} )

#cet_find_library( CAENDIGITIZER    NAMES CAENDigitizer    PATHS ENV CAENDIGITIZER_LIB   NO_DEFAULT_PATH )
#cet_find_library( CAEN_VME         NAMES CAENVME          PATHS ENV CAENVME_LIB         NO_DEFAULT_PATH )
cet_find_library( CAENCOMM         NAMES CAENComm         PATHS ENV CAENCOMM_LIB        NO_DEFAULT_PATH )


art_make_library( LIBRARY_NAME sbndaq-artdaq_ICARUS_PhysCrate_GeneratorBase
		  SOURCE PhysCrate_GeneratorBase.cc
		  LIBRARIES
			sbndaq-artdaq-core_Overlays
			sbndaq-artdaq-core_Overlays_ICARUS
			sbndaq-artdaq_Common_Utilities
			artdaq_Generators
			artdaq_Application
			artdaq_DAQdata
			artdaq_core::artdaq-core_Utilities
			artdaq_core::artdaq-core_Data
			${Boost_SYSTEM_LIBRARY}
			${ART_UTILITIES}
			${FHICLCPP}
			${MF_MESSAGELOGGER}
			${MF_UTILITIES}
			${CETLIB}
			${CETLIB_EXCEPT}
		)

simple_plugin(PhysCrateData "generator"
  icarus-base_A2795Board
  icarus-base_VetoUDP
  icarus-base_PhysCrate
  sbndaq-artdaq_ICARUS_PhysCrate_GeneratorBase
  sbndaq-artdaq-core_Overlays
  sbndaq-artdaq-core_Overlays_ICARUS
  sbndaq-artdaq_Common_Utilities
  artdaq_Generators
  artdaq_Application
  artdaq_DAQdata
  artdaq_core::artdaq-core_Utilities
  artdaq_core::artdaq-core_Data
  ${REDIS}
  ${Boost_SYSTEM_LIBRARY}
  ${FHICLCPP}
  ${MF_MESSAGELOGGER}
  ${CETLIB}
  ${CETLIB_EXCEPT}
  ${CAENCOMM}
  )

# simple_plugin(PhysCrateFakeData "generator"
#   sbndaq-artdaq_ICARUS_PhysCrate_GeneratorBase
#   sbndaq-artdaq-core_Overlays
#   sbndaq-artdaq-core_Overlays_ICARUS
#   sbndaq-artdaq_Common_Utilities
#   artdaq_Application
#   artdaq_DAQdata
#   artdaq-core_Utilities
#   artdaq-core_Data
#   ${Boost_SYSTEM_LIBRARY}
#   ${FHICLCPP}
#   ${MF_MESSAGELOGGER}
#   ${CETLIB}
#   ${CETLIB_EXCEPT}
#   )

simple_plugin(ICARUSTriggerUDP "generator"
#    artdaq-core-demo_Overlays
    sbndaq-artdaq-core_Overlays
    sbndaq-artdaq-core_Overlays_ICARUS
    sbndaq-artdaq_Common_Utilities
    artdaq_Generators
    artdaq_Application
    artdaq_DAQdata
    artdaq_core::artdaq-core_Utilities
    artdaq_core::artdaq-core_Data
  ${Boost_SYSTEM_LIBRARY}
  canvas
    ${FHICLCPP}
  ${MESSAGEFACILITY_LIBS}
    ${CETLIB}
 ${CETLIB_EXCEPT}
    )

#JCZ - obsolete interpretation
#simple_plugin(ICARUSPMTGate "generator"
#    artdaq-core-demo_Overlays
#    sbndaq-artdaq-core_Overlays
#    sbndaq-artdaq-core_Overlays_ICARUS
#    sbndaq-artdaq_Common_Utilities
#    artdaq_Generators
#    artdaq_DAQdata
#    artdaq-core_Utilities
#    artdaq-core_Data
#  ${Boost_SYSTEM_LIBRARY}
#  canvas
#    ${FHICLCPP}
#  ${MESSAGEFACILITY_LIBS}
#    ${CETLIB}
# ${CETLIB_EXCEPT}
#    )
