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
			sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays
			sbndaq-artdaq_nevishwutils
			sbndaq-artdaq_Common_Utilities
			art_Utilities
			fhiclcpp
			MF_MessageLogger
			${MF_UTILITIES}
			cetlib
			cetlib_except
		)

simple_plugin(NevisTPCData "generator"
  sbndaq-artdaq_NevisTPCGeneratorBase
  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays
  sbndaq-artdaq_nevishwutils
  artdaq_Generators
  artdaq_Application
  artdaq_DAQdata
  artdaq_core::artdaq-core_Utilities
  artdaq_core::artdaq-core_Data
  art_Utilities
  fhiclcpp
  MF_MessageLogger
  ${MF_UTILITIES}
  cetlib
  cetlib_except
  )

simple_plugin(NevisTPC2StreamNUandSNXMIT "generator"
  sbndaq-artdaq_NevisTPCGeneratorBase
  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays
  sbndaq-artdaq_nevishwutils
  artdaq_Generators
  artdaq_Application
  artdaq_DAQdata
  artdaq_core::artdaq-core_Utilities
  artdaq_core::artdaq-core_Data
  art_Utilities
  fhiclcpp
  MF_MessageLogger
  ${MF_UTILITIES}
  cetlib
  cetlib_except
  )

simple_plugin(NevisTPCNUXMIT "generator"
  sbndaq-artdaq_NevisTPCGeneratorBase
  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays
  sbndaq-artdaq_nevishwutils
  artdaq_Generators
  artdaq_Application
  artdaq_DAQdata
  artdaq_core::artdaq-core_Utilities
  artdaq_core::artdaq-core_Data
  art_Utilities
  fhiclcpp
  MF_MessageLogger
  ${MF_UTILITIES}
  cetlib
  cetlib_except
  )

simple_plugin(NevisTPCCALIB "generator"
  sbndaq-artdaq_NevisTPCGeneratorBase
  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays
  sbndaq-artdaq_nevishwutils
  artdaq_Generators
  artdaq_Application
  artdaq_DAQdata
  artdaq_core::artdaq-core_Utilities
  artdaq_core::artdaq-core_Data
  art_Utilities
  fhiclcpp
  MF_MessageLogger
  ${MF_UTILITIES}
  cetlib
  cetlib_except
  )

simple_plugin(NevisTPCFile "generator"
  sbndaq-artdaq_NevisTPCGeneratorBase
  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays
  artdaq_Application
  artdaq_DAQdata
  artdaq_core::artdaq-core_Utilities
  artdaq_core::artdaq-core_Data
  artdaq_Generators
  art_Utilities
  fhiclcpp
  MF_MessageLogger
  ${MF_UTILITIES}
  cetlib
  cetlib_except
  )

install_source()
install_headers()
