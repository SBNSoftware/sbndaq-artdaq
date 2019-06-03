add_subdirectory(nevishwutils)


SET_SOURCE_FILES_PROPERTIES(NevisTPCCrate_generator.cc PROPERTIES COMPILE_FLAGS -Wno-error=unused-function)
SET_SOURCE_FILES_PROPERTIES(NevisTPCNUXMIT_generator.cc PROPERTIES COMPILE_FLAGS -Wno-error=unused-function)
SET_SOURCE_FILES_PROPERTIES(NevisTPCCALIB_generator.cc PROPERTIES COMPILE_FLAGS -Wno-error=unused-function)

cet_make_library( LIBRARY_NAME sbnddaq-readout_NevisTPCGeneratorBase
		  SOURCE 
			 NevisTPC_generatorBase.cc
		  LIBRARIES
			artdaq_Application
			artdaq_DAQdata
			artdaq-core_Utilities
			artdaq-core_Data
			sbnddaq-datatypes_Overlays
			sbnddaq-readout_nevishwutils
			sbnddaq-readout_GeneratorUtilities
			${ART_UTILITIES}
			${FHICLCPP}
			${MF_MESSAGELOGGER}
			${MF_UTILITIES}
			${CETLIB}
			${CETLIB_EXCEPT}
		)

simple_plugin(NevisTPCData "generator"
  sbnddaq-readout_NevisTPCGeneratorBase
  sbnddaq-datatypes_Overlays
  sbnddaq-readout_nevishwutils
  artdaq_Application
  artdaq_DAQdata
  artdaq-core_Utilities
  artdaq-core_Data
  ${ART_UTILITIES}
  ${FHICLCPP}
  ${MF_MESSAGELOGGER}
  ${MF_UTILITIES}
  ${CETLIB}
  ${CETLIB_EXCEPT}
  )

simple_plugin(NevisTPCCrate "generator"
  sbnddaq-readout_NevisTPCGeneratorBase
  sbnddaq-datatypes_Overlays
  sbnddaq-readout_nevishwutils
  artdaq_Application
  artdaq_DAQdata
  artdaq-core_Utilities
  artdaq-core_Data
  ${ART_UTILITIES}
  ${FHICLCPP}
  ${MF_MESSAGELOGGER}
  ${MF_UTILITIES}
  ${CETLIB}
  ${CETLIB_EXCEPT}
  )

simple_plugin(NevisTPCNUXMIT "generator"
  sbnddaq-readout_NevisTPCGeneratorBase
  sbnddaq-datatypes_Overlays
  sbnddaq-readout_nevishwutils
  artdaq_Application
  artdaq_DAQdata
  artdaq-core_Utilities
  artdaq-core_Data
  ${ART_UTILITIES}
  ${FHICLCPP}
  ${MF_MESSAGELOGGER}
  ${MF_UTILITIES}
  ${CETLIB}
  ${CETLIB_EXCEPT}
  )

simple_plugin(NevisTPCCALIB "generator"
  sbnddaq-readout_NevisTPCGeneratorBase
  sbnddaq-datatypes_Overlays
  sbnddaq-readout_nevishwutils
  artdaq_Application
  artdaq_DAQdata
  artdaq-core_Utilities
  artdaq-core_Data
  ${ART_UTILITIES}
  ${FHICLCPP}
  ${MF_MESSAGELOGGER}
  ${MF_UTILITIES}
  ${CETLIB}
  ${CETLIB_EXCEPT}
  )

simple_plugin(NevisTPCFile "generator"
  sbnddaq-readout_NevisTPCGeneratorBase
  sbnddaq-datatypes_Overlays
  artdaq_Application
  artdaq_DAQdata
  artdaq-core_Utilities
  artdaq-core_Data
  ${ART_UTILITIES}
  ${FHICLCPP}
  ${MF_MESSAGELOGGER}
  ${MF_UTILITIES}
  ${CETLIB}
  ${CETLIB_EXCEPT}
  )

install_source()
install_headers()