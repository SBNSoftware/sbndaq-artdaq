set (ART_build_LIB_LIST
  art_Framework_Core
  art_Framework_Principal
  art_Persistency_Provenance
  art_Utilities
  canvas
  art_root_io_tfile_support
  art_root_io_TFileService_service
)

set (ARTDAQ_build_LIB_LIST
  artdaq_core::artdaq-core_Data
  artdaq_core::artdaq-core_Utilities
)

set(ROOT_build_LIB_LIST
  Tree
  TreePlayer
  Hist
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
  MF_MessageLogger
  fhiclcpp
  cetlib_except
)

set(BUILD_PLUGIN_BUILDINFO_LIB_LIST
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

add_subdirectory(Common)

add_subdirectory(DQM)

IF(ICARUS_BUILD)
  add_subdirectory(ICARUS)
ENDIF()

IF(SBND_BUILD)
  add_subdirectory(SBND)
ENDIF()
