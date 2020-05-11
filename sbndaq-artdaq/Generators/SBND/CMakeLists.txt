add_subdirectory(TriggerBoard)
add_subdirectory(NevisTPC)

find_ups_product( wibtools v0_03_03 )

# Temporary, until wibtools is refactored, check for local wibtools
include_directories(../../../../wibtools/WIB/include)
include_directories(../../../../wibtools/MBB/include)
include_directories(../../../../wibtools/exception/include)

include_directories($ENV{JSONCPP_INC})
include_directories($ENV{TRACE_INC})
include_directories($ENV{WIBTOOLS_INC})
include_directories(./TriggerBoard)

#cet_find_library( WIBTOOLS_WIB    NAMES WIB           PATHS ENV WIBTOOLS_LIB   NO_DEFAULT_PATH )
#cet_find_library( WIBTOOLS_MBB    NAMES MBB           PATHS ENV WIBTOOLS_LIB   NO_DEFAULT_PATH )
#cet_find_library( WIBEXCEPTION    NAMES ToolException PATHS ENV WIBTOOLS_LIB   NO_DEFAULT_PATH )

#MESSAGE (STATUS "************* WIBTOOLS_WIB=${WIBTOOLS_WIB}<")
#MESSAGE (STATUS "************* WIBTOOLS_MBB=${WIBTOOLS_MBB}<")
#MESSAGE (STATUS "************* WIBEXCEPTION=${WIBEXCEPTION}<")

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

cet_make_library( LIBRARY_NAME sbndaq-artdaq_Generators_SBND_WIBReader
		  SOURCE WIBReader_generator.cc
		  LIBRARIES
			artdaq_DAQdata
			artdaq_Generators
  			artdaq-core_Utilities
  			sbndaq-artdaq_Common_Utilities
                        WIB
                        ToolException
			${Boost_SYSTEM_LIBRARY}
			${ART_UTILITIES}
			${FHICLCPP}
			${MF_MESSAGELOGGER}
			${MF_UTILITIES}
			${TRACE}
			${CETLIB}
			${CETLIB_EXCEPT}
		#	${WIBTOOLS_WIB}
		#	${WIBEXCEPTION}
)

cet_make_library( LIBRARY_NAME sbndaq-artdaq_Generators_SBND_MBBReader
		  SOURCE MBBReader_generator.cc
		  LIBRARIES
			artdaq_DAQdata
			artdaq_Generators
  			artdaq-core_Utilities
  			sbndaq-artdaq_Common_Utilities
                        WIB
                        MBB
                        ToolException
			${Boost_SYSTEM_LIBRARY}
			${ART_UTILITIES}
			${FHICLCPP}
			${MF_MESSAGELOGGER}
			${MF_UTILITIES}
			${TRACE}
			${CETLIB}
			${CETLIB_EXCEPT}
		#	${WIBTOOLS_WIB}
		#	${WIBTOOLS_MBB}
		#	${WIBEXCEPTION}
)


simple_plugin(WIBReader "generator"
  sbndaq-artdaq-core_Overlays
  sbndaq-artdaq-core_Overlays_Common
  sbndaq-artdaq-core_Overlays_SBND
  sbndaq-artdaq_Generators_SBND_WIBReader
  sbndaq-artdaq_Common_Utilities
  artdaq-core_Data
  artdaq_Generators
  artdaq_Application
  artdaq_DAQdata
  artdaq-core_Utilities
  artdaq-utilities_Plugins
  WIB
  ToolException
  ${ART_UTILITIES}
  ${CANVAS}
  ${TRACE}
  ${FHICLCPP}
  ${MF_MESSAGELOGGER}
  ${CETLIB}
  ${CETLIB_EXCEPT}
#  ${WIBTOOLS}
#  ${WIBEXCEPTION}
  )

simple_plugin(MBBReader "generator"
  sbndaq-artdaq-core_Overlays
  sbndaq-artdaq-core_Overlays_Common
  sbndaq-artdaq-core_Overlays_SBND
  sbndaq-artdaq_Generators_SBND_MBBReader
  sbndaq-artdaq_Common_Utilities
  artdaq-core_Data
  artdaq_Generators
  artdaq_Application
  artdaq_DAQdata
  artdaq-core_Utilities
  artdaq-utilities_Plugins
  WIB
  ToolException
  ${ART_UTILITIES}
  ${CANVAS}
  ${TRACE}
  ${FHICLCPP}
  ${MF_MESSAGELOGGER}
  ${CETLIB}
  ${CETLIB_EXCEPT}
#  ${WIBTOOLS}
#  ${WIBEXCEPTION}
  )

