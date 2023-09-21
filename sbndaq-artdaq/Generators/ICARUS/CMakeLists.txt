add_subdirectory(icarus-base)
add_subdirectory(BottomInterface)

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
target_include_directories(sbndaq-artdaq_Generators_ICARUS_PhysCrateData_generator PRIVATE $ENV{REDIS_INC} $ENV{CAENCOMM_INC})
target_link_directories(sbndaq-artdaq_Generators_ICARUS_PhysCrateData_generator PRIVATE $ENV{REDIS_LIB} $ENV{CAENCOMM_LIB})

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
  LIBRARIES REG
  sbndaq-artdaq_ICARUS_BottomInterface_StartBackend
  sbndaq-artdaq_ICARUS_BottomInterface_StopBackend
  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays
  sbndaq-artdaq_Generators_ICARUS_BottomInterface
  artdaq::Application
  artdaq::DAQdata
  artdaq_utilities::artdaq-utilities_Plugins
  
  )

install_source()
install_headers()
