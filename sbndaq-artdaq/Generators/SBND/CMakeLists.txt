link_directories($ENV{JSONCPP_LIB})

list(APPEND LIBRARY_CORE_LIB_LIST sbndaq-artdaq_Common_Utilities sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_SBND)
list(APPEND BUILD_PLUGIN_CORE_LIB_LIST sbndaq-artdaq_Common_Utilities sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_SBND)

add_subdirectory(TriggerBoard)
add_subdirectory(NevisTPC)


find_ups_product( wibtools )

# Temporary, until wibtools is refactored, check for local wibtools
include_directories(../../../../wibtools/WIB/include)
include_directories(../../../../wibtools/MBB/include)
include_directories(../../../../wibtools/exception/include)


cet_make_library(
  LIBRARY_NAME
    sbndaq-artdaq_Generators_SBND_WIBReader
  SOURCE
    WIBReader_generator.cc
  LIBRARIES
    ${LIBRARY_CORE_LIB_LIST}
    wibtools::WIB
    wibtools::ToolException
)

cet_make_library(
  LIBRARY_NAME
    sbndaq-artdaq_Generators_SBND_MBBReader
  SOURCE
    MBBReader_generator.cc
  LIBRARIES
    ${LIBRARY_CORE_LIB_LIST}
    wibtools::WIB
    wibtools::MBB
    wibtools::ToolException
)


basic_plugin(WIBReader "generator"
  LIBRARIES
    ${LIBRARY_CORE_LIB_LIST}
    sbndaq-artdaq_Generators_SBND_WIBReader
    wibtools::WIB
    wibtools::ToolException
)

basic_plugin(MBBReader "generator"
  LIBRARIES
    ${LIBRARY_CORE_LIB_LIST}
    sbndaq-artdaq_Generators_SBND_MBBReader
    wibtools::WIB
    wibtools::ToolException
)

basic_plugin(TriggerBoardReader "generator"
  LIBRARIES REG
    ${BUILD_PLUGIN_CORE_LIB_LIST}
    sbndaq-artdaq_Generators_SBND_TriggerBoard
    jsoncpp
)

install_source()
install_headers()
