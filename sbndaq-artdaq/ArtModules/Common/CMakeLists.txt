

list(APPEND BUILD_PLUGIN_CORE_LIB_LIST sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_Common)
list(APPEND BUILD_PLUGIN_FRAGNAMEHELPER_LIB_LIST  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_Common)


cet_build_plugin( IncompleteEventAna art::module LIBRARIES REG ${BUILD_PLUGIN_CORE_LIB_LIST} )
if ( ${BUILD_SBND} )
cet_build_plugin( EventAna art::module LIBRARIES REG ${BUILD_PLUGIN_CORE_LIB_LIST} )
endif()
cet_build_plugin( CAENV1730Dump art::module LIBRARIES REG ${BUILD_PLUGIN_CORE_LIB_LIST} )
cet_build_plugin( CAENV1730WaveformAna art::module LIBRARIES REG ${BUILD_PLUGIN_CORE_LIB_LIST} )
cet_build_plugin( BernCRTZMQAna art::module LIBRARIES REG ${BUILD_PLUGIN_CORE_LIB_LIST} )
cet_build_plugin( BernCRTAna art::module LIBRARIES REG ${BUILD_PLUGIN_CORE_LIB_LIST} )
cet_build_plugin( BernCRTAnaV2 art::module LIBRARIES REG ${BUILD_PLUGIN_CORE_LIB_LIST} )
cet_build_plugin( BernCRTDumpV2 art::module LIBRARIES REG ${BUILD_PLUGIN_CORE_LIB_LIST} )
cet_build_plugin( MultiDump art::module LIBRARIES REG ${BUILD_PLUGIN_CORE_LIB_LIST} )

