add_subdirectory(TriggerBoard)
add_subdirectory(NevisTPC)
include_directories($ENV{JSONCPP_INC})
include_directories($ENV{TRACE_INC})
include_directories(./TriggerBoard)

simple_plugin(TriggerBoardReader "generator"
  sbndaq-artdaq-core_Overlays
  sbndaq-artdaq-core_Overlays_Common
  sbndaq-artdaq-core_Overlays_SBND
  sbndaq-artdaq_Generators_SBND_TriggerBoard
  sbndaq-artdaq_Common_Utilities
  artdaq-core_Data
  artdaq_Generators
  artdaq_Application
  artdaq_DAQdata
  artdaq-core_Utilities
  artdaq-utilities_Plugins
  ${ART_UTILITIES}
  ${CANVAS}
  ${TRACE}
  ${FHICLCPP}
  ${MF_MESSAGELOGGER}
  ${CETLIB}
  ${CETLIB_EXCEPT}
  $ENV{JSONCPP_LIB}/libjsoncpp.so
  )

