set ( LIBRARY_CORE_LIB_LIST
      artdaq::DAQdata
      artdaq_plugin_types::CommandableFragmentGenerator
      artdaq_core::artdaq-core_Utilities
      artdaq_core::artdaq-core_Data
      Boost::boost
      art::Utilities
      fhiclcpp::fhiclcpp
      messagefacility::MF_MessageLogger
      cetlib::cetlib
      cetlib_except::cetlib_except
)

set(BUILD_PLUGIN_CORE_LIB_LIST
  ${LIBRARY_CORE_LIB_LIST}
  artdaq::Application
  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays
  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_Common
)

include(artdaq::commandableGenerator)

add_subdirectory(Common)

IF(ICARUS_BUILD EQUAL 1)
  add_subdirectory(ICARUS)
ENDIF()

IF(SBND_BUILD EQUAL 1)
  add_subdirectory(SBND)
ENDIF()
