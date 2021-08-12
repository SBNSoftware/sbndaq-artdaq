link_directories($ENV{JSONCPP_LIB})

art_make( LIBRARY_NAME sbndaq-artdaq_Generators_SBND_TriggerBoard
	LIB_LIBRARIES
	canvas
	fhiclcpp
	cetlib
	xml2
	MF_MessageLogger
	${MF_UTILITIES}
	${Boost_SYSTEM_LIBRARY}
	
	-lpthread
	sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays
	sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_Common
	sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_SBND
	jsoncpp
	)

install_headers()

