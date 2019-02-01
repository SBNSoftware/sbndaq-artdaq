
include_directories( ${SBNDAQ_ARTDAQ_CORE_INC} )
include_directories( ${EPICS_INC}/ )
include_directories( ${EPICS_INC}/os/Linux/ )
include_directories( ${EPICS_INC}/compiler/gcc/ )
include_directories( . )

cet_find_library( EPICS_CA	NAMES ca	PATHS ENV EPICS_LIBRARY	NO_DEFAULT_PATH )

cet_make_exec(gps
		SOURCE GPSReceiver.cc
		       GPSInfo.cc
		       NetworkReceiver.cc
		LIBRARIES boost_system
			  boost_thread
			  pthread
			  rt
		       )
		     
#cet_make_exec(gps-bcast
#		SOURCE GPSBroadcast.cc
#		       GPSInfo.cc
#		LIBRARIES ${EPICS_CA}
#			  pqxx
#			  boost_system
#			  rt
#		       )
		       

