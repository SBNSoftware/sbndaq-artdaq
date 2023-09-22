
cet_add_compiler_flags(-Wno-deprecated-copy )
#cet_add_compiler_flags(-Wno-deprecated-copy -DBOOST_BIND_GLOBAL_PLACEHOLDERS)

find_package(artdaq_epics_plugin 1.05.04 REQUIRED)
find_package(EPICS 7.0.6 REQUIRED)

set(PQXX_MAJOR_VERSION 7)
if(DEFINED ENV{PQXX_DIR})
    include_directories($ENV{PQXX_INC})
    link_directories($ENV{PQXX_LIB})
    set(PQXX_MAJOR_VERSION 6)
endif()

cet_make_library(LIBRARY_NAME sbndaq-artdaq_Generators_Common_GPS_GPSInfoBase
SOURCE
GPSInfo.cc 
LIBRARIES PUBLIC
Boost::thread
    sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_Common
)
target_include_directories(sbndaq-artdaq_Generators_Common_GPS_GPSInfoBase PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

cet_make_exec(NAME gps
  SOURCE
    GPSReceiver.cc
    NetworkReceiver.cc
  LIBRARIES
    sbndaq_artdaq::sbndaq-artdaq_Generators_Common_GPS_GPSInfoBase
    pthread
    rt
)

cet_make_exec(NAME gps-bcast
  SOURCE
    GPSBroadcast.cc
  LIBRARIES
    sbndaq_artdaq::sbndaq-artdaq_Generators_Common_GPS_GPSInfoBase
   EPICS::ca
   pqxx
   pq
   pthread
   rt
)
target_compile_definitions(gps-bcast PUBLIC PQXX_MAJOR_VERSION=${PQXX_MAJOR_VERSION})

install_source()
install_headers()
