add_subdirectory(GPS)
add_subdirectory(Bern)

include_directories( ${CAENCOMM_INC} )
include_directories( ${CAENVME_INC} )
include_directories( ${CAENDIGITIZER_INC} )
include_directories( ${ARTDAQ_INC} )
include_directories( ./GPS )

cet_find_library( CAENDIGITIZER    NAMES CAENDigitizer    PATHS ENV CAENDIGITIZER_LIB   NO_DEFAULT_PATH )
cet_find_library( CAEN_VME         NAMES CAENVME          PATHS ENV CAENVME_LIB         NO_DEFAULT_PATH )
cet_find_library( CAENCOMM         NAMES CAENComm         PATHS ENV CAENCOMM_LIB        NO_DEFAULT_PATH )

art_make_library( LIBRARY_NAME sbndaq-artdaq_Common_Utilities
		  SOURCE workerThread.cpp 
		  CircularBuffer.cc
		  PoolBuffer.cc
		  LIBRARIES 
		  pthread
		  artdaq_DAQdata
		  ${MF_UTILITIES}
		  ${TRACE}
		  ${MF_MESSAGELOGGER}
		  ${Boost_SYSTEM_LIBRARY}
		)

		  
cet_make_library( LIBRARY_NAME sbndaq-artdaq_Common_CAENV1730_GeneratorBase
		  SOURCE CAENDecoder.cc
			 CAENConfiguration.cc
			 CAENException.cc
		  LIBRARIES
			artdaq_DAQdata
			artdaq_Generators
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

cet_make_library( LIBRARY_NAME sbndaq-artdaq_Common_SpectraTime_GeneratorBase
                  SOURCE  SpectraTimeReadout_generator.cc
                          GPS/GPSInfo.cc
		  LIBRARIES
			artdaq_DAQdata
			artdaq_Generators
  			artdaq-core_Utilities
                        artdaq-utilities_Plugins
                        artdaq-core_Data
  			sbndaq-artdaq_Common_Utilities
			${Boost_SYSTEM_LIBRARY}
			${ART_UTILITIES}
			${FHICLCPP}
			${MF_MESSAGELOGGER}
			${MF_UTILITIES}
			${TRACE}
			${CETLIB}
			${CETLIB_EXCEPT}
)


cet_make_library( LIBRARY_NAME sbndaq-artdaq_Common_WhiteRabbit_GeneratorBase
                  SOURCE  WhiteRabbitReadout_generator.cc
		  LIBRARIES
			artdaq_DAQdata
			artdaq_Generators
  			artdaq-core_Utilities
                        artdaq-utilities_Plugins
                        artdaq-core_Data
  			sbndaq-artdaq_Common_Utilities
			${Boost_SYSTEM_LIBRARY}
			${ART_UTILITIES}
			${FHICLCPP}
			${MF_MESSAGELOGGER}
			${MF_UTILITIES}
			${TRACE}
			${CETLIB}
			${CETLIB_EXCEPT}
)


#Generator modules now

simple_plugin(CAENV1730Readout "generator"
  sbndaq-artdaq_Common_CAENV1730_GeneratorBase
  sbndaq-artdaq-core_Overlays
  sbndaq-artdaq-core_Overlays_Common
  sbndaq-artdaq_Common_Utilities
  artdaq_Generators
  artdaq_Application
  artdaq_DAQdata
  artdaq-core_Utilities
  artdaq-core_Data
  artdaq-utilities_Plugins
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

simple_plugin(SpectraTimeReadout "generator"
  sbndaq-artdaq_Common_SpectraTime_GeneratorBase
  sbndaq-artdaq-core_Overlays
  sbndaq-artdaq-core_Overlays_Common
  sbndaq-artdaq_Common_Utilities
  artdaq_Generators
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
)

simple_plugin(WhiteRabbitReadout "generator"
  sbndaq-artdaq_Common_WhiteRabbit_GeneratorBase
  sbndaq-artdaq-core_Overlays
  sbndaq-artdaq-core_Overlays_Common
  sbndaq-artdaq_Common_Utilities
  artdaq_Generators
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
)

art_make_library( LIBRARY_NAME sbndaq-artdaq_Common_BernCRTZMQ_GeneratorBase
                  SOURCE BernCRTZMQ_GeneratorBase.cc
                  LIBRARIES
                        sbndaq-artdaq-core_Overlays
                        sbndaq-artdaq-core_Overlays_Common
                        sbndaq-artdaq_Common_Utilities
                        artdaq_Generators
                        artdaq_Application
                        artdaq_DAQdata
                        artdaq-core_Utilities
                        artdaq-core_Data
                        artdaq-utilities_Plugins
                        BernCRTFEBConfiguration
                        ${ART_UTILITIES}
                        ${FHICLCPP}
                        ${MF_MESSAGELOGGER}
                        ${MF_UTILITIES}
                        ${CETLIB}
                        ${CETLIB_EXCEPT}
)

art_make_library( LIBRARY_NAME sbndaq-artdaq_Common_BernCRT_GeneratorBase
                  SOURCE BernCRT_GeneratorBase.cc
                  LIBRARIES
                        sbndaq-artdaq-core_Overlays
                        sbndaq-artdaq-core_Overlays_Common
                        sbndaq-artdaq_Common_Utilities
                        artdaq_Generators
                        artdaq_Application
                        artdaq_DAQdata
                        artdaq-core_Utilities
                        artdaq-core_Data
                        artdaq-utilities_Plugins
                        BernCRTFEBConfiguration
                        ${ART_UTILITIES}
                        ${FHICLCPP}
                        ${MF_MESSAGELOGGER}
                        ${MF_UTILITIES}
                        ${CETLIB}
                        ${CETLIB_EXCEPT}
)

art_make_library( LIBRARY_NAME BernCRTFEBConfiguration
  SOURCE BernCRTFEBConfiguration.cc
  LIBRARIES
    ${FHICLCPP}
    ${CETLIB_EXCEPT}
)

art_make_library( LIBRARY_NAME febdrv
  SOURCE febdrv.cc
  LIBRARIES
    sbndaq-artdaq-core_Overlays_Common
    ${FHICLCPP}
    ${CETLIB_EXCEPT}
)

cet_make_exec( ConvertCRTConfiguration
  SOURCE ConvertCRTConfiguration.cc
  LIBRARIES
    BernCRTFEBConfiguration
    ${FHICLCPP}
    ${CETLIB}
    ${CETLIB_EXCEPT}
    ${Boost_PROGRAM_OPTIONS_LIBRARY_RELEASE}
)

simple_plugin(BernCRTZMQData "generator"
  sbndaq-artdaq_Common_BernCRTZMQ_GeneratorBase
  sbndaq-artdaq-core_Overlays
  sbndaq-artdaq-core_Overlays_Common
  zmq
  artdaq_Generators
  artdaq_Application
  artdaq_DAQdata
  artdaq-core_Utilities
  artdaq-core_Data
  artdaq-utilities_Plugins
  BernCRTFEBConfiguration
  febdrv
  ${ART_UTILITIES}
  ${FHICLCPP}
  ${MF_MESSAGELOGGER}
  ${MF_UTILITIES}
  ${CETLIB}
  ${CETLIB_EXCEPT}
  )

simple_plugin(BernCRTData "generator"
  sbndaq-artdaq_Common_BernCRT_GeneratorBase
  sbndaq-artdaq-core_Overlays
  sbndaq-artdaq-core_Overlays_Common
  zmq
  artdaq_Generators
  artdaq_Application
  artdaq_DAQdata
  artdaq-core_Utilities
  artdaq-core_Data
  artdaq-utilities_Plugins
  BernCRTFEBConfiguration
  febdrv
 # ${ARTDAQ_UTILITIES}
  ${ART_UTILITIES}
  ${FHICLCPP}
  ${MF_MESSAGELOGGER}
  ${MF_UTILITIES}
  ${CETLIB}
  ${CETLIB_EXCEPT}
  )

simple_plugin(DummyGenerator "generator"
  sbndaq-artdaq-core_Overlays
  sbndaq-artdaq-core_Overlays_Common
  sbndaq-artdaq_Common_Utilities
  artdaq_Generators
  artdaq_DAQdata
  artdaq_DAQrate
  artdaq-core_Utilities
  artdaq-core_Data
  artdaq-utilities_Plugins
  ${FHICLCPP}
  ${MESSAGEFACILITY_LIBS}
  ${CETLIB}
  ${CETLIB_EXCEPT}
  )

