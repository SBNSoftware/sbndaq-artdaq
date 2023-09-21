
include_directories(.)

cet_add_compiler_flags(-Wno-deprecated-copy )
#cet_add_compiler_flags(-Wno-deprecated-copy -DBOOST_BIND_GLOBAL_PLACEHOLDERS)

find_package(artdaq_epics_plugin 1.05.04 REQUIRED)
find_package(EPICS 7.0.6 REQUIRED)

if(DEFINED ENV{PQXX_DIR})
    include_directories($ENV{PQXX_INC})
    link_directories($ENV{PQXX_LIB})
endif()

cet_make_exec(NAME gps
  SOURCE
    GPSReceiver.cc
    GPSInfo.cc
    NetworkReceiver.cc
  LIBRARIES
    Boost::thread
    sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_Common
    pthread
    rt
)

cet_make_exec(NAME gps-bcast
  SOURCE
    GPSBroadcast.cc
    GPSInfo.cc
  LIBRARIES
    Boost::thread
    sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_Common
   EPICS::ca
   pqxx
   pthread
   rt
)


install_source()
install_headers()
