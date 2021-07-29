art_make(
        LIB_LIBRARIES
        art_Utilities
        ${FHICLCPP}
        ${CETLIB}
        ${CETLIB_EXCEPT}
        ${MF_MESSAGELOGGER}
        artdaq_DAQdata
	sbndaq-artdaq-core_Overlays
	sbndaq-artdaq-core_Overlays_ICARUS
        ${Boost_SYSTEM_LIBRARY}
        )

