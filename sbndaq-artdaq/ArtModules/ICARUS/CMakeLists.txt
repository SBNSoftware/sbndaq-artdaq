art_make(NO_PLUGINS # WAnt different link lists for modues
 BASENAME_ONLY
 LIBRARY_NAME sbndaq-artdaq_ArtModules_ICARUS
 LIB_LIBRARIES sbndaq-artdaq-core_Overlays
 	       sbndaq-artdaq-core_Overlays_Common
 	       sbndaq-artdaq-core_Overlays_ICARUS #icarus-artdaq_Products
)

#simple_plugin(IcarusArtdaqBuildInfo "module"
#  artdaq_DAQdata
#  artdaq_BuildInfo
#  artdaq-core_BuildInfo
#  sbndaq-artdaq_BuildInfo
#  sbndaq-artdaq-core_BuildInfo
#  ${MF_MESSAGELOGGER}
#  ${FHICLCPP}
#  ${CETLIB}
#  ${CETLIB_EXCEPT}
#)

simple_plugin(IcarusInput "source"
  sbndaq-artdaq-core_Overlays
  sbndaq-artdaq-core_Overlays_Common
  sbndaq-artdaq-core_Overlays_ICARUS
  artdaq-core_Core
  artdaq_DAQdata
  artdaq-utilities_Plugins
  artdaq-core_Utilities
  rt
  artdaq_ArtModules
  art_Framework_Core
  art_Framework_IO_Sources
  art_Framework_Principal
  art_Persistency_Provenance
  art_Utilities
  MF_MessageLogger
  ${CETLIB_EXCEPT}
)
#simple_plugin(PhysCrateDump "module"
#              sbndaq-artdaq-core_Overlays
#              sbndaq-artdaq-core_Overlays_ICARUS)
#simple_plugin(PhysCrateWriteICARUSDataFile "module"
#              sbndaq-artdaq-core_Overlays
#              sbndaq-artdaq-core_Overlays_ICARUS)
#simple_plugin(PhysCrateViewer "module"
#              sbndaq-artdaq-core_Overlays
#              sbndaq-artdaq-core_Overlays_ICARUS
#	      art_Framework_Services_Optional
#	      art_Framework_Services_Registry
#	      art_Framework_Services_Optional_TFileService_service
#	      ${ROOT_BASIC_LIB_LIST} )
#simple_plugin(PhysCrateSimpleAna "module"
#              sbndaq-artdaq-core_Overlays
#              sbndaq-artdaq-core_Overlays_ICARUS
#	      art_Framework_Services_Optional
#	      art_Framework_Services_Registry
#	      art_Framework_Services_Optional_TFileService_service
#	      ${ROOT_BASIC_LIB_LIST} )
#
# Subdirectories.
#add_subdirectory(DQM)
