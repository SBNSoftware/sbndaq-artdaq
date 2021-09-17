add_subdirectory(icarus-base)

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

basic_plugin(PhysCrateData "generator"
  LIBRARIES REG
    ${BUILD_PLUGIN_CORE_LIB_LIST}
    sbndaq-artdaq_ICARUS_PhysCrate_GeneratorBase
    icarus-base_A2795Board
    icarus-base_VetoUDP
    icarus-base_PhysCrate
    hiredis
    CAENComm
)

# basic_plugin(PhysCrateFakeData "generator"
#  LIBRARIES REG
#    ${BUILD_PLUGIN_CORE_LIB_LIST}
#    sbndaq-artdaq_ICARUS_PhysCrate_GeneratorBase
#   )

basic_plugin(ICARUSTriggerUDP "generator"
  LIBRARIES REG
    ${BUILD_PLUGIN_CORE_LIB_LIST}
)

#JCZ - obsolete interpretation
#basic_plugin(ICARUSPMTGate "generator"
#  LIBRARIES REG
#    ${BUILD_PLUGIN_CORE_LIB_LIST}
#    artdaq-core-demo_Overlays
#)

install_source()
install_headers()
