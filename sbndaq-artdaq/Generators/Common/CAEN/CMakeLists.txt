
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

cet_make_exec(CAENStatus
    SOURCE
      CAENStatus.cc
    LIBRARIES
    ${LIBRARY_CORE_LIB_LIST}
    CAENComm
    CAENVME
    CAENDigitizer
)

install_source()
install_headers()
