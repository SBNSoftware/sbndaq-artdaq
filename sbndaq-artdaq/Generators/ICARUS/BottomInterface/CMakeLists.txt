add_subdirectory(Backend_DAQ)

art_make(
        LIB_LIBRARIES
        art_Utilities
        ${FHICLCPP}
        ${CETLIB}
        ${CETLIB_EXCEPT}
        ${MF_MESSAGELOGGER}
        artdaq_DAQdata
        sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays
        sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_ICARUS
        ${Boost_SYSTEM_LIBRARY}
        )

