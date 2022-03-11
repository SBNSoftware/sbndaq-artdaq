cet_make(
 NO_PLUGINS # WAnt different link lists for modues
 BASENAME_ONLY
 LIBRARY_NAME
   sbndaq-artdaq_ArtModules_SBND
 LIB_LIBRARIES
   sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays
   sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_Common
   sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_SBND
)

build_plugin( CoincidenceFilter "module" LIBRARIES REG ${BUILD_PLUGIN_CORE_LIB_LIST} )
build_plugin( testStandTriggerProducer "module" LIBRARIES REG ${BUILD_PLUGIN_CORE_LIB_LIST} )
