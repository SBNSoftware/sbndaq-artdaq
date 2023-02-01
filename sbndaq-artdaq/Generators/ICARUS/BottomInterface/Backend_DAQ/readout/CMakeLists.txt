list(APPEND LIBRARY_CORE_LIB_LIST sbndaq-artdaq_Common_Utilities )
add_compile_options(-fpermissive)
SET_SOURCE_FILES_PROPERTIES( main.cc  PROPERTIES COMPILE_FLAGS "-Wno-unused-parameter -Wno-unused-label -Wno-unused-but-set-variable -Wno-unused-function -Wno-return-type -Wno-format")

cet_make_library(
  LIBRARY_NAME
    sbndaq-artdaq_ICARUS_BottomInterface_readout
  SOURCE
    main.cc
  LIBRARIES
    libusb-1.0.so.0.1.0
    -lrt libaio.so
    ${LIBRARY_CORE_LIB_LIST}
)
