add_subdirectory(GPS)
add_subdirectory(Bern)

include_directories( ${CAENCOMM_INC} )
include_directories( ${CAENVME_INC} )
include_directories( ${CAENDIGITIZER_INC} )
include_directories( ${ARTDAQ_INC} )
include_directories( ./GPS )

#cet_add_compiler_flags(-DBOOST_BIND_GLOBAL_PLACEHOLDERS)

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
		  
		  MF_MessageLogger
		  ${Boost_SYSTEM_LIBRARY}
		)

		  
cet_make_library( LIBRARY_NAME sbndaq-artdaq_Common_CAENV1730_GeneratorBase
		  SOURCE CAENDecoder.cc
			 CAENConfiguration.cc
			 CAENException.cc
		  LIBRARIES
			artdaq_DAQdata
			artdaq_Generators
  			artdaq_core::artdaq-core_Utilities
  			sbndaq-artdaq_Common_Utilities
			${Boost_SYSTEM_LIBRARY}
			art_Utilities
			fhiclcpp
			MF_MessageLogger
			${MF_UTILITIES}
			
			cetlib
			cetlib_except
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
  			artdaq_core::artdaq-core_Utilities
                        artdaq_core::artdaq-core_Data
  			sbndaq-artdaq_Common_Utilities
			${Boost_SYSTEM_LIBRARY}
			art_Utilities
			fhiclcpp
			MF_MessageLogger
			${MF_UTILITIES}
			
			cetlib
			cetlib_except
)


cet_make_library( LIBRARY_NAME sbndaq-artdaq_Common_WhiteRabbit_GeneratorBase
                  SOURCE  WhiteRabbitReadout_generator.cc
		  LIBRARIES
			artdaq_DAQdata
			artdaq_Generators
  			artdaq_core::artdaq-core_Utilities
                        artdaq_core::artdaq-core_Data
  			sbndaq-artdaq_Common_Utilities
			${Boost_SYSTEM_LIBRARY}
			art_Utilities
			fhiclcpp
			MF_MessageLogger
			${MF_UTILITIES}
			
			cetlib
			cetlib_except
)


#Generator modules now

simple_plugin(CAENV1730Readout "generator"
  sbndaq-artdaq_Common_CAENV1730_GeneratorBase
  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays
  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_Common
  sbndaq-artdaq_Common_Utilities
  artdaq_Generators
  artdaq_Application
  artdaq_DAQdata
  artdaq_core::artdaq-core_Utilities
  artdaq_core::artdaq-core_Data
  art_Utilities
  fhiclcpp
  MF_MessageLogger
  ${MF_UTILITIES}
  cetlib
  cetlib_except
  ${CAENDIGITIZER}
  ${CAEN_VME}
  ${CAENCOMM}
  )

simple_plugin(SpectraTimeReadout "generator"
  sbndaq-artdaq_Common_SpectraTime_GeneratorBase
  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays
  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_Common
  sbndaq-artdaq_Common_Utilities
  artdaq_Generators
  artdaq_Application
  artdaq_DAQdata
  artdaq_core::artdaq-core_Utilities
  artdaq_core::artdaq-core_Data
  art_Utilities
  fhiclcpp
  MF_MessageLogger
  ${MF_UTILITIES}
  cetlib
  cetlib_except
)

simple_plugin(WhiteRabbitReadout "generator"
  sbndaq-artdaq_Common_WhiteRabbit_GeneratorBase
  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays
  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_Common
  sbndaq-artdaq_Common_Utilities
  artdaq_Generators
  artdaq_Application
  artdaq_DAQdata
  artdaq_core::artdaq-core_Utilities
  artdaq_core::artdaq-core_Data
  art_Utilities
  fhiclcpp
  MF_MessageLogger
  ${MF_UTILITIES}
  cetlib
  cetlib_except
)

art_make_library( LIBRARY_NAME sbndaq-artdaq_Common_BernCRTZMQ_GeneratorBase
                  SOURCE BernCRTZMQ_GeneratorBase.cc
                  LIBRARIES
                        sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays
                        sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_Common
                        sbndaq-artdaq_Common_Utilities
                        artdaq_Generators
                        artdaq_Application
                        artdaq_DAQdata
                        artdaq_core::artdaq-core_Utilities
                        artdaq_core::artdaq-core_Data
                        BernCRTFEBConfiguration
                        art_Utilities
                        fhiclcpp
                        MF_MessageLogger
                        ${MF_UTILITIES}
                        cetlib
                        cetlib_except
)

art_make_library( LIBRARY_NAME sbndaq-artdaq_Common_BernCRT_GeneratorBase
                  SOURCE BernCRT_GeneratorBase.cc
                  LIBRARIES
                        sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays
                        sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_Common
                        sbndaq-artdaq_Common_Utilities
                        artdaq_Generators
                        artdaq_Application
                        artdaq_DAQdata
                        artdaq_core::artdaq-core_Utilities
                        artdaq_core::artdaq-core_Data
                        BernCRTFEBConfiguration
                        art_Utilities
                        fhiclcpp
                        MF_MessageLogger
                        ${MF_UTILITIES}
                        cetlib
                        cetlib_except
)

art_make_library( LIBRARY_NAME BernCRTFEBConfiguration
  SOURCE BernCRTFEBConfiguration.cc
  LIBRARIES
    fhiclcpp
    cetlib_except
)

art_make_library( LIBRARY_NAME febdrv
  SOURCE febdrv.cc
  LIBRARIES
    sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_Common
    fhiclcpp
    cetlib_except
)

cet_make_exec( ConvertCRTConfiguration
  SOURCE ConvertCRTConfiguration.cc
  LIBRARIES
    BernCRTFEBConfiguration
    fhiclcpp
    cetlib
    cetlib_except
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
)

simple_plugin(BernCRTZMQData "generator"
  sbndaq-artdaq_Common_BernCRTZMQ_GeneratorBase
  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays
  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_Common
  zmq
  artdaq_Generators
  artdaq_Application
  artdaq_DAQdata
  artdaq_core::artdaq-core_Utilities
  artdaq_core::artdaq-core_Data
  BernCRTFEBConfiguration
  febdrv
  art_Utilities
  fhiclcpp
  MF_MessageLogger
  ${MF_UTILITIES}
  cetlib
  cetlib_except
  )

simple_plugin(BernCRTData "generator"
  sbndaq-artdaq_Common_BernCRT_GeneratorBase
  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays
  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_Common
  zmq
  artdaq_Generators
  artdaq_Application
  artdaq_DAQdata
  artdaq_core::artdaq-core_Utilities
  artdaq_core::artdaq-core_Data
  BernCRTFEBConfiguration
  febdrv
 # ${ARTDAQ_UTILITIES}
  art_Utilities
  fhiclcpp
  MF_MessageLogger
  ${MF_UTILITIES}
  cetlib
  cetlib_except
  )

simple_plugin(DummyGenerator "generator"
  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays
  sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_Common
  sbndaq-artdaq_Common_Utilities
  artdaq_Generators
  artdaq_DAQdata
  artdaq_DAQrate
  artdaq_core::artdaq-core_Utilities
  artdaq_core::artdaq-core_Data
  fhiclcpp
  ${MESSAGEFACILITY_LIBS}
  cetlib
  cetlib_except
  )

