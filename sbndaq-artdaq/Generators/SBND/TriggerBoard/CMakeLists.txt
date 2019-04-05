include_directories( $ENV{JSONCPP_INC} )

art_make( LIBRARY_NAME sbndaq-artdaq_Generators_SBND_TriggerBoard
	LIB_LIBRARIES
	${CANVAS}
	${FHICLCPP}
	${CETLIB}
	$ENV{LIBXML2_FQ_DIR}/lib/libxml2.so
	${MF_MESSAGELOGGER}
	${MF_UTILITIES}
	${Boost_SYSTEM_LIBRARY}
	${TRACE}
	-lpthread
	sbndaq-artdaq-core_Overlays
	sbndaq-artdaq-core_Overlays_Common
	sbndaq-artdaq-core_Overlays_SBND
	$ENV{JSONCPP_LIB}/libjsoncpp.so
	)

install_headers()

