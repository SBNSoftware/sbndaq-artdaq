#execute_process(COMMAND xml2-config --libs OUTPUT_VARIABLE LIBXML2_LIBS)
#MESSAGE( STATUS "LIBXML2_LIBS: " ${LIBXML2_LIBS} "<<<")
MESSAGE( STATUS "JSONCPP_LIB: " ${JSONCPP_LIB} "<<<")

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
	jsoncpp
	#$ENV{JSONCPP_LIB}/libjsoncpp.so
	#/usr/lib64/libjsoncpp.so.0
	)

install_headers()

