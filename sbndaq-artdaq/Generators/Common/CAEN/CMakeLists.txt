
link_directories($ENV{CAENCOMM_LIB})
link_directories($ENV{CAENVME_LIB})
link_directories($ENV{CAENDIGITIZER_LIB})

cet_make_exec(CAENStatus
    SOURCE
      CAENStatus.cc
    LIBRARIES
    ${LIBRARY_CORE_LIB_LIST}
    CAENComm
    CAENVME
    CAENDigitizer
)

cet_make_exec(CAENTemperature
    SOURCE
      CAENTemperature.cc
    LIBRARIES
    ${LIBRARY_CORE_LIB_LIST}
    CAENComm
    CAENVME
    CAENDigitizer
)

install_source()
install_headers()
