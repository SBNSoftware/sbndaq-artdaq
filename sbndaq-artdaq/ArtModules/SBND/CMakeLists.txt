
list(APPEND BUILD_PLUGIN_CORE_LIB_LIST sbndaq-artdaq_Common_Utilities sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_SBND artdaq::DAQdata artdaq_core::artdaq-core_Data ${ROOT_BASIC_LIB_LIST} ROOT::Spectrum)
list(APPEND BUILD_PLUGIN_FRAGNAMEHELPER_LIB_LIST  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_SBND)

cet_build_plugin(SBNDArtdaqBuildInfo art::module LIBRARIES REG ${BUILD_PLUGIN_BUILDINFO_LIB_LIST})
#cet_build_plugin( PMTmetrics art::module LIBRARIES REG ${BUILD_PLUGIN_CORE_LIB_LIST} )
cet_build_plugin( EventAna art::module LIBRARIES REG ${BUILD_PLUGIN_CORE_LIB_LIST} )
cet_build_plugin( CRTHitAna art::module LIBRARIES REG ${BUILD_PLUGIN_CORE_LIB_LIST} )
cet_build_plugin( CRTSinglePEAna art::module LIBRARIES REG ${BUILD_PLUGIN_CORE_LIB_LIST} )
cet_build_plugin( SBNDPMTV1730Ana art::module LIBRARIES REG ${BUILD_PLUGIN_CORE_LIB_LIST} )
cet_build_plugin( SBNDGateFilter art::module LIBRARIES REG ${BUILD_PLUGIN_CORE_LIB_LIST} )

include(ArtdaqCorePlugins)
cet_build_plugin(SBND artdaq::fragmentNameHelper LIBRARIES REG ${BUILD_PLUGIN_FRAGNAMEHELPER_LIB_LIST})


add_subdirectory(testStandTrigger)
add_subdirectory(SoftwareTrigger)
