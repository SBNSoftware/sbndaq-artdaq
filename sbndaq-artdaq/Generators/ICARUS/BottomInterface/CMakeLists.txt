add_subdirectory(Backend_DAQ)

cet_make_library(
        SOURCE
        CRTdecode.cc
        CRTInterface.cc
        LIBRARIES
        art::Utilities
        artdaq::DAQdata
        sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays
        sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_ICARUS
        Boost::system
        )

