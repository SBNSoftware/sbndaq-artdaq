include_directories( ${CAENCOMM_INC} )
include_directories( ${CAENVME_INC} )
include_directories( ${CAENDIGITIZER_INC} )

cet_find_library( CAENDIGITIZER    NAMES CAENDigitizer    PATHS ENV CAENDIGITIZER_LIB   NO_DEFAULT_PATH )
cet_find_library( CAEN_VME         NAMES CAENVME          PATHS ENV CAENVME_LIB         NO_DEFAULT_PATH )
cet_find_library( CAENCOMM         NAMES CAENComm         PATHS ENV CAENCOMM_LIB        NO_DEFAULT_PATH )

art_make_library( LIBRARY_NAME sbndaq-artdaq_Common_Utilities
		  SOURCE workerThread.cpp 
		         CircularBuffer.cc
		  LIBRARIES 
		    	 pthread
		    	 ${MF_MESSAGELOGGER}
			 ${Boost_SYSTEM_LIBRARY}
		)

		  
cet_make_library( LIBRARY_NAME sbndaq-artdaq_Common_CAENV1730_GeneratorBase
		  SOURCE CAENDecoder.cc
			 CAENConfiguration.cc
			 CAENException.cc
		  LIBRARIES
			artdaq_DAQdata
  			artdaq-core_Utilities
  			sbndaq-artdaq_Common_Utilities
			${Boost_SYSTEM_LIBRARY}
			${ART_UTILITIES}
			${FHICLCPP}
			${MF_MESSAGELOGGER}
			${MF_UTILITIES}
			${TRACE}
			${CETLIB}
			${CETLIB_EXCEPT}
			${CAENVME}
			${CAENCOMM}
			${CAENDIGITIZER}
		)

simple_plugin(CAENV1730Readout "generator"
  sbndaq-artdaq_Common_CAENV1730_GeneratorBase
  sbndaq-artdaq-core_Overlays
  sbndaq-artdaq-core_Overlays_Common
  sbndaq-artdaq_Common_Utilities
  artdaq_Application
  artdaq_DAQdata
  artdaq-core_Utilities
  artdaq-core_Data
  ${ART_UTILITIES}
  ${FHICLCPP}
  ${MF_MESSAGELOGGER}
  ${MF_UTILITIES}
  ${CETLIB}
  ${CETLIB_EXCEPT}
  ${CAENDIGITIZER}
  ${CAEN_VME}
  ${CAENCOMM}
  )
