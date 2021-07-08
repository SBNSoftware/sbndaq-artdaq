add_subdirectory(nevishwutils)
include_directories ( $ENV{WINDRIVER_INC} )

SET_SOURCE_FILES_PROPERTIES(NevisTPC2StreamNUandSNXMIT_generator.cc PROPERTIES COMPILE_FLAGS -Wno-error=unused-function)
SET_SOURCE_FILES_PROPERTIES(NevisTPCNUXMIT_generator.cc PROPERTIES COMPILE_FLAGS -Wno-error=unused-function)
SET_SOURCE_FILES_PROPERTIES(NevisTPCCALIB_generator.cc PROPERTIES COMPILE_FLAGS -Wno-error=unused-function)

cet_make_library( LIBRARY_NAME sbndaq-artdaq_NevisTPCGeneratorBase
		  SOURCE 
			 NevisTPC_generatorBase.cc
		  LIBRARIES
			artdaq_Generators
			artdaq_Application
			artdaq_DAQdata
      artdaq_core::artdaq-core_Utilities
      artdaq_core::artdaq-core_Data
			sbndaq-artdaq-core_Overlays
			sbndaq-artdaq_nevishwutils
			sbndaq-artdaq_Common_Utilities
			${ART_UTILITIES}
			${FHICLCPP}
			${MF_MESSAGELOGGER}
			${MF_UTILITIES}
			${CETLIB}
			${CETLIB_EXCEPT}
		)

simple_plugin(NevisTPCData "generator"
  sbndaq-artdaq_NevisTPCGeneratorBase
  sbndaq-artdaq-core_Overlays
  sbndaq-artdaq_nevishwutils
  artdaq_Generators
  artdaq_Application
  artdaq_DAQdata
  artdaq_core::artdaq-core_Utilities
  artdaq_core::artdaq-core_Data
  ${ART_UTILITIES}
  ${FHICLCPP}
  ${MF_MESSAGELOGGER}
  ${MF_UTILITIES}
  ${CETLIB}
  ${CETLIB_EXCEPT}
  )

simple_plugin(NevisTPC2StreamNUandSNXMIT "generator"
  sbndaq-artdaq_NevisTPCGeneratorBase
  sbndaq-artdaq-core_Overlays
  sbndaq-artdaq_nevishwutils
  artdaq_Generators
  artdaq_Application
  artdaq_DAQdata
  artdaq_core::artdaq-core_Utilities
  artdaq_core::artdaq-core_Data
  ${ART_UTILITIES}
  ${FHICLCPP}
  ${MF_MESSAGELOGGER}
  ${MF_UTILITIES}
  ${CETLIB}
  ${CETLIB_EXCEPT}
  )

simple_plugin(NevisTPCNUXMIT "generator"
  sbndaq-artdaq_NevisTPCGeneratorBase
  sbndaq-artdaq-core_Overlays
  sbndaq-artdaq_nevishwutils
  artdaq_Generators
  artdaq_Application
  artdaq_DAQdata
  artdaq_core::artdaq-core_Utilities
  artdaq_core::artdaq-core_Data
  ${ART_UTILITIES}
  ${FHICLCPP}
  ${MF_MESSAGELOGGER}
  ${MF_UTILITIES}
  ${CETLIB}
  ${CETLIB_EXCEPT}
  )

simple_plugin(NevisTPCCALIB "generator"
  sbndaq-artdaq_NevisTPCGeneratorBase
  sbndaq-artdaq-core_Overlays
  sbndaq-artdaq_nevishwutils
  artdaq_Generators
  artdaq_Application
  artdaq_DAQdata
  artdaq_core::artdaq-core_Utilities
  artdaq_core::artdaq-core_Data
  ${ART_UTILITIES}
  ${FHICLCPP}
  ${MF_MESSAGELOGGER}
  ${MF_UTILITIES}
  ${CETLIB}
  ${CETLIB_EXCEPT}
  )

simple_plugin(NevisTPCFile "generator"
  sbndaq-artdaq_NevisTPCGeneratorBase
  sbndaq-artdaq-core_Overlays
  artdaq_Application
  artdaq_DAQdata
  artdaq_core::artdaq-core_Utilities
  artdaq_core::artdaq-core_Data
  artdaq_Generators
  ${ART_UTILITIES}
  ${FHICLCPP}
  ${MF_MESSAGELOGGER}
  ${MF_UTILITIES}
  ${CETLIB}
  ${CETLIB_EXCEPT}
  )

install_source()
install_headers()
