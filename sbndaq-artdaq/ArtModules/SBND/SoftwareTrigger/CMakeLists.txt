
list(APPEND BUILD_PLUGIN_CORE_LIB_LIST sbndaq-artdaq_Common_Utilities sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_SBND artdaq::DAQdata artdaq_core::artdaq-core_Data)

#cet_build_plugin(MetricProducer art::module LIBRARIES REG ${BUILD_PLUGIN_CORE_LIB_LIST} )
cet_build_plugin(CRTMetricProducer art::module LIBRARIES REG ${BUILD_PLUGIN_CORE_LIB_LIST} )
cet_build_plugin(PMTMetricProducer art::module LIBRARIES REG ${BUILD_PLUGIN_CORE_LIB_LIST} )
cet_build_plugin(NeutrinoStreamFilter art::module LIBRARIES REG ${BUILD_PLUGIN_CORE_LIB_LIST} )
