
list(APPEND LIBRARY_CORE_LIB_LIST sbndaq-artdaq_Common_Utilities sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_SBND)
list(APPEND BUILD_PLUGIN_CORE_LIB_LIST sbndaq-artdaq_Common_Utilities sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_SBND)

add_subdirectory(TriggerBoard)
add_subdirectory(NevisTPC)
add_subdirectory(SPECTDC)

find_package(wibtools REQUIRED)

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


include(artdaq::commandableGenerator)
cet_build_plugin(WIBReader  artdaq::commandableGenerator
  LIBRARIES REG
    ${LIBRARY_CORE_LIB_LIST}
    sbndaq-artdaq_Generators_SBND_WIBReader
    wibtools::WIB
    wibtools::ToolException
)

cet_build_plugin(MBBReader  artdaq::commandableGenerator
  LIBRARIES REG
    ${LIBRARY_CORE_LIB_LIST}
    sbndaq-artdaq_Generators_SBND_MBBReader
    wibtools::WIB
    wibtools::ToolException
)

cet_build_plugin(TriggerBoardReader  artdaq::commandableGenerator
  LIBRARIES REG
    ${BUILD_PLUGIN_CORE_LIB_LIST}
    sbndaq-artdaq_Generators_SBND_TriggerBoard
    jsoncpp
)
target_include_directories(sbndaq-artdaq_Generators_SBND_TriggerBoardReader_generator PUBLIC $ENV{JSONCPP_INC})
target_link_directories(sbndaq-artdaq_Generators_SBND_TriggerBoardReader_generator PUBLIC $ENV{JSONCPP_LIB})

cet_build_plugin(SPECTDCTimestampReader artdaq::commandableGenerator
  LIBRARIES REG
    ${BUILD_PLUGIN_CORE_LIB_LIST}
    sbndaq-artdaq_Generators_SBND_SPECTDC
)

cet_make_exec(NAME SPECTDCTimestampReader_Test
  SOURCE
    SPECTDCTimestampReader_Test.cc
    SPECTDCTimestampReader_generator.cc
  LIBRARIES
    sbndaq-artdaq_fmctdclib
    sbndaq-artdaq_Generators_SBND_SPECTDC
    Boost::program_options
 )

install_source()
install_headers()
