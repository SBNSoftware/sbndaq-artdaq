art_make(NO_PLUGINS # WAnt different link lists for modues
 BASENAME_ONLY
 LIBRARY_NAME sbndaq-artdaq_ArtModules_ICARUS
 LIB_LIBRARIES sbndaq-artdaq-core_Overlays
 	       sbndaq-artdaq-core_Overlays_Common
 	       sbndaq-artdaq-core_Overlays_ICARUS #icarus-artdaq_Products
 EXCLUDE
 Icarus_fragmentNameHelper.cc
)

simple_plugin(IcarusArtdaqBuildInfo "module"
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

simple_plugin(PhysCrateDump "module"
         			${ARTDAQ-CORE_UTILITIES}
              sbndaq-artdaq-core_Overlays
              sbndaq-artdaq-core_Overlays_ICARUS)

simple_plugin(PhysCrateWriteICARUSDataFile "module"
         			${ARTDAQ-CORE_UTILITIES}
              sbndaq-artdaq-core_Overlays
              sbndaq-artdaq-core_Overlays_ICARUS)

simple_plugin(PhysCrateViewer "module"
              sbndaq-artdaq-core_Overlays
              sbndaq-artdaq-core_Overlays_ICARUS
	      art_root_io_tfile_support ${ROOT_CORE}
	      art_Framework_Services_Registry
	      art_root_io_TFileService_service
	      ${ARTDAQ-CORE_UTILITIES}
	      ${ROOT_BASIC_LIB_LIST} )

simple_plugin(PhysCrateSimpleAna "module"
	sbndaq-artdaq-core_Overlays
              sbndaq-artdaq-core_Overlays_ICARUS
	      art_root_io_tfile_support ${ROOT_CORE}
	      art_Framework_Services_Registry
	      art_root_io_TFileService_service
	      ${ARTDAQ-CORE_UTILITIES}
	      ${ROOT_BASIC_LIB_LIST} )


simple_plugin(Icarus "fragmentNameHelper"
  artdaq-core_Data_Artdaq_fragmentNameHelper
  artdaq-core_Data
  sbndaq-artdaq-core_Overlays
  sbndaq-artdaq-core_Overlays_Common
  sbndaq-artdaq-core_Overlays_ICARUS
  ${ARTDAQ-CORE_UTILITIES}
  ${MF_MESSAGELOGGER}
  ${FHICLCPP}
  )

