
if(DEFINED ENV{WINDRIVER_INC})

add_subdirectory(nevishwutils)
include_directories ( $ENV{WINDRIVER_INC} )

SET_SOURCE_FILES_PROPERTIES(NevisTPC2StreamNUandSNXMIT_generator.cc PROPERTIES COMPILE_FLAGS -Wno-error=unused-function)
SET_SOURCE_FILES_PROPERTIES(NevisTBStream_generator.cc PROPERTIES COMPILE_FLAGS -Wno-error=unused-function)
SET_SOURCE_FILES_PROPERTIES(NevisTPCNUXMIT_generator.cc PROPERTIES COMPILE_FLAGS -Wno-error=unused-function)
SET_SOURCE_FILES_PROPERTIES(NevisTPCCALIB_generator.cc PROPERTIES COMPILE_FLAGS -Wno-error=unused-function)

cet_make_library(
  LIBRARY_NAME
    sbndaq-artdaq_NevisTPCGeneratorBase
  SOURCE
    NevisTPC_generatorBase.cc
   LIBRARIES
    ${LIBRARY_CORE_LIB_LIST}
    sbndaq-artdaq_nevishwutils
)

cet_make_library(
  LIBRARY_NAME
    sbndaq-artdaq_NevisTBGeneratorBase
  SOURCE
    NevisTB_generatorBase.cc
   LIBRARIES
    ${LIBRARY_CORE_LIB_LIST}
    sbndaq-artdaq_nevishwutils
)



list(APPEND BUILD_PLUGIN_CORE_LIB_LIST sbndaq-artdaq_NevisTPCGeneratorBase)


basic_plugin(NevisTPCData "generator" LIBRARIES REG ${BUILD_PLUGIN_CORE_LIB_LIST})
basic_plugin(NevisTPC2StreamNUandSNXMIT "generator" LIBRARIES REG ${BUILD_PLUGIN_CORE_LIB_LIST})
basic_plugin(NevisTBStream "generator" LIBRARIES REG ${BUILD_PLUGIN_CORE_LIB_LIST} sbndaq-artdaq_NevisTBGeneratorBase)
basic_plugin(NevisTPCNUXMIT "generator" LIBRARIES REG ${BUILD_PLUGIN_CORE_LIB_LIST})
basic_plugin(NevisTPCCALIB "generator" LIBRARIES REG ${BUILD_PLUGIN_CORE_LIB_LIST})
basic_plugin(NevisTPCFile "generator" LIBRARIES REG ${BUILD_PLUGIN_CORE_LIB_LIST})


install_source()
install_headers()

endif()