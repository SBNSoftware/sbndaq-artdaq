basic_plugin(ToySimulator "generator"
  sbndaq-artdaq_Generators_Toy_ToyHardwareInterface
  sbndaq-artdaq_Generators_Toy_Overlay
  artdaq-utilities_Plugins
  ${BUILD_PLUGIN_CORE_LIB_LIST}

)

add_subdirectory(ToyHardwareInterface)
add_subdirectory(Overlay)
