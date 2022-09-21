add_compile_options(-fpermissive)

add_definitions(-DGIT_VERSION="123")
add_definitions(-DZIO_GIT_VERSION="123")

cet_make_library(
  LIBRARY_NAME
  sbndaq-artdaq_fmctdclib
  SOURCE
    fmctdc-lib.cc
    fmctdc-lib-math.cc
  LIBRARIES
)

install_source()
install_headers()
