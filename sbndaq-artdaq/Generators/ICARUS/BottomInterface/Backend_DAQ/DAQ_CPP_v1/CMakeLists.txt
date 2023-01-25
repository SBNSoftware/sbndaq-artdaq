list(APPEND LIBRARY_CORE_LIB_LIST sbndaq-artdaq_Common_Utilities )

cet_make_library(
  LIBRARY_NAME
    sbndaq-artdaq_ICARUS_BottomInterface_StartBackend
  SOURCE
    startallboards.cc
    CRT.cc
    usbreadout.cc
    decode.h
    baselines.h
  LIBRARIES
    ${LIBRARY_CORE_LIB_LIST}
)

cet_make_library(
  LIBRARY_NAME
    sbndaq-artdaq_ICARUS_BottomInterface_StopBackend
  SOURCE
    stopallboards.cc
    CRT.cc
    usbreadout.cc
    decode.h
    baselines.h
  LIBRARIES
    ${LIBRARY_CORE_LIB_LIST}
)
