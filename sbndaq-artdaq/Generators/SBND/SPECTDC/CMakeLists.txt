add_subdirectory(fmctdc-lib)

SET_SOURCE_FILES_PROPERTIES( SPECCard.cc TDCCard.cc TDCChan.cc SPECTDC.cc SPECTDC_Test.cc  PROPERTIES COMPILE_FLAGS "-Wno-unused-parameter -Wno-unused-but-set-variable")

cet_make_library(
  LIBRARY_NAME
  sbndaq-artdaq_Generators_SBND_SPECTDC
  SOURCE
    SPECCard.cc
    TDCCard.cc
    TDCChan.cc
    TDCChan_configure.cc
    TDCChan_drain.cc
    TDCChan_read.cc
    TDCTimeUtils.cc
    StringUtils.cc
    TAI2UTCAdjustment.cc
  LIBRARIES
    ${LIBRARY_CORE_LIB_LIST}
    sbndaq-artdaq_fmctdclib
    pthread
)

cet_make_exec(NAME SPECTDC_Test
  SOURCE
    SPECTDC_Test.cc
  LIBRARIES
    fhiclcpp
    cetlib
    cetlib_except
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    MF_MessageLogger
    sbndaq-artdaq_fmctdclib
    sbndaq-artdaq_Generators_SBND_SPECTDC
 )


install_source()
install_headers()
