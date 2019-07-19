
include_directories( $ENV{SBNDAQ_ARTDAQ_CORE_INC} )
include_directories( . )

cet_make_exec(febdrv 
		SOURCE febdrv.c
		LIBRARIES zmq
		       )
