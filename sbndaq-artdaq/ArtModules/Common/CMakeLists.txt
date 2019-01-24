art_make(NO_PLUGINS # WAnt different link lists for modues
 BASENAME_ONLY
 LIBRARY_NAME sbndaq-artdaq_ArtModules_Common
 LIB_LIBRARIES sbndaq-artdaq-core_Overlays
 	       sbndaq-artdaq-core_Overlays_Common
)

simple_plugin( CAENV1730Dump "module"
  art_Framework_Services_Registry
  art_Framework_Services_Optional
  art_Framework_Services_Optional_TFileService_service
  ${ROOT_BASIC_LIB_LIST}
  sbndaq-artdaq-core_Overlays_Common 
)

simple_plugin( CAENV1730WaveformAna "module"
  art_Framework_Services_Registry
  art_Framework_Services_Optional
  art_Framework_Services_Optional_TFileService_service
  ${ROOT_BASIC_LIB_LIST}
  sbndaq-artdaq-core_Overlays_Common 
)

# Subdirectories.
#add_subdirectory(DQM)
