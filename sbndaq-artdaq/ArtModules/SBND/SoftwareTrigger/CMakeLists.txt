cet_make(
 NO_PLUGINS # Want different link lists for modules
 BASENAME_ONLY
 LIBRARY_NAME
    sbndaq-artdaq_ArtModules_SBND
 LIB_LIBRARIES
    sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays
    sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_Common
    sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_SBND
)

list(APPEND BUILD_PLUGIN_CORE_LIB_LIST sbndaq-artdaq_Common_Utilities sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_SBND artdaq::DAQdata artdaq_core::artdaq-core_Data)

#build_plugin(MetricProducer "module" LIBRARIES REG ${BUILD_PLUGIN_CORE_LIB_LIST} )
build_plugin(CRTMetricProducer "module" LIBRARIES REG ${BUILD_PLUGIN_CORE_LIB_LIST} )
build_plugin(NeutrinoStreamFilter "module" LIBRARIES REG ${BUILD_PLUGIN_CORE_LIB_LIST} )
