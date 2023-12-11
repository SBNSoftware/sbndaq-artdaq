add_subdirectory(GPS)
add_subdirectory(Bern)

add_definitions(-DLINUX)

if(DEFINED ENV{CAENCOMM_DIR})
    include_directories($ENV{CAENCOMM_INC})
    link_directories($ENV{CAENCOMM_LIB})
endif()
if(DEFINED ENV{CAENVME_DIR})
    include_directories($ENV{CAENVME_INC})
    link_directories($ENV{CAENVME_LIB})
endif()
if(DEFINED ENV{CAENDIGITIZER_DIR})
    include_directories($ENV{CAENDIGITIZER_INC})
    link_directories($ENV{CAENDIGITIZER_LIB})
endif()

cet_make_library(
  LIBRARY_NAME
    sbndaq-artdaq_Common_Utilities
  SOURCE
    workerThread.cpp
    CircularBuffer.cc
    PoolBuffer.cc
    ExitTimerManager.cc
  LIBRARIES
    pthread
    artdaq::DAQdata
    messagefacility::MF_MessageLogger
    Boost::boost
)

list(APPEND LIBRARY_CORE_LIB_LIST sbndaq-artdaq_Common_Utilities )

cet_make_library(
  LIBRARY_NAME
    sbndaq-artdaq_Common_CAENV1730_GeneratorBase
  SOURCE
    CAENDecoder.cc
    CAENConfiguration.cc
    CAENException.cc
  LIBRARIES
    ${LIBRARY_CORE_LIB_LIST}
    CAENComm
    CAENVME
    CAENDigitizer
)

cet_make_library(
  LIBRARY_NAME
    sbndaq-artdaq_Common_SpectraTime_GeneratorBase
  SOURCE
    SpectraTimeReadout_generator.cc
  LIBRARIES
    ${LIBRARY_CORE_LIB_LIST}
    sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_Common
    sbndaq_artdaq::sbndaq-artdaq_Generators_Common_GPS_GPSInfoBase
)

cet_make_library(
  LIBRARY_NAME
    sbndaq-artdaq_Common_WhiteRabbit_GeneratorBase
  SOURCE
    WhiteRabbitReadout_generator.cc
  LIBRARIES
    ${LIBRARY_CORE_LIB_LIST}
    sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_Common
)

cet_make_library(
  LIBRARY_NAME
    sbndaq-artdaq_Common_BernCRTFEBConfiguration
  SOURCE
    BernCRTFEBConfiguration.cc
  LIBRARIES
    TRACE::TRACE
    fhiclcpp::fhiclcpp
    cetlib_except::cetlib_except
)

cet_make_library(
  LIBRARY_NAME
    sbndaq-artdaq_Common_BernCRTZMQ_GeneratorBase
  SOURCE
     BernCRTZMQ_GeneratorBase.cc
  LIBRARIES
    ${LIBRARY_CORE_LIB_LIST}
    sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_Common
    sbndaq-artdaq_Common_BernCRTFEBConfiguration
)

cet_make_library(
  LIBRARY_NAME
    sbndaq-artdaq_Common_BernCRT_GeneratorBase
  SOURCE
    BernCRT_GeneratorBase.cc
  LIBRARIES
    ${LIBRARY_CORE_LIB_LIST}
    sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_Common
    sbndaq-artdaq_Common_BernCRTFEBConfiguration
)

cet_make_library(
  LIBRARY_NAME
    febdrv
  SOURCE
    febdrv.cc
  LIBRARIES
    sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_Common
    fhiclcpp::fhiclcpp
    cetlib_except::cetlib_except
)


cet_build_plugin(CAENV1730Readout artdaq::commandableGenerator
  LIBRARIES REG
    ${BUILD_PLUGIN_CORE_LIB_LIST}
    sbndaq-artdaq_Common_CAENV1730_GeneratorBase
    CAENComm
    CAENVME
    CAENDigitizer
  )

cet_build_plugin(SpectraTimeReadout artdaq::commandableGenerator
  LIBRARIES REG
    ${BUILD_PLUGIN_CORE_LIB_LIST}
    sbndaq-artdaq_Common_SpectraTime_GeneratorBase
)

cet_build_plugin(WhiteRabbitReadout artdaq::commandableGenerator
  LIBRARIES REG
    ${BUILD_PLUGIN_CORE_LIB_LIST}
    sbndaq-artdaq_Common_WhiteRabbit_GeneratorBase
)

cet_build_plugin(BernCRTZMQData artdaq::commandableGenerator
  LIBRARIES REG
    ${BUILD_PLUGIN_CORE_LIB_LIST}
    sbndaq-artdaq_Common_BernCRTZMQ_GeneratorBase
    zmq
    febdrv
)

cet_build_plugin(BernCRTData artdaq::commandableGenerator
  LIBRARIES REG
    ${BUILD_PLUGIN_CORE_LIB_LIST}
    sbndaq-artdaq_Common_BernCRT_GeneratorBase
    zmq
    febdrv
)
if(DEFINED ENV{ZMQ_DIR})
target_include_directories(sbndaq-artdaq_Generators_Common_BernCRTZMQData_generator PRIVATE  $ENV{ZMQ_INC})
target_include_directories(sbndaq-artdaq_Generators_Common_BernCRTData_generator PRIVATE  $ENV{ZMQ_INC})
target_link_directories(sbndaq-artdaq_Generators_Common_BernCRTZMQData_generator PRIVATE  $ENV{ZMQ_LIB})
target_link_directories(sbndaq-artdaq_Generators_Common_BernCRTData_generator PRIVATE  $ENV{ZMQ_LIB})
endif()

cet_build_plugin(DummyGenerator artdaq::commandableGenerator
  LIBRARIES REG
    ${BUILD_PLUGIN_CORE_LIB_LIST}
)

cet_make_exec(NAME ConvertCRTConfiguration
  SOURCE
    ConvertCRTConfiguration.cc
  LIBRARIES
    sbndaq-artdaq_Common_BernCRTFEBConfiguration
    fhiclcpp::fhiclcpp
    cetlib::cetlib
    cetlib_except::cetlib_except
    Boost::program_options
)

install_source()
install_headers()
