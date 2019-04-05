add_subdirectory(TriggerBoard)
include_directories(./TriggerBoard)

simple_plugin(TriggerBoardReader "generator"
  sbndaq-artdaq-core_Overlays
  sbndaq-artdaq-core_Overlays_Common
  sbndaq-artdaq-core_Overlays_SBND
  sbndaq-artdaq_Generators_SBND_TriggerBoard
  sbndaq-artdaq_Common_Utilities
  artdaq-core_Data
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
  jsoncpp
  )

#original from ProtoDUNE:
#simple_plugin(TriggerBoardReader "generator"
#  dune-raw-data_Overlays
#  dune-artdaq_Generators_TriggerBoard
#  artdaq-core_Data
#  artdaq_Application
#  artdaq_DAQdata
#  artdaq-core_Utilities
#  artdaq-utilities_Plugins
#  ${CANVAS}
#  ${FHICLCPP}
#  ${MF_MESSAGELOGGER}
#  ${CETLIB}
#  ${CETLIB_EXCEPT}
#  jsoncpp
#  )
