set ( LIBRARY_CORE_LIB_LIST
      artdaq_DAQdata
      artdaq_CommandableFragmentGenerator
      artdaq-utilities_Plugins
      artdaq_core::artdaq-core_Utilities
      artdaq_core::artdaq-core_Data
      ${Boost_SYSTEM_LIBRARY}
      art_Utilities
      fhiclcpp
      MF_MessageLogger
      cetlib
      cetlib_except
)

set(BUILD_PLUGIN_CORE_LIB_LIST
  ${LIBRARY_CORE_LIB_LIST}
  artdaq_Application
  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays
  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_Common
)

add_subdirectory(Common)

IF(ICARUS_BUILD)
  add_subdirectory(ICARUS)
ENDIF()

IF(SBND_BUILD)
  add_subdirectory(SBND)
ENDIF()
