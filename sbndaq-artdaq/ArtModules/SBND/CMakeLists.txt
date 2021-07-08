art_make(NO_PLUGINS # WAnt different link lists for modues
 BASENAME_ONLY
 LIBRARY_NAME sbndaq-artdaq_ArtModules_SBND
 LIB_LIBRARIES sbndaq-artdaq-core_Overlays
 	       sbndaq-artdaq-core_Overlays_Common
 	       sbndaq-artdaq-core_Overlays_SBND
 EXCLUDE
 SBND_fragmentNameHelper.cc

)

simple_plugin(SBNDArtdaqBuildInfo "module"
	artdaq_DAQdata
  artdaq_BuildInfo
  artdaq_core::artdaq-core_BuildInfo
  artdaq_utilities::artdaq-utilities_BuildInfo
  sbndaq-artdaq_BuildInfo
  sbndaq-artdaq-core_BuildInfo
  ${MF_MESSAGELOGGER}
  ${FHICLCPP}
  ${CETLIB}
  ${CETLIB_EXCEPT}
)

simple_plugin(SBND "fragmentNameHelper"
  artdaq_core::artdaq-core_Utilities
	artdaq_core::artdaq-core_Data
  artdaq_core::artdaq-core_Data_Artdaq_fragmentNameHelper
  sbndaq-artdaq-core_Overlays
  sbndaq-artdaq-core_Overlays_Common
  sbndaq-artdaq-core_Overlays_ICARUS
  ${MF_MESSAGELOGGER}
  ${FHICLCPP} 
  )

