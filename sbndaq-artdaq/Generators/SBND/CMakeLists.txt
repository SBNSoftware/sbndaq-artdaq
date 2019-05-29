add_subdirectory(TriggerBoard)
include_directories($ENV{JSONCPP_INC})
include_directories($ENV{TRACE_INC})
include_directories(./TriggerBoard)

cet_make_library(LIBRARY_NAME sbndaq-artdaq_Generator_SBND_NevisTPCBase 
                 SOURCE NevisTPC_generatorBase.cc
                 LIBRARIES 
			artdaq_Generators
			artdaq_Application
			artdaq_DAQdata
			artdaq-core_Utilities
			artdaq-core_Data
			sbndaq-artdaq_Common_Utilities
			sbndaq-artdaq-core_Overlays
			${ART_UTILITIES}
			${FHICLCPP}
			${MF_MESSAGELOGGER}
			${MF_UTILITIES}
			${CETLIB}
			${CETLIB_EXCEPT})

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

simple_plugin(NevisTPCData "generator"
  sbndaq-artdaq-core_Overlays
  sbndaq-artdaq-core_Overlays_Common
  sbndaq-artdaq-core_Overlays_SBND
  sbndaq-artdaq_Common_Utilities
  sbndaq-artdaq_Generator_SBND_NevisTPCBase
  artdaq-core_Data
  artdaq_Generators
  artdaq_Application
  artdaq_DAQdata
  artdaq-core_Utilities
  ${ART_UTILITIES}
  ${CANVAS}
  ${TRACE}
  ${FHICLCPP}
  ${MF_MESSAGELOGGER}
  ${MF_UTILITIES}
  ${CETLIB}
  ${CETLIB_EXCEPT}
  )

