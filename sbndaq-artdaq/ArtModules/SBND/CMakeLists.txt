art_make(NO_PLUGINS # WAnt different link lists for modues
 BASENAME_ONLY
 LIBRARY_NAME sbndaq-artdaq_ArtModules_SBND
 LIB_LIBRARIES sbndaq-artdaq-core_Overlays
 	       sbndaq-artdaq-core_Overlays_Common
 	       sbndaq-artdaq-core_Overlays_SBND
)

simple_plugin(SBNDArtdaqBuildInfo "module"
  artdaq_DAQdata
  artdaq_BuildInfo
  artdaq-core_BuildInfo
  sbndaq-artdaq_BuildInfo
  sbndaq-artdaq-core_BuildInfo
  ${MF_MESSAGELOGGER}
  ${FHICLCPP}
  ${CETLIB}
  ${CETLIB_EXCEPT}
)

simple_plugin(SBNDInput "source"
  sbndaq-artdaq-core_Overlays
  sbndaq-artdaq-core_Overlays_Common
  sbndaq-artdaq-core_Overlays_SBND
  artdaq-core_Core
  artdaq_DAQdata
  artdaq-utilities_Plugins
  artdaq-core_Utilities
  artdaq_ArtModules
  artdaq_ArtModules_ArtdaqSharedMemoryService_service
  art_Framework_Core
  art_Framework_IO_Sources
  art_Framework_Principal
  art_Framework_Services_Registry
  art_Persistency_Provenance
  art_Utilities
  MF_MessageLogger
  ${CETLIB_EXCEPT}
)

# Subdirectories.
#add_subdirectory(DQM)
