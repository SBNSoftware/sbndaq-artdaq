include_directories($ENV{JSONCPP_INC})
link_directories($ENV{JSONCPP_LIB})


cet_make_library(
  LIBRARY_NAME
    sbndaq-artdaq_Generators_SBND_TriggerBoard
  SOURCE
    PTB_Controller.cc
    PTB_Receiver.cc
  LIBRARIES
    ${LIBRARY_CORE_LIB_LIST}
    xml2
    jsoncpp
    pthread
)

install_source()
install_headers()
