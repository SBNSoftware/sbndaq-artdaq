art_make(NO_PLUGINS # WAnt different link lists for modues
 BASENAME_ONLY
 LIBRARY_NAME sbndaq-artdaq_ArtModules_SBND
 LIB_LIBRARIES sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays
 	       sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_Common
 	       sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_SBND
 EXCLUDE
 SBND_fragmentNameHelper.cc

)

simple_plugin(SBNDArtdaqBuildInfo "module"
	artdaq_DAQdata
  artdaq_BuildInfo
  artdaq_core::artdaq-core_BuildInfo
  artdaq_utilities::artdaq-utilities_BuildInfo
  sbndaq-artdaq_BuildInfo
  sbndaq_artdaq_core::sbndaq-artdaq-core_BuildInfo
  MF_MessageLogger
  fhiclcpp
  cetlib
  cetlib_except
)

simple_plugin(SBND "fragmentNameHelper"
  artdaq_core::artdaq-core_Utilities
	artdaq_core::artdaq-core_Data
  artdaq_core::artdaq-core_Data_Artdaq_fragmentNameHelper
  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays
  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_Common
  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_ICARUS
  MF_MessageLogger
  fhiclcpp 
  )

