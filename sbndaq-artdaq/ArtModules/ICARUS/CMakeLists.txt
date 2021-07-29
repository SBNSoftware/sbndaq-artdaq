art_make(NO_PLUGINS # WAnt different link lists for modues
 BASENAME_ONLY
 LIBRARY_NAME sbndaq-artdaq_ArtModules_ICARUS
 LIB_LIBRARIES sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays
 	       sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_Common
 	       sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_ICARUS #icarus-artdaq_Products
 EXCLUDE
 Icarus_fragmentNameHelper.cc
)

simple_plugin(IcarusArtdaqBuildInfo "module"
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

simple_plugin(PhysCrateDump "module"
							artdaq_core::artdaq-core_Utilities
              sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays
              sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_ICARUS)

simple_plugin(ICARUSTriggerUDPDump "module" 
							artdaq_core::artdaq-core_Utilities
              sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays
	      sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_ICARUS)

simple_plugin(ICARUSGateFilter "module" 
				   ${ARTDAQ-CORE_UTILITIES}
              sbndaq-artdaq-core_Overlays
	      sbndaq-artdaq-core_Overlays_ICARUS)

simple_plugin(PhysCrateWriteICARUSDataFile "module"
							artdaq_core::artdaq-core_Utilities
              sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays
              sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_ICARUS)

simple_plugin(PhysCrateViewer "module"
							artdaq_core::artdaq-core_Utilities
              sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays
              sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_ICARUS
	      art_root_io_tfile_support ${ROOT_CORE}
	      art_Framework_Services_Registry
	      art_root_io_TFileService_service
	      ${ROOT_BASIC_LIB_LIST} )

simple_plugin(PhysCrateSimpleAna "module"
       artdaq_core::artdaq-core_Utilities
	     sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays
        sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_ICARUS
	      art_root_io_tfile_support ${ROOT_CORE}
	      art_Framework_Services_Registry
	      art_root_io_TFileService_service
	      ${ROOT_BASIC_LIB_LIST} )

simple_plugin(ICARUSTriggerUDPSimpleAna "module"
        artdaq_core::artdaq-core_Utilities
				sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays
	      sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_ICARUS
	      art_root_io_tfile_support ${ROOT_CORE}
	      art_Framework_Services_Registry
	      art_root_io_TFileService_service
	      ${ROOT_BASIC_LIB_LIST} )	

simple_plugin(Icarus "fragmentNameHelper"
  artdaq_core::artdaq-core_Utilities
	artdaq_core::artdaq-core_Data
  artdaq_core::artdaq-core_Data_Artdaq_fragmentNameHelper
  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays
  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_Common
  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_ICARUS
  MF_MessageLogger
  fhiclcpp
  cetlib_except
  )

