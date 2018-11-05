add_subdirectory(pennBoard)

simple_plugin(PennReceiver "generator"
  sbndaq-artdaq-core_Overlays
  sbndaq-artdaq-core_Overlays_Common
  sbndaq-artdaq_Common_Utilities
  sbndaq-artdaq_Generators_SBND_pennBoard
  artdaq-core_Data
  artdaq_Application
  artdaq_DAQdata
  artdaq-core_Utilities
  ${ART_UTILITIES}
  ${FHICLCPP}
  ${MF_MESSAGELOGGER}
  ${CETLIB}
)