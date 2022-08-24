cet_make(
 NO_PLUGINS # WAnt different link lists for modues
 BASENAME_ONLY
 LIBRARY_NAME
    sbndaq-artdaq_ArtModules_SBND
 LIB_LIBRARIES
    sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays
    sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_Common
    sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_SBND
 EXCLUDE
    SBND_fragmentNameHelper.cc
)

list(APPEND BUILD_PLUGIN_CORE_LIB_LIST sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_SBND)
list(APPEND BUILD_PLUGIN_FRAGNAMEHELPER_LIB_LIST  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_SBND)

build_plugin(SBNDArtdaqBuildInfo "module" LIBRARIES REG ${BUILD_PLUGIN_BUILDINFO_LIB_LIST})
build_plugin(FragmentToBinary "module" LIBRARIES REG ${BUILD_PLUGIN_CORE_LIB_LIST})
build_plugin(BinaryToFragment "module" LIBRARIES REG ${BUILD_PLUGIN_CORE_LIB_LIST})

basic_plugin(SBND "fragmentNameHelper" LIBRARIES REG ${BUILD_PLUGIN_FRAGNAMEHELPER_LIB_LIST})

add_subdirectory(testStandTrigger)

