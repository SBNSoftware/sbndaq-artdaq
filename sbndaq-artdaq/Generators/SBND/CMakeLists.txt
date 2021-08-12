add_subdirectory(TriggerBoard)
add_subdirectory(NevisTPC)


find_ups_product( wibtools )

# Temporary, until wibtools is refactored, check for local wibtools
include_directories(../../../../wibtools/WIB/include)
include_directories(../../../../wibtools/MBB/include)
include_directories(../../../../wibtools/exception/include)

link_directories($ENV{JSONCPP_LIB})

simple_plugin(TriggerBoardReader "generator"
  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays
  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_Common
  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_SBND
  sbndaq-artdaq_Generators_SBND_TriggerBoard
  sbndaq-artdaq_Common_Utilities
  artdaq_core::artdaq-core_Data
  artdaq_Generators
  artdaq_Application
  artdaq_DAQdata
  artdaq_core::artdaq-core_Utilities
  art_Utilities
  canvas
  fhiclcpp
  MF_MessageLogger
  cetlib
  cetlib_except
  jsoncpp
  )

cet_make_library( LIBRARY_NAME sbndaq-artdaq_Generators_SBND_WIBReader
		  SOURCE WIBReader_generator.cc
		  LIBRARIES
			artdaq_DAQdata
			artdaq_Generators
  		artdaq_core::artdaq-core_Utilities
 			sbndaq-artdaq_Common_Utilities
			wibtools::WIB
			wibtools::ToolException
			${Boost_SYSTEM_LIBRARY}
			art_Utilities
			fhiclcpp
			MF_MessageLogger
			${MF_UTILITIES}
			
			cetlib
			cetlib_except
)

cet_make_library( LIBRARY_NAME sbndaq-artdaq_Generators_SBND_MBBReader
		  SOURCE MBBReader_generator.cc
		  LIBRARIES
			artdaq_DAQdata
			artdaq_Generators
  	  artdaq_core::artdaq-core_Utilities
 			sbndaq-artdaq_Common_Utilities
			wibtools::WIB
			wibtools::MBB
			wibtools::ToolException
			${Boost_SYSTEM_LIBRARY}
			art_Utilities
			fhiclcpp
			MF_MessageLogger
			${MF_UTILITIES}
			cetlib
			cetlib_except
)


simple_plugin(WIBReader "generator"
  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays
  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_Common
  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_SBND
  sbndaq-artdaq_Generators_SBND_WIBReader
  sbndaq-artdaq_Common_Utilities
  artdaq_core::artdaq-core_Data
  artdaq_Generators
  artdaq_Application
  artdaq_DAQdata
  artdaq_core::artdaq-core_Utilities
	wibtools::WIB
	wibtools::ToolException
  art_Utilities
  canvas
  fhiclcpp
  MF_MessageLogger
  cetlib
  cetlib_except
  )

simple_plugin(MBBReader "generator"
  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays
  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_Common
  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_SBND
  sbndaq-artdaq_Generators_SBND_MBBReader
  sbndaq-artdaq_Common_Utilities
  artdaq_core::artdaq-core_Data
  artdaq_Generators
  artdaq_Application
  artdaq_DAQdata
  artdaq_core::artdaq-core_Utilities
	wibtools::WIB
	wibtools::ToolException
  art_Utilities
  canvas
  fhiclcpp
  MF_MessageLogger
  cetlib
  cetlib_except
  )

