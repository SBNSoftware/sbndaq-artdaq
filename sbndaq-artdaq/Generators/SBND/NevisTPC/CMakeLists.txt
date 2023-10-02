
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


cet_build_plugin(NevisTPCData artdaq::commandableGenerator LIBRARIES REG ${BUILD_PLUGIN_CORE_LIB_LIST})
cet_build_plugin(NevisTPC2StreamNUandSNXMIT artdaq::commandableGenerator LIBRARIES REG ${BUILD_PLUGIN_CORE_LIB_LIST})
cet_build_plugin(NevisTBStream artdaq::commandableGenerator LIBRARIES REG ${BUILD_PLUGIN_CORE_LIB_LIST} sbndaq-artdaq_NevisTBGeneratorBase)
cet_build_plugin(NevisTPCNUXMIT artdaq::commandableGenerator LIBRARIES REG ${BUILD_PLUGIN_CORE_LIB_LIST})
cet_build_plugin(NevisTPCCALIB artdaq::commandableGenerator LIBRARIES REG ${BUILD_PLUGIN_CORE_LIB_LIST})
cet_build_plugin(NevisTPCFile artdaq::commandableGenerator LIBRARIES REG ${BUILD_PLUGIN_CORE_LIB_LIST})


install_source()
install_headers()

endif()