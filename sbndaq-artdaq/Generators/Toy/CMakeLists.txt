
simple_plugin(ToySimulator "generator"
  sbndaq-artdaq_Generators_Toy_ToyHardwareInterface
  sbndaq-artdaq_Generators_Toy_Overlay
  artdaq_Generators
  artdaq_DAQdata
  artdaq-core_Utilities
  artdaq-core_Data
  artdaq-utilities_Plugins
  canvas
  ${FHICLCPP}
  ${MESSAGEFACILITY_LIBS}
  ${CETLIB}
${CETLIB_EXCEPT}
  )

add_subdirectory(ToyHardwareInterface)
add_subdirectory(Overlay)
