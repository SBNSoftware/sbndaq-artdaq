art_make_library( LIBRARY_NAME sbndaq-artdaq_ICARUS_PhysCrate_GeneratorBase
		  SOURCE PhysCrate_GeneratorBase.cc
		  LIBRARIES
			sbndaq-artdaq-core_Overlays
			sbndaq-artdaq-core_Overlays_ICARUS
			sbndaq-artdaq_Common_Utilities
			artdaq_Application
			artdaq_DAQdata
			artdaq-core_Utilities
			artdaq-utilities_Plugins
			artdaq-core_Data
			${Boost_SYSTEM_LIBRARY}
			${ART_UTILITIES}
			${FHICLCPP}
			${MF_MESSAGELOGGER}
			${MF_UTILITIES}
			${CETLIB}
			${CETLIB_EXCEPT}
		)

simple_plugin(PhysCrateData "generator"
  icarus-artdaq-base_A2795Board
  icarus-artdaq-base_VetoUDP
  icarus-artdaq-base_PhysCrate
  sbndaq-artdaq_ICARUS_PhysCrate_GeneratorBase
  sbndaq-artdaq-core_Overlays
  sbndaq-artdaq-core_Overlays_ICARUS
  sbndaq-artdaq_Common_Utilities
  artdaq_Application
  artdaq_DAQdata
  artdaq-core_Utilities
  artdaq-core_Data
  ${Boost_SYSTEM_LIBRARY}
  ${FHICLCPP}
  ${MF_MESSAGELOGGER}
  ${CETLIB}
  ${CETLIB_EXCEPT}
  )

simple_plugin(PhysCrateFakeData "generator"
  sbndaq-artdaq_ICARUS_PhysCrate_GeneratorBase
  sbndaq-artdaq-core_Overlays
  sbndaq-artdaq-core_Overlays_ICARUS
  sbndaq-artdaq_Common_Utilities
  artdaq_Application
  artdaq_DAQdata
  artdaq-core_Utilities
  artdaq-core_Data
  ${Boost_SYSTEM_LIBRARY}
  ${FHICLCPP}
  ${MF_MESSAGELOGGER}
  ${CETLIB}
  ${CETLIB_EXCEPT}
  )
