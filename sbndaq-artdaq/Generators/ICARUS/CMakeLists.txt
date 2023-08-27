add_subdirectory(icarus-base)
add_subdirectory(BottomInterface)

link_directories($ENV{CAENCOMM_LIB})
link_directories($ENV{REDIS_LIB})

list(APPEND LIBRARY_CORE_LIB_LIST sbndaq-artdaq_Common_Utilities sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_ICARUS)

cet_make_library(
  LIBRARY_NAME
    sbndaq-artdaq_ICARUS_PhysCrate_GeneratorBase
  SOURCE
    PhysCrate_GeneratorBase.cc
  LIBRARIES
    ${LIBRARY_CORE_LIB_LIST}
)

cet_build_plugin(PhysCrateData artdaq::commandableGenerator
  LIBRARIES REG
    ${BUILD_PLUGIN_CORE_LIB_LIST}
    sbndaq-artdaq_ICARUS_PhysCrate_GeneratorBase
    icarus-base_A2795Board
    icarus-base_VetoUDP
    icarus-base_PhysCrate
    hiredis
    CAENComm
)

# cet_build_plugin(PhysCrateFakeData artdaq::commandableGenerator
#  LIBRARIES REG
#    ${BUILD_PLUGIN_CORE_LIB_LIST}
#    sbndaq-artdaq_ICARUS_PhysCrate_GeneratorBase
#   )

cet_build_plugin(ICARUSTriggerUDP artdaq::commandableGenerator
  LIBRARIES REG
    ${BUILD_PLUGIN_CORE_LIB_LIST}
)

cet_build_plugin(ICARUSTriggerV2 artdaq::commandableGenerator
  LIBRARIES REG
    ${BUILD_PLUGIN_CORE_LIB_LIST}
)

cet_build_plugin(ICARUSTriggerV3 artdaq::commandableGenerator
  LIBRARIES REG
    ${BUILD_PLUGIN_CORE_LIB_LIST}
)  

#JCZ - obsolete interpretation
#cet_build_plugin(ICARUSPMTGate artdaq::commandableGenerator
#  LIBRARIES REG
#    ${BUILD_PLUGIN_CORE_LIB_LIST}
#    artdaq-core-demo_Overlays
#)

cet_build_plugin(CRTFragGen artdaq::commandableGenerator
  LIBRARIES
  sbndaq-artdaq_ICARUS_BottomInterface_StartBackend
  sbndaq-artdaq_ICARUS_BottomInterface_StopBackend
  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays
  sbndaq-artdaq_Generators_ICARUS_BottomInterface
  artdaq_Application
  #artdaq_Generators 
  artdaq_DAQdata
  #artdaq-core_Utilities
  #artdaq-core_Data
  artdaq-utilities_Plugins
  ${CANVAS_LIB_LIST}
  ${FHICLCPP}
  ${MF_MESSAGELOGGER}
  ${MF_UTILITIES}
  ${CETLIB}
  ${CETLIB_EXCEPT}
  )

install_source()
install_headers()
