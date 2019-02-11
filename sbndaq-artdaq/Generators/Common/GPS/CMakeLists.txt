
include_directories( $ENV{SBNDAQ_ARTDAQ_CORE_INC} )
include_directories( $ENV{EPICS_INC}/ )
include_directories( $ENV{EPICS_INC}/os/Linux/ )
include_directories( $ENV{EPICS_INC}/compiler/gcc/ )
include_directories( $ENV{PQXX_INC} )
include_directories( $ENV{BOOST_INC} )
include_directories( . )

cet_find_library( EPICS_CA	NAMES ca	PATHS ENV EPICS_LIBRARY		NO_DEFAULT_PATH )
cet_find_library( PQXX		NAMES pqxx	PATHS ENV PQXX_LIBRARIES	NO_DEFAULT_PATH )
cet_find_library( BOOST_SYSTEM	NAMES boost_system	PATHS ENV BOOST_LIB	NO_DEFAULT_PATH )
cet_find_library( BOOST_THREAD	NAMES boost_thread	PATHS ENV BOOST_LIB	NO_DEFAULT_PATH )

cet_make_exec(gps
		SOURCE GPSReceiver.cc
		       GPSInfo.cc
		       NetworkReceiver.cc
		LIBRARIES ${BOOST_SYSTEM}
			  ${BOOST_THREAD}
			  pthread
			  rt
		       )

cet_make_exec(gps-bcast
		SOURCE GPSBroadcast.cc
		       GPSInfo.cc
		LIBRARIES ${EPICS_CA}
			  ${PQXX}
			  ${BOOST_SYSTEM}
			  pthread
			  rt
		       )
		       

