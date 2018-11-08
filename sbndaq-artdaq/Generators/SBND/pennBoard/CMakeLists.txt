
cet_make_library( LIBRARY_NAME sbndaq-artdaq_Generators_SBND_pennBoard
		  SOURCE PennClient.cpp
		         PennDataReceiver.cpp
		  LIBRARIES
		   	${CANVAS_UTILITIES}
		 	${FHICLCPP}
			${CETLIB}
			$ENV{LIBXML2_FQ_DIR}/lib/libxml2.so
			${MF_MESSAGELOGGER}
			${MF_UTILITIES}
			${Boost_SYSTEM_LIBRARY}
			-lpthread
			sbndaq-artdaq-core_Overlays_SBND
			sbndaq-artdaq-core_Overlays_SBND_Penn
			)
	
 add_subdirectory(emulator)
