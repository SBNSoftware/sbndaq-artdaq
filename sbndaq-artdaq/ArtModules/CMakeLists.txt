set (ART_build_LIB_LIST
  art::Framework_Core
  art::Framework_Principal
  art::Persistency_Provenance
  art::Utilities
  canvas::canvas
  art_root_io::tfile_support
  art_root_io::TFileService_service
)

set (ARTDAQ_build_LIB_LIST
  artdaq_core::artdaq-core_Data
  artdaq_core::artdaq-core_Utilities
)

set(ROOT_build_LIB_LIST
  ROOT::Tree
  ROOT::TreePlayer
  ROOT::Hist
)

set(BUILD_PLUGIN_CORE_LIB_LIST
  ${ART_build_LIB_LIST}
  ${ARTDAQ_build_LIB_LIST}
  ${ROOT_build_LIB_LIST}
  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_Common
)

set(BUILD_PLUGIN_FRAGNAMEHELPER_LIB_LIST
  artdaq_core::artdaq-core_Utilities
  artdaq_core::artdaq-core_Data
  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays
  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_Common
  messagefacility::MF_MessageLogger
  fhiclcpp::fhiclcpp
  cetlib_except::cetlib_except
)

set(BUILD_PLUGIN_BUILDINFO_LIB_LIST
  artdaq::BuildInfo
  artdaq_core::artdaq-core_BuildInfo
  artdaq_utilities::artdaq-utilities_BuildInfo
  sbndaq-artdaq_BuildInfo
  sbndaq_artdaq_core::sbndaq-artdaq-core_BuildInfo
  messagefacility::MF_MessageLogger
  fhiclcpp::fhiclcpp
  cetlib::cetlib
  cetlib_except::cetlib_except
)

add_subdirectory(Common)

add_subdirectory(DQM)

IF(ICARUS_BUILD)
  add_subdirectory(ICARUS)
ENDIF()

IF(SBND_BUILD)
  add_subdirectory(SBND)
ENDIF()
