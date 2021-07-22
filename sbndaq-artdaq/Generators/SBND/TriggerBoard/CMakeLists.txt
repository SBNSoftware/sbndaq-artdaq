include_directories( $ENV{JSONCPP_INC} )
include_directories( $ENV{TRACE_INC} )

art_make( LIBRARY_NAME sbndaq-artdaq_Generators_SBND_TriggerBoard
	LIB_LIBRARIES
	canvas
	fhiclcpp
	cetlib
	$ENV{LIBXML2_FQ_DIR}/lib/libxml2.so
	MF_MessageLogger
	${MF_UTILITIES}
	${Boost_SYSTEM_LIBRARY}
	
	-lpthread
	sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays
	sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_Common
	sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_SBND
	$ENV{JSONCPP_LIB}/libjsoncpp.so
	)

install_headers()

