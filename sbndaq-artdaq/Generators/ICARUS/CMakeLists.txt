add_subdirectory(icarus-base)

link_directories($ENV{CAENCOMM_LIB})
link_directories($ENV{REDIS_LIB})

art_make_library( LIBRARY_NAME sbndaq-artdaq_ICARUS_PhysCrate_GeneratorBase
		  SOURCE PhysCrate_GeneratorBase.cc
		  LIBRARIES
			sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays
			sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_ICARUS
			sbndaq-artdaq_Common_Utilities
			artdaq_Generators
			artdaq_Application
			artdaq_DAQdata
			artdaq_core::artdaq-core_Utilities
			artdaq_core::artdaq-core_Data
			${Boost_SYSTEM_LIBRARY}
			art_Utilities
			fhiclcpp
			MF_MessageLogger
			${MF_UTILITIES}
			cetlib
			cetlib_except
		)

simple_plugin(PhysCrateData "generator"
  icarus-base_A2795Board
  icarus-base_VetoUDP
  icarus-base_PhysCrate
  sbndaq-artdaq_ICARUS_PhysCrate_GeneratorBase
  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays
  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_ICARUS
  sbndaq-artdaq_Common_Utilities
  artdaq_Generators
  artdaq_Application
  artdaq_DAQdata
  artdaq_core::artdaq-core_Utilities
  artdaq_core::artdaq-core_Data
  hiredis
  ${Boost_SYSTEM_LIBRARY}
  fhiclcpp
  MF_MessageLogger
  cetlib
  cetlib_except
  CAENComm
  )

# simple_plugin(PhysCrateFakeData "generator"
#   sbndaq-artdaq_ICARUS_PhysCrate_GeneratorBase
#   sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays
#   sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_ICARUS
#   sbndaq-artdaq_Common_Utilities
#   artdaq_Application
#   artdaq_DAQdata
#   artdaq-core_Utilities
#   artdaq-core_Data
#   ${Boost_SYSTEM_LIBRARY}
#   fhiclcpp
#   MF_MessageLogger
#   cetlib
#   cetlib_except
#   )

simple_plugin(ICARUSTriggerUDP "generator"
#    artdaq-core-demo_Overlays
    sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays
    sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_ICARUS
    sbndaq-artdaq_Common_Utilities
    artdaq_Generators
    artdaq_Application
    artdaq_DAQdata
    artdaq_core::artdaq-core_Utilities
    artdaq_core::artdaq-core_Data
  ${Boost_SYSTEM_LIBRARY}
  canvas
    fhiclcpp
  ${MESSAGEFACILITY_LIBS}
    cetlib
 cetlib_except
    )

#JCZ - obsolete interpretation
#simple_plugin(ICARUSPMTGate "generator"
#    artdaq-core-demo_Overlays
#    sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays
#    sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_ICARUS
#    sbndaq-artdaq_Common_Utilities
#    artdaq_Generators
#    artdaq_DAQdata
#    artdaq-core_Utilities
#    artdaq-core_Data
#  ${Boost_SYSTEM_LIBRARY}
#  canvas
#    fhiclcpp
#  ${MESSAGEFACILITY_LIBS}
#    cetlib
# cetlib_except
#    )
