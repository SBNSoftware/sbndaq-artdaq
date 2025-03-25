
list(APPEND BUILD_PLUGIN_CORE_LIB_LIST sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_ICARUS)
list(APPEND BUILD_PLUGIN_CORE_LIB_LIST artdaq::DAQdata)
list(APPEND BUILD_PLUGIN_FRAGNAMEHELPER_LIB_LIST  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_ICARUS)

cet_build_plugin(IcarusArtdaqBuildInfo art::module LIBRARIES REG ${BUILD_PLUGIN_BUILDINFO_LIB_LIST})
cet_build_plugin(PhysCrateDump art::module LIBRARIES REG ${BUILD_PLUGIN_CORE_LIB_LIST})
cet_build_plugin(ICARUSTriggerUDPDump art::module LIBRARIES REG ${BUILD_PLUGIN_CORE_LIB_LIST})
cet_build_plugin(ICARUSGateFilter  art::module LIBRARIES REG ${BUILD_PLUGIN_CORE_LIB_LIST})
cet_build_plugin(PhysCrateWriteICARUSDataFile art::module LIBRARIES REG ${BUILD_PLUGIN_CORE_LIB_LIST})
cet_build_plugin(PhysCrateViewer art::module LIBRARIES REG ${BUILD_PLUGIN_CORE_LIB_LIST})
cet_build_plugin(PhysCrateSimpleAna art::module LIBRARIES REG ${BUILD_PLUGIN_CORE_LIB_LIST})
cet_build_plugin(ICARUSTriggerUDPSimpleAna art::module LIBRARIES REG ${BUILD_PLUGIN_CORE_LIB_LIST})


include(ArtdaqCorePlugins)
cet_build_plugin(Icarus artdaq::fragmentNameHelper LIBRARIES REG ${BUILD_PLUGIN_FRAGNAMEHELPER_LIB_LIST})

cet_build_plugin(IcarusFragmentWatcher art::module LIBRARIES REG
   artdaq::DAQdata
   artdaq_core::artdaq-core_Data
   art::Framework_Services_Registry
   art::Utilities
   messagefacility::MF_MessageLogger
   fhiclcpp::fhiclcpp
   cetlib::cetlib
)

