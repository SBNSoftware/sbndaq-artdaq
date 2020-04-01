#include_directories( ${ARTDAQ_INC} )

art_make_exec(NAME sbnRequestSender
    SOURCE sbnRequestSender.cc
    LIBRARIES
    artdaq_Application
    artdaq_DAQrate
    artdaq-core_Utilities
    ${MESSAGEFACILITY_LIBS}
    ${CETLIB}
    ${CETLIB_EXCEPT}
    ${FHICLCPP}
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    )

